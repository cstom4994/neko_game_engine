
#ifndef NEKO_HPP
#define NEKO_HPP

// #include <intrin.h>

#include <stdint.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "engine/neko.h"
#include "engine/neko_platform.h"

#if defined(NEKO_PF_WIN)
#include <Windows.h>
#elif defined(NEKO_PF_APPLE) || defined(NEKO_PF_LINUX)
#include <dlfcn.h>
#endif

#define NEKO_VA_COUNT(...) detail::va_count(__VA_ARGS__)

#if defined(__cplusplus)
#include <string>
#if defined(__cpp_char8_t)
template <typename T>
const_str u8Cpp20(T&& t) noexcept {
#pragma warning(disable : 26490)
    return reinterpret_cast<const_str>(t);
#pragma warning(default : 26490)
}
#define NEKO_STR(x) u8Cpp20(u8##x)
#else
#define NEKO_STR(x) u8##x
#endif
#else
#define NEKO_STR(x) x
#endif

#define NEKO_DYNAMIC_CAST(type, input_var, cast_var_name) \
    NEKO_ASSERT(value);                                   \
    type* cast_var_name = dynamic_cast<type*>(input_var); \
    NEKO_ASSERT(cast_var_name)
#define NEKO_STATIC_CAST(type, input_var, cast_var_name) \
    NEKO_ASSERT(value);                                  \
    type* cast_var_name = static_cast<type*>(input_var); \
    NEKO_ASSERT(cast_var_name)

#define NEKO_ENUM_FLAG(T)                                                                                                                                                  \
    inline T operator~(T a) { return static_cast<T>(~static_cast<std::underlying_type<T>::type>(a)); }                                                                     \
    inline T operator|(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) | static_cast<std::underlying_type<T>::type>(b)); }                 \
    inline T operator&(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) & static_cast<std::underlying_type<T>::type>(b)); }                 \
    inline T operator^(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) ^ static_cast<std::underlying_type<T>::type>(b)); }                 \
    inline T& operator|=(T& a, T b) { return reinterpret_cast<T&>(reinterpret_cast<std::underlying_type<T>::type&>(a) |= static_cast<std::underlying_type<T>::type>(b)); } \
    inline T& operator&=(T& a, T b) { return reinterpret_cast<T&>(reinterpret_cast<std::underlying_type<T>::type&>(a) &= static_cast<std::underlying_type<T>::type>(b)); } \
    inline T& operator^=(T& a, T b) { return reinterpret_cast<T&>(reinterpret_cast<std::underlying_type<T>::type&>(a) ^= static_cast<std::underlying_type<T>::type>(b)); }

#define NEKO_MOVEONLY(class_name)                      \
    class_name(const class_name&) = delete;            \
    class_name& operator=(const class_name&) = delete; \
    class_name(class_name&&) = default;                \
    class_name& operator=(class_name&&) = default

namespace detail {
template <typename... Args>
constexpr std::size_t va_count(Args&&...) {
    return sizeof...(Args);
}
}  // namespace detail

#define neko_macro_overload(fun, a, ...)                                               \
    do {                                                                               \
        if (const bool a_ = (a); a_)                                                   \
            [&](auto&&... args) {                                                      \
                const auto t = std::make_tuple(std::forward<decltype(args)>(args)...); \
                constexpr auto N = std::tuple_size<decltype(t)>::value;                \
                                                                                       \
                if constexpr (N == 0) {                                                \
                    fun(a_);                                                           \
                } else {                                                               \
                    fun(a_, __VA_ARGS__);                                              \
                }                                                                      \
            }(__VA_ARGS__);                                                            \
    } while (0)

#define neko_check_is_trivial(type, ...) static_assert(std::is_trivial<type>::value, __VA_ARGS__)

//#define neko_malloc_init(type)                   \
//    (type*)_neko_malloc_init_impl(sizeof(type)); \
//    neko_check_is_trivial(type, "try to init a non-trivial object")

#define neko_malloc_init_ex(name, type)                              \
    neko_check_is_trivial(type, "try to init a non-trivial object"); \
    struct type* name = neko_malloc_init(type)

// 一种向任何指针添加字节偏移量的可移植且安全的方法
// https://stackoverflow.com/questions/15934111/portable-and-safe-way-to-add-byte-offset-to-any-pointer
template <typename T>
NEKO_INLINE void neko_addoffset(std::ptrdiff_t offset, T*& ptr) {
    if (!ptr) return;
    ptr = (T*)((unsigned char*)ptr + offset);
}

template <typename T>
NEKO_INLINE T* neko_addoffset_r(std::ptrdiff_t offset, T* ptr) {
    if (!ptr) return nullptr;
    return (T*)((unsigned char*)ptr + offset);
}

template <typename T, size_t size>
class size_checker {
    static_assert(sizeof(T) == size, "check the size of integral types");
};

template class size_checker<s64, 8>;
template class size_checker<s32, 4>;
template class size_checker<s16, 2>;
template class size_checker<s8, 1>;
template class size_checker<u64, 8>;
template class size_checker<u32, 4>;
template class size_checker<u16, 2>;
template class size_checker<u8, 1>;

struct lua_State;

