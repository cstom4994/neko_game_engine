
#ifndef NEKO_HPP
#define NEKO_HPP

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "engine/neko.h"

#define neko_va_count(...) detail::va_count(__VA_ARGS__)

#if defined(__cplusplus)
#include <string>
#if defined(__cpp_char8_t)
template <typename T>
const char* u8Cpp20(T&& t) noexcept {
#pragma warning(disable : 26490)
    return reinterpret_cast<const char*>(t);
#pragma warning(default : 26490)
}
#define neko_str(x) u8Cpp20(u8##x)
#else
#define neko_str(x) u8##x
#endif
#else
#define neko_str(x) x
#endif

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

typedef std::string neko_string;
typedef std::string_view neko_string_view;

// 一种向任何指针添加字节偏移量的可移植且安全的方法
// https://stackoverflow.com/questions/15934111/portable-and-safe-way-to-add-byte-offset-to-any-pointer
template <typename T>
neko_inline void neko_addoffset(std::ptrdiff_t offset, T*& ptr) {
    if (!ptr) return;
    ptr = (T*)((unsigned char*)ptr + offset);
}

template <typename T>
neko_inline T* neko_addoffset_r(std::ptrdiff_t offset, T* ptr) {
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

#include <algorithm>
#include <chrono>
#include <fstream>

neko_force_inline auto neko_get_time() -> s64 {
    s64 ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return ms;
}

neko_force_inline f64 neko_get_time_d() {
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

neko_static_inline time_t neko_get_time_mkgmtime(struct tm* unixdate) {
    neko_assert(unixdate != nullptr);
    time_t fakeUnixtime = mktime(unixdate);
    struct tm* fakeDate = gmtime(&fakeUnixtime);

    s32 nOffSet = fakeDate->tm_hour - unixdate->tm_hour;
    if (nOffSet > 12) {
        nOffSet = 24 - nOffSet;
    }
    return fakeUnixtime - nOffSet * 3600;
}

neko_static_inline auto neko_time_to_string(std::time_t now = std::time(nullptr)) -> std::string {
    const auto tp = std::localtime(&now);
    char buffer[32];
    return std::strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", tp) ? buffer : "1970-01-01_00:00:00";
}

#define neko_time_count(x) std::chrono::time_point_cast<std::chrono::microseconds>(x).time_since_epoch().count()

class neko_timer {
public:
    inline void start() noexcept { startPos = std::chrono::high_resolution_clock::now(); }
    inline void stop() noexcept {
        auto endTime = std::chrono::high_resolution_clock::now();
        duration = static_cast<f64>(neko_time_count(endTime) - neko_time_count(startPos)) * 0.001;
    }
    [[nodiscard]] inline f64 get() const noexcept { return duration; }
    ~neko_timer() noexcept { stop(); }

private:
    f64 duration = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> startPos;
};

neko_static_inline std::string neko_fs_normalize_path(const std::string& path, char delimiter = '/') {
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

neko_inline bool neko_fs_exists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

neko_inline void neko_utils_write_ppm(const int width, const int height, unsigned char* buffer, const char* filename) {

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

neko_inline void neko_tex_flip_vertically(int width, int height, u8* data) {
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

#include <array>
#include <string_view>

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
neko_inline void neko_swap(T& a, T& b) {
    T tmp = a;
    a = b;
    b = tmp;
}

template <typename T>
neko_static_inline void write_var(u8*& _buffer, T _var) {
    memcpy(_buffer, &_var, sizeof(T));
    _buffer += sizeof(T);
}

neko_static_inline void write_str(u8*& _buffer, const char* _str) {
    u32 len = (u32)strlen(_str);
    write_var(_buffer, len);
    memcpy(_buffer, _str, len);
    _buffer += len;
}

template <typename T>
neko_static_inline void read_var(u8*& _buffer, T& _var) {
    memcpy(&_var, _buffer, sizeof(T));
    _buffer += sizeof(T);
}

neko_static_inline char* read_string(u8*& _buffer) {
    u32 len;
    read_var(_buffer, len);
    char* str = new char[len + 1];
    memcpy(str, _buffer, len);
    str[len] = 0;
    _buffer += len;
    return str;
}

neko_static_inline const char* duplicate_string(const char* _str) {
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

template <typename T>
struct neko_span {
    neko_span() : __begin(nullptr), __end(nullptr) {}
    neko_span(T* begin, u32 len) : __begin(begin), __end(begin + len) {}
    neko_span(T* begin, T* end) : __begin(begin), __end(end) {}
    template <int N>
    neko_span(T (&value)[N]) : __begin(value), __end(value + N) {}
    T& operator[](u32 idx) const {
        neko_assert(__begin + idx < __end);
        return __begin[idx];
    }
    operator neko_span<const T>() const { return neko_span<const T>(__begin, __end); }
    void remove_prefix(u32 count) {
        neko_assert(count <= length());
        __begin += count;
    }
    void remove_suffix(u32 count) {
        neko_assert(count <= length());
        __end -= count;
    }
    [[nodiscard]] neko_span from_left(u32 count) const {
        neko_assert(count <= length());
        return neko_span(__begin + count, __end);
    }
    [[nodiscard]] neko_span from_right(u32 count) const {
        neko_assert(count <= length());
        return neko_span(__begin, __end - count);
    }
    T& back() {
        neko_assert(length() > 0);
        return *(__end - 1);
    }
    const T& back() const {
        neko_assert(length() > 0);
        return *(__end - 1);
    }
    bool equals(const neko_span<T>& rhs) {
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

#endif