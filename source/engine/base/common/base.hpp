#pragma once

#include <bitset>
#include <cassert>
#include <condition_variable>
#include <cstdarg>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <string>
#include <thread>

#if defined(_WIN32)
#define NEKO_IS_WIN32
#elif defined(__EMSCRIPTEN__)
#define NEKO_IS_WEB
#elif defined(__linux__) || defined(__unix__)
#define NEKO_IS_LINUX
#elif defined(__APPLE__) || defined(_APPLE)
#define NEKO_IS_APPLE
#elif defined(__ANDROID__)
#define NEKO_IS_ANDROID
#endif

typedef size_t usize;

typedef intptr_t iptr;
typedef uintptr_t uptr;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;
typedef const char *const_str;

#define u8_max UINT8_MAX
#define u16_max UINT16_MAX
#define u32_max UINT32_MAX
#define u64_max UINT64_MAX
#define s32_max INT32_MAX
#define f32_max FLT_MAX
#define f32_min FLT_MIN

typedef char Char;
typedef unsigned char Uchar;
typedef __int32 Int32;
typedef unsigned __int32 Uint32;
typedef __int16 Int16;
typedef unsigned __int16 Uint16;
typedef __int64 Int64;
typedef unsigned __int64 Uint64;
typedef float Float;
typedef double Double;
typedef Uint32 string_t;
typedef Int32 entindex_t;

struct color32_t {
    color32_t() : r(0), g(0), b(0), a(255) {}
    color32_t(u8 _r, u8 _g, u8 _b, u8 _a) {
        r = _r;
        g = _g;
        b = _b;
        a = _a;
    }
    u8 &operator[](Uint32 n) {
        assert(n <= 3);

        if (n == 0)
            return r;
        else if (n == 1)
            return g;
        else if (n == 2)
            return b;
        else
            return a;
    }

    u8 r;
    u8 g;
    u8 b;
    u8 a;
};

struct color24_t {
    color24_t() : r(0), g(0), b(0) {}
    color24_t(u8 _r, u8 _g, u8 _b) {
        r = _r;
        g = _g;
        b = _b;
    }
    u8 &operator[](Uint32 n) {
        assert(n <= 2);

        if (n == 0)
            return r;
        else if (n == 1)
            return g;
        else
            return b;
    }

    u8 r;
    u8 g;
    u8 b;
};

#ifndef NDEBUG
#if !defined(USE_PROFILER) && !defined(__EMSCRIPTEN__)
// #define USE_PROFILER
#endif
// #define HACK_MEM_CHECK
#endif

#if (defined _WIN32 || defined _WIN64)
#define NEKO_FORCE_INLINE inline
#elif (defined __APPLE__ || defined _APPLE)
#define NEKO_FORCE_INLINE static __attribute__((always_inline))
#else
#define NEKO_FORCE_INLINE static inline
#endif

#if defined(__cplusplus)
#define NEKO_DEFAULT_VAL() \
    {                      \
    }
#define NEKO_API() extern "C"
#else
#define NEKO_DEFAULT_VAL() {0}
#define NEKO_API() extern
#endif

#if defined(__cplusplus)

#define NEKO_SCRIPT(name, ...)                             \
    static const char *nekogame_ffi_##name = #__VA_ARGS__; \
    __VA_ARGS__

#ifdef _MSC_VER
#define NEKO_EXPORT extern "C" __declspec(dllexport)
#else
#define NEKO_EXPORT extern "C"
#endif

#else

#define NEKO_SCRIPT(name, ...)                             \
    static const char *nekogame_ffi_##name = #__VA_ARGS__; \
    __VA_ARGS__

#ifdef _MSC_VER
#define NEKO_EXPORT extern __declspec(dllexport)
#else
#define NEKO_EXPORT extern
#endif

#endif

#define NEKO_ARR_SIZE(__ARR) (sizeof(__ARR) / sizeof(__ARR[0]))