namespace neko {

template <typename V, typename Alloc = std::allocator<V>>
using vector = std::vector<V, Alloc>;

template <typename T>
struct cpp_remove_reference {
    using type = T;
};

template <typename T>
struct cpp_remove_reference<T&> {
    using type = T;
};

template <typename T>
struct cpp_remove_reference<T&&> {
    using type = T;
};

template <typename T>
constexpr typename cpp_remove_reference<T>::type&& cpp_move(T&& arg) noexcept {
    return (typename cpp_remove_reference<T>::type&&)arg;
}

template <typename T>
using initializer_list = std::initializer_list<T>;

template <typename T>
using neko_function = std::function<T>;

template <typename T>
using scope = std::unique_ptr<T>;
template <typename T, typename... Args>
constexpr scope<T> create_scope(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T>
using ref = std::shared_ptr<T>;
template <typename T, typename... Args>
constexpr ref<T> create_ref(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T>
struct neko_named_func {
    std::string name;
    neko_function<T> func;
};

template <typename F>
struct neko_defer {
    F f;
    neko_defer(F f) : f(f) {}
    ~neko_defer() { f(); }
};

template <typename F>
neko_defer<F> defer_func(F f) {
    return neko_defer<F>(f);
}

#define neko_defer(code) auto NEKO_CONCAT(__defer_, __LINE__) = neko::defer_func([&]() { code; })

template <typename T>
concept concept_is_pair = requires(T t) {
    t.first;
    t.second;
};

template <class T>
struct is_pair : public std::false_type {};

template <class T1, class T2>
struct is_pair<std::pair<T1, T2>> : public std::true_type {};

}  // namespace neko

// 单纯用来测试的 new 和 delete
// 不用于开发目的

#ifndef TEST_NEW
#define TEST_NEW(_name, _class, ...)    \
    (_class*)ME_MALLOC(sizeof(_class)); \
    new ((void*)_name) _class(__VA_ARGS__)
#endif

template <typename T>
struct alloc {
    template <typename... Args>
    static T* safe_malloc(Args&&... args) {
        void* mem = neko_safe_malloc(sizeof(T));
        if (!mem) {
        }
        return new (mem) T(std::forward<Args>(args)...);
    }
};

#ifndef TEST_DELETE
#define TEST_DELETE(_name, _class) \
    {                              \
        _name->~_class();          \
        neko_safe_free(_name);     \
    }
#endif

#if __has_include(<version>)
#include <version>
#endif

#if defined(__cpp_lib_debugging)
#include <debugging>
#else
#if defined(_WIN32)
#include <intrin.h>
#include <windows.h>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#include <unistd.h>
#endif

namespace std {
inline void breakpoint() noexcept { NEKO_DEBUGBREAK(); }
inline bool is_debugger_present() noexcept {
#if defined(_WIN32)
    return IsDebuggerPresent() != 0;
#elif defined(__APPLE__)
    int mib[4];
    struct kinfo_proc info;
    size_t size = sizeof(info);
    info.kp_proc.p_flag = 0;
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();
    if (sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, nullptr, 0) != 0) {
        return false;
    }
    return (info.kp_proc.p_flag & P_TRACED) != 0;
#else
    return false;
#endif
}
inline void breakpoint_if_debugging() noexcept {
    if (is_debugger_present()) {
        breakpoint();
    }
}
}  // namespace std

namespace std {
template <typename E, typename = std::enable_if_t<std::is_enum_v<E>>>
constexpr std::underlying_type_t<E> to_underlying(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}
}  // namespace std

#endif

namespace neko {

#define neko_time_count(x) std::chrono::time_point_cast<std::chrono::microseconds>(x).time_since_epoch().count()

class timer {
public:
    inline void start() noexcept { startPos = std::chrono::high_resolution_clock::now(); }
    inline void stop() noexcept {
        auto endTime = std::chrono::high_resolution_clock::now();
        duration = static_cast<f64>(neko_time_count(endTime) - neko_time_count(startPos)) * 0.001;
    }
    [[nodiscard]] inline f64 get() const noexcept { return duration; }
    ~timer() noexcept { stop(); }

private:
    f64 duration = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> startPos;
};

NEKO_FORCE_INLINE auto time() -> s64 {
    s64 ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return ms;
}

NEKO_FORCE_INLINE f64 time_d() {
    f64 t;
    // #ifdef _WIN32
    //     FILETIME ft;
    //     GetSystemTimeAsFileTime(&ft);
    //     t = (ft.dwHighDateTime * 4294967296.0 / 1e7) + ft.dwLowDateTime / 1e7;
    //     t -= 11644473600.0;
    // #else
    //     struct timeval tv;
    //     gettimeofday(&tv, NULL);
    //     t = tv.tv_sec + tv.tv_usec / 1e6;
    // #endif
    return t;
}

NEKO_STATIC_INLINE time_t time_mkgmtime(struct tm* unixdate) {
    NEKO_ASSERT(unixdate != nullptr);
    time_t fakeUnixtime = mktime(unixdate);
    struct tm* fakeDate = gmtime(&fakeUnixtime);

    s32 nOffSet = fakeDate->tm_hour - unixdate->tm_hour;
    if (nOffSet > 12) {
        nOffSet = 24 - nOffSet;
    }
    return fakeUnixtime - nOffSet * 3600;
}

NEKO_STATIC_INLINE auto time_to_string(std::time_t now = std::time(nullptr)) -> std::string {
    const auto tp = std::localtime(&now);
    char buffer[32];
    return std::strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", tp) ? buffer : "1970-01-01_00:00:00";
}

NEKO_STATIC_INLINE std::string fs_normalize_path(const std::string& path, char delimiter = '/') {
    static constexpr char delims[] = "/\\";

    std::string norm;
    norm.reserve(path.size() / 2);  // random guess, should be benchmarked

    for (auto it = path.begin(); it != path.end(); it++) {
        if (std::any_of(std::begin(delims), std::end(delims), [it](auto c) { return c == *it; })) {
            if (norm.empty() || norm.back() != delimiter) norm.push_back(delimiter);
        } else if (*it == '.') {
            if (++it == path.end()) break;
            if (std::any_of(std::begin(delims), std::end(delims), [it](auto c) { return c == *it; })) {
                continue;
            }
            if (*it != '.') throw std::logic_error("bad path");
            if (norm.empty() || norm.back() != delimiter) throw std::logic_error("bad path");

            norm.pop_back();
            while (!norm.empty()) {
                norm.pop_back();
                if (norm.back() == delimiter) {
                    norm.pop_back();
                    break;
                }
            }
        } else
            norm.push_back(*it);
    }
    if (!norm.empty() && norm.back() != delimiter) norm.push_back(delimiter);
    return norm;
}

NEKO_INLINE bool fs_exists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

}  // namespace neko

#define NEKO_PROP

#if !defined(NEKO_PROP)

#include <string_view>

//
// Type names
//

#ifndef __FUNCSIG__
#define __FUNCSIG__ __PRETTY_FUNCTION__
#endif

template <typename T>
constexpr std::string_view getTypeName() {
    constexpr auto prefixLength = 36, suffixLength = 1;
    const_str data = __FUNCSIG__;
    auto end = data;
    while (*end) {
        ++end;
    }
    return {data + prefixLength, size_t(end - data - prefixLength - suffixLength)};
}

//
// Component types list
//

template <int N>
struct neko_prop_component_type_counter : neko_prop_component_type_counter<N - 1> {
    static constexpr auto num = N;
};
template <>
struct neko_prop_component_type_counter<0> {
    static constexpr auto num = 0;
};
neko_prop_component_type_counter<0> numComponentTypes(neko_prop_component_type_counter<0>);

template <int I>
struct neko_prop_component_typelist;
template <>
struct neko_prop_component_typelist<0> {
    static void each(auto&& f) {}
};

inline constexpr auto maxNumComponentTypes = 32;

template <typename T>
inline constexpr auto isComponentType = false;

#define ComponentTypeListAdd(T)                                                                                                                       \
    template <>                                                                                                                                       \
    inline constexpr auto isComponentType<T> = true;                                                                                                  \
    constexpr auto ComponentTypeList_##T##_Size = decltype(numComponentTypes(neko_prop_component_type_counter<maxNumComponentTypes>()))::num + 1;     \
    static_assert(ComponentTypeList_##T##_Size < maxNumComponentTypes);                                                                               \
    neko_prop_component_type_counter<ComponentTypeList_##T##_Size> numComponentTypes(neko_prop_component_type_counter<ComponentTypeList_##T##_Size>); \
    template <>                                                                                                                                       \
    struct neko_prop_component_typelist<ComponentTypeList_##T##_Size> {                                                                               \
        static void each(auto&& f) {                                                                                                                  \
            neko_prop_component_typelist<ComponentTypeList_##T##_Size - 1>::each(f);                                                                  \
            f.template operator()<T>();                                                                                                               \
        }                                                                                                                                             \
    }

#define Comp(T)              \
    T;                       \
    ComponentTypeListAdd(T); \
    struct T

#define UseComponentTypes()                                                                                                                                           \
    static void forEachComponentType(auto&& f) {                                                                                                                      \
        neko_prop_component_typelist<decltype(numComponentTypes(neko_prop_component_type_counter<maxNumComponentTypes>()))::num>::each(std::forward<decltype(f)>(f)); \
    }

//
// Props
//

constexpr u32 props_hash(std::string_view str) {
    constexpr u32 offset = 2166136261;
    constexpr u32 prime = 16777619;
    auto result = offset;
    for (auto c : str) {
        result = (result ^ c) * prime;
    }
    return result;
}

struct neko_prop_attribs {
    std::string_view name;
    u32 nameHash = props_hash(name);

