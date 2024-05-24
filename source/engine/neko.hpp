
#ifndef NEKO_HPP
#define NEKO_HPP

// #include <intrin.h>

#include <stdint.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <string_view>
#include <vector>

#include "engine/neko.h"

#define NEKO_VA_COUNT(...) detail::va_count(__VA_ARGS__)

#if defined(__cplusplus)
#include <string>
#if defined(__cpp_char8_t)
template <typename T>
const char* u8Cpp20(T&& t) noexcept {
#pragma warning(disable : 26490)
    return reinterpret_cast<const char*>(t);
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
    neko_assert(value);                                   \
    type* cast_var_name = dynamic_cast<type*>(input_var); \
    neko_assert(cast_var_name)
#define NEKO_STATIC_CAST(type, input_var, cast_var_name) \
    neko_assert(value);                                  \
    type* cast_var_name = static_cast<type*>(input_var); \
    neko_assert(cast_var_name)

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

#define neko_c_ref(T, ...)                                                                                       \
    [&]() -> T* {                                                                                                \
        static T neko_macro_cat(__neko_gen_, neko_macro_cat(T, neko_macro_cat(_cref_, __LINE__))) = __VA_ARGS__; \
        return &neko_macro_cat(__neko_gen_, neko_macro_cat(T, neko_macro_cat(_cref_, __LINE__)));                \
    }()

#define neko_arr_ref(T, ...)                                                                                                            \
    [&]() -> T* {                                                                                                                       \
        static T neko_macro_cat(neko_macro_cat(__neko_gen_, neko_macro_cat(T, neko_macro_cat(_arr_ref_, __LINE__))), []) = __VA_ARGS__; \
        return neko_macro_cat(__neko_gen_, neko_macro_cat(T, neko_macro_cat(_arr_ref_, __LINE__)));                                     \
    }()

#ifndef neko_check_is_trivial
#define neko_check_is_trivial(type, err) static_assert(std::is_trivial<type>::value, err)
#endif

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
template class size_checker<b32, 4>;

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

#define neko_defer(code) auto neko_concat(_defer_, __COUNTER__) = neko::defer_func([&]() { code; })

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
    neko_assert(unixdate != nullptr);
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

NEKO_INLINE void neko_utils_write_ppm(const int width, const int height, unsigned char* buffer, const char* filename) {

    FILE* fp = fopen(filename, "wb");

    (void)fprintf(fp, "P6\n%d %d\n255\n", width, height);

    for (int i = 0; i < width * height; ++i) {
        static unsigned char color[3];
        color[0] = buffer[i];
        color[1] = buffer[i];
        color[2] = buffer[i];
        (void)fwrite(color, 1, 3, fp);
    }

    (void)fclose(fp);
};

NEKO_INLINE void neko_tex_flip_vertically(int width, int height, u8* data) {
    u8 rgb[4];
    for (int y = 0; y < height / 2; y++) {
        for (int x = 0; x < width; x++) {
            int top = 4 * (x + y * width);
            int bottom = 4 * (x + (height - y - 1) * width);
            memcpy(rgb, data + top, sizeof(rgb));
            memcpy(data + top, data + bottom, sizeof(rgb));
            memcpy(data + bottom, rgb, sizeof(rgb));
        }
    }
}

#if !defined(NEKO_PROP)
#define NEKO_PROP

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
    const char* data = __FUNCSIG__;
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
[[maybe_unused]] inline static neko_prop_counter<0> numProps(neko_prop_counter<0>);

template <int N>
struct neko_prop_index {
    static constexpr auto index = N;
};

template <typename T, int N>
struct neko_prop_tag_wrapper {
    struct tag {
        inline static neko_prop_attribs attribs = T::getPropAttribs(neko_prop_index<N>{});
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
    inline static neko_prop_counter<decltype(numProps(neko_prop_counter<maxNumProps>()))::num + 1> numProps(neko_prop_counter<decltype(numProps(neko_prop_counter<maxNumProps>()))::num + 1>); \
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

#pragma region neko_enum_name

namespace neko {

template <auto value>
constexpr auto enum_name() {
    std::string_view name;
#ifdef __clang__
    name = __PRETTY_FUNCTION__;
    auto start = name.find("value = ") + 8;  // 8 is length of "value = "
    auto end = name.find_last_of(']');
    return std::string_view{name.data() + start, end - start};

#elif defined(__GNUC__)
    name = __PRETTY_FUNCTION__;
    auto start = name.find("value = ") + 8;  // 8 is length of "value = "
    auto end = name.find_last_of(']');
    return std::string_view{name.data() + start, end - start};

#elif defined(_MSC_VER)
    name = __FUNCSIG__;
    auto start = name.find("neko::enum_name<") + 16;  // 16 is length of "neko::enum_name<"
    auto end = name.find_last_of('>');
    return std::string_view{name.data() + start, end - start};
#endif
}

template <typename T>
concept enum_check = std::is_enum_v<T>;

// 获取枚举变量数量
template <enum_check T, std::size_t N = 0>
constexpr auto enum_max() {
    constexpr auto value = static_cast<T>(N);
    if constexpr (neko::enum_name<value>().find("(") == std::string_view::npos)  // 如果超出了连续有名枚举 将会是"(enum the_name)0xN"
        return neko::enum_max<T, N + 1>();
    else
        return N;
}

// 打表
template <typename T>
    requires std::is_enum_v<T>
constexpr auto enum_name(T value) {
    constexpr auto num = neko::enum_max<T>();
    constexpr std::array<std::string_view, num> names{[]<std::size_t... Is>(std::index_sequence<Is...>) {
        return std::array<std::string_view, num>{neko::enum_name<static_cast<T>(Is)>()...};
    }(std::make_index_sequence<num>{})};  // 打表获得枚举名称
    return names[static_cast<std::size_t>(value)];
}

}  // namespace neko

#pragma endregion

template <typename T>
NEKO_INLINE void neko_swap(T& a, T& b) {
    T tmp = a;
    a = b;
    b = tmp;
}

template <typename T>
NEKO_STATIC_INLINE void write_var(u8*& _buffer, T _var) {
    memcpy(_buffer, &_var, sizeof(T));
    _buffer += sizeof(T);
}

NEKO_STATIC_INLINE void write_str(u8*& _buffer, const char* _str) {
    u32 len = (u32)strlen(_str);
    write_var(_buffer, len);
    memcpy(_buffer, _str, len);
    _buffer += len;
}

template <typename T>
NEKO_STATIC_INLINE void read_var(u8*& _buffer, T& _var) {
    memcpy(&_var, _buffer, sizeof(T));
    _buffer += sizeof(T);
}

NEKO_STATIC_INLINE char* read_string(u8*& _buffer) {
    u32 len;
    read_var(_buffer, len);
    char* str = new char[len + 1];
    memcpy(str, _buffer, len);
    str[len] = 0;
    _buffer += len;
    return str;
}

NEKO_STATIC_INLINE const char* duplicate_string(const char* _str) {
    char* str = new char[strlen(_str) + 1];
    strcpy(str, _str);
    return str;
}

struct string_store {
    typedef std::unordered_map<std::string, u32> string_to_index_type;
    typedef std::unordered_map<u32, std::string> index_to_string_type;

    u32 total_size;
    string_to_index_type str_index_map;
    index_to_string_type strings;

    string_store() : total_size(0) {}

    void add_string(const char* _str) {
        string_to_index_type::iterator it = str_index_map.find(_str);
        if (it == str_index_map.end()) {
            u32 index = (u32)str_index_map.size();
            total_size += 4 + (u32)strlen(_str);
            str_index_map[_str] = index;
            strings[index] = _str;
        }
    }

    u32 get_string(const char* _str) { return str_index_map[_str]; }
};

namespace neko {

template <typename T>
struct span {
    span() : __begin(nullptr), __end(nullptr) {}
    span(T* begin, u32 len) : __begin(begin), __end(begin + len) {}
    span(T* begin, T* end) : __begin(begin), __end(end) {}
    template <int N>
    span(T (&value)[N]) : __begin(value), __end(value + N) {}
    T& operator[](u32 idx) const {
        neko_assert(__begin + idx < __end);
        return __begin[idx];
    }
    operator span<const T>() const { return span<const T>(__begin, __end); }
    void remove_prefix(u32 count) {
        neko_assert(count <= length());
        __begin += count;
    }
    void remove_suffix(u32 count) {
        neko_assert(count <= length());
        __end -= count;
    }
    [[nodiscard]] span from_left(u32 count) const {
        neko_assert(count <= length());
        return span(__begin + count, __end);
    }
    [[nodiscard]] span from_right(u32 count) const {
        neko_assert(count <= length());
        return span(__begin, __end - count);
    }
    T& back() {
        neko_assert(length() > 0);
        return *(__end - 1);
    }
    const T& back() const {
        neko_assert(length() > 0);
        return *(__end - 1);
    }
    bool equals(const span<T>& rhs) {
        bool res = true;
        if (length() != rhs.length()) return false;
        for (const T& v : *this) {
            u32 i = u32(&v - __begin);
            if (v != rhs.__begin[i]) return false;
        }
        return true;
    }

    template <typename F>
    s32 find(const F& f) const {
        for (u32 i = 0, c = length(); i < c; ++i) {
            if (f(__begin[i])) return i;
        }
        return -1;
    }

    u32 length() const { return (u32)(__end - __begin); }

    T* begin() const { return __begin; }
    T* end() const { return __end; }

    T* __begin;
    T* __end;
};

// hash 计算相关函数

typedef unsigned long long hash_value;

static_assert(sizeof(hash_value) == 8 && sizeof(hash_value) == sizeof(size_t));

inline uint64_t hash_fnv(const void* data, int size) {
    const char* s = (const char*)data;
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
constexpr hash_value hash_fnv(const char* string) {
    hash_value MagicPrime = 0x00000100000001b3ULL;
    hash_value Hash = 0xcbf29ce484222325ULL;

    for (; *string; string++) Hash = (Hash ^ *string) * MagicPrime;

    return Hash;
}

constexpr hash_value hash_fnv(const char* string, const char* end) {
    hash_value MagicPrime = 0x00000100000001b3ULL;
    hash_value Hash = 0xcbf29ce484222325ULL;

    for (; string != end; string++) Hash = (Hash ^ *string) * MagicPrime;

    return Hash;
}

constexpr void next(hash_value& h) { h = xor64(h); }

inline hash_value hash_from_clock() {
    hash_value h = __rdtsc();
    next(h);
    return h;
}

template <typename T>
T next_range(hash_value& h, T min, T max) {
    if (max < min) return next_range(h, max, min);
    next(h);
    min += h % (max - min);
    return min;
}

template <typename T1, typename T2>
T1 next_range(hash_value& h, T1 min, T2 max) {
    return next_range<T1>(h, min, (T1)max);
}

inline float nextf(hash_value& h) {
    next(h);

    union {
        hash_value u_x;
        float u_f;
    };

    // 以某种方式操作浮点数的指数和分数 使得从 1（包括）和 2（不包括）中得到一个数字
    u_x = h;
    u_x &= 0x007FFFFF007FFFFF;
    u_x |= 0x3F8000003F800000;

    return u_f - 1.0f;
}

inline float nextf(hash_value& h, float min, float max) {
    if (max < min) return nextf(h, max, min);
    return min + nextf(h) * (max - min);
}

inline int nexti(hash_value& h) {
    next(h);

    union {
        hash_value u_x;
        int u_i;
    };

    u_x = h;

    return u_i;
}

template <class T>
inline hash_value hash_simple(T value) {
    static_assert(sizeof(T) <= sizeof(hash_value), "sizeof(T) can't be bigger than sizeof(hash_value)");
    union {
        hash_value u_h;
        T u_f;
    };
    u_h = 0;
    u_f = value;
    return u_h;
}

constexpr hash_value hash(unsigned char v) { return v; }
constexpr hash_value hash(unsigned int v) { return v; }
constexpr hash_value hash(unsigned long int v) { return v; }
constexpr hash_value hash(unsigned long long int v) { return v; }
constexpr hash_value hash(unsigned short int v) { return v; }
constexpr hash_value hash(bool v) { return v ? 1 : 0; }
inline hash_value hash(signed char v) { return hash_simple(v); }
inline hash_value hash(int v) { return hash_simple(v); }
inline hash_value hash(long int v) { return hash_simple(v); }
inline hash_value hash(long long int v) { return hash_simple(v); }
inline hash_value hash(short int v) { return hash_simple(v); }
inline hash_value hash(double v) { return hash_simple(v); }
inline hash_value hash(float v) { return hash_simple(v); }
// inline hash_value hash(long double v) { return hash_simple(v); }
inline hash_value hash(wchar_t v) { return hash_simple(v); }
constexpr hash_value hash(char const* input) { return hash_fnv(input); }
constexpr hash_value hash(const std::string& input) { return hash_fnv(input.c_str()); }

template <class T>
constexpr hash_value hash(const std::vector<T>& v) {
    hash_value h = 0;
    for (auto& o : v) {
        h ^= hash(o);
        h = xor64(h);
    }
    return h;
}

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
    string(const char* cstr) : data((char*)cstr), len(strlen(cstr)) {}
    string(const char* cstr, u64 n) : data((char*)cstr), len(n) {}

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

constexpr u64 fnv1a(const char* str, u64 len) {
    u64 hash = 14695981039346656037u;
    for (u64 i = 0; i < len; i++) {
        hash ^= (u8)str[i];
        hash *= 1099511628211;
    }
    return hash;
}

inline u64 fnv1a(string str) { return fnv1a(str.data, str.len); }

constexpr u64 operator"" _hash(const char* str, size_t len) { return fnv1a(str, len); }

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

NEKO_INLINE string str_fmt(const char* fmt, ...) {
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

NEKO_INLINE string tmp_fmt(const char* fmt, ...) {
    static char s_buf[1024] = {};

    va_list args;
    va_start(args, fmt);
    s32 len = vsnprintf(s_buf, sizeof(s_buf), fmt, args);
    va_end(args);
    return {s_buf, (u64)len};
}

}  // namespace neko

// 哈希映射容器

enum neko_hashmap_kind : u8 {
    neko_hashmap_kind_none,
    neko_hashmap_kind_some,
    neko_hashmap_kind_tombstone,
};

#define HASH_MAP_LOAD_FACTOR 0.75f

NEKO_INLINE u64 neko_hashmap_reserve_size(u64 size) {
    u64 n = (u64)(size / HASH_MAP_LOAD_FACTOR) + 1;

    // next pow of 2
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    n++;

    return n;
}

template <typename T>
struct neko_hashmap {
    u64* keys = nullptr;
    T* values = nullptr;
    neko_hashmap_kind* kinds = nullptr;
    u64 load = 0;
    u64 capacity = 0;
    T& operator[](u64 key);
    T& operator[](const std::string& key);
};

template <typename T>
void neko_hashmap_dctor(neko_hashmap<T>* map) {
    neko_safe_free(map->keys);
    neko_safe_free(map->values);
    neko_safe_free(map->kinds);
}

template <typename T>
u64 neko_hashmap_find_entry(neko_hashmap<T>* map, u64 key) {
    u64 index = key & (map->capacity - 1);
    u64 tombstone = (u64)-1;
    while (true) {
        neko_hashmap_kind kind = map->kinds[index];
        if (kind == neko_hashmap_kind_none) {
            return tombstone != (u64)-1 ? tombstone : index;
        } else if (kind == neko_hashmap_kind_tombstone) {
            tombstone = index;
        } else if (map->keys[index] == key) {
            return index;
        }

        index = (index + 1) & (map->capacity - 1);
    }
}

// capacity must be a power of 2
template <typename T>
void neko_hashmap_reserve(neko_hashmap<T>* old, u64 capacity) {
    if (capacity <= old->capacity) {
        return;
    }

    neko_hashmap<T> map = {};
    map.capacity = capacity;

    size_t bytes = sizeof(u64) * capacity;
    map.keys = (u64*)neko_safe_malloc(bytes);
    memset(map.keys, 0, bytes);

    map.values = (T*)neko_safe_malloc(sizeof(T) * capacity);
    memset(map.values, 0, sizeof(T) * capacity);

    map.kinds = (neko_hashmap_kind*)neko_safe_malloc(sizeof(neko_hashmap_kind) * capacity);
    memset(map.kinds, 0, sizeof(neko_hashmap_kind) * capacity);

    for (u64 i = 0; i < old->capacity; i++) {
        neko_hashmap_kind kind = old->kinds[i];
        if (kind != neko_hashmap_kind_some) {
            continue;
        }

        u64 index = neko_hashmap_find_entry(&map, old->keys[i]);
        map.keys[index] = old->keys[i];
        map.values[index] = old->values[i];
        map.kinds[index] = neko_hashmap_kind_some;
        map.load++;
    }

    neko_safe_free(old->keys);
    neko_safe_free(old->values);
    neko_safe_free(old->kinds);
    *old = map;
}

template <typename T>
T* neko_hashmap_get(neko_hashmap<T>* map, u64 key) {
    if (map->load == 0) {
        return nullptr;
    }

    u64 index = neko_hashmap_find_entry(map, key);
    return map->kinds[index] == neko_hashmap_kind_some ? &map->values[index] : nullptr;
}

template <typename T>
bool neko_hashmap_get(neko_hashmap<T>* map, u64 key, T** value) {
    if (map->load >= map->capacity * HASH_MAP_LOAD_FACTOR) {
        neko_hashmap_reserve(map, map->capacity > 0 ? map->capacity * 2 : 16);
    }

    u64 index = neko_hashmap_find_entry(map, key);
    bool exists = map->kinds[index] == neko_hashmap_kind_some;
    if (map->kinds[index] == neko_hashmap_kind_none) {
        map->load++;
        map->keys[index] = key;
        map->values[index] = {};
        map->kinds[index] = neko_hashmap_kind_some;
    }

    *value = &map->values[index];
    return exists;
}

template <typename T>
T& neko_hashmap<T>::operator[](u64 key) {
    T* value;
    neko_hashmap_get(this, key, &value);
    return *value;
}

template <typename T>
T& neko_hashmap<T>::operator[](const std::string& key) {
    T* value;
    neko_hashmap_get(this, neko::hash(key.c_str()), &value);
    return *value;
}

template <typename T>
void neko_hashmap_unset(neko_hashmap<T>* map, u64 key) {
    if (map->load == 0) {
        return;
    }

    u64 index = neko_hashmap_find_entry(map, key);
    if (map->kinds[index] != neko_hashmap_kind_none) {
        map->kinds[index] = neko_hashmap_kind_tombstone;
    }
}

template <typename T>
void clear(neko_hashmap<T>* map) {
    memset(map->keys, 0, sizeof(u64) * map->capacity);
    memset(map->values, 0, sizeof(T) * map->capacity);
    memset(map->kinds, 0, sizeof(neko_hashmap_kind) * map->capacity);
    map->load = 0;
}

template <typename T>
struct neko_hashmap_kv {
    u64 key;
    T* value;
};

template <typename T>
struct neko_hashmap_iterator {
    neko_hashmap<T>* map;
    u64 cursor;

    neko_hashmap_kv<T> operator*() const {
        neko_hashmap_kv<T> kv;
        kv.key = map->keys[cursor];
        kv.value = &map->values[cursor];
        return kv;
    }

    neko_hashmap_iterator& operator++() {
        cursor++;
        while (cursor != map->capacity) {
            if (map->kinds[cursor] == neko_hashmap_kind_some) {
                return *this;
            }
            cursor++;
        }

        return *this;
    }
};

template <typename T>
bool operator!=(neko_hashmap_iterator<T> lhs, neko_hashmap_iterator<T> rhs) {
    return lhs.map != rhs.map || lhs.cursor != rhs.cursor;
}

template <typename T>
neko_hashmap_iterator<T> begin(neko_hashmap<T>& map) {
    neko_hashmap_iterator<T> it = {};
    it.map = &map;
    it.cursor = map.capacity;

    for (u64 i = 0; i < map.capacity; i++) {
        if (map.kinds[i] == neko_hashmap_kind_some) {
            it.cursor = i;
            break;
        }
    }

    return it;
}

template <typename T>
neko_hashmap_iterator<T> end(neko_hashmap<T>& map) {
    neko_hashmap_iterator<T> it = {};
    it.map = &map;
    it.cursor = map.capacity;
    return it;
}

// template <typename T>
// neko_hashmap_iterator<T> begin(const neko_hashmap<T>& map) {
//     neko_hashmap_iterator<T> it = {};
//     it.map = &map;
//     it.cursor = map.capacity;
//
//     for (u64 i = 0; i < map.capacity; i++) {
//         if (map.kinds[i] == neko_hashmap_kind_some) {
//             it.cursor = i;
//             break;
//         }
//     }
//
//     return it;
// }
//
// template <typename T>
// neko_hashmap_iterator<T> end(const neko_hashmap<T>& map) {
//     neko_hashmap_iterator<T> it = {};
//     it.map = &map;
//     it.cursor = map.capacity;
//     return it;
// }

namespace neko {

// 基本泛型容器

template <typename T>
struct array {
    T* data = nullptr;
    u64 len = 0;
    u64 capacity = 0;

    T& operator[](size_t i) {
        neko_assert(i >= 0 && i < len);
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
        neko_assert(data->len != 0);
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

mount_result vfs_mount(const char* filepath);
void vfs_trash();

bool vfs_file_exists(string filepath);
bool vfs_read_entire_file(string* out, string filepath);
bool vfs_write_entire_file(string filepath, string contents);
bool vfs_list_all_files(array<string>* files);

void* vfs_for_miniaudio();

template <typename T>
class safe_queue {
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;

public:
    safe_queue() {}

    //    safe_queue(safe_queue& other) {}

    ~safe_queue() {}

    bool empty() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    int size() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    void enqueue(T& t) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(t);
    }

    bool dequeue(T& t) {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (m_queue.empty()) {
            return false;
        }
        t = std::move(m_queue.front());

        m_queue.pop();
        return true;
    }
};

class thread_pool {
private:
    class thread_worker {
    private:
        int m_id;
        thread_pool* m_pool;

    public:
        thread_worker(thread_pool* pool, const int id) : m_pool(pool), m_id(id) {}

        void operator()() {
            std::function<void()> func;
            bool dequeued;
            while (!m_pool->m_shutdown) {
                {
                    std::unique_lock<std::mutex> lock(m_pool->m_conditional_mutex);
                    if (m_pool->m_queue.empty()) {
                        m_pool->m_conditional_lock.wait(lock);
                    }
                    dequeued = m_pool->m_queue.dequeue(func);
                }
                if (dequeued) {
                    func();
                }
            }
        }
    };

    bool m_shutdown;
    safe_queue<std::function<void()>> m_queue;
    std::vector<std::thread> m_threads;
    std::mutex m_conditional_mutex;
    std::condition_variable m_conditional_lock;

public:
    thread_pool(const int n_threads) : m_threads(std::vector<std::thread>(n_threads)), m_shutdown(false) {}

    thread_pool(const thread_pool&) = delete;
    thread_pool(thread_pool&&) = delete;

    thread_pool& operator=(const thread_pool&) = delete;
    thread_pool& operator=(thread_pool&&) = delete;

    void init() {
        for (int i = 0; i < m_threads.size(); ++i) {
            m_threads[i] = std::thread(thread_worker(this, i));
        }
    }

    void shutdown() {
        m_shutdown = true;
        m_conditional_lock.notify_all();

        for (int i = 0; i < m_threads.size(); ++i) {
            if (m_threads[i].joinable()) {
                m_threads[i].join();
            }
        }
    }

    // 提交要由池异步执行的函数
    template <typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        // 创建一个具有边界参数的函数 准备执行
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        // 将其封装到共享的ptr中 以便能够复制构造
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        std::function<void()> wrapper_func = [task_ptr]() { (*task_ptr)(); };
        m_queue.enqueue(wrapper_func);

        // 如果一个线程正在等待 则唤醒它
        m_conditional_lock.notify_one();
        return task_ptr->get_future();
    }
};

}  // namespace neko

#endif