#if defined(_MSC_VER)
#define NEKO_UNUSED(x) (void)x
#else
#define NEKO_UNUSED(x) (void)(sizeof(x))
#endif

#define NEKO_FOR_RANGE_N(__COUNT, N) for (u32 N = 0; N < __COUNT; ++N)
#define NEKO_FOR_RANGE(__COUNT) for (u32 NEKO_TOKEN_PASTE(__T, __LINE__) = 0; NEKO_TOKEN_PASTE(__T, __LINE__) < __COUNT; ++(NEKO_TOKEN_PASTE(__T, __LINE__)))

#define NEKO_STR_CPY(des, src)          \
    strncpy(des, src, sizeof(des) - 1); \
    des[sizeof(des) - 1] = '\0'

#define NEKO_MAX(A, B) ((A) > (B) ? (A) : (B))
#define NEKO_MIN(A, B) ((A) < (B) ? (A) : (B))
#define NEKO_CLAMP(V, MIN, MAX) ((V) > (MAX) ? (MAX) : (V) < (MIN) ? (MIN) : (V))
#define NEKO_IS_NAN(V) ((V) != (V))
#define NEKO_BOOL_STR(V) (V ? "true" : "false")

#ifdef __cplusplus
#define NEKO_CTOR(TYPE, ...) (TYPE{__VA_ARGS__})
#else
#define NEKO_CTOR(TYPE, ...) ((TYPE){__VA_ARGS__})
#endif

#define NEKO_OFFSET(TYPE, ELEMENT) ((size_t)(&(((TYPE *)(0))->ELEMENT)))
#define NEKO_TO_STR(TYPE) ((const char *)#TYPE)
#define NEKO_TOKEN_PASTE(X, Y) X##Y
#define NEKO_CONCAT(X, Y) NEKO_TOKEN_PASTE(X, Y)

#define NEKO_COL255(x) x / 255.f

#define NEKO_STR(x) #x

#define TimedAction(INTERVAL, ...)                                     \
    do {                                                               \
        static u32 NEKO_CONCAT(NEKO_CONCAT(__T, __LINE__), t) = 0;     \
        if (NEKO_CONCAT(NEKO_CONCAT(__T, __LINE__), t)++ > INTERVAL) { \
            NEKO_CONCAT(NEKO_CONCAT(__T, __LINE__), t) = 0;            \
            __VA_ARGS__                                                \
        }                                                              \
    } while (0)

#ifdef __cplusplus
#define NEKO_INVOKE_ONCE(...)                           \
    static char NEKO_CONCAT(unused, __LINE__) = [&]() { \
        __VA_ARGS__;                                    \
        return '\0';                                    \
    }();                                                \
    (void)NEKO_CONCAT(unused, __LINE__)
#else
#define NEKO_INVOKE_ONCE(...) \
    do {                      \
        static u8 did = 0;    \
        if (!did) {           \
            __VA_ARGS__;      \
            did = 1;          \
        }                     \
    } while (0)
#endif

#define NEKO_INT2VOIDP(I) (void *)(uintptr_t)(I)