    bool exampleFlag = false;
};

inline constexpr auto maxNumProps = 24;

template <int N>
struct neko_prop_counter : neko_prop_counter<N - 1> {
    static constexpr auto num = N;
};
template <>
struct neko_prop_counter<0> {
    static constexpr auto num = 0;
};
[[maybe_unused]] static inline neko_prop_counter<0> numProps(neko_prop_counter<0>);

template <int N>
struct neko_prop_index {
    static constexpr auto index = N;
};

template <typename T, int N>
struct neko_prop_tag_wrapper {
    struct tag {
        static inline neko_prop_attribs attribs = T::getPropAttribs(neko_prop_index<N>{});
    };
};
template <typename T, int N>
struct neko_prop_tag_wrapper<const T, N> {
    using tag = typename neko_prop_tag_wrapper<T, N>::tag;
};
template <typename T, int N>
using neko_prop_tag = typename neko_prop_tag_wrapper<T, N>::tag;

#define neko_prop(type, name_, ...) neko_prop_named(#name_, type, name_, __VA_ARGS__)
#define neko_prop_named(nameStr, type, name_, ...)                                                                                                                                             \
    using name_##_Index = neko_prop_index<decltype(numProps(neko_prop_counter<maxNumProps>()))::num>;                                                                                          \
    static inline neko_prop_counter<decltype(numProps(neko_prop_counter<maxNumProps>()))::num + 1> numProps(neko_prop_counter<decltype(numProps(neko_prop_counter<maxNumProps>()))::num + 1>); \
    static std::type_identity<PROP_PARENS_1(PROP_PARENS_3 type)> propType(name_##_Index);                                                                                                      \
    static constexpr neko_prop_attribs getPropAttribs(name_##_Index) { return {.name = #name_, __VA_ARGS__}; };                                                                                \
    std::type_identity_t<PROP_PARENS_1(PROP_PARENS_3 type)> name_

#define PROP_PARENS_1(...) PROP_PARENS_2(__VA_ARGS__)
#define PROP_PARENS_2(...) NO##__VA_ARGS__
#define PROP_PARENS_3(...) PROP_PARENS_3 __VA_ARGS__
#define NOPROP_PARENS_3

template <auto memPtr>
struct neko_prop_containing_type {};
template <typename C, typename R, R C::*memPtr>
struct neko_prop_containing_type<memPtr> {
    using Type = C;
};
#define neko_prop_tag(field) neko_prop_tag<neko_prop_containing_type<&field>::Type, field##_Index::index>

struct __neko_prop_any {
    template <typename T>
    operator T() const;  // NOLINT(google-explicit-constructor)
};

template <typename Aggregate, typename Base = std::index_sequence<>, typename = void>
struct __neko_prop_count_fields : Base {};
template <typename Aggregate, int... Indices>
struct __neko_prop_count_fields<Aggregate, std::index_sequence<Indices...>,
                                std::void_t<decltype(Aggregate{{(static_cast<void>(Indices), std::declval<__neko_prop_any>())}..., {std::declval<__neko_prop_any>()}})>>
    : __neko_prop_count_fields<Aggregate, std::index_sequence<Indices..., sizeof...(Indices)>> {};
template <typename T>
constexpr int countFields() {
    return __neko_prop_count_fields<std::remove_cvref_t<T>>().size();
}

template <typename T>
concept neko_props = std::is_aggregate_v<T>;

template <neko_props T, typename F>
inline void forEachProp(T& val, F&& func) {
    if constexpr (requires { forEachField(const_cast<std::remove_cvref_t<T>&>(val), func); }) {
        forEachField(const_cast<std::remove_cvref_t<T>&>(val), func);
    } else if constexpr (requires { T::propType(neko_prop_index<0>{}); }) {
        constexpr auto n = countFields<T>();
        const auto call = [&]<typename Index>(Index index, auto& val) {
            if constexpr (requires { T::propType(index); }) {
                static_assert(std::is_same_v<typename decltype(T::propType(index))::type, std::remove_cvref_t<decltype(val)>>);
                func(neko_prop_tag<T, Index::index>{}, val);
            }
        };
#define C(i) call(neko_prop_index<i>{}, f##i)
        if constexpr (n == 1) {
            auto& [f0] = val;
            (C(0));
        } else if constexpr (n == 2) {
            auto& [f0, f1] = val;
            (C(0), C(1));
        } else if constexpr (n == 3) {
            auto& [f0, f1, f2] = val;
            (C(0), C(1), C(2));
        } else if constexpr (n == 4) {
            auto& [f0, f1, f2, f3] = val;
            (C(0), C(1), C(2), C(3));
        } else if constexpr (n == 5) {
            auto& [f0, f1, f2, f3, f4] = val;
            (C(0), C(1), C(2), C(3), C(4));
        } else if constexpr (n == 6) {
            auto& [f0, f1, f2, f3, f4, f5] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5));
        } else if constexpr (n == 7) {
            auto& [f0, f1, f2, f3, f4, f5, f6] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6));
        } else if constexpr (n == 8) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7));
        } else if constexpr (n == 9) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8));
        } else if constexpr (n == 10) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9));
        } else if constexpr (n == 11) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10));
        } else if constexpr (n == 12) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11));
        } else if constexpr (n == 13) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12));
        } else if constexpr (n == 14) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13));
        } else if constexpr (n == 15) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14));
        } else if constexpr (n == 16) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15));
        } else if constexpr (n == 17) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16));
        } else if constexpr (n == 18) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16), C(17));
        } else if constexpr (n == 19) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16), C(17), C(18));
        } else if constexpr (n == 20) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16), C(17), C(18), C(19));
        } else if constexpr (n == 21) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16), C(17), C(18), C(19), C(20));
        } else if constexpr (n == 22) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16), C(17), C(18), C(19), C(20), C(21));
        } else if constexpr (n == 23) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16), C(17), C(18), C(19), C(20), C(21), C(22));
        } else if constexpr (n == 24) {
            auto& [f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23] = val;
            (C(0), C(1), C(2), C(3), C(4), C(5), C(6), C(7), C(8), C(9), C(10), C(11), C(12), C(13), C(14), C(15), C(16), C(17), C(18), C(19), C(20), C(21), C(22), C(23));
        }
#undef C
    }
}

#endif

#include <string>

namespace neko::wtf8 {
std::wstring u2w(std::string_view str) noexcept;
std::string w2u(std::wstring_view wstr) noexcept;
}  // namespace neko::wtf8

namespace neko::win {
std::wstring u2w(std::string_view str) noexcept;
std::string w2u(std::wstring_view wstr) noexcept;
std::wstring a2w(std::string_view str) noexcept;
std::string w2a(std::wstring_view wstr) noexcept;
std::string a2u(std::string_view str) noexcept;
std::string u2a(std::string_view str) noexcept;
}  // namespace neko::win

template <typename F>
struct function_traits : public function_traits<decltype(&F::operator())> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType (ClassType::*)(Args...) const> {
    using return_type = ReturnType;
    using pointer = ReturnType (*)(Args...);
    using std_function = std::function<ReturnType(Args...)>;
};

template <typename F>
typename function_traits<F>::std_function to_function(F& lambda) {
    return typename function_traits<F>::std_function(lambda);
}

namespace neko {

template <typename T>
class moveonly {
public:
    static_assert(std::is_default_constructible_v<T>);
    static_assert(std::is_trivially_copy_constructible_v<T>);
    static_assert(std::is_trivially_copy_assignable_v<T>);
    static_assert(std::is_trivially_move_constructible_v<T>);
    static_assert(std::is_trivially_move_assignable_v<T>);
    static_assert(std::is_swappable_v<T>);

    moveonly() = default;

    ~moveonly() = default;

    explicit moveonly(const T& val) noexcept { m_value = val; }

    moveonly(const moveonly&) = delete;

    moveonly& operator=(const moveonly&) = delete;

    moveonly(moveonly&& other) noexcept { m_value = std::exchange(other.m_value, T{}); }

    moveonly& operator=(moveonly&& other) noexcept {
        std::swap(m_value, other.m_value);
        return *this;
    }

    operator const T&() const { return m_value; }

    moveonly& operator=(const T& val) {
        m_value = val;
        return *this;
    }

    // MoveOnly<T> has the same memory layout as T
    // So it's perfectly safe to overload operator&
    T* operator&() { return &m_value; }

    const T* operator&() const { return &m_value; }

    T* operator->() { return &m_value; }

    const T* operator->() const { return &m_value; }

    bool operator==(const T& val) const { return m_value == val; }

    bool operator!=(const T& val) const { return m_value != val; }

private:
    T m_value{};
};

static_assert(sizeof(int) == sizeof(moveonly<int>));

class noncopyable {
public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

struct format_str {
    constexpr format_str(const char* str) noexcept : str(str) {}
    const char* str;
};

template <format_str F>
constexpr auto operator""_f() {
    return [=]<typename... T>(T... args) { return std::format(F.str, args...); };
}

// 成员函数返回值类型确定
// https://stackoverflow.com/questions/26107041/how-can-i-determine-the-return-type-of-a-c11-member-function

template <typename T>
struct return_type;
template <typename R, typename... Args>
struct return_type<R (*)(Args...)> {
    using type = R;
};
template <typename R, typename C, typename... Args>
struct return_type<R (C::*)(Args...)> {
    using type = R;
};
template <typename R, typename C, typename... Args>
struct return_type<R (C::*)(Args...) const> {
    using type = R;
};
template <typename R, typename C, typename... Args>
struct return_type<R (C::*)(Args...) volatile> {
    using type = R;
};
template <typename R, typename C, typename... Args>
struct return_type<R (C::*)(Args...) const volatile> {
    using type = R;
};
template <typename T>
using return_type_t = typename return_type<T>::type;

// std::function 合并方法

template <typename, typename...>
struct lastFnType;

template <typename F0, typename F1, typename... Fn>
struct lastFnType<F0, F1, Fn...> {
    using type = typename lastFnType<F1, Fn...>::type;
};

template <typename T1, typename T2>
struct lastFnType<neko_function<T2(T1)>> {
    using type = T1;
};

template <typename T1, typename T2>
neko_function<T1(T2)> func_combine(neko_function<T1(T2)> conv) {
    return conv;
}

template <typename T1, typename T2, typename T3, typename... Fn>
auto func_combine(neko_function<T1(T2)> conv1, neko_function<T2(T3)> conv2, Fn... fn) -> neko_function<T1(typename lastFnType<neko_function<T2(T3)>, Fn...>::type)> {
    using In = typename lastFnType<neko_function<T2(T3)>, Fn...>::type;

    return [=](In const& in) { return conv1(func_combine(conv2, fn...)(in)); };
}

template <typename T>
struct tuple_size;

template <typename... Args>
struct tuple_size<std::tuple<Args...>> {
    static constexpr std::size_t value = sizeof...(Args);
};

template <typename T>
constexpr std::size_t tuple_size_v = tuple_size<T>::value;

}  // namespace neko

template <typename T>
struct neko_is_vector : std::false_type {};

template <typename T, typename Alloc>
struct neko_is_vector<std::vector<T, Alloc>> : std::true_type {};

namespace detail {
// 某些旧版本的 GCC 需要
template <typename...>
struct voider {
    using type = void;
};

// std::void_t 将成为 C++17 的一部分 但在这里我还是自己实现吧
template <typename... T>
using void_t = typename voider<T...>::type;

template <typename T, typename U = void>
struct is_mappish_impl : std::false_type {};

template <typename T>
struct is_mappish_impl<T, void_t<typename T::key_type, typename T::mapped_type, decltype(std::declval<T&>()[std::declval<const typename T::key_type&>()])>> : std::true_type {};
}  // namespace detail

template <typename T>
struct neko_is_mappish : detail::is_mappish_impl<T>::type {};

template <class... Ts>
struct neko_overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
neko_overloaded(Ts...) -> neko_overloaded<Ts...>;

namespace neko {

// hash 计算相关函数

typedef unsigned long long hash_value;

static_assert(sizeof(hash_value) == 8 && sizeof(hash_value) == sizeof(size_t));

inline uint64_t hash_fnv(const void* data, int size) {
    const_str s = (const_str)data;
    uint64_t h = 14695981039346656037ULL;
    char c = 0;
    while (size--) {
        h = h ^ (uint64_t)(*s++);
        h = h * 1099511628211ULL;
    }
    return h;
}

// http://www.jstatsoft.org/article/view/v008i14/xorshift.pdf page 4
// https://en.wikipedia.org/wiki/Xorshift#xorshift
constexpr hash_value xor64(hash_value h) {
    h ^= 88172645463325252ULL;  // 与常数进行异或 因此种子 0 不会导致无限循环
    h ^= h >> 12;
    h ^= h << 25;
    h ^= h >> 27;
    return h * 0x2545F4914F6CDD1DULL;
}

// https://de.wikipedia.org/wiki/FNV_(Informatik)
constexpr hash_value hash_fnv(const_str string) {
    hash_value MagicPrime = 0x00000100000001b3ULL;
    hash_value Hash = 0xcbf29ce484222325ULL;

    for (; *string; string++) Hash = (Hash ^ *string) * MagicPrime;

    return Hash;
}

constexpr hash_value hash(char const* input) { return hash_fnv(input); }
constexpr hash_value hash(const std::string& input) { return hash_fnv(input.c_str()); }

template <typename ForwardIterator, typename SpaceDetector>
constexpr ForwardIterator find_terminating_word(ForwardIterator begin, ForwardIterator end, SpaceDetector&& is_space_pred) {
    auto rend = std::reverse_iterator(begin);
    auto rbegin = std::reverse_iterator(end);

    int sp_size = 0;
    auto is_space = [&sp_size, &is_space_pred, &end](char c) {
        sp_size = is_space_pred(std::string_view{&c, static_cast<unsigned>(&*std::prev(end) - &c)});
        return sp_size > 0;
    };

    auto search = std::find_if(rbegin, rend, is_space);
    if (search == rend) {
        return begin;
    }
    ForwardIterator it = std::prev(search.base());
    it += sp_size;
    return it;
}

template <typename ForwardIt, typename OutputIt>
constexpr void copy(ForwardIt src_beg, ForwardIt src_end, OutputIt dest_beg, OutputIt dest_end) {
    while (src_beg != src_end && dest_beg != dest_end) {
        *dest_beg++ = *src_beg++;
    }
}

struct string {
    char* data = nullptr;
    u64 len = 0;

    string() = default;
    string(const_str cstr) : data((char*)cstr), len(strlen(cstr)) {}
    string(const_str cstr, u64 n) : data((char*)cstr), len(n) {}

    inline bool is_cstr() { return data[len] == '\0'; }

    inline string substr(u64 i, u64 j) {
        assert(i <= j);
        assert(j <= (s64)len);
        return {&data[i], j - i};
    }

    bool starts_with(string match);

    bool ends_with(string match);

    inline u64 first_of(char c) {
        for (u64 i = 0; i < len; i++) {
            if (data[i] == c) {
                return i;
            }
        }

        return (u64)-1;
    }

    inline u64 last_of(char c) {
        for (u64 i = len; i > 0; i--) {
            if (data[i - 1] == c) {
                return i - 1;
            }
        }

        return (u64)-1;
    }

    char* begin() { return data; }
    char* end() { return &data[len]; }
};

inline string to_cstr(string str) {
    char* buf = (char*)neko_safe_malloc(str.len + 1);
    memcpy(buf, str.data, str.len);
    buf[str.len] = 0;
    return {buf, str.len};
}

constexpr u64 fnv1a(const_str str, u64 len) {
    u64 hash = 14695981039346656037u;
    for (u64 i = 0; i < len; i++) {
        hash ^= (u8)str[i];
        hash *= 1099511628211;
    }
    return hash;
}

inline u64 fnv1a(string str) { return fnv1a(str.data, str.len); }

constexpr u64 operator"" _hash(const_str str, size_t len) { return fnv1a(str, len); }

inline bool operator==(string lhs, string rhs) {
    if (lhs.len != rhs.len) {
        return false;
    }
    return memcmp(lhs.data, rhs.data, lhs.len) == 0;
}

inline bool operator!=(string lhs, string rhs) { return !(lhs == rhs); }

inline bool string::starts_with(string match) {
    if (len < match.len) {
        return false;
    }
    return substr(0, match.len) == match;
}

inline bool string::ends_with(string match) {
    if (len < match.len) {
        return false;
    }
    return substr(len - match.len, len) == match;
}

NEKO_INLINE string str_fmt(const_str fmt, ...) {
    va_list args;
    va_start(args, fmt);
    s32 len = vsnprintf(nullptr, 0, fmt, args);
    va_end(args);

    if (len > 0) {
        char* data = (char*)neko_safe_malloc(len + 1);
        va_start(args, fmt);
        vsnprintf(data, len + 1, fmt, args);
        va_end(args);
        return {data, (u64)len};
    }

    return {};
}

NEKO_INLINE string tmp_fmt(const_str fmt, ...) {
    static char s_buf[1024] = {};

    va_list args;
    va_start(args, fmt);
    s32 len = vsnprintf(s_buf, sizeof(s_buf), fmt, args);
    va_end(args);
    return {s_buf, (u64)len};
}

}  // namespace neko

namespace neko {

// 哈希映射容器

enum hashmap_kind : u8 {
    HashMapKind_None,
    HashMapKind_Some,
    HashMapKind_Tombstone,
};

#define HASH_MAP_LOAD_FACTOR 0.75f

template <typename T>
struct hashmap {
    u64* keys = nullptr;
    T* values = nullptr;
    hashmap_kind* kinds = nullptr;
    u64 load = 0;
    u64 capacity = 0;

    void trash() {
        neko_safe_free(keys);
        neko_safe_free(values);
        neko_safe_free(kinds);
    }

    u64 find_entry(u64 key) const {
        u64 index = key & (capacity - 1);
        u64 tombstone = (u64)-1;
        while (true) {
            hashmap_kind kind = kinds[index];
            if (kind == HashMapKind_None) {
                return tombstone != (u64)-1 ? tombstone : index;
            } else if (kind == HashMapKind_Tombstone) {
                tombstone = index;
            } else if (keys[index] == key) {
                return index;
            }

            index = (index + 1) & (capacity - 1);
        }
    }

    void real_reserve(u64 cap) {
        if (cap <= capacity) {
            return;
        }

        hashmap<T> map = {};
        map.capacity = cap;

        size_t bytes = sizeof(u64) * cap;
        map.keys = (u64*)neko_safe_malloc(bytes);
        memset(map.keys, 0, bytes);

        map.values = (T*)neko_safe_malloc(sizeof(T) * cap);
        memset(map.values, 0, sizeof(T) * cap);

        map.kinds = (hashmap_kind*)neko_safe_malloc(sizeof(hashmap_kind) * cap);
        memset(map.kinds, 0, sizeof(hashmap_kind) * cap);

        for (u64 i = 0; i < capacity; i++) {
            hashmap_kind kind = kinds[i];
            if (kind != HashMapKind_Some) {
                continue;
            }

            u64 index = map.find_entry(keys[i]);
            map.keys[index] = keys[i];
            map.values[index] = values[i];
            map.kinds[index] = HashMapKind_Some;
            map.load++;
        }

        neko_safe_free(keys);
        neko_safe_free(values);
        neko_safe_free(kinds);
        *this = map;
    }

    void reserve(u64 capacity) {
        u64 n = (u64)(capacity / HASH_MAP_LOAD_FACTOR) + 1;

        // next pow of 2
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n |= n >> 32;
        n++;

        real_reserve(n);
    }

    T* get(u64 key) {
        if (load == 0) {
            return nullptr;
        }
        u64 index = find_entry(key);
        return kinds[index] == HashMapKind_Some ? &values[index] : nullptr;
    }

    const T* get(u64 key) const {
        if (load == 0) {
            return nullptr;
        }
        u64 index = find_entry(key);
        return kinds[index] == HashMapKind_Some ? &values[index] : nullptr;
    }

    bool find_or_insert(u64 key, T** value) {
        if (load >= capacity * HASH_MAP_LOAD_FACTOR) {
            real_reserve(capacity > 0 ? capacity * 2 : 16);
        }

        u64 index = find_entry(key);
        bool exists = kinds[index] == HashMapKind_Some;
        if (!exists) {
            values[index] = {};
        }

        if (kinds[index] == HashMapKind_None) {
            load++;
            keys[index] = key;
            kinds[index] = HashMapKind_Some;
        }

        *value = &values[index];
        return exists;
    }

    T& operator[](u64 key) {
        T* value;
        find_or_insert(key, &value);
        return *value;
    }

    void unset(u64 key) {
        if (load == 0) {
            return;
        }

        u64 index = find_entry(key);
        if (kinds[index] != HashMapKind_None) {
            kinds[index] = HashMapKind_Tombstone;
        }
    }

    void clear() {
        memset(keys, 0, sizeof(u64) * capacity);
        memset(values, 0, sizeof(T) * capacity);
        memset(kinds, 0, sizeof(hashmap_kind) * capacity);
        load = 0;
    }
};

template <typename T>
struct hashmap_kv {
    u64 key;
    T* value;
};

template <typename T>
struct hashmap_iter {
    hashmap<T>* map;
    u64 cursor;

    hashmap_kv<T> operator*() const {
        hashmap_kv<T> kv;
        kv.key = map->keys[cursor];
        kv.value = &map->values[cursor];
        return kv;
    }

    hashmap_iter& operator++() {
        cursor++;
        while (cursor != map->capacity) {
            if (map->kinds[cursor] == HashMapKind_Some) {
                return *this;
            }
            cursor++;
        }

        return *this;
    }
};

template <typename T>
bool operator!=(hashmap_iter<T> lhs, hashmap_iter<T> rhs) {
    return lhs.map != rhs.map || lhs.cursor != rhs.cursor;
}

template <typename T>
hashmap_iter<T> begin(hashmap<T>& map) {
    hashmap_iter<T> it = {};
    it.map = &map;
    it.cursor = map.capacity;

    for (u64 i = 0; i < map.capacity; i++) {
        if (map.kinds[i] == HashMapKind_Some) {
            it.cursor = i;
            break;
        }
    }

    return it;
}

template <typename T>
hashmap_iter<T> end(hashmap<T>& map) {
    hashmap_iter<T> it = {};
    it.map = &map;
    it.cursor = map.capacity;
    return it;
}

// 基本泛型容器

template <typename T>
struct array {
    T* data = nullptr;
    u64 len = 0;
    u64 capacity = 0;

    T& operator[](size_t i) {
        NEKO_ASSERT(i >= 0 && i < len);
        return data[i];
    }

    void trash() { neko_safe_free(data); }

    void reserve(u64 cap) {
        if (cap > capacity) {
            T* buf = (T*)neko_safe_malloc(sizeof(T) * cap);
            memcpy(buf, data, sizeof(T) * len);
            neko_safe_free(data);
            data = buf;
            capacity = cap;
        }
    }

    void resize(u64 n) {
        reserve(n);
        len = n;
    }

    void push(T item) {
        if (len == capacity) {
            reserve(len > 0 ? len * 2 : 8);
        }
        data[len] = item;
        len++;
    }

    void pop() {
        NEKO_ASSERT(data->len != 0);
        // 直接标记长度简短 优化方法
        data->len--;
    }

    T* begin() { return data; }
    T* end() { return &data[len]; }
};

struct arena_node;
struct arena {
    arena_node* head;

    void trash();
    void* bump(u64 size);
    void* rebump(void* ptr, u64 old, u64 size);
    string bump_string(string s);
};

template <typename T>
struct slice {
    T* data = nullptr;
    u64 len = 0;

    slice() = default;
    explicit slice(array<T> arr) : data(arr.data), len(arr.len) {}

    T& operator[](size_t i) {
        assert(i >= 0 && i < len);
        return data[i];
    }

    const T& operator[](size_t i) const {
        assert(i >= 0 && i < len);
        return data[i];
    }

    void resize(u64 n) {
        T* buf = (T*)neko_safe_malloc(sizeof(T) * n);
        memcpy(buf, data, sizeof(T) * len);
        neko_safe_free(data);
        data = buf;
        len = n;
    }

    void resize(arena* arena, u64 n) {
        T* buf = (T*)arena->rebump(data, sizeof(T) * len, sizeof(T) * n);
        data = buf;
        len = n;
    }

    T* begin() { return data; }
    T* end() { return &data[len]; }
    const T* begin() const { return data; }
    const T* end() const { return &data[len]; }
};

struct string_builder {
    char* data;
    u64 len;       // 不包括空项
    u64 capacity;  // 包括空项

    string_builder();

    void trash();
    void reserve(u64 capacity);
    void clear();
    void swap_filename(string filepath, string file);
    void concat(string str, s32 times);

    string_builder& operator<<(string str);
    explicit operator string();
};

struct mount_result {
    bool ok;
    bool can_hot_reload;
    bool is_fused;
};

mount_result vfs_mount(const_str fsname, const_str filepath);
void vfs_fini(std::optional<std::string> name);
bool vfs_file_exists(std::string fsname, string filepath);
bool vfs_read_entire_file(std::string fsname, string* out, string filepath);

void* vfs_for_miniaudio();

s64 luax_len(lua_State* L, s32 arg);
string luax_check_string(lua_State* L, s32 arg);

}  // namespace neko

namespace neko {

enum JSONKind : s32 {
    JSONKind_Null,
    JSONKind_Object,
    JSONKind_Array,
    JSONKind_String,
    JSONKind_Number,
    JSONKind_Boolean,
};

struct JSONObject;
struct JSONArray;
struct JSON {
    union {
        JSONObject* object;
        JSONArray* array;
        string str;
        double number;
        bool boolean;
    };
    JSONKind kind;

    JSON lookup(string key, bool* ok);
    JSON index(s32 i, bool* ok);

    JSONObject* as_object(bool* ok);
    JSONArray* as_array(bool* ok);
    string as_string(bool* ok);
    double as_number(bool* ok);

    JSONObject* lookup_object(string key, bool* ok);
    JSONArray* lookup_array(string key, bool* ok);
    string lookup_string(string key, bool* ok);
    double lookup_number(string key, bool* ok);

    double index_number(s32 i, bool* ok);
};

struct JSONObject {
    JSON value;
    string key;
    JSONObject* next;
    u64 hash;
};

struct JSONArray {
    JSON value;
    JSONArray* next;
    u64 index;
};

struct JSONDocument {
    JSON root;
    string error;
    arena arena;

    void parse(string contents);
    void trash();
};

struct StringBuilder;
void json_write_string(StringBuilder* sb, JSON* json);
void json_print(JSON* json);

void json_to_lua(lua_State* L, JSON* json);
string lua_to_json_string(lua_State* L, s32 arg, string* contents, s32 width);

NEKO_INLINE bool str_is_chinese_c(const char str) { return str & 0x80; }

NEKO_INLINE bool str_is_chinese_str(const std::string& str) {
    for (int i = 0; i < str.length(); i++)
        if (str_is_chinese_c(str[i])) return true;
    return false;
}

NEKO_INLINE bool str_equals(const char* a, const char* c) { return strcmp(a, c) == 0; }

NEKO_INLINE bool str_starts_with(std::string_view s, std::string_view prefix) { return prefix.size() <= s.size() && (strncmp(prefix.data(), s.data(), prefix.size()) == 0); }

NEKO_INLINE bool str_starts_with(std::string_view s, char prefix) { return !s.empty() && s[0] == prefix; }

NEKO_INLINE bool str_starts_with(const char* s, const char* prefix) { return strncmp(s, prefix, strlen(prefix)) == 0; }

NEKO_INLINE bool str_ends_with(std::string_view s, std::string_view suffix) { return suffix.size() <= s.size() && strncmp(suffix.data(), s.data() + s.size() - suffix.size(), suffix.size()) == 0; }

NEKO_INLINE bool str_ends_with(std::string_view s, char suffix) { return !s.empty() && s[s.size() - 1] == suffix; }

NEKO_INLINE bool str_ends_with(const char* s, const char* suffix) {
    auto sizeS = strlen(s);
    auto sizeSuf = strlen(suffix);

    return sizeSuf <= sizeS && strncmp(suffix, s + sizeS - sizeSuf, sizeSuf) == 0;
}

NEKO_INLINE void str_to_lower(char* s) {
    int l = strlen(s);
    int ind = 0;
    // spec of "simd"
    for (int i = 0; i < l / 4; i++) {
        s[ind] = std::tolower(s[ind]);
        s[ind + 1] = std::tolower(s[ind + 1]);
        s[ind + 2] = std::tolower(s[ind + 2]);
        s[ind + 3] = std::tolower(s[ind + 3]);
        ind += 4;
    }
    // do the rest linearly
    for (int i = 0; i < (l & 3); ++i) {
        s[ind++] = std::tolower(s[ind]);
    }
}

NEKO_INLINE void str_to_lower(std::string& ss) {
    int l = ss.size();
    auto s = ss.data();
    int ind = 0;
    // spec of "simd"
    for (int i = 0; i < l / 4; i++) {
        s[ind] = std::tolower(s[ind]);
        s[ind + 1] = std::tolower(s[ind + 1]);
        s[ind + 2] = std::tolower(s[ind + 2]);
        s[ind + 3] = std::tolower(s[ind + 3]);
        ind += 4;
    }
    // do the rest linearly
    for (int i = 0; i < (l & 3); ++i) s[ind++] = std::tolower(s[ind]);
}

NEKO_INLINE void str_to_upper(char* s) {
    int l = strlen(s);
    int ind = 0;
    // spec of "simd"
    for (int i = 0; i < l / 4; i++) {
        s[ind] = std::toupper(s[ind]);
        s[ind + 1] = std::toupper(s[ind + 1]);
        s[ind + 2] = std::toupper(s[ind + 2]);
        s[ind + 3] = std::toupper(s[ind + 3]);
        ind += 4;
    }
    // do the rest linearly
    for (int i = 0; i < (l & 3); ++i) {
        s[ind++] = std::toupper(s[ind]);
    }
}

NEKO_INLINE void str_to_upper(std::string& ss) {
    int l = ss.size();
    auto s = ss.data();
    int ind = 0;
    // spec of "simd"
    for (int i = 0; i < l / 4; i++) {
        s[ind] = std::toupper(s[ind]);
        s[ind + 1] = std::toupper(s[ind + 1]);
        s[ind + 2] = std::toupper(s[ind + 2]);
        s[ind + 3] = std::toupper(s[ind + 3]);
        ind += 4;
    }
    // do the rest linearly
    for (int i = 0; i < (l & 3); ++i) s[ind++] = std::toupper(s[ind]);
}

NEKO_INLINE bool str_replace_with(char* src, char what, char with) {
    for (int i = 0; true; ++i) {
        auto& id = src[i];
        if (id == '\0') return true;
        bool isWhat = id == what;
        id = isWhat * with + src[i] * (!isWhat);
    }
}

NEKO_INLINE bool str_replace_with(std::string& src, char what, char with) {
    for (int i = 0; i < src.size(); ++i) {
        auto& id = src.data()[i];
        bool isWhat = id == what;
        id = isWhat * with + src[i] * (!isWhat);
    }
    return true;
}

NEKO_INLINE bool str_replace_with(std::string& src, const char* what, const char* with) {
    std::string out;
    size_t whatlen = strlen(what);
    out.reserve(src.size());
    size_t ind = 0;
    size_t lastInd = 0;
    while (true) {
        ind = src.find(what, ind);
        if (ind == std::string::npos) {
            out += src.substr(lastInd);
            break;
        }
        out += src.substr(lastInd, ind - lastInd) + with;
        ind += whatlen;
        lastInd = ind;
    }
    src = out;
    return true;
}

NEKO_INLINE bool str_replace_with(std::string& src, const char* what, const char* with, int times) {
    for (int i = 0; i < times; ++i) str_replace_with(src, what, with);
    return true;
}

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <semaphore.h>
#endif

struct mutex {
#ifdef _WIN32
    SRWLOCK srwlock;
#else
    pthread_mutex_t pt;
#endif

    void make();
    void trash();
    void lock();
    void unlock();
    bool try_lock();
};

struct cond {
#ifdef _WIN32
    CONDITION_VARIABLE cv;
#else
    pthread_cond_t pt;
#endif

    void make();
    void trash();
    void signal();
    void broadcast();
    void wait(mutex* mtx);
    bool timed_wait(mutex* mtx, uint32_t ms);
};

struct rwlock {
#if _WIN32
    SRWLOCK srwlock;
#else
    pthread_rwlock_t pt;
#endif

    void make();
    void trash();
    void shared_lock();
    void shared_unlock();
    void unique_lock();
    void unique_unlock();
};

struct sema {
#ifdef _WIN32
    HANDLE handle;
#else
    sem_t* sem;
#endif

    void make(int n = 0);
    void trash();
    void post(int n = 1);
    void wait();
};

typedef void (*thread_proc)(void*);

struct thread {
    void* ptr = nullptr;

    void make(thread_proc fn, void* udata);
    void join();
};

struct lock_guard {
    mutex* mtx;

    lock_guard(mutex* mtx) : mtx(mtx) { mtx->lock(); };
    ~lock_guard() { mtx->unlock(); };
    lock_guard(lock_guard&&) = delete;
    lock_guard& operator=(lock_guard&&) = delete;

    operator bool() { return true; }
};

uint64_t this_thread_id();

struct LuaThread {
    mutex mtx;
    string contents;
    string name;
    thread thread;

    void make(string code, string thread_name);
    void join();
};

struct lua_table_entry;
struct lua_variant {
    s32 type;
    union {
        bool boolean;
        double number;
        string string;
        slice<lua_table_entry> table;
        struct {
            void* ptr;
            neko::string tname;
        } udata;
    };

    void make(lua_State* L, s32 arg);
    void trash();
    void push(lua_State* L);
};

struct lua_table_entry {
    lua_variant key;
    lua_variant value;
};

struct lua_channel {
    std::atomic<char*> name;

    mutex mtx;
    cond received;
    cond sent;

    u64 received_total;
    u64 sent_total;

    slice<lua_variant> items;
    u64 front;
    u64 back;
    u64 len;

    void make(string n, u64 buf);
    void trash();
    void send(lua_variant item);
    lua_variant recv();
    bool try_recv(lua_variant* v);
};

lua_channel* lua_channel_make(string name, u64 buf);
lua_channel* lua_channel_get(string name);
lua_channel* lua_channels_select(lua_State* L, lua_variant* v);
void lua_channels_setup();
void lua_channels_shutdown();

template <typename T>
struct queue {
    mutex mtx = {};
    cond cv = {};

    T* data = nullptr;
    u64 front = 0;
    u64 back = 0;
    u64 len = 0;
    u64 capacity = 0;

    void make() {
        mtx.make();
        cv.make();
    }

    void trash() {
        mtx.trash();
        cv.trash();
        neko_safe_free(data);
    }

    void reserve(u64 cap) {
        if (cap <= capacity) {
            return;
        }

        T* buf = (T*)neko_safe_malloc(sizeof(T) * cap);

        if (front < back) {
            memcpy(buf, &data[front], sizeof(T) * len);
        } else {
            u64 lhs = back;
            u64 rhs = (capacity - front);

            memcpy(buf, &data[front], sizeof(T) * rhs);
            memcpy(&buf[rhs], &data[0], sizeof(T) * lhs);
        }

        neko_safe_free(data);

        data = buf;
        front = 0;
        back = len;
        capacity = cap;
    }

    void enqueue(T item) {
        lock_guard lock{&mtx};

        if (len == capacity) {
            reserve(len > 0 ? len * 2 : 8);
        }

        data[back] = item;
        back = (back + 1) % capacity;
        len++;

        cv.signal();
    }

    T demand() {
        lock_guard lock{&mtx};

        while (len == 0) {
            cv.wait(&mtx);
        }

        T item = data[front];
        front = (front + 1) % capacity;
        len--;

        return item;
    }
};

s32 os_change_dir(const char* path);
string os_program_dir();
string os_program_path();
u64 os_file_modtime(const char* filename);
void os_high_timer_resolution();
void os_sleep(u32 ms);
void os_yield();

NEKO_API_DECL void neko_tm_init(void);

void profile_setup();
void profile_shutdown();

struct TraceEvent {
    const char* cat;
    const char* name;
    u64 ts;
    u16 tid;
    char ph;
};

struct Instrument {
    const char* cat;
    const char* name;
    s32 tid;

    Instrument(const char* cat, const char* name);
    ~Instrument();
};

#if NEKO_PROFILE_ENABLE
#define PROFILE_FUNC() auto NEKO_CONCAT(__gen_profile_, __COUNTER__) = ::neko::Instrument("function", __func__);
#define PROFILE_BLOCK(name) auto NEKO_CONCAT(__gen_profile_, __COUNTER__) = ::neko::Instrument("block", name);
#else
#define PROFILE_FUNC()
#define PROFILE_BLOCK(name)
#endif

}  // namespace neko

#define CVAR_TYPES() bool, s32, f32, f32*

typedef struct engine_cvar_t {
    bool show_editor;
    bool show_demo_window;
    bool show_pack_editor;
    bool show_profiler_window;
    bool show_test_window;
    bool show_gui;
    bool shader_inspect;
    bool hello_ai_shit;
    bool vsync;
    bool is_hotfix;

    // 实验性功能开关
    bool enable_nekolua;

    f32 bg[3];  //
} neko_client_cvar_t;

void neko_cvar_gui(neko_client_cvar_t& cvar);

extern neko_console_command_t commands[];
extern neko_console_t g_console;

namespace neko {}  // namespace neko

#endif