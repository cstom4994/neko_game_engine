#ifndef NEKO_TYPES_H
#define NEKO_TYPES_H

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cctype>
#include <cfloat>
#include <codecvt>
#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <functional>
#include <limits>
#include <locale>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>

#ifndef neko_inline
#define neko_inline inline
#endif
#ifndef neko_static_inline
#define neko_static_inline static neko_inline
#endif
#ifndef neko_local_persist
#define neko_local_persist static
#endif
#ifndef neko_global
#define neko_global static
#endif

#ifndef neko_private
#define neko_private(_result_type) static _result_type
#endif
#ifndef neko_public
#define neko_public(_result_type) _result_type
#endif

#if (defined _WIN32 || defined _WIN64)
#define neko_force_inline neko_static_inline
#elif (defined __APPLE__ || defined _APPLE)
#define neko_force_inline static __attribute__((always_inline))
#else
#define neko_force_inline neko_static_inline
#endif

#ifndef neko_little_endian
#define neko_little_endian 1
#endif
#ifndef neko_bit
#define neko_bit(x) (1 << x)
#endif

/*============================================================
// Resource Declarations
============================================================*/

#define neko_resource(type) neko_resource_##type

// Strongly typed declarations for resource handles (slot array handles)
#define neko_declare_resource_type(type)                                   \
    typedef struct neko_resource(type) { u32 id; }                         \
    neko_resource(type);                                                   \
                                                                           \
    neko_force_inline neko_resource(type) neko_resource_invalid_##type() { \
        neko_resource(type) r;                                             \
        r.id = u32_max;                                                    \
        return r;                                                          \
    }

#define neko_resource_invalid(type) neko_resource_invalid_##type()

/*============================================================
// Result
============================================================*/

typedef enum { neko_result_success, neko_result_in_progress, neko_result_incomplete, neko_result_failure } neko_result;

/*================
// Color
=================*/

#define neko_hsv(...) neko_hsv_ctor(__VA_ARGS__)
#define neko_color(...) neko_color_ctor(__VA_ARGS__)

typedef struct neko_hsv_t {
    union {
        float hsv[3];
        struct {
            float h, s, v;
        };
    };
} neko_hsv_t;

neko_force_inline neko_hsv_t neko_hsv_ctor(float h, float s, float v) {
    neko_hsv_t hsv;
    hsv.h = h;
    hsv.s = s;
    hsv.v = v;
    return hsv;
}

typedef struct neko_color_t {
    union {
        uint8_t rgba[4];
        struct {
            uint8_t r, g, b, a;
        };
    };
} neko_color_t;

neko_force_inline neko_color_t neko_color_ctor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    neko_color_t color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;
    return color;
}

#define neko_color_black neko_color(0, 0, 0, 255)
#define neko_color_white neko_color(255, 255, 255, 255)
#define neko_color_red neko_color(255, 0, 0, 255)
#define neko_color_green neko_color(0, 255, 0, 255)
#define neko_color_blue neko_color(0, 0, 255, 255)
#define neko_color_orange neko_color(255, 100, 0, 255)
#define neko_color_purple neko_color(128, 0, 128, 255)

neko_force_inline neko_color_t neko_color_alpha(neko_color_t c, uint8_t a) { return neko_color(c.r, c.g, c.b, a); }

/*============================================================
// Primitives
============================================================*/

#ifndef __cplusplus
#define false 0
#define true 1
#endif

typedef size_t usize;

#ifdef __cplusplus
typedef bool b8;
#else
typedef _Bool b8;
#endif

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef s32 b32;
typedef uint64_t u64;
typedef int64_t s64;
typedef float f32;
typedef double f64;
typedef const char *const_str;

typedef std::string neko_string;
typedef std::string_view neko_string_view;

#define u16_max UINT16_MAX
#define u32_max UINT32_MAX
#define s32_max INT32_MAX
#define f32_max FLT_MAX
#define f32_min FLT_MIN

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
struct cpp_remove_reference<T &> {
    using type = T;
};

template <typename T>
struct cpp_remove_reference<T &&> {
    using type = T;
};

template <typename T>
constexpr typename cpp_remove_reference<T>::type &&cpp_move(T &&arg) noexcept {
    return (typename cpp_remove_reference<T>::type &&) arg;
}

template <typename T>
using initializer_list = std::initializer_list<T>;

template <typename T>
using neko_function = std::function<T>;

template <typename T>
using scope = std::unique_ptr<T>;
template <typename T, typename... Args>
constexpr scope<T> create_scope(Args &&...args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T>
using ref = std::shared_ptr<T>;
template <typename T, typename... Args>
constexpr ref<T> create_ref(Args &&...args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T>
struct neko_named_func {
    std::string name;
    neko_function<T> func;
};

}  // namespace neko

#endif  // NEKO_TYPES_H