#define NEKO_EXPECT(x)                                             \
    do {                                                           \
        if (!(x)) {                                                \
            LOG_INFO("Unexpect error: assertion '{}' failed", #x); \
            abort();                                               \
        }                                                          \
    } while (0)

#define NEKO_CHOOSE(type, ...) ((type[]){__VA_ARGS__})[rand() % (sizeof((type[]){__VA_ARGS__}) / sizeof(type))]

// Custom printf defines
#ifndef neko_printf
#ifdef __MINGW32__
#define neko_printf(__FMT, ...) __mingw_printf(__FMT, ##__VA_ARGS__)
#elif (defined NEKO_IS_ANDROID)
#include <android/log.h>
#define neko_printf(__FMT, ...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __FMT, ##__VA_ARGS__))
#else
inline void neko_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
#endif
#endif

#define neko_println(__FMT, ...)           \
    do {                                   \
        neko_printf(__FMT, ##__VA_ARGS__); \
        neko_printf("\n");                 \
    } while (0)

#define MATH_PI 3.1415926535897f

#ifdef __clang__
#define FORMAT_ARGS(n) __attribute__((format(printf, n, n + 1)))
#else
#define FORMAT_ARGS(n)
#endif

#define JOIN_1(x, y) x##y
#define JOIN_2(x, y) JOIN_1(x, y)

NEKO_API() void errorf(const char *fmt, ...);

#define line_str__(line) __FILE__ ":" #line ": "
#define line_str_(line) line_str__(line)
#define line_str() line_str_(__LINE__)

#define error(...) errorf(line_str() __VA_ARGS__)

#define error_assert(cond, ...) ((cond) ? 0 : (error("assertion '" #cond "' failed ... " __VA_ARGS__), 0))

#define normal_assert(cond, ...) ((cond) ? 0 : (neko_printf("    assertion failed: (%s), function %s, file %s, line %d.\n" __VA_ARGS__, #cond, __func__, __FILE__, __LINE__), 0))

#define neko_assert normal_assert

#ifdef __MINGW32__
#define neko_snprintf(__NAME, __SZ, __FMT, ...) __mingw_snprintf(__NAME, __SZ, __FMT, ##__VA_ARGS__)
#else
inline void neko_snprintf(char *buffer, size_t buffer_size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, buffer_size, fmt, args);
    va_end(args);
}
#endif

#define neko_transient_buffer(__N, __SZ) \
    char __N[__SZ] = NEKO_DEFAULT_VAL(); \
    memset(__N, 0, __SZ);

#define neko_snprintfc(__NAME, __SZ, __FMT, ...) \
    char __NAME[__SZ] = NEKO_DEFAULT_VAL();      \
    neko_snprintf(__NAME, __SZ, __FMT, ##__VA_ARGS__);

#define NEKO_VA_COUNT(...) detail::va_count(__VA_ARGS__)

#define NEKO_DYNAMIC_CAST(type, input_var, cast_var_name)  \
    neko_assert(value);                                    \
    type *cast_var_name = dynamic_cast<type *>(input_var); \
    neko_assert(cast_var_name)
#define NEKO_STATIC_CAST(type, input_var, cast_var_name)  \
    neko_assert(value);                                   \
    type *cast_var_name = static_cast<type *>(input_var); \
    neko_assert(cast_var_name)

#define NEKO_ENUM_FLAG(T)                                                                                                                                                    \
    inline T operator~(T a) { return static_cast<T>(~static_cast<std::underlying_type<T>::type>(a)); }                                                                       \
    inline T operator|(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) | static_cast<std::underlying_type<T>::type>(b)); }                   \
    inline T operator&(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) & static_cast<std::underlying_type<T>::type>(b)); }                   \
    inline T operator^(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) ^ static_cast<std::underlying_type<T>::type>(b)); }                   \
    inline T &operator|=(T &a, T b) { return reinterpret_cast<T &>(reinterpret_cast<std::underlying_type<T>::type &>(a) |= static_cast<std::underlying_type<T>::type>(b)); } \
    inline T &operator&=(T &a, T b) { return reinterpret_cast<T &>(reinterpret_cast<std::underlying_type<T>::type &>(a) &= static_cast<std::underlying_type<T>::type>(b)); } \
    inline T &operator^=(T &a, T b) { return reinterpret_cast<T &>(reinterpret_cast<std::underlying_type<T>::type &>(a) ^= static_cast<std::underlying_type<T>::type>(b)); }

#define NEKO_MOVEONLY(class_name)                       \
    class_name(const class_name &) = delete;            \
    class_name &operator=(const class_name &) = delete; \
    class_name(class_name &&) = default;                \
    class_name &operator=(class_name &&) = default

namespace Neko {}  // namespace Neko