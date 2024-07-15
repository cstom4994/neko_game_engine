#pragma once

#include <assert.h>
#include <ctype.h>
#include <float.h>
#include <limits.h>
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

/*===================
// NEKO_API_DECL
===================*/

#ifdef __cplusplus
#define NEKO_API_EXTERN extern "C"
#else
#define NEKO_API_EXTERN extern
#endif

#define NEKO_API_DECL NEKO_API_EXTERN
#define NEKO_API_PRIVATE NEKO_API_EXTERN

#define NEKO_INLINE inline
#define NEKO_STATIC static
#define NEKO_STATIC_INLINE static inline

#if (defined _WIN32 || defined _WIN64)
#define NEKO_FORCE_INLINE NEKO_INLINE
#elif (defined __APPLE__ || defined _APPLE)
#define NEKO_FORCE_INLINE static __attribute__((always_inline))
#else
#define NEKO_FORCE_INLINE static NEKO_INLINE
#endif

#define neko_little_endian 1
#define neko_bit(x) (1 << x)

/*============================================================
// C primitive types
============================================================*/

typedef size_t usize;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;
typedef float f32;
typedef double f64;
typedef const char *const_str;
typedef intptr_t iptr;
typedef uintptr_t uptr;

#define u16_max UINT16_MAX
#define u32_max UINT32_MAX
#define u64_max UINT64_MAX
#define s32_max INT32_MAX
#define f32_max FLT_MAX
#define f32_min FLT_MIN

#define NEKO_ARR_SIZE(__ARR) sizeof(__ARR) / sizeof(__ARR[0])

#define NEKO_ASSERT(x, ...)                                                                                            \
    do {                                                                                                               \
        if (!(x)) {                                                                                                    \
            neko_printf("assertion failed: (%s), function %s, file %s, line %d.\n", #x, __func__, __FILE__, __LINE__); \
        }                                                                                                              \
    } while (0)

#if defined(__cplusplus)
#define NEKO_DEFAULT_VAL() \
    {}
#else
#define NEKO_DEFAULT_VAL() \
    { 0 }
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

#define NEKO_TIMED_ACTION(INTERVAL, ...)                               \
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

#define NEKO_EXPECT(x)                                               \
    do {                                                             \
        if (!(x)) {                                                  \
            NEKO_ERROR("Unexpect error: assertion '%s' failed", #x); \
            abort();                                                 \
        }                                                            \
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
NEKO_API_DECL void neko_printf(const char *fmt, ...);
#endif
#endif

// Logging
#define NEKO_ERROR(...) neko_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define NEKO_DEBUG_LOG(...) neko_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define NEKO_INFO(...) neko_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define NEKO_TRACE(...) neko_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define NEKO_WARN(...) neko_log(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)

#define NEKO_VA_UNPACK(...) __VA_ARGS__  // 用于解包括号 带逗号的宏参数需要它

#if defined(__clang__)
#define NEKO_DEBUGBREAK() __builtin_debugtrap()
#elif defined(__GNUC__)
#define NEKO_DEBUGBREAK() __builtin_trap()
#elif defined(_MSC_VER)
#define NEKO_DEBUGBREAK() __debugbreak()
#else
#define NEKO_DEBUGBREAK()
#endif

#ifdef __clang__
#define FORMAT_ARGS(n) __attribute__((format(printf, n, n + 1)))
#else
#define FORMAT_ARGS(n)
#endif

FORMAT_ARGS(1)
inline void panic(const char *fmt, ...) {
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

struct String {
    char *data = nullptr;
    u64 len = 0;

    String() = default;
    String(const char *cstr) : data((char *)cstr), len(strlen(cstr)) {}
    String(const char *cstr, u64 n) : data((char *)cstr), len(n) {}

    bool is_cstr();
    String substr(u64 i, u64 j);
    bool starts_with(String match);
    bool ends_with(String match);
    u64 first_of(char c);
    u64 last_of(char c);

    char *begin() { return data; }
    char *end() { return &data[len]; }
};

String to_cstr(String str);

constexpr u64 fnv1a(const char *str, u64 len) {
    u64 hash = 14695981039346656037u;
    for (u64 i = 0; i < len; i++) {
        hash ^= (u8)str[i];
        hash *= 1099511628211;
    }
    return hash;
}

inline u64 fnv1a(String str) { return fnv1a(str.data, str.len); }

constexpr u64 operator"" _hash(const char *str, size_t len) { return fnv1a(str, len); }

inline bool operator==(String lhs, String rhs) {
    if (lhs.len != rhs.len) {
        return false;
    }
    return memcmp(lhs.data, rhs.data, lhs.len) == 0;
}

inline bool operator!=(String lhs, String rhs) { return !(lhs == rhs); }

// Profile

void profile_setup();
void profile_fini();

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
    s32 tid;

    Instrument(const char *cat, const char *name);
    ~Instrument();
};

#if defined(NEKO_PROFILE_ENABLE)
#define PROFILE_FUNC() auto NEKO_CONCAT(__gen_profile_, __COUNTER__) = ::neko::Instrument("function", __func__);
#define PROFILE_BLOCK(name) auto NEKO_CONCAT(__gen_profile_, __COUNTER__) = ::neko::Instrument("block", name);
#else
#define PROFILE_FUNC()
#define PROFILE_BLOCK(name)
#endif

/*===================================
// Dynamic Array
===================================*/

/** @defgroup neko_dyn_array Dynamic Array
 *  @ingroup neko_containers
 *  Dynamic Array
 */

/** @addtogroup neko_dyn_array
 */
typedef struct neko_dyn_array {
    int32_t size;
    int32_t capacity;
} neko_dyn_array;

#define neko_dyn_array_head(__ARR) ((neko_dyn_array *)((u8 *)(__ARR) - sizeof(neko_dyn_array)))

#define neko_dyn_array_size(__ARR) (__ARR == NULL ? 0 : neko_dyn_array_head((__ARR))->size)

#define neko_dyn_array_capacity(__ARR) (__ARR == NULL ? 0 : neko_dyn_array_head((__ARR))->capacity)

#define neko_dyn_array_full(__ARR) ((neko_dyn_array_size((__ARR)) == neko_dyn_array_capacity((__ARR))))

#define neko_dyn_array_byte_size(__ARR) (neko_dyn_array_size((__ARR)) * sizeof(*__ARR))

void *neko_dyn_array_resize_impl(void *arr, size_t sz, size_t amount);

#define neko_dyn_array_need_grow(__ARR, __N) ((__ARR) == 0 || neko_dyn_array_size(__ARR) + (__N) >= neko_dyn_array_capacity(__ARR))

#define neko_dyn_array_grow(__ARR) neko_dyn_array_resize_impl((__ARR), sizeof(*(__ARR)), neko_dyn_array_capacity(__ARR) ? neko_dyn_array_capacity(__ARR) * 2 : 1)

#define neko_dyn_array_grow_size(__ARR, __SZ) neko_dyn_array_resize_impl((__ARR), (__SZ), neko_dyn_array_capacity(__ARR) ? neko_dyn_array_capacity(__ARR) * 2 : 1)

void **neko_dyn_array_init(void **arr, size_t val_len);

void neko_dyn_array_push_data(void **arr, void *val, size_t val_len);

NEKO_FORCE_INLINE void neko_dyn_array_set_data_i(void **arr, void *val, size_t val_len, u32 offset) { memcpy(((char *)(*arr)) + offset * val_len, val, val_len); }

#define neko_dyn_array_push(__ARR, __ARRVAL)                               \
    do {                                                                   \
        neko_dyn_array_init((void **)&(__ARR), sizeof(*(__ARR)));          \
        if (!(__ARR) || ((__ARR) && neko_dyn_array_need_grow(__ARR, 1))) { \
            *((void **)&(__ARR)) = neko_dyn_array_grow(__ARR);             \
        }                                                                  \
        (__ARR)[neko_dyn_array_size(__ARR)] = (__ARRVAL);                  \
        neko_dyn_array_head(__ARR)->size++;                                \
    } while (0)

#define neko_dyn_array_reserve(__ARR, __AMOUNT)                                                 \
    do {                                                                                        \
        if ((!__ARR)) neko_dyn_array_init((void **)&(__ARR), sizeof(*(__ARR)));                 \
        if ((!__ARR) || (size_t)__AMOUNT > neko_dyn_array_capacity(__ARR)) {                    \
            *((void **)&(__ARR)) = neko_dyn_array_resize_impl(__ARR, sizeof(*__ARR), __AMOUNT); \
        }                                                                                       \
    } while (0)

#define neko_dyn_array_empty(__ARR) (neko_dyn_array_init((void **)&(__ARR), sizeof(*(__ARR))), (neko_dyn_array_size(__ARR) == 0))

#define neko_dyn_array_pop(__ARR)                    \
    do {                                             \
        if (__ARR && !neko_dyn_array_empty(__ARR)) { \
            neko_dyn_array_head(__ARR)->size -= 1;   \
        }                                            \
    } while (0)

#define neko_dyn_array_back(__ARR) *(__ARR + (neko_dyn_array_size(__ARR) ? neko_dyn_array_size(__ARR) - 1 : 0))

#define neko_dyn_array_for(__ARR, __T, __IT_NAME) for (__T *__IT_NAME = __ARR; __IT_NAME != neko_dyn_array_back(__ARR); ++__IT_NAME)

#define neko_dyn_array_new(__T) ((__T *)neko_dyn_array_resize_impl(NULL, sizeof(__T), 0))

#define neko_dyn_array_clear(__ARR)               \
    do {                                          \
        if (__ARR) {                              \
            neko_dyn_array_head(__ARR)->size = 0; \
        }                                         \
    } while (0)

#define neko_dyn_array(__T) __T *

#define neko_dyn_array_free(__ARR)                \
    do {                                          \
        if (__ARR) {                              \
            mem_free(neko_dyn_array_head(__ARR)); \
            (__ARR) = NULL;                       \
        }                                         \
    } while (0)

/*===================================
// Slot Array
===================================*/

#define NEKO_SLOT_ARRAY_INVALID_HANDLE UINT32_MAX

#define neko_slot_array_handle_valid(__SA, __ID) ((__SA) && __ID < neko_dyn_array_size((__SA)->indices) && (__SA)->indices[__ID] != NEKO_SLOT_ARRAY_INVALID_HANDLE)

typedef struct __neko_slot_array_dummy_header {
    neko_dyn_array(u32) indices;
    neko_dyn_array(u32) data;
} __neko_slot_array_dummy_header;

#define neko_slot_array(__T)         \
    struct {                         \
        neko_dyn_array(u32) indices; \
        neko_dyn_array(__T) data;    \
        __T tmp;                     \
    } *

#define neko_slot_array_new(__T) NULL

NEKO_FORCE_INLINE u32 __neko_slot_array_find_next_available_index(neko_dyn_array(u32) indices) {
    u32 idx = NEKO_SLOT_ARRAY_INVALID_HANDLE;
    for (u32 i = 0; i < (u32)neko_dyn_array_size(indices); ++i) {
        u32 handle = indices[i];
        if (handle == NEKO_SLOT_ARRAY_INVALID_HANDLE) {
            idx = i;
            break;
        }
    }
    if (idx == NEKO_SLOT_ARRAY_INVALID_HANDLE) {
        idx = neko_dyn_array_size(indices);
    }

    return idx;
}

void **neko_slot_array_init(void **sa, size_t sz);

#define neko_slot_array_init_all(__SA) \
    (neko_slot_array_init((void **)&(__SA), sizeof(*(__SA))), neko_dyn_array_init((void **)&((__SA)->indices), sizeof(u32)), neko_dyn_array_init((void **)&((__SA)->data), sizeof((__SA)->tmp)))

NEKO_FORCE_INLINE u32 neko_slot_array_insert_func(void **indices, void **data, void *val, size_t val_len, u32 *ip) {
    // Find next available index
    u32 idx = __neko_slot_array_find_next_available_index((u32 *)*indices);

    if (idx == neko_dyn_array_size(*indices)) {
        u32 v = 0;
        neko_dyn_array_push_data(indices, &v, sizeof(u32));
        idx = neko_dyn_array_size(*indices) - 1;
    }

    // Push data to array
    neko_dyn_array_push_data(data, val, val_len);

    // Set data in indices
    u32 bi = neko_dyn_array_size(*data) - 1;
    neko_dyn_array_set_data_i(indices, &bi, sizeof(u32), idx);

    if (ip) {
        *ip = idx;
    }

    return idx;
}

#define neko_slot_array_reserve(__SA, __NUM)            \
    do {                                                \
        neko_slot_array_init_all(__SA);                 \
        neko_dyn_array_reserve((__SA)->data, __NUM);    \
        neko_dyn_array_reserve((__SA)->indices, __NUM); \
    } while (0)

#define neko_slot_array_insert(__SA, __VAL) \
    (neko_slot_array_init_all(__SA), (__SA)->tmp = (__VAL), neko_slot_array_insert_func((void **)&((__SA)->indices), (void **)&((__SA)->data), (void *)&((__SA)->tmp), sizeof(((__SA)->tmp)), NULL))

#define neko_slot_array_insert_hp(__SA, __VAL, __hp) \
    (neko_slot_array_init_all(__SA), (__SA)->tmp = (__VAL), neko_slot_array_insert_func((void **)&((__SA)->indices), (void **)&((__SA)->data), &((__SA)->tmp), sizeof(((__SA)->tmp)), (__hp)))

#define neko_slot_array_insert_no_init(__SA, __VAL) \
    ((__SA)->tmp = (__VAL), neko_slot_array_insert_func((void **)&((__SA)->indices), (void **)&((__SA)->data), &((__SA)->tmp), sizeof(((__SA)->tmp)), NULL))

#define neko_slot_array_size(__SA) ((__SA) == NULL ? 0 : neko_dyn_array_size((__SA)->data))

#define neko_slot_array_empty(__SA) (neko_slot_array_size(__SA) == 0)

#define neko_slot_array_clear(__SA)                \
    do {                                           \
        if ((__SA) != NULL) {                      \
            neko_dyn_array_clear((__SA)->data);    \
            neko_dyn_array_clear((__SA)->indices); \
        }                                          \
    } while (0)

#define neko_slot_array_exists(__SA, __SID) ((__SA) && (__SID) < (u32)neko_dyn_array_size((__SA)->indices) && (__SA)->indices[__SID] != NEKO_SLOT_ARRAY_INVALID_HANDLE)

#define neko_slot_array_get(__SA, __SID) ((__SA)->data[(__SA)->indices[(__SID) % neko_dyn_array_size(((__SA)->indices))]])

#define neko_slot_array_getp(__SA, __SID) (&(neko_slot_array_get(__SA, (__SID))))

#define neko_slot_array_free(__SA)                \
    do {                                          \
        if ((__SA) != NULL) {                     \
            neko_dyn_array_free((__SA)->data);    \
            neko_dyn_array_free((__SA)->indices); \
            (__SA)->indices = NULL;               \
            (__SA)->data = NULL;                  \
            mem_free((__SA));                     \
            (__SA) = NULL;                        \
        }                                         \
    } while (0)

#define neko_slot_array_erase(__SA, __id)                                                       \
    do {                                                                                        \
        u32 __H0 = (__id) /*% neko_dyn_array_size((__SA)->indices)*/;                           \
        if (neko_slot_array_size(__SA) == 1) {                                                  \
            neko_slot_array_clear(__SA);                                                        \
        } else if (!neko_slot_array_handle_valid(__SA, __H0)) {                                 \
            neko_println("Warning: Attempting to erase invalid slot array handle (%zu)", __H0); \
        } else {                                                                                \
            u32 __OG_DATA_IDX = (__SA)->indices[__H0];                                          \
            /* Iterate through handles until last index of data found */                        \
            u32 __H = 0;                                                                        \
            for (u32 __I = 0; __I < neko_dyn_array_size((__SA)->indices); ++__I) {              \
                if ((__SA)->indices[__I] == neko_dyn_array_size((__SA)->data) - 1) {            \
                    __H = __I;                                                                  \
                    break;                                                                      \
                }                                                                               \
            }                                                                                   \
                                                                                                \
            /* Swap and pop data */                                                             \
            (__SA)->data[__OG_DATA_IDX] = neko_dyn_array_back((__SA)->data);                    \
            neko_dyn_array_pop((__SA)->data);                                                   \
                                                                                                \
            /* Point new handle, Set og handle to invalid */                                    \
            (__SA)->indices[__H] = __OG_DATA_IDX;                                               \
            (__SA)->indices[__H0] = NEKO_SLOT_ARRAY_INVALID_HANDLE;                             \
        }                                                                                       \
    } while (0)

/*=== Slot Array Iterator ===*/

// Slot array iterator new
typedef u32 neko_slot_array_iter;

#define neko_slot_array_iter_valid(__SA, __IT) (__SA && neko_slot_array_exists(__SA, __IT))

NEKO_FORCE_INLINE void _neko_slot_array_iter_advance_func(neko_dyn_array(u32) indices, u32 *it) {
    if (!indices) {
        *it = NEKO_SLOT_ARRAY_INVALID_HANDLE;
        return;
    }

    (*it)++;
    for (; *it < (u32)neko_dyn_array_size(indices); ++*it) {
        if (indices[*it] != NEKO_SLOT_ARRAY_INVALID_HANDLE) {
            break;
        }
    }
}

NEKO_FORCE_INLINE u32 _neko_slot_array_iter_find_first_valid_index(neko_dyn_array(u32) indices) {
    if (!indices) return NEKO_SLOT_ARRAY_INVALID_HANDLE;

    for (u32 i = 0; i < (u32)neko_dyn_array_size(indices); ++i) {
        if (indices[i] != NEKO_SLOT_ARRAY_INVALID_HANDLE) {
            return i;
        }
    }
    return NEKO_SLOT_ARRAY_INVALID_HANDLE;
}

#define neko_slot_array_iter_new(__SA) (_neko_slot_array_iter_find_first_valid_index((__SA) ? (__SA)->indices : 0))

#define neko_slot_array_iter_advance(__SA, __IT) _neko_slot_array_iter_advance_func((__SA) ? (__SA)->indices : NULL, &(__IT))

#define neko_slot_array_iter_get(__SA, __IT) neko_slot_array_get(__SA, __IT)

#define neko_slot_array_iter_getp(__SA, __IT) neko_slot_array_getp(__SA, __IT)
