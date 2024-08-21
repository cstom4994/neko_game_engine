#pragma once

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

struct lua_State;

#define u8_max UINT8_MAX
#define u16_max UINT16_MAX
#define u32_max UINT32_MAX
#define u64_max UINT64_MAX
#define s32_max INT32_MAX
#define f32_max FLT_MAX
#define f32_min FLT_MIN

#if (defined _WIN32 || defined _WIN64)
#define NEKO_FORCE_INLINE inline
#elif (defined __APPLE__ || defined _APPLE)
#define NEKO_FORCE_INLINE static __attribute__((always_inline))
#else
#define NEKO_FORCE_INLINE static inline
#endif

#define NEKO_ARR_SIZE(__ARR) sizeof(__ARR) / sizeof(__ARR[0])

#if defined(_MSC_VER)
#define NEKO_UNUSED(x) (void)x
#else
#define NEKO_UNUSED(x) (void)(sizeof(x))
#endif

#define neko_assert(x, ...)                                                                                                \
    do {                                                                                                                   \
        if (!(x)) {                                                                                                        \
            neko_printf("    assertion failed: (%s), function %s, file %s, line %d.\n", #x, __func__, __FILE__, __LINE__); \
        }                                                                                                                  \
    } while (0)

#if defined(__cplusplus)
#define NEKO_DEFAULT_VAL() \
    {}
#define NEKO_EXTERN extern "C"
#else
#define NEKO_DEFAULT_VAL() \
    { 0 }
#define NEKO_EXTERN extern
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

#define NEKO_EXPECT(x)                                                \
    do {                                                              \
        if (!(x)) {                                                   \
            console_log("Unexpect error: assertion '%s' failed", #x); \
            abort();                                                  \
        }                                                             \
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

#define array_size(a) (sizeof(a) / sizeof(a[0]))
#define MATH_PI 3.1415926535897f

#ifdef __clang__
#define FORMAT_ARGS(n) __attribute__((format(printf, n, n + 1)))
#else
#define FORMAT_ARGS(n)
#endif

#define JOIN_1(x, y) x##y
#define JOIN_2(x, y) JOIN_1(x, y)

void neko_log(const char *file, int line, const char *fmt, ...);

#define console_log(...) neko_log(__FILE__, __LINE__, __VA_ARGS__)

void errorf(const char *fmt, ...);

#define line_str__(line) __FILE__ ":" #line ": "
#define line_str_(line) line_str__(line)
#define line_str() line_str_(__LINE__)

#define error(...) errorf(line_str() __VA_ARGS__)

#define error_assert(cond, ...) ((cond) ? 0 : (error("assertion '" #cond "' failed ... " __VA_ARGS__), 0))

FORMAT_ARGS(1)
inline void neko_panic(const char *fmt, ...) {
    va_list args = {};
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");

    exit(1);
}

inline bool is_whitespace(char c) {
    switch (c) {
        case '\n':
        case '\r':
        case '\t':
        case ' ':
            return true;
    }
    return false;
}

inline bool is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

inline bool is_digit(char c) { return c >= '0' && c <= '9'; }

void profile_setup();
void profile_shutdown();

#ifndef NDEBUG
#if !defined(USE_PROFILER) && !defined(__EMSCRIPTEN__)
// #define USE_PROFILER
#endif
#endif

#ifdef USE_PROFILER

struct TraceEvent {
    const char *cat;
    const char *name;
    u64 ts;
    u16 tid;
    char ph;
};

struct Instrument {
    const char *cat;
    const char *name;
    i32 tid;

    Instrument(const char *cat, const char *name);
    ~Instrument();
};

#define PROFILE_FUNC() auto JOIN_2(_profile_, __COUNTER__) = Instrument("function", __func__);

#define PROFILE_BLOCK(name) auto JOIN_2(_profile_, __COUNTER__) = Instrument("block", name);

#endif  // USE_PROFILER

#ifndef USE_PROFILER
#define PROFILE_FUNC()
#define PROFILE_BLOCK(name)
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

/*===================================
// Resource Handles
===================================*/

// Useful typedefs for typesafe, internal resource handles

#define neko_handle(TYPE) neko_handle_##TYPE

#define neko_handle_decl(TYPE)                                              \
    typedef struct {                                                        \
        u32 id;                                                             \
    } neko_handle(TYPE);                                                    \
    NEKO_FORCE_INLINE neko_handle(TYPE) neko_handle_invalid_##TYPE() {      \
        neko_handle(TYPE) h;                                                \
        h.id = UINT32_MAX;                                                  \
        return h;                                                           \
    }                                                                       \
                                                                            \
    NEKO_FORCE_INLINE neko_handle(TYPE) neko_handle_create_##TYPE(u32 id) { \
        neko_handle(TYPE) h;                                                \
        h.id = id;                                                          \
        return h;                                                           \
    }

#define neko_handle_invalid(__TYPE) neko_handle_invalid_##__TYPE()

#define neko_handle_create(__TYPE, __ID) neko_handle_create_##__TYPE(__ID)

#define neko_handle_is_valid(HNDL) ((HNDL.id) != UINT32_MAX)
