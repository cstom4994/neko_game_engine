

#ifndef NEKO_H
#define NEKO_H

#include <assert.h>
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <malloc.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*===================
// PLATFORM DEFINES
===================*/

/* Platform Android */
#if (defined __ANDROID__)

#define NEKO_PLATFORM_ANDROID

/* Platform Apple */
#elif (defined __APPLE__ || defined _APPLE)

#define NEKO_PLATFORM_APPLE

/* Platform Windows */
#elif (defined _WIN32 || defined _WIN64)

#define __USE_MINGW_ANSI_STDIO 1

// Necessary windows defines before including windows.h, because it's retarded.
#define OEMRESOURCE

#define NEKO_PLATFORM_WIN

#define _WINSOCKAPI_
#include <windows.h>

#define WIN32_LEAN_AND_MEAN

/* Platform Linux */
#elif (defined linux || defined _linux || defined __linux__)

#define NEKO_PLATFORM_LINUX

#include <string.h>
#include <time.h>
#include <unistd.h>

/* Platform Emscripten */
#elif (defined __EMSCRIPTEN__)

#define NEKO_PLATFORM_WEB

/* Else - Platform Undefined and Unsupported or custom */

#endif

#if defined(DEBUG) || defined(_DEBUG)
#define NEKO_DEBUG
#endif

/*========================
// Defines
========================*/

#ifndef neko_inline
#if defined(NEKO_PLATFORM_LINUX)
#define neko_inline static inline
#else
#define neko_inline inline
#endif
#endif

#ifndef neko_local_persist
#define neko_local_persist static
#endif

#ifndef neko_global
#define neko_global static
#endif

#ifndef neko_static_inline
#define neko_static_inline static inline
#endif

#if (defined _WIN32 || defined _WIN64)
#define neko_force_inline neko_inline
#elif (defined __APPLE__ || defined _APPLE)
#define neko_force_inline static __attribute__((always_inline))
#else
#define neko_force_inline neko_inline
#endif

#ifndef neko_private
#define neko_private(_result_type) static _result_type
#endif

#ifndef neko_public
#define neko_public(_result_type) _result_type
#endif

#ifndef neko_little_endian
#define neko_little_endian 1
#endif

#ifndef neko_bit
#define neko_bit(x) (1 << x)
#endif

/*===================
// NEKO_API_DECL
===================*/

#ifdef NEKO_API_DLL_EXPORT
#ifdef __cplusplus
#define NEKO_API_EXTERN extern "C" __declspec(dllexport)
#else
#define NEKO_API_EXTERN extern __declspec(dllexport)
#endif
#else
#ifdef __cplusplus
#define NEKO_API_EXTERN extern "C"
#define NEKO_CPP_SRC
#else
#define NEKO_API_EXTERN extern
#endif
#endif

#define NEKO_API_DECL NEKO_API_EXTERN
#define NEKO_API_PRIVATE NEKO_API_EXTERN

/*============================================================
// C primitive types
============================================================*/

#ifndef NEKO_CPP_SRC
#define false 0
#define true 1
#endif

#ifdef NEKO_CPP_SRC
typedef bool b8;
#else
#ifndef __bool_true_false_are_defined
typedef _Bool bool;
#endif
typedef bool b8;
#endif

typedef size_t usize;

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
typedef const char* const_str;
typedef int32_t bool32_t;

typedef bool32_t b32;
typedef intptr_t iptr;
typedef uintptr_t uptr;

#define u16_max UINT16_MAX
#define u32_max UINT32_MAX
#define s32_max INT32_MAX
#define f32_max FLT_MAX
#define f32_min FLT_MIN

// 提供于静态反射和序列化模块
enum neko_type_kind {
    TYPE_KIND_S8 = 0x00,
    TYPE_KIND_S16,
    TYPE_KIND_S32,
    TYPE_KIND_S64,
    TYPE_KIND_U8,
    TYPE_KIND_U16,
    TYPE_KIND_U32,
    TYPE_KIND_U64,
    TYPE_KIND_F32,
    TYPE_KIND_F64,
    TYPE_KIND_BOOL,
    TYPE_KIND_CHAR,
    TYPE_KIND_STR,
    TYPE_KIND_VOID,
    TYPE_KIND_POINTER,
    TYPE_KIND_ARRAY,
    TYPE_KIND_ENUM,
    TYPE_KIND_STRUCT,
    TYPE_KIND_COUNT
};

// Helper macro for compiling to nothing
#define neko_empty_instruction(...)

#define neko_arr_size(__ARR) sizeof(__ARR) / sizeof(__ARR[0])

#ifdef NEKO_DEBUG
#define neko_assert(x)                                                                                                 \
    do {                                                                                                               \
        if (!(x)) {                                                                                                    \
            neko_printf("assertion failed: (%s), function %s, file %s, line %d.\n", #x, __func__, __FILE__, __LINE__); \
            neko_debugbreak();                                                                                         \
        }                                                                                                              \
    } while (0)
#else
#define neko_assert(x, ...)                                                                                            \
    do {                                                                                                               \
        if (!(x)) {                                                                                                    \
            neko_printf("assertion failed: (%s), function %s, file %s, line %d.\n", #x, __func__, __FILE__, __LINE__); \
        }                                                                                                              \
    } while (0)
#endif

#if defined(NEKO_CPP_SRC)
#define neko_default_val() \
    {}
#else
#define neko_default_val() \
    { 0 }
#endif

// Helper macro for an in place for-range loop
#define neko_for_range_i(__COUNT) for (u32 i = 0; i < __COUNT; ++i)

// Helper macro for an in place for-range loop
#define neko_for_range_j(__COUNT) for (u32 j = 0; j < __COUNT; ++j)

#define neko_for_range(__COUNT) for (u32 neko_macro_token_paste(__T, __LINE__) = 0; neko_macro_token_paste(__T, __LINE__) < __COUNT; ++(neko_macro_token_paste(__T, __LINE__)))

#define neko_max(A, B) ((A) > (B) ? (A) : (B))

#define neko_min(A, B) ((A) < (B) ? (A) : (B))

#define neko_clamp(V, MIN, MAX) ((V) > (MAX) ? (MAX) : (V) < (MIN) ? (MIN) : (V))

#define neko_is_nan(V) ((V) != (V))

#define neko_bool_str(V) (V ? "true" : "false")

// Helpful macro for casting one type to another
#define neko_cast(A, B) ((A*)(B))

#define neko_r_cast reinterpret_cast
#define neko_s_cast static_cast
#define neko_c_cast const_cast

#ifdef __cplusplus
#define neko_ctor(TYPE, ...) (TYPE{__VA_ARGS__})
#else
#define neko_ctor(TYPE, ...) ((TYPE){__VA_ARGS__})
#endif

// Helpful marco for calculating offset (in bytes) of an element from a given structure type
#define neko_offset(TYPE, ELEMENT) ((size_t)(&(((TYPE*)(0))->ELEMENT)))

// macro for turning any given type into a const char* of itself
#define neko_to_str(TYPE) ((const char*)#TYPE)

#define neko_macro_token_paste(X, Y) X##Y
#define neko_macro_cat(X, Y) neko_macro_token_paste(X, Y)

#define neko_timed_action(INTERVAL, ...)                                     \
    do {                                                                     \
        static u32 neko_macro_cat(neko_macro_cat(__T, __LINE__), t) = 0;     \
        if (neko_macro_cat(neko_macro_cat(__T, __LINE__), t)++ > INTERVAL) { \
            neko_macro_cat(neko_macro_cat(__T, __LINE__), t) = 0;            \
            __VA_ARGS__                                                      \
        }                                                                    \
    } while (0)

#define neko_concat(x, y) neko_concat_impl(x, y)
#define neko_concat_impl(x, y) x##y

#ifdef __cplusplus
#define neko_invoke_once(...)                           \
    static char neko_concat(unused, __LINE__) = [&]() { \
        __VA_ARGS__;                                    \
        return '\0';                                    \
    }();                                                \
    (void)neko_concat(unused, __LINE__)
#else
#define neko_invoke_once(...) \
    do {                      \
        static u8 did = 0;    \
        if (!did) {           \
            __VA_ARGS__;      \
            did = 1;          \
        }                     \
    } while (0)
#endif

#define neko_int2voidp(I) (void*)(uintptr_t)(I)

#define neko_if(INIT, CHECK, ...) \
    do {                          \
        INIT;                     \
        if (CHECK) {              \
            __VA_ARGS__           \
        }                         \
    } while (0)

#define neko_expect(x)                                                   \
    do {                                                                 \
        if (!(x)) {                                                      \
            neko_log_error("Unexpect error: assertion '%s' failed", #x); \
            abort();                                                     \
        }                                                                \
    } while (0)

#define neko_choose(type, ...) ((type[]){__VA_ARGS__})[rand() % (sizeof((type[]){__VA_ARGS__}) / sizeof(type))]

// Custom printf defines
#ifndef neko_printf

#ifdef __MINGW32__

#define neko_printf(__FMT, ...) __mingw_printf(__FMT, ##__VA_ARGS__)

#elif (defined NEKO_PLATFORM_ANDROID)

#include <android/log.h>

#define neko_printf(__FMT, ...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __FMT, ##__VA_ARGS__))

#else
neko_force_inline void neko_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
#endif

#endif

// Logging
#define neko_log_info(...) log_info(__VA_ARGS__)
#define neko_log_trace(...) log_trace(__VA_ARGS__)
#define neko_log_warning(...) log_warn(__VA_ARGS__)
#define neko_log_error(msg, ...)                                            \
    do {                                                                    \
        char tmp[512];                                                      \
        neko_snprintf(tmp, 512, msg, __VA_ARGS__);                          \
        log_error("%s (%s:%s:%zu)", tmp, __FILE__, __FUNCTION__, __LINE__); \
    } while (0)

#define neko_enum_flag_operator(T)                                                                                                                                         \
    inline T operator~(T a) { return static_cast<T>(~static_cast<std::underlying_type<T>::type>(a)); }                                                                     \
    inline T operator|(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) | static_cast<std::underlying_type<T>::type>(b)); }                 \
    inline T operator&(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) & static_cast<std::underlying_type<T>::type>(b)); }                 \
    inline T operator^(T a, T b) { return static_cast<T>(static_cast<std::underlying_type<T>::type>(a) ^ static_cast<std::underlying_type<T>::type>(b)); }                 \
    inline T& operator|=(T& a, T b) { return reinterpret_cast<T&>(reinterpret_cast<std::underlying_type<T>::type&>(a) |= static_cast<std::underlying_type<T>::type>(b)); } \
    inline T& operator&=(T& a, T b) { return reinterpret_cast<T&>(reinterpret_cast<std::underlying_type<T>::type&>(a) &= static_cast<std::underlying_type<T>::type>(b)); } \
    inline T& operator^=(T& a, T b) { return reinterpret_cast<T&>(reinterpret_cast<std::underlying_type<T>::type&>(a) ^= static_cast<std::underlying_type<T>::type>(b)); }

#define neko_move_only(class_name)                     \
    class_name(const class_name&) = delete;            \
    class_name& operator=(const class_name&) = delete; \
    class_name(class_name&&) = default;                \
    class_name& operator=(class_name&&) = default

#define neko_aligned_buffer(name) alignas(16) static const std::u8 name[]

#define neko_va_unpack(...) __VA_ARGS__  // 用于解包括号 带逗号的宏参数需要它

#define neko_offsetof(s, m) ((::size_t) & reinterpret_cast<char const volatile&>((((s*)0)->m)))

#if defined(_MSC_VER)
#define neko_unused(x) (void)x
#else
#define neko_unused(x) (void)(sizeof(x))
#endif

#if defined(__clang__)
#define neko_debugbreak() __builtin_debugtrap()
#elif defined(_MSC_VER)
#define neko_debugbreak() __debugbreak()
#else
#define neko_debugbreak()
#endif

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

// Helper macro for typedefing a struture definition
#define neko_struct_def(name, ...) \
    typedef struct {               \
        __VA_ARGS__                \
    } name

// Definition for derived struct (based another parent struct)
#define neko_derive_def(name, parent, ...) neko_struct_def(name, parent _base; __VA_ARGS__)

#define neko_engine_check(statement) (statement) ? true : false

#define _base(base_type) base_type _base

/*=============================
// Memory
=============================*/

// Operating system function pointer
typedef struct neko_os_api_s {
    void* (*malloc)(size_t sz);
    void (*free)(void* ptr);
    void* (*realloc)(void* ptr, size_t sz);
    void* (*calloc)(size_t num, size_t sz);
    void* (*malloc_init)(size_t sz);
    char* (*strdup)(const char* str);
} neko_os_api_t;

// TODO: Check if all defaults need to be set, in case neko context will not be used

NEKO_API_DECL
void* _neko_malloc_init_impl(size_t sz);

// Default memory allocations
#define neko_malloc malloc
#define neko_free free
#define neko_realloc realloc
#define neko_calloc calloc

#define neko_malloc_init(__T) (__T*)_neko_malloc_init_impl(sizeof(__T))

NEKO_API_DECL neko_os_api_t neko_os_api_new_default();

#ifndef neko_os_api_new
#define neko_os_api_new neko_os_api_new_default
#endif

#ifndef neko_strdup
#define neko_strdup(__STR) (neko_ctx()->os.strdup(__STR))
#endif

#define NEKO_MEM_CHECK 0

NEKO_API_DECL void* __neko_mem_safe_alloc(size_t size, const char* file, int line, size_t* statistics);
NEKO_API_DECL void* __neko_mem_safe_calloc(size_t count, size_t element_size, const char* file, int line, size_t* statistics);
NEKO_API_DECL void __neko_mem_safe_free(void* mem, size_t* statistics);
NEKO_API_DECL void* __neko_mem_safe_realloc(void* ptr, size_t new_size, const char* file, int line, size_t* statistics);

#if 1

#define neko_safe_malloc(size) __neko_mem_safe_alloc((size), (char*)__FILE__, __LINE__, NULL)
#define neko_safe_free(mem) __neko_mem_safe_free((void*)mem, NULL)
#define neko_safe_realloc(ptr, size) __neko_mem_safe_realloc((ptr), (size), (char*)__FILE__, __LINE__, NULL)
#define neko_safe_calloc(count, element_size) __neko_mem_safe_calloc(count, element_size, (char*)__FILE__, __LINE__, NULL)

#else

#define neko_safe_malloc neko_malloc
#define neko_safe_free neko_free
#define neko_safe_realloc neko_realloc
#define neko_safe_calloc neko_calloc

#endif

NEKO_API_DECL int neko_mem_check_leaks(bool detailed);
NEKO_API_DECL int neko_mem_bytes_inuse();

typedef struct neko_allocation_metrics {
    u64 total_allocated;
    u64 total_free;
} neko_allocation_metrics;

NEKO_API_DECL neko_allocation_metrics g_allocation_metrics;

NEKO_API_DECL void __neko_mem_init(int argc, char** argv);
NEKO_API_DECL void __neko_mem_end();

// logging

typedef struct {
    va_list ap;
    const char* fmt;
    const char* file;
    struct tm* time;
    FILE* udata;
    int line;
    int level;
} neko_log_event;

typedef void (*neko_log_fn)(neko_log_event* ev);
typedef void (*neko_log_lock_fn)(bool lock, void* udata);

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

#define log_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) log_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) log_log(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

NEKO_API_DECL const char* log_level_string(int level);
NEKO_API_DECL void log_set_lock(neko_log_lock_fn fn, void* udata);
NEKO_API_DECL void log_set_level(int level);
NEKO_API_DECL void log_set_quiet(bool enable);
NEKO_API_DECL int log_add_callback(neko_log_fn fn, void* udata, int level);
NEKO_API_DECL int log_add_fp(FILE* fp, int level);

NEKO_API_DECL void log_log(int level, const char* file, int line, const char* fmt, ...);

/*============================================================
// Result
============================================================*/

typedef enum neko_result { NEKO_RESULT_SUCCESS, NEKO_RESULT_IN_PROGRESS, NEKO_RESULT_INCOMPLETE, NEKO_RESULT_FAILURE } neko_result;

/*===================================
// Resource Handles
===================================*/

// Useful typedefs for typesafe, internal resource handles

#define neko_handle(TYPE) neko_handle_##TYPE

#define neko_handle_decl(TYPE)                                        \
    typedef struct {                                                  \
        u32 id;                                                       \
    } neko_handle(TYPE);                                              \
    neko_inline neko_handle(TYPE) neko_handle_invalid_##TYPE() {      \
        neko_handle(TYPE) h;                                          \
        h.id = UINT32_MAX;                                            \
        return h;                                                     \
    }                                                                 \
                                                                      \
    neko_inline neko_handle(TYPE) neko_handle_create_##TYPE(u32 id) { \
        neko_handle(TYPE) h;                                          \
        h.id = id;                                                    \
        return h;                                                     \
    }

#define neko_handle_invalid(__TYPE) neko_handle_invalid_##__TYPE()

#define neko_handle_create(__TYPE, __ID) neko_handle_create_##__TYPE(__ID)

#define neko_handle_is_valid(HNDL) ((HNDL.id) != UINT32_MAX)

/*===================================
// Color
===================================*/

#define neko_hsv(...) neko_hsv_ctor(__VA_ARGS__)
#define neko_color(...) neko_color_ctor(__VA_ARGS__)

typedef struct neko_hsv_t {
    union {
        f32 hsv[3];
        struct {
            f32 h, s, v;
        };
    };
} neko_hsv_t;

neko_force_inline neko_hsv_t neko_hsv_ctor(f32 h, f32 s, f32 v) {
    neko_hsv_t hsv;
    hsv.h = h;
    hsv.s = s;
    hsv.v = v;
    return hsv;
}

typedef struct neko_color_t {
    union {
        u8 rgba[4];
        struct {
            u8 r, g, b, a;
        };
    };
} neko_color_t;

neko_force_inline neko_color_t neko_color_ctor(u8 r, u8 g, u8 b, u8 a) {
    neko_color_t color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;
    return color;
}

#define NEKO_COLOR_BLACK neko_color(0, 0, 0, 255)
#define NEKO_COLOR_WHITE neko_color(255, 255, 255, 255)
#define NEKO_COLOR_RED neko_color(255, 0, 0, 255)
#define NEKO_COLOR_GREEN neko_color(0, 255, 0, 255)
#define NEKO_COLOR_BLUE neko_color(0, 0, 255, 255)
#define NEKO_COLOR_ORANGE neko_color(255, 100, 0, 255)
#define NEKO_COLOR_YELLOW neko_color(255, 255, 0, 255)
#define NEKO_COLOR_PURPLE neko_color(128, 0, 128, 255)
#define NEKO_COLOR_MAROON neko_color(128, 0, 0, 255)
#define NEKO_COLOR_BROWN neko_color(165, 42, 42, 255)

neko_force_inline neko_color_t neko_color_alpha(neko_color_t c, u8 a) { return neko_color(c.r, c.g, c.b, a); }

neko_force_inline neko_hsv_t neko_rgb2hsv(neko_color_t in) {
    f32 ir = (f32)in.r / 255.f;
    f32 ig = (f32)in.g / 255.f;
    f32 ib = (f32)in.b / 255.f;
    f32 ia = (f32)in.a / 255.f;

    neko_hsv_t out = neko_default_val();
    f64 min, max, delta;

    min = ir < ig ? ir : ig;
    min = min < ib ? min : ib;

    max = ir > ig ? ir : ig;
    max = max > ib ? max : ib;

    out.v = max;  // v
    delta = max - min;
    if (delta < 0.00001) {
        out.s = 0;
        out.h = 0;  // undefined, maybe nan?
        return out;
    }

    if (max > 0.0) {            // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);  // s
    } else {
        // if max is 0, then r = g = b = 0
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = NAN;  // its now undefined
        return out;
    }
    if (ir >= max)                  // > is bogus, just keeps compilor happy
        out.h = (ig - ib) / delta;  // between yellow & magenta
    else if (ig >= max)
        out.h = 2.0 + (ib - ir) / delta;  // between cyan & yellow
    else
        out.h = 4.0 + (ir - ig) / delta;  // between magenta & cyan

    out.h *= 60.0;  // degrees

    if (out.h < 0.0) out.h += 360.0;

    return out;
}

neko_force_inline neko_color_t neko_hsv2rgb(neko_hsv_t in) {
    f64 hh, p, q, t, ff;
    long i;
    neko_color_t out;

    if (in.s <= 0.0) {  // < is bogus, just shuts up warnings
        out.r = in.v * 255;
        out.g = in.v * 255;
        out.b = in.v * 255;
        out.a = 255;
        return out;
    }
    hh = in.h;
    if (hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    u8 iv = in.v * 255;
    u8 it = t * 255;
    u8 ip = p * 255;
    u8 iq = q * 255;

    switch (i) {
        case 0:
            out.r = iv;
            out.g = it;
            out.b = ip;
            break;
        case 1:
            out.r = iq;
            out.g = iv;
            out.b = ip;
            break;
        case 2:
            out.r = ip;
            out.g = iv;
            out.b = it;
            break;

        case 3:
            out.r = ip;
            out.g = iq;
            out.b = iv;
            break;
        case 4:
            out.r = it;
            out.g = ip;
            out.b = iv;
            break;
        case 5:
        default:
            out.r = iv;
            out.g = ip;
            out.b = iq;
            break;
    }
    return out;
}

/*===================================
// String Utils
===================================*/

neko_force_inline u32 neko_string_length(const char* txt) {
    u32 sz = 0;
    while (txt != NULL && txt[sz] != '\0') {
        sz++;
    }
    return sz;
}

#define neko_strlen neko_string_length

// Expects null terminated strings
neko_force_inline b32 neko_string_compare_equal(const char* txt, const char* cmp) {
    // Grab sizes of both strings
    u32 a_sz = neko_string_length(txt);
    u32 b_sz = neko_string_length(cmp);

    // Return false if sizes do not match
    if (a_sz != b_sz) {
        return false;
    }

    for (u32 i = 0; i < a_sz; ++i) {
        if (*txt++ != *cmp++) {
            return false;
        }
    };

    return true;
}

neko_force_inline b32 neko_string_compare_equal_n(const char* txt, const char* cmp, u32 n) {
    u32 a_sz = neko_string_length(txt);
    u32 b_sz = neko_string_length(cmp);

    // Not enough characters to do operation
    if (a_sz < n || b_sz < n) {
        return false;
    }

    for (u32 i = 0; i < n; ++i) {
        if (*txt++ != *cmp++) {
            return false;
        }
    };

    return true;
}

neko_force_inline char* neko_util_string_concat(char* s1, const char* s2) {
    const size_t a = strlen(s1);
    const size_t b = strlen(s2);
    const size_t ab = a + b + 1;
    s1 = (char*)neko_safe_realloc((void*)s1, ab);
    memcpy(s1 + a, s2, b + 1);
    return s1;
}

neko_force_inline void neko_util_str_to_lower(const char* src, char* buffer, size_t buffer_sz) {
    size_t src_sz = neko_string_length(src);
    size_t len = neko_min(src_sz, buffer_sz - 1);

    for (u32 i = 0; i < len; ++i) {
        buffer[i] = tolower(src[i]);
    }
    if (len) buffer[len] = '\0';
}

neko_force_inline b32 neko_util_str_is_numeric(const char* str) {
    const char* at = str;
    while (at && *at) {
        while (*at == '\n' || *at == '\t' || *at == ' ' || *at == '\r') at++;
        ;
        char c = *at++;
        if (c < '0' || c > '9') {
            return false;
        }
    }

    return true;
}

// Will return a null buffer if file does not exist or allocation fails
NEKO_API_DECL char* neko_read_file_contents(const char* file_path, const char* mode, size_t* _sz);

neko_force_inline b32 neko_util_file_exists(const char* file_path) {
    FILE* fp = fopen(file_path, "r");
    if (fp) {
        fclose(fp);
        return true;
    }
    return false;
}

neko_force_inline void neko_util_get_file_extension(char* buffer, u32 buffer_size, const char* file_path) {
    neko_assert(buffer && buffer_size);
    const char* extension = strrchr(file_path, '.');
    if (extension) {
        uint32_t extension_len = strlen(extension + 1);
        uint32_t len = (extension_len >= buffer_size) ? buffer_size - 1 : extension_len;
        memcpy(buffer, extension + 1, len);
        buffer[len] = '\0';
    } else {
        buffer[0] = '\0';
    }
}

neko_force_inline void neko_util_get_dir_from_file(char* buffer, u32 buffer_size, const char* file_path) {
    u32 str_len = neko_string_length(file_path);
    const char* end = (file_path + str_len);
    for (u32 i = 0; i < str_len; ++i) {
        if (file_path[i] == '/' || file_path[i] == '\\') {
            end = &file_path[i];
        }
    }

    size_t dir_len = end - file_path;
    memcpy(buffer, file_path, neko_min(buffer_size, dir_len + 1));
    if (dir_len + 1 <= buffer_size) {
        buffer[dir_len] = '\0';
    }
}

neko_force_inline void neko_util_get_file_name(char* buffer, u32 buffer_size, const char* file_path) {
    u32 str_len = neko_string_length(file_path);
    const char* file_start = file_path;
    const char* file_end = (file_path + str_len);
    for (u32 i = 0; i < str_len; ++i) {
        if (file_path[i] == '/' || file_path[i] == '\\') {
            file_start = &file_path[i + 1];
        } else if (file_path[i] == '.') {
            file_end = &file_path[i];
        }
    }

    size_t dir_len = file_end - file_start;
    memcpy(buffer, file_start, neko_min(buffer_size, dir_len + 1));
    if (dir_len + 1 <= buffer_size) {
        buffer[dir_len] = '\0';
    }
}

neko_force_inline void neko_util_string_substring(const char* src, char* dst, size_t sz, u32 start, u32 end) {
    u32 str_len = neko_string_length(src);
    if (end > str_len) {
        end = str_len;
    }
    if (start > str_len) {
        start = str_len;
    }

    const char* at = src + start;
    const char* e = src + end;
    u32 ct = 0;
    while (at && *at != '\0' && at != e) {
        dst[ct] = *at;
        at++;
        ct++;
    }
}

neko_force_inline void neko_util_string_remove_character(const char* src, char* buffer, u32 buffer_size, char delimiter) {
    u32 ct = 0;
    u32 str_len = neko_string_length(src);
    const char* at = src;
    while (at && *at != '\0' && ct < buffer_size) {
        char c = *at;
        if (c != delimiter) {
            buffer[ct] = c;
            ct++;
        }
        at++;
    }
}

neko_force_inline void neko_util_string_replace(char* buffer, size_t buffer_sz, const char* replace, char fallback) {
    // Replace all characters with characters of keyword, then the rest replace with spaces
    size_t len = neko_string_length(replace);
    for (u32 c = 0; c < buffer_sz; ++c) {
        if (c < len) {
            buffer[c] = replace[c];
        } else {
            buffer[c] = fallback;
        }
    }
}

neko_force_inline void neko_util_string_replace_delim(const char* source_str, char* buffer, u32 buffer_size, char delimiter, char replace) {
    u32 str_len = neko_string_length(source_str);
    const char* at = source_str;
    while (at && *at != '\0') {
        char c = *at;
        if (c == delimiter) {
            c = replace;
        }
        buffer[(at - source_str)] = c;
        at++;
    }
}

neko_force_inline void neko_util_normalize_path(const char* path, char* buffer, u32 buffer_size) {
    // Normalize the path somehow...
}

#define neko_println(__FMT, ...)           \
    do {                                   \
        neko_printf(__FMT, ##__VA_ARGS__); \
        neko_printf("\n");                 \
    } while (0)

#ifdef NEKO_DEBUG
#define neko_println_debug(...) neko_println(__VA_ARGS__)
#else
#define neko_println_debug(...)
#endif

#ifndef neko_fprintf
neko_force_inline void neko_fprintf(FILE* fp, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);
}
#endif

neko_force_inline void neko_fprintln(FILE* fp, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);
    neko_fprintf(fp, "\n");
}

neko_force_inline void neko_fprintln_t(FILE* fp, u32 tabs, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    for (u32 i = 0; i < tabs; ++i) {
        neko_fprintf(fp, "\t");
    }
    vfprintf(fp, fmt, args);
    va_end(args);
    neko_fprintf(fp, "\n");
}

#ifdef __MINGW32__
#define neko_snprintf(__NAME, __SZ, __FMT, ...) __mingw_snprintf(__NAME, __SZ, __FMT, ##__VA_ARGS__)
#else
neko_force_inline void neko_snprintf(char* buffer, size_t buffer_size, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, buffer_size, fmt, args);
    va_end(args);
}
#endif

#define neko_transient_buffer(__N, __SZ) \
    char __N[__SZ] = neko_default_val(); \
    memset(__N, 0, __SZ);

#define neko_snprintfc(__NAME, __SZ, __FMT, ...) \
    char __NAME[__SZ] = neko_default_val();      \
    neko_snprintf(__NAME, __SZ, __FMT, ##__VA_ARGS__);

neko_force_inline u32 neko_util_safe_truncate_u64(u64 value) {
    neko_assert(value <= 0xFFFFFFFF);
    u32 result = (u32)value;
    return result;
}

neko_force_inline u32 neko_hash_u32(u32 x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

#define neko_hash_u32_ip(__X, __OUT)                 \
    do {                                             \
        __OUT = ((__X >> 16) ^ __X) * 0x45d9f3b;     \
        __OUT = ((__OUT >> 16) ^ __OUT) * 0x45d9f3b; \
        __OUT = (__OUT >> 16) ^ __OUT;               \
    } while (0)

neko_force_inline u32 neko_hash_u64(u64 x) {
    x = (x ^ (x >> 31) ^ (x >> 62)) * UINT64_C(0x319642b2d24d8ec3);
    x = (x ^ (x >> 27) ^ (x >> 54)) * UINT64_C(0x96de1b173f119089);
    x = x ^ (x >> 30) ^ (x >> 60);
    return (u32)x;
}

// Note: source: http://www.cse.yorku.ca/~oz/hash.html
// djb2 hash by dan bernstein
neko_force_inline u32 neko_hash_str(const char* str) {
    u32 hash = 5381;
    s32 c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

neko_force_inline u64 neko_hash_str64(const char* str) {
    u32 hash1 = 5381;
    u32 hash2 = 52711;
    u32 i = neko_string_length(str);
    while (i--) {
        char c = str[i];
        hash1 = (hash1 * 33) ^ c;
        hash2 = (hash2 * 33) ^ c;
    }

    return (hash1 >> 0) * 4096 + (hash2 >> 0);
}

neko_force_inline bool neko_compare_bytes(void* b0, void* b1, size_t len) { return 0 == memcmp(b0, b1, len); }

// Hash generic bytes using (ripped directly from Sean Barret's stb_ds.h)
#define NEKO_SIZE_T_BITS ((sizeof(size_t)) * 8)
#define NEKO_SIPHASH_C_ROUNDS 1
#define NEKO_SIPHASH_D_ROUNDS 1
#define neko_rotate_left(__V, __N) (((__V) << (__N)) | ((__V) >> (NEKO_SIZE_T_BITS - (__N))))
#define neko_rotate_right(__V, __N) (((__V) >> (__N)) | ((__V) << (NEKO_SIZE_T_BITS - (__N))))

neko_force_inline size_t neko_hash_siphash_bytes(void* p, size_t len, size_t seed) {
    unsigned char* d = (unsigned char*)p;
    size_t i, j;
    size_t v0, v1, v2, v3, data;

    // hash that works on 32- or 64-bit registers without knowing which we have
    // (computes different results on 32-bit and 64-bit platform)
    // derived from siphash, but on 32-bit platforms very different as it uses 4 32-bit state not 4 64-bit
    v0 = ((((size_t)0x736f6d65 << 16) << 16) + 0x70736575) ^ seed;
    v1 = ((((size_t)0x646f7261 << 16) << 16) + 0x6e646f6d) ^ ~seed;
    v2 = ((((size_t)0x6c796765 << 16) << 16) + 0x6e657261) ^ seed;
    v3 = ((((size_t)0x74656462 << 16) << 16) + 0x79746573) ^ ~seed;

#ifdef STBDS_TEST_SIPHASH_2_4
    // hardcoded with key material in the siphash test vectors
    v0 ^= 0x0706050403020100ull ^ seed;
    v1 ^= 0x0f0e0d0c0b0a0908ull ^ ~seed;
    v2 ^= 0x0706050403020100ull ^ seed;
    v3 ^= 0x0f0e0d0c0b0a0908ull ^ ~seed;
#endif

#define neko_sipround()                                  \
    do {                                                 \
        v0 += v1;                                        \
        v1 = neko_rotate_left(v1, 13);                   \
        v1 ^= v0;                                        \
        v0 = neko_rotate_left(v0, NEKO_SIZE_T_BITS / 2); \
        v2 += v3;                                        \
        v3 = neko_rotate_left(v3, 16);                   \
        v3 ^= v2;                                        \
        v2 += v1;                                        \
        v1 = neko_rotate_left(v1, 17);                   \
        v1 ^= v2;                                        \
        v2 = neko_rotate_left(v2, NEKO_SIZE_T_BITS / 2); \
        v0 += v3;                                        \
        v3 = neko_rotate_left(v3, 21);                   \
        v3 ^= v0;                                        \
    } while (0)

    for (i = 0; i + sizeof(size_t) <= len; i += sizeof(size_t), d += sizeof(size_t)) {
        data = d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
        data |= (size_t)(d[4] | (d[5] << 8) | (d[6] << 16) | (d[7] << 24)) << 16 << 16;  // discarded if size_t == 4

        v3 ^= data;
        for (j = 0; j < NEKO_SIPHASH_C_ROUNDS; ++j) neko_sipround();
        v0 ^= data;
    }
    data = len << (NEKO_SIZE_T_BITS - 8);
    switch (len - i) {
        case 7:
            data |= ((size_t)d[6] << 24) << 24;  // fall through
        case 6:
            data |= ((size_t)d[5] << 20) << 20;  // fall through
        case 5:
            data |= ((size_t)d[4] << 16) << 16;  // fall through
        case 4:
            data |= (d[3] << 24);  // fall through
        case 3:
            data |= (d[2] << 16);  // fall through
        case 2:
            data |= (d[1] << 8);  // fall through
        case 1:
            data |= d[0];  // fall through
        case 0:
            break;
    }
    v3 ^= data;
    for (j = 0; j < NEKO_SIPHASH_C_ROUNDS; ++j) neko_sipround();
    v0 ^= data;
    v2 ^= 0xff;
    for (j = 0; j < NEKO_SIPHASH_D_ROUNDS; ++j) neko_sipround();

#if 0
  return v0^v1^v2^v3;
#else
    return v1 ^ v2 ^ v3;  // slightly stronger since v0^v3 in above cancels out final round operation? I tweeted at the authors of SipHash about this but they didn't reply
#endif
}

neko_force_inline size_t neko_hash_bytes(void* p, size_t len, size_t seed) {
#if 0
  return neko_hash_siphash_bytes(p,len,seed);
#else
    unsigned char* d = (unsigned char*)p;

    // Len == 4 (off for now, so to force 64 bit hash)
    if (len == 4) {
        unsigned int hash = d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
        hash ^= seed;
        hash *= 0xcc9e2d51;
        hash = (hash << 17) | (hash >> 15);
        hash *= 0x1b873593;
        hash ^= seed;
        hash = (hash << 19) | (hash >> 13);
        hash = hash * 5 + 0xe6546b64;
        hash ^= hash >> 16;
        hash *= 0x85ebca6b;
        hash ^= seed;
        hash ^= hash >> 13;
        hash *= 0xc2b2ae35;
        hash ^= hash >> 16;
        return (((size_t)hash << 16 << 16) | hash) ^ seed;
    } else if (len == 8 && sizeof(size_t) == 8) {
        size_t hash = d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
        hash |= (size_t)(d[4] | (d[5] << 8) | (d[6] << 16) | (d[7] << 24)) << 16 << 16;  // avoid warning if size_t == 4
        hash ^= seed;
        hash = (~hash) + (hash << 21);
        hash ^= neko_rotate_right(hash, 24);
        hash *= 265;
        hash ^= neko_rotate_right(hash, 14);
        hash ^= seed;
        hash *= 21;
        hash ^= neko_rotate_right(hash, 28);
        hash += (hash << 31);
        hash = (~hash) + (hash << 18);
        return hash;
    } else {
        return neko_hash_siphash_bytes(p, len, seed);
    }
#endif
}

/* Resource Loading Util */
NEKO_API_DECL bool32_t neko_util_load_texture_data_from_file(const char* file_path, s32* width, s32* height, u32* num_comps, void** data, bool32_t flip_vertically_on_load);
NEKO_API_DECL bool32_t neko_util_load_texture_data_from_memory(const void* memory, size_t sz, s32* width, s32* height, u32* num_comps, void** data, bool32_t flip_vertically_on_load);

neko_static_inline u16 neko_read_LE16(const u8* data) { return (u16)((data[1] << 8) | data[0]); }

neko_static_inline u32 neko_read_LE32(const u8* data) { return (((u32)data[3] << 24) | ((u32)data[2] << 16) | ((u32)data[1] << 8) | (u32)data[0]); }

/*========================
// NEKO_MEMORY
========================*/

#define neko_ptr_add(P, BYTES) (((u8*)P + (BYTES)))

typedef struct neko_memory_block_t {
    u8* data;
    size_t size;
} neko_memory_block_t;

NEKO_API_DECL neko_memory_block_t neko_memory_block_new(size_t sz);
NEKO_API_DECL void neko_memory_block_free(neko_memory_block_t* mem);
NEKO_API_DECL size_t neko_memory_calc_padding(size_t base_address, size_t alignment);
NEKO_API_DECL size_t neko_memory_calc_padding_w_header(size_t base_address, size_t alignment, size_t header_sz);

/*================================================================================
// Linear Allocator
================================================================================*/

typedef struct neko_linear_allocator_t {
    u8* memory;
    size_t total_size;
    size_t offset;
} neko_linear_allocator_t;

NEKO_API_DECL neko_linear_allocator_t neko_linear_allocator_new(size_t sz);
NEKO_API_DECL void neko_linear_allocator_free(neko_linear_allocator_t* la);
NEKO_API_DECL void* neko_linear_allocator_allocate(neko_linear_allocator_t* la, size_t sz, size_t alignment);
NEKO_API_DECL void neko_linear_allocator_clear(neko_linear_allocator_t* la);

/*================================================================================
// Stack Allocator
================================================================================*/

typedef struct neko_stack_allocator_header_t {
    u32 size;
} neko_stack_allocator_header_t;

typedef struct neko_stack_allocator_t {
    neko_memory_block_t memory;
    size_t offset;
} neko_stack_allocator_t;

NEKO_API_DECL neko_stack_allocator_t neko_stack_allocator_new(size_t sz);
NEKO_API_DECL void neko_stack_allocator_free(neko_stack_allocator_t* sa);
NEKO_API_DECL void* neko_stack_allocator_allocate(neko_stack_allocator_t* sa, size_t sz);
NEKO_API_DECL void* neko_stack_allocator_peek(neko_stack_allocator_t* sa);
NEKO_API_DECL void* neko_stack_allocator_pop(neko_stack_allocator_t* sa);
NEKO_API_DECL void neko_stack_allocator_clear(neko_stack_allocator_t* sa);

/*================================================================================
// Heap Allocator
================================================================================*/

#ifndef NEKO_HEAP_ALLOC_DEFAULT_SIZE
#define NEKO_HEAP_ALLOC_DEFAULT_SIZE 1024 * 1024 * 20
#endif

#ifndef NEKO_HEAP_ALLOC_DEFAULT_CAPCITY
#define NEKO_HEAP_ALLOC_DEFAULT_CAPCITY 1024
#endif

typedef struct neko_heap_allocator_header_t {
    struct neko_heap_allocator_header_t* next;
    struct neko_heap_allocator_header_t* prev;
    size_t size;
} neko_heap_allocator_header_t;

typedef struct neko_heap_allocator_free_block_t {
    neko_heap_allocator_header_t* header;
    size_t size;
} neko_heap_allocator_free_block_t;

typedef struct neko_heap_allocator_t {
    neko_heap_allocator_header_t* memory;
    neko_heap_allocator_free_block_t* free_blocks;
    u32 free_block_count;
    u32 free_block_capacity;
} neko_heap_allocator_t;

NEKO_API_DECL neko_heap_allocator_t neko_heap_allocate_new();
NEKO_API_DECL void neko_heap_allocator_free(neko_heap_allocator_t* ha);
NEKO_API_DECL void* neko_heap_allocator_allocate(neko_heap_allocator_t* ha, size_t sz);
NEKO_API_DECL void neko_heap_allocator_deallocate(neko_heap_allocator_t* ha, void* memory);

/*================================================================================
// Pool Allocator
================================================================================*/

/*================================================================================
// Paged Allocator
================================================================================*/

typedef struct neko_paged_allocator_block_t {
    struct neko_paged_allocator_block_t* next;
} neko_paged_allocator_block_t;

typedef struct neko_paged_allocator_page_t {
    struct neko_paged_allocator_page_t* next;
    struct neko_paged_allocator_block_t* data;
} neko_paged_allocator_page_t;

typedef struct neko_paged_allocator_t {
    u32 block_size;
    u32 blocks_per_page;
    neko_paged_allocator_page_t* pages;
    u32 page_count;
    neko_paged_allocator_block_t* free_list;
} neko_paged_allocator_t;

NEKO_API_DECL neko_paged_allocator_t neko_paged_allocator_new(size_t element_size, size_t elements_per_page);
NEKO_API_DECL void neko_paged_allocator_free(neko_paged_allocator_t* pa);
NEKO_API_DECL void* neko_paged_allocator_allocate(neko_paged_allocator_t* pa);
NEKO_API_DECL void neko_paged_allocator_deallocate(neko_paged_allocator_t* pa, void* data);
NEKO_API_DECL void neko_paged_allocator_clear(neko_paged_allocator_t* pa);

#include "neko_math.h"

neko_inline const_str neko_fs_get_filename(const_str path) {
    int len = strlen(path);
    int flag = 0;

    for (int i = len - 1; i > 0; i--) {
        if (path[i] == '\\' || path[i] == '//' || path[i] == '/') {
            flag = 1;
            path = path + i + 1;
            break;
        }
    }
    return path;
}

/*================================================================================
// Random
================================================================================*/

#define NEKO_STATE_VECTOR_LENGTH 624
#define NEKO_STATE_VECTOR_M 397 /* changes to STATE_VECTOR_LENGTH also require changes to this */

typedef struct neko_mt_rand_t {
    u64 mt[NEKO_STATE_VECTOR_LENGTH];
    int32_t index;
} neko_mt_rand_t;

NEKO_API_DECL neko_mt_rand_t neko_rand_seed(uint64_t seed);
NEKO_API_DECL uint64_t neko_rand_gen_long(neko_mt_rand_t* rand);
NEKO_API_DECL double neko_rand_gen(neko_mt_rand_t* rand);
NEKO_API_DECL double neko_rand_gen_range(neko_mt_rand_t* rand, double min, double max);
NEKO_API_DECL uint64_t neko_rand_gen_range_long(neko_mt_rand_t* rand, int32_t min, int32_t max);
NEKO_API_DECL neko_color_t neko_rand_gen_color(neko_mt_rand_t* rand);

// xorshf32随机数算法
// http://www.iro.umontreal.ca/~lecuyer/myftp/papers/xorshift.pdf
// xyzw -> [0, 2^32 - 1]

neko_inline u32 neko_rand_xorshf32(void) {

    u32 __neko_rand_xorshf32_x = time(NULL), __neko_rand_xorshf32_y = time(NULL), __neko_rand_xorshf32_z = time(NULL), __neko_rand_xorshf32_w = time(NULL);

    // period 2^128 - 1
    u32 tmp = (__neko_rand_xorshf32_x ^ (__neko_rand_xorshf32_x << 15));
    __neko_rand_xorshf32_x = __neko_rand_xorshf32_y;
    __neko_rand_xorshf32_y = __neko_rand_xorshf32_z;
    __neko_rand_xorshf32_z = __neko_rand_xorshf32_w;

    __neko_rand_xorshf32_w = (__neko_rand_xorshf32_w ^ (__neko_rand_xorshf32_w >> 21) ^ (tmp ^ (tmp >> 4)));
    return __neko_rand_xorshf32_w;
}

#define neko_rand_xorshf32_max 0xFFFFFFFF

// Random number in range [-1,1]
neko_inline f32 neko_rand_xor() {
    f32 r = (f32)neko_rand_xorshf32();
    r /= neko_rand_xorshf32_max;
    r = 2.0f * r - 1.0f;
    return r;
}

neko_inline f32 neko_rand_range_xor(f32 lo, f32 hi) {
    f32 r = (f32)neko_rand_xorshf32();
    r /= neko_rand_xorshf32_max;
    r = (hi - lo) * r + lo;
    return r;
}

/*================================================================================
// Camera
================================================================================*/

typedef enum neko_projection_type { NEKO_PROJECTION_TYPE_ORTHOGRAPHIC, NEKO_PROJECTION_TYPE_PERSPECTIVE } neko_projection_type;

typedef struct neko_camera_t {
    neko_vqs transform;
    f32 fov;
    f32 aspect_ratio;
    f32 near_plane;
    f32 far_plane;
    f32 ortho_scale;
    neko_projection_type proj_type;
} neko_camera_t;

NEKO_API_DECL neko_camera_t neko_camera_default();
NEKO_API_DECL neko_camera_t neko_camera_perspective();
NEKO_API_DECL neko_mat4 neko_camera_get_view(const neko_camera_t* cam);
NEKO_API_DECL neko_mat4 neko_camera_get_proj(const neko_camera_t* cam, s32 view_width, s32 view_height);
NEKO_API_DECL neko_mat4 neko_camera_get_view_projection(const neko_camera_t* cam, s32 view_width, s32 view_height);
NEKO_API_DECL neko_vec3 neko_camera_forward(const neko_camera_t* cam);
NEKO_API_DECL neko_vec3 neko_camera_backward(const neko_camera_t* cam);
NEKO_API_DECL neko_vec3 neko_camera_up(const neko_camera_t* cam);
NEKO_API_DECL neko_vec3 neko_camera_down(const neko_camera_t* cam);
NEKO_API_DECL neko_vec3 neko_camera_right(const neko_camera_t* cam);
NEKO_API_DECL neko_vec3 neko_camera_left(const neko_camera_t* cam);
NEKO_API_DECL neko_vec3 neko_camera_screen_to_world(const neko_camera_t* cam, neko_vec3 coords, s32 view_x, s32 view_y, s32 view_width, s32 view_height);
NEKO_API_DECL neko_vec3 neko_camera_world_to_screen(const neko_camera_t* cam, neko_vec3 coords, s32 view_width, s32 view_height);
NEKO_API_DECL void neko_camera_offset_orientation(neko_camera_t* cam, f32 yaw, f32 picth);

/*=============================
// Containers
=============================*/

#include "neko_containers.inl"

/*================================================================================
// Utils
================================================================================*/

neko_inline u32 neko_abs(s32 v) {
    unsigned int r;
    int const mask = v >> sizeof(int) * CHAR_BIT - 1;
    r = (v + mask) ^ mask;
    return r;
}

// 将UTF-8编码字符转换为Unicode
neko_inline u32 neko_utf8_to_unicode(const_str utf8, s32* bytes_read) {
    u32 unicode = 0;
    s32 len = 0;
    unsigned char utf8char = utf8[0];

    if ((utf8char & 0x80) == 0) {
        unicode = utf8char;
        len = 1;
    } else if ((utf8char & 0xE0) == 0xC0) {
        unicode = utf8char & 0x1F;
        len = 2;
    } else if ((utf8char & 0xF0) == 0xE0) {
        unicode = utf8char & 0x0F;
        len = 3;
    } else {
        // 无效的 UTF-8 序列
        len = 1;
    }

    for (s32 i = 1; i < len; i++) {
        utf8char = utf8[i];
        if ((utf8char & 0xC0) != 0x80) {
            // 无效的 UTF-8 序列
            len = 1;
            break;
        }
        unicode = (unicode << 6) | (utf8char & 0x3F);
    }

    *bytes_read = len;
    return unicode;
}

// AABBs
/*
    min is top left of rect,
    max is bottom right
*/
/*
typedef struct neko_aabb_t
{
    neko_vec2 min;
    neko_vec2 max;
} neko_aabb_t;

// Collision Resolution: Minimum Translation Vector
neko_force_inline
neko_vec2 neko_aabb_aabb_mtv(neko_aabb_t* a0, neko_aabb_t* a1)
{
    neko_vec2 diff = neko_v2(a0->min.x - a1->min.x, a0->min.y - a1->min.y);

    f32 l, r, b, t;
    neko_vec2 mtv = neko_v2(0.f, 0.f);

    l = a1->min.x - a0->max.x;
    r = a1->max.x - a0->min.x;
    b = a1->min.y - a0->max.y;
    t = a1->max.y - a0->min.y;

    mtv.x = fabsf(l) > r ? r : l;
    mtv.y = fabsf(b) > t ? t : b;

    if ( fabsf(mtv.x) <= fabsf(mtv.y)) {
        mtv.y = 0.f;
    } else {
        mtv.x = 0.f;
    }

    return mtv;
}

// 2D AABB collision detection (rect. vs. rect.)
neko_force_inline
b32 neko_aabb_vs_aabb(neko_aabb_t* a, neko_aabb_t* b)
{
    if (a->max.x > b->min.x &&
         a->max.y > b->min.y &&
         a->min.x < b->max.x &&
         a->min.y < b->max.y)
    {
        return true;
    }

    return false;
}

neko_force_inline
neko_vec4 neko_aabb_window_coords(neko_aabb_t* aabb, neko_camera_t* camera, neko_vec2 window_size)
{
    // AABB of the player
    neko_vec4 bounds = neko_default_val();
    neko_vec4 tl = neko_v4(aabb->min.x, aabb->min.y, 0.f, 1.f);
    neko_vec4 br = neko_v4(aabb->max.x, aabb->max.y, 0.f, 1.f);

    neko_mat4 view_mtx = neko_camera_get_view(camera);
    neko_mat4 proj_mtx = neko_camera_get_proj(camera, (s32)window_size.x, (s32)window_size.y);
    neko_mat4 vp = neko_mat4_mul(proj_mtx, view_mtx);

    // Transform verts
    tl = neko_mat4_mul_vec4(vp, tl);
    br = neko_mat4_mul_vec4(vp, br);

    // Perspective divide
    tl = neko_vec4_scale(tl, 1.f / tl.w);
    br = neko_vec4_scale(br, 1.f / br.w);

    // NDC [0.f, 1.f] and NDC
    tl.x = (tl.x * 0.5f + 0.5f);
    tl.y = (tl.y * 0.5f + 0.5f);
    br.x = (br.x * 0.5f + 0.5f);
    br.y = (br.y * 0.5f + 0.5f);

    // Window Space
    tl.x = tl.x * window_size.x;
    tl.y = neko_map_range(1.f, 0.f, 0.f, 1.f, tl.y) * window_size.y;
    br.x = br.x * window_size.x;
    br.y = neko_map_range(1.f, 0.f, 0.f, 1.f, br.y) * window_size.y;

    bounds = neko_v4(tl.x, tl.y, br.x, br.y);

    return bounds;
}
*/

/** @} */  // end of neko_math

/*========================
// NEKO_LEXER
========================*/

//==== [ Token ] ============================================================//

typedef enum neko_token_type {
    NEKO_TOKEN_UNKNOWN = 0x00,
    NEKO_TOKEN_LPAREN,
    NEKO_TOKEN_RPAREN,
    NEKO_TOKEN_LTHAN,
    NEKO_TOKEN_GTHAN,
    NEKO_TOKEN_SEMICOLON,
    NEKO_TOKEN_COLON,
    NEKO_TOKEN_COMMA,
    NEKO_TOKEN_EQUAL,
    NEKO_TOKEN_NOT,
    NEKO_TOKEN_HASH,
    NEKO_TOKEN_PIPE,
    NEKO_TOKEN_AMPERSAND,
    NEKO_TOKEN_LBRACE,
    NEKO_TOKEN_RBRACE,
    NEKO_TOKEN_LBRACKET,
    NEKO_TOKEN_RBRACKET,
    NEKO_TOKEN_MINUS,
    NEKO_TOKEN_PLUS,
    NEKO_TOKEN_ASTERISK,
    NEKO_TOKEN_BSLASH,
    NEKO_TOKEN_FSLASH,
    NEKO_TOKEN_QMARK,
    NEKO_TOKEN_SPACE,
    NEKO_TOKEN_PERCENT,
    NEKO_TOKEN_DOLLAR,
    NEKO_TOKEN_NEWLINE,
    NEKO_TOKEN_TAB,
    NEKO_TOKEN_UNDERSCORE,
    NEKO_TOKEN_SINGLE_LINE_COMMENT,
    NEKO_TOKEN_MULTI_LINE_COMMENT,
    NEKO_TOKEN_IDENTIFIER,
    NEKO_TOKEN_SINGLE_QUOTE,
    NEKO_TOKEN_DOUBLE_QUOTE,
    NEKO_TOKEN_STRING,
    NEKO_TOKEN_PERIOD,
    NEKO_TOKEN_NUMBER
} neko_token_type;

typedef struct neko_token_t {
    const char* text;
    neko_token_type type;
    u32 len;
} neko_token_t;

NEKO_API_DECL neko_token_t neko_token_invalid_token();
NEKO_API_DECL bool neko_token_compare_type(const neko_token_t* t, neko_token_type type);
NEKO_API_DECL bool neko_token_compare_text(const neko_token_t* t, const char* match);
NEKO_API_DECL void neko_token_print_text(const neko_token_t* t);
NEKO_API_DECL void neko_token_debug_print(const neko_token_t* t);
NEKO_API_DECL const char* neko_token_type_to_str(neko_token_type type);
NEKO_API_DECL bool neko_char_is_end_of_line(char c);
NEKO_API_DECL bool neko_char_is_white_space(char c);
NEKO_API_DECL bool neko_char_is_alpha(char c);
NEKO_API_DECL bool neko_char_is_numeric(char c);

neko_inline b8 neko_token_is_end_of_line(char c) { return (c == '\n' || c == '\r'); }
neko_inline b8 neko_token_char_is_white_space(char c) { return (c == '\t' || c == ' ' || neko_token_is_end_of_line(c)); }
neko_inline b8 neko_token_char_is_alpha(char c) { return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')); }
neko_inline b8 neko_token_char_is_numeric(char c) { return (c >= '0' && c <= '9'); }

//==== [ Lexer ] ============================================================//

typedef struct neko_lexer_t {
    const char* at;
    const char* contents;
    neko_token_t current_token;
    bool (*can_lex)(struct neko_lexer_t* lex);
    void (*eat_white_space)(struct neko_lexer_t* lex);
    neko_token_t (*next_token)(struct neko_lexer_t*);
    b32 skip_white_space;
    size_t size;           // Optional
    size_t contents_size;  // Optional
} neko_lexer_t;

NEKO_API_DECL void neko_lexer_set_contents(neko_lexer_t* lex, const char* contents);
NEKO_API_DECL neko_token_t neko_lexer_next_token(neko_lexer_t* lex);
NEKO_API_DECL bool neko_lexer_can_lex(neko_lexer_t* lex);
NEKO_API_DECL neko_token_t neko_lexer_current_token(const neko_lexer_t* lex);
NEKO_API_DECL bool neko_lexer_require_token_text(neko_lexer_t* lex, const char* match);
NEKO_API_DECL bool neko_lexer_require_token_type(neko_lexer_t* lex, neko_token_type type);
NEKO_API_DECL bool neko_lexer_current_token_compare_type(const neko_lexer_t* lex, neko_token_type type);
NEKO_API_DECL neko_token_t neko_lexer_peek(neko_lexer_t* lex);
NEKO_API_DECL bool neko_lexer_find_next_token_type(neko_lexer_t* lex, neko_token_type type);
NEKO_API_DECL neko_token_t neko_lexer_advance_before_next_token_type(neko_lexer_t* lex, neko_token_type type);

// C specific functions for lexing
NEKO_API_DECL neko_lexer_t neko_lexer_c_ctor(const char* contents);
NEKO_API_DECL bool neko_lexer_c_can_lex(neko_lexer_t* lex);
NEKO_API_DECL void neko_lexer_c_eat_white_space(neko_lexer_t* lex);
NEKO_API_DECL neko_token_t neko_lexer_c_next_token(neko_lexer_t* lex);
NEKO_API_DECL void neko_lexer_set_token(neko_lexer_t* lex, neko_token_t token);

neko_inline u32 neko_darken_color(u32 color, f32 brightness) {
    s32 a = (color >> 24) & 0xFF;
    s32 r = (s32)(((color >> 16) & 0xFF) * brightness);
    s32 g = (s32)(((color >> 8) & 0xFF) * brightness);
    s32 b = (s32)((color & 0xFF) * brightness);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

/*=============================
// Console
=============================*/

typedef void (*neko_console_func)(int argc, char** argv);

typedef struct neko_console_command_s {
    neko_console_func func;
    const char* name;
    const char* desc;
} neko_console_command_t;

typedef struct neko_console_s {
    char tb[2048];     // text buffer
    char cb[10][256];  // "command" buffer
    int current_cb_idx;

    f32 y;
    f32 size;
    f32 open_speed;
    f32 close_speed;

    bool open;
    int last_open_state;
    bool autoscroll;

    neko_console_command_t* commands;
    int commands_len;
} neko_console_t;

/*=============================
// CVar
=============================*/

enum cmd_type { CVAR_VAR = 0, CVAR_FUNC = 1 };

void __neko_engine_cvar_init();

#define __NEKO_CVAR_STR_LEN 64

typedef enum neko_cvar_type {
    __NEKO_CONFIG_TYPE_INT,
    __NEKO_CONFIG_TYPE_FLOAT,
    __NEKO_CONFIG_TYPE_STRING,
    __NEKO_CONFIG_TYPE_COUNT,
} neko_cvar_type;

typedef struct neko_cvar_t {
    char name[__NEKO_CVAR_STR_LEN];  // 直接开辟好空间
    neko_cvar_type type;             // cvar 类型
    union {                          // 联合体存值
        f32 f;
        s32 i;
        char* s;
    } value;
} neko_cvar_t;

typedef struct neko_config_t {
    neko_dyn_array(neko_cvar_t) cvars;
} neko_config_t;

NEKO_API_DECL void __neko_config_init();
NEKO_API_DECL void __neko_config_free();

NEKO_API_DECL neko_cvar_t* __neko_config_get(const_str name);
NEKO_API_DECL void neko_config_print();

#define neko_cvar_new(n, t, v)                                                \
    do {                                                                      \
        neko_cvar_t cvar = {.name = n, .type = t, .value = 0};                \
        if (t == __NEKO_CONFIG_TYPE_INT) {                                    \
            cvar.value.i = v;                                                 \
        } else if (t == __NEKO_CONFIG_TYPE_FLOAT) {                           \
            cvar.value.f = v;                                                 \
        } else {                                                              \
            neko_assert(false);                                               \
        }                                                                     \
        neko_dyn_array_push((neko_instance()->ctx.game.config)->cvars, cvar); \
    } while (0)

#define neko_cvar_lnew(n, t, v)                                               \
    do {                                                                      \
        neko_cvar_t cvar = {.type = t, .value = 0};                           \
        strncpy(cvar.name, n, sizeof(cvar.name) - 1);                         \
        cvar.name[sizeof(cvar.name) - 1] = '\0';                              \
        if (t == __NEKO_CONFIG_TYPE_INT) {                                    \
            cvar.value.i = v;                                                 \
        } else if (t == __NEKO_CONFIG_TYPE_FLOAT) {                           \
            cvar.value.f = v;                                                 \
        } else {                                                              \
            neko_assert(false);                                               \
        }                                                                     \
        neko_dyn_array_push((neko_instance()->ctx.game.config)->cvars, cvar); \
    } while (0)

// 由于 float -> char* 转换 很难将其写成单个宏
#define neko_cvar_new_str(n, t, v)                                                         \
    do {                                                                                   \
        neko_cvar_t cvar = {.name = n, .type = t, .value = {0}};                           \
        cvar.value.s = (char*)neko_malloc(__NEKO_CVAR_STR_LEN);                            \
        memset(cvar.value.s, 0, __NEKO_CVAR_STR_LEN);                                      \
        memcpy(cvar.value.s, v, neko_min(__NEKO_CVAR_STR_LEN - 1, neko_string_length(v))); \
        neko_dyn_array_push((neko_instance()->ctx.game.config)->cvars, cvar);              \
    } while (0)

#define neko_cvar_lnew_str(n, t, v)                                                        \
    do {                                                                                   \
        neko_cvar_t cvar = {.type = t, .value = {0}};                                      \
        strncpy(cvar.name, n, sizeof(cvar.name) - 1);                                      \
        cvar.name[sizeof(cvar.name) - 1] = '\0';                                           \
        cvar.value.s = (char*)neko_malloc(__NEKO_CVAR_STR_LEN);                            \
        memset(cvar.value.s, 0, __NEKO_CVAR_STR_LEN);                                      \
        memcpy(cvar.value.s, v, neko_min(__NEKO_CVAR_STR_LEN - 1, neko_string_length(v))); \
        neko_dyn_array_push((neko_instance()->ctx.game.config)->cvars, cvar);              \
    } while (0)

#define neko_cvar(n) __neko_config_get(n)

#define neko_cvar_print(cvar)                                           \
    do {                                                                \
        switch ((cvar)->type) {                                         \
            default:                                                    \
            case __NEKO_CONFIG_TYPE_STRING:                             \
                neko_println("%s = %s", (cvar)->name, (cvar)->value.s); \
                break;                                                  \
                                                                        \
            case __NEKO_CONFIG_TYPE_FLOAT:                              \
                neko_println("%s = %f", (cvar)->name, (cvar)->value.f); \
                break;                                                  \
                                                                        \
            case __NEKO_CONFIG_TYPE_INT:                                \
                neko_println("%s = %d", (cvar)->name, (cvar)->value.i); \
                break;                                                  \
        };                                                              \
    } while (0)

#define neko_cvar_set(cvar, str)                                           \
    do {                                                                   \
        switch ((cvar)->type) {                                            \
            default:                                                       \
            case __NEKO_CONFIG_TYPE_STRING:                                \
                memcpy((cvar)->value.s, str, neko_string_length(str) + 1); \
                break;                                                     \
                                                                           \
            case __NEKO_CONFIG_TYPE_FLOAT:                                 \
                (cvar)->value.f = atof(str);                               \
                break;                                                     \
                                                                           \
            case __NEKO_CONFIG_TYPE_INT:                                   \
                (cvar)->value.i = atoi(str);                               \
                break;                                                     \
        };                                                                 \
    } while (0)

/*===================================
// Profiler Internal
===================================*/

NEKO_API_DECL void neko_profiler_init();
NEKO_API_DECL void neko_profiler_shutdown();
NEKO_API_DECL void neko_profiler_register_thread(const_str _name, u64 _threadID);
NEKO_API_DECL void neko_profiler_unregister_thread(u64 _threadID);
NEKO_API_DECL void neko_profiler_begin_frame();
NEKO_API_DECL uintptr_t neko_profiler_begin_scope(const_str _file, s32 _line, const_str _name);
NEKO_API_DECL void neko_profiler_end_scope(uintptr_t _scopeHandle);

#endif  // NEKO_H
