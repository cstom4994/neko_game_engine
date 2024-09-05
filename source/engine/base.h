#ifndef NEKO_BASE_H
#define NEKO_BASE_H

#include <assert.h>
#include <ctype.h>
#include <float.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
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
typedef const char* const_str;

typedef struct lua_State lua_State;

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

#if defined(__cplusplus)
#define NEKO_DEFAULT_VAL() \
    {}
#define NEKO_API() extern "C"
#else
#define NEKO_DEFAULT_VAL() \
    { 0 }
#define NEKO_API() extern
#endif

#if defined(__cplusplus)

#define NEKO_SCRIPT(name, ...)                             \
    static const char* nekogame_ffi_##name = #__VA_ARGS__; \
    __VA_ARGS__

#ifdef _MSC_VER
#define NEKO_EXPORT extern "C" __declspec(dllexport)
#else
#define NEKO_EXPORT extern "C"
#endif

#else

#define NEKO_SCRIPT(name, ...)                             \
    static const char* nekogame_ffi_##name = #__VA_ARGS__; \
    __VA_ARGS__

#ifdef _MSC_VER
#define NEKO_EXPORT extern __declspec(dllexport)
#else
#define NEKO_EXPORT extern
#endif

#endif

#define NEKO_ARR_SIZE(__ARR) sizeof(__ARR) / sizeof(__ARR[0])

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

#define NEKO_OFFSET(TYPE, ELEMENT) ((size_t)(&(((TYPE*)(0))->ELEMENT)))
#define NEKO_TO_STR(TYPE) ((const char*)#TYPE)
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

#define NEKO_INT2VOIDP(I) (void*)(uintptr_t)(I)

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
inline void neko_printf(const char* fmt, ...) {
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

NEKO_API() void neko_log(const char* file, int line, const char* fmt, ...);

#define console_log(...) neko_log(__FILE__, __LINE__, __VA_ARGS__)

NEKO_API() void errorf(const char* fmt, ...);

#define line_str__(line) __FILE__ ":" #line ": "
#define line_str_(line) line_str__(line)
#define line_str() line_str_(__LINE__)

#define error(...) errorf(line_str() __VA_ARGS__)

#define error_assert(cond, ...) ((cond) ? 0 : (error("assertion '" #cond "' failed ... " __VA_ARGS__), 0))

#define normal_assert(cond, ...) ((cond) ? 0 : (neko_printf("    assertion failed: (%s), function %s, file %s, line %d.\n" __VA_ARGS__, #cond, __func__, __FILE__, __LINE__), 0))

#define neko_assert normal_assert

FORMAT_ARGS(1)
inline void neko_panic(const char* fmt, ...) {
    va_list args;
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

neko_handle_decl(gfx_texture_t);
typedef neko_handle(gfx_texture_t) gfx_texture_t;

/*===================================
// String Utils
===================================*/

inline uint8_t wtf8_decode(const char* input, uint32_t* res) {
    uint8_t b1 = input[0];
    if (b1 <= 0x7F) {
        *res = b1;
        return 1;
    }
    if (b1 < 0xC2) {
        return 0;
    }
    uint32_t code_point = b1;
    uint8_t b2 = input[1];
    if ((b2 & 0xC0) != 0x80) {
        return 0;
    }
    code_point = (code_point << 6) | (b2 & 0x3F);
    if (b1 <= 0xDF) {
        *res = 0x7FF & code_point;
        return 2;
    }

    uint8_t b3 = input[2];
    if ((b3 & 0xC0) != 0x80) {
        return 0;
    }
    code_point = (code_point << 6) | (b3 & 0x3F);
    if (b1 <= 0xEF) {
        *res = 0xFFFF & code_point;
        return 3;
    }

    uint8_t b4 = input[3];
    if ((b4 & 0xC0) != 0x80) {
        return 0;
    }
    code_point = (code_point << 6) | (b4 & 0x3F);
    if (b1 <= 0xF4) {
        code_point &= 0x1FFFFF;
        if (code_point <= 0x10FFFF) {
            *res = code_point;
            return 4;
        }
    }
    return 0;
}

inline size_t wtf8_to_utf16_length(const char* input, size_t length) {
    size_t output_len = 0;
    uint32_t code_point;
    for (size_t i = 0; i < length;) {
        uint8_t n = wtf8_decode(&input[i], &code_point);
        if (n == 0) {
            return (size_t)-1;
        }
        if (code_point > 0xFFFF) {
            output_len += 2;
        } else {
            output_len += 1;
        }
        i += n;
    }
    return output_len;
}

inline void wtf8_to_utf16(const char* input, size_t length, wchar_t* output, size_t output_len) {
    uint32_t code_point;
    for (size_t i = 0; i < length;) {
        uint8_t n = wtf8_decode(&input[i], &code_point);
        neko_assert(n > 0);
        if (code_point > 0x10000) {
            neko_assert(code_point < 0x10FFFF);
            *output++ = (((code_point - 0x10000) >> 10) + 0xD800);
            *output++ = ((code_point - 0x10000) & 0x3FF) + 0xDC00;
            output_len -= 2;
        } else {
            *output++ = code_point;
            output_len -= 1;
        }
        i += n;
    }
    (void)output_len;
    neko_assert(output_len == 0);
}

inline uint32_t wtf8_surrogate(const wchar_t* input, bool eof) {
    uint32_t u = input[0];
    if (u >= 0xD800 && u <= 0xDBFF && !eof) {
        uint32_t next = input[1];
        if (next >= 0xDC00 && next <= 0xDFFF) {
            return 0x10000 + ((u - 0xD800) << 10) + (next - 0xDC00);
        }
    }
    return u;
}

inline size_t wtf8_from_utf16_length(const wchar_t* input, size_t length) {
    size_t output_len = 0;
    for (size_t i = 0; i < length; ++i) {
        uint32_t code_point = wtf8_surrogate(&input[i], length == i + 1);
        if (code_point == 0) {
            break;
        }
        if (code_point < 0x80) {
            output_len += 1;
        } else if (code_point < 0x800) {
            output_len += 2;
        } else if (code_point < 0x10000) {
            output_len += 3;
        } else {
            output_len += 4;
            i++;
        }
    }
    return output_len;
}

inline void wtf8_from_utf16(const wchar_t* input, size_t length, char* output, size_t output_len) {
    for (size_t i = 0; i < length; ++i) {
        uint32_t code_point = wtf8_surrogate(&input[i], length == i + 1);
        if (code_point == 0) {
            break;
        }
        if (code_point < 0x80) {
            *output++ = code_point;
            output_len -= 1;
        } else if (code_point < 0x800) {
            *output++ = 0xC0 | (code_point >> 6);
            *output++ = 0x80 | (code_point & 0x3F);
            output_len -= 2;
        } else if (code_point < 0x10000) {
            *output++ = 0xE0 | (code_point >> 12);
            *output++ = 0x80 | ((code_point >> 6) & 0x3F);
            *output++ = 0x80 | (code_point & 0x3F);
            output_len -= 3;
        } else {
            *output++ = 0xF0 | (code_point >> 18);
            *output++ = 0x80 | ((code_point >> 12) & 0x3F);
            *output++ = 0x80 | ((code_point >> 6) & 0x3F);
            *output++ = 0x80 | (code_point & 0x3F);
            output_len -= 4;
            i++;
        }
    }
    (void)output_len;
    neko_assert(output_len == 0);
}

NEKO_FORCE_INLINE u32 neko_string_length(const char* txt) {
    u32 sz = 0;
    while (txt != NULL && txt[sz] != '\0') sz++;
    return sz;
}

#define neko_strlen(str) neko_string_length((const char*)str)

// Expects null terminated strings
NEKO_FORCE_INLINE bool neko_string_compare_equal(const char* txt, const char* cmp) {
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

NEKO_FORCE_INLINE bool neko_string_compare_equal_n(const char* txt, const char* cmp, u32 n) {
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

// NEKO_FORCE_INLINE char* neko_util_string_concat(char* s1, const char* s2) {
//     const size_t a = strlen(s1);
//     const size_t b = strlen(s2);
//     const size_t ab = a + b + 1;
//     s1 = (char*)neko_safe_realloc((void*)s1, ab);
//     memcpy(s1 + a, s2, b + 1);
//     return s1;
// }

NEKO_FORCE_INLINE void neko_util_str_to_lower(const char* src, char* buffer, size_t buffer_sz) {
    size_t src_sz = neko_string_length(src);
    size_t len = NEKO_MIN(src_sz, buffer_sz - 1);

    for (u32 i = 0; i < len; ++i) {
        buffer[i] = tolower(src[i]);
    }
    if (len) buffer[len] = '\0';
}

NEKO_FORCE_INLINE bool neko_util_str_is_numeric(const char* str) {
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

NEKO_FORCE_INLINE void neko_util_get_file_extension(char* buffer, u32 buffer_size, const_str file_path) {
    neko_assert(buffer && buffer_size);
    const_str extension = strrchr(file_path, '.');
    if (extension) {
        uint32_t extension_len = strlen(extension + 1);
        uint32_t len = (extension_len >= buffer_size) ? buffer_size - 1 : extension_len;
        memcpy(buffer, extension + 1, len);
        buffer[len] = '\0';
    } else {
        buffer[0] = '\0';
    }
}

NEKO_FORCE_INLINE void neko_util_get_dir_from_file(char* buffer, u32 buffer_size, const char* file_path) {
    u32 str_len = neko_string_length(file_path);
    const char* end = (file_path + str_len);
    for (u32 i = 0; i < str_len; ++i) {
        if (file_path[i] == '/' || file_path[i] == '\\') {
            end = &file_path[i];
        }
    }

    size_t dir_len = end - file_path;
    memcpy(buffer, file_path, NEKO_MIN(buffer_size, dir_len + 1));
    if (dir_len + 1 <= buffer_size) {
        buffer[dir_len] = '\0';
    }
}

NEKO_FORCE_INLINE const_str neko_util_get_filename(const_str path) {
    neko_assert(path);
    int len = strlen(path);
    for (int i = len - 1; i >= 0; i--) {
        if (path[i] == '\\' || path[i] == '/') {
            return path + i + 1;
        }
    }
    return path;
}

NEKO_FORCE_INLINE void neko_util_string_substring(const char* src, char* dst, size_t sz, u32 start, u32 end) {
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

NEKO_FORCE_INLINE void neko_util_string_remove_character(const char* src, char* buffer, u32 buffer_size, char delimiter) {
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

NEKO_FORCE_INLINE void neko_util_string_replace(char* buffer, size_t buffer_sz, const char* replace, char fallback) {
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

NEKO_FORCE_INLINE void neko_util_string_replace_delim(const char* source_str, char* buffer, u32 buffer_size, char delimiter, char replace) {
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

#ifdef __MINGW32__
#define neko_snprintf(__NAME, __SZ, __FMT, ...) __mingw_snprintf(__NAME, __SZ, __FMT, ##__VA_ARGS__)
#else
NEKO_FORCE_INLINE void neko_snprintf(char* buffer, size_t buffer_size, const char* fmt, ...) {
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

NEKO_FORCE_INLINE u32 neko_util_safe_truncate_u64(u64 value) {
    neko_assert(value <= 0xFFFFFFFF);
    u32 result = (u32)value;
    return result;
}

NEKO_FORCE_INLINE u32 neko_hash_u32(u32 x) {
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

NEKO_FORCE_INLINE u32 neko_hash_u64(u64 x) {
    x = (x ^ (x >> 31) ^ (x >> 62)) * UINT64_C(0x319642b2d24d8ec3);
    x = (x ^ (x >> 27) ^ (x >> 54)) * UINT64_C(0x96de1b173f119089);
    x = x ^ (x >> 30) ^ (x >> 60);
    return (u32)x;
}

// Note: source: http://www.cse.yorku.ca/~oz/hash.html
// djb2 hash by dan bernstein
NEKO_FORCE_INLINE u32 neko_hash_str(const char* str) {
    u32 hash = 5381;
    i32 c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

NEKO_FORCE_INLINE u64 neko_hash_str64(const char* str) {
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

NEKO_FORCE_INLINE bool neko_compare_bytes(void* b0, void* b1, size_t len) { return 0 == memcmp(b0, b1, len); }

// Hash generic bytes using (ripped directly from Sean Barret's stb_ds.h)
#define NEKO_SIZE_T_BITS ((sizeof(size_t)) * 8)
#define NEKO_SIPHASH_C_ROUNDS 1
#define NEKO_SIPHASH_D_ROUNDS 1
#define neko_rotate_left(__V, __N) (((__V) << (__N)) | ((__V) >> (NEKO_SIZE_T_BITS - (__N))))
#define neko_rotate_right(__V, __N) (((__V) >> (__N)) | ((__V) << (NEKO_SIZE_T_BITS - (__N))))

NEKO_FORCE_INLINE size_t neko_hash_siphash_bytes(void* p, size_t len, size_t seed) {
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

NEKO_FORCE_INLINE size_t neko_hash_bytes(void* p, size_t len, size_t seed) {
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

// 在内存中存储连续的对象
// 对象可能会在内存中移动 因此不要依赖指向 CArray 中对象的指针
typedef struct CArray CArray;

CArray* array_new_(size_t object_size);  // object_size 是每个元素的大小
void array_free(CArray* arr);
void* array_get(CArray* arr, unsigned int i);
void* array_top(CArray* arr);
unsigned int array_length(CArray* arr);  // 数组中的对象数

// 添加/删除可能会更改开始/结束 因此在迭代时要小心
void* array_begin(CArray* arr);                                         // 指向第一个元素的指针
void* array_end(CArray* arr);                                           // 指向one past last元素的指针
void* array_add(CArray* arr);                                           // 添加新对象, index is length - 1
void array_reset(CArray* arr, unsigned int num);                        // 将数据未定义的对象调整为 num 对象
void array_pop(CArray* arr);                                            // 删除具有最高索引的对象
bool array_quick_remove(CArray* arr, unsigned int i);                   // 快速删除, 可以将其他一些元素交换成 arr[i] 如果是这样 则返回 true
void array_sort(CArray* arr, int (*compar)(const void*, const void*));  // compare 是一个比较器函数

#define array_clear(arr) array_reset(arr, 0)  //
#define array_add_val(type, arr) (*((type*)array_add(arr)))
#define array_top_val(type, arr) (*((type*)array_top(arr)))
#define array_get_val(type, arr, i) (*((type*)array_get(arr, i)))
#define array_new(type) array_new_(sizeof(type))

#define array_foreach(var, arr) for (void* __end = (var = (decltype(var))array_begin(arr), array_end(arr)); var != __end; ++var)

NEKO_SCRIPT(scalar,

            typedef float Scalar;

            typedef float f32;

            typedef double f64;

            typedef uint16_t u16;

            typedef uint32_t u32;

            typedef uint64_t u64;

            typedef const char* const_str;

)

#ifdef M_PI
#define SCALAR_PI M_PI
#else
#define SCALAR_PI 3.14159265358979323846264338327950288
#endif

#define SCALAR_INFINITY INFINITY

#define SCALAR_EPSILON FLT_EPSILON

#define scalar_cos cosf
#define scalar_sin sinf
#define scalar_atan2 atan2f

#define scalar_sqrt sqrtf

#define scalar_min fminf
#define scalar_max fmaxf

#define scalar_floor floor

NEKO_SCRIPT(saveload,

            // 请记住 *_close(...) 当完成以释放资源时

            typedef struct Store Store;

            NEKO_EXPORT Store * store_open();

            NEKO_EXPORT Store * store_open_str(const char* str);

            NEKO_EXPORT const char* store_write_str(Store* s);

            NEKO_EXPORT Store * store_open_file(const char* filename);

            NEKO_EXPORT void store_write_file(Store* s, const char* filename);

            NEKO_EXPORT void store_close(Store* s);

)

// 存储树有助于向后兼容 save/load
bool store_child_save(Store** sp, const char* name, Store* parent);
bool store_child_save_compressed(Store** sp, const char* name, Store* parent);
bool store_child_load(Store** sp, const char* name, Store* parent);
void scalar_save(const Scalar* f, const char* name, Store* s);
bool scalar_load(Scalar* f, const char* name, Scalar d, Store* s);
void uint_save(const unsigned int* u, const char* name, Store* s);
bool uint_load(unsigned int* u, const char* name, unsigned int d, Store* s);
void int_save(const int* i, const char* name, Store* s);
bool int_load(int* i, const char* name, int d, Store* s);

#define enum_save(val, n, s)    \
    do {                        \
        int e__;                \
        e__ = *(val);           \
        int_save(&e__, n, (s)); \
    } while (0)
#define enum_load(val, n, d, s)         \
    do {                                \
        int e__;                        \
        int_load(&e__, n, (int)d, (s)); \
        *((int*)val) = e__;             \
    } while (0)

void bool_save(const bool* b, const char* name, Store* s);
bool bool_load(bool* b, const char* name, bool d, Store* s);

void string_save(const char** c, const char* name, Store* s);
bool string_load(char** c, const char* name, const char* d, Store* s);

typedef struct AssetTexture {
    u32 id;  // 如果未初始化或纹理错误 则为 0
    int width;
    int height;
    int components;
    bool flip_image_vertical;
} AssetTexture;

typedef enum neko_resource_type_t {
    NEKO_RESOURCE_STRING,
    NEKO_RESOURCE_BINARY,
    NEKO_RESOURCE_TEXTURE,
    NEKO_RESOURCE_SHADER,
    NEKO_RESOURCE_ASSEMBLY,
    NEKO_RESOURCE_SCRIPT,
    NEKO_RESOURCE_MODEL,
    NEKO_RESOURCE_MATERIAL,
    NEKO_RESOURCE_FONT
} neko_resource_type_t;

typedef struct neko_resource_t {
    neko_resource_type_t type;

    void* payload;
    u32 payload_size;

    i64 modtime;

    char* file_name;
    u32 file_name_length;
    u32 file_name_hash;
} neko_resource_t;

NEKO_SCRIPT(
        fs,

        typedef struct vfs_file {
            const_str data;
            size_t len;
            u64 offset;
        } vfs_file;

        NEKO_EXPORT size_t neko_capi_vfs_fread(void* dest, size_t size, size_t count, vfs_file* vf);

        NEKO_EXPORT int neko_capi_vfs_fseek(vfs_file* vf, u64 of, int whence);

        NEKO_EXPORT u64 neko_capi_vfs_ftell(vfs_file * vf);

        NEKO_EXPORT vfs_file neko_capi_vfs_fopen(const_str path);

        NEKO_EXPORT int neko_capi_vfs_fclose(vfs_file* vf);

        NEKO_EXPORT int neko_capi_vfs_fscanf(vfs_file* vf, const char* format, ...);

        NEKO_EXPORT bool neko_capi_vfs_file_exists(const_str fsname, const_str filepath);

        NEKO_EXPORT const_str neko_capi_vfs_read_file(const_str fsname, const_str filepath, size_t* size);

)

NEKO_SCRIPT(
        vec2,

        typedef struct {
            union {
                struct {
                    f32 x, y;
                };
                f32 xy[2];
            };
        } vec2_t;

        typedef vec2_t vec2;

        NEKO_EXPORT vec2 luavec2(Scalar x, Scalar y);

        NEKO_EXPORT vec2 vec2_zero;

        NEKO_EXPORT vec2 vec2_add(vec2 u, vec2 v);

        NEKO_EXPORT vec2 vec2_sub(vec2 u, vec2 v);

        NEKO_EXPORT vec2 vec2_mul(vec2 u, vec2 v);  // u * v componentwise

        NEKO_EXPORT vec2 vec2_div(vec2 u, vec2 v);  // u / v componentwise

        NEKO_EXPORT vec2 vec2_scalar_mul(vec2 v, Scalar f);

        NEKO_EXPORT vec2 vec2_scalar_div(vec2 v, Scalar f);  // (v.x / f, v.y / f)

        NEKO_EXPORT vec2 scalar_vec2_div(Scalar f, vec2 v);  // (f / v.x, f / v.y)

        NEKO_EXPORT vec2 vec2_neg(vec2 v);

        NEKO_EXPORT Scalar vec2_len(vec2 v);

        NEKO_EXPORT vec2 vec2_normalize(vec2 v);

        NEKO_EXPORT Scalar vec2_dot(vec2 u, vec2 v);

        NEKO_EXPORT Scalar vec2_dist(vec2 u, vec2 v);

        NEKO_EXPORT vec2 vec2_rot(vec2 v, Scalar rot);

        NEKO_EXPORT Scalar vec2_atan2(vec2 v);

        NEKO_EXPORT void vec2_save(vec2* v, const char* name, Store* s);

        NEKO_EXPORT bool vec2_load(vec2* v, const char* name, vec2 d, Store* s);

)

#define luavec2(x, y) (vec2{(float)(x), (float)(y)})

NEKO_SCRIPT(
        mat3,

        /*
         * 按列优先顺序存储
         *
         *     m = /                                 \
         *         | m.m[0][0]  m.m[1][0]  m.m[2][0] |
         *         | m.m[0][1]  m.m[1][1]  m.m[2][1] |
         *         | m.m[0][2]  m.m[1][2]  m.m[2][2] |
         *         \                                 /
         */
        typedef struct mat3 mat3;
        struct mat3 {
            union {
                Scalar v[9];
                Scalar m[3][3];
            };
        };

        NEKO_EXPORT mat3 luamat3(Scalar m00, Scalar m01, Scalar m02, Scalar m10, Scalar m11, Scalar m12, Scalar m20, Scalar m21, Scalar m22);

        NEKO_EXPORT mat3 mat3_identity();  // 返回单位矩阵

        NEKO_EXPORT mat3 mat3_mul(mat3 m, mat3 n);

        // 按顺序应用 scale rot 和 trans 的矩阵
        NEKO_EXPORT mat3 mat3_scaling_rotation_translation(vec2 scale, Scalar rot, vec2 trans);

        NEKO_EXPORT vec2 mat3_get_translation(mat3 m);

        NEKO_EXPORT Scalar mat3_get_rotation(mat3 m);

        NEKO_EXPORT vec2 mat3_get_scale(mat3 m);

        NEKO_EXPORT mat3 mat3_inverse(mat3 m);

        NEKO_EXPORT vec2 mat3_transform(mat3 m, vec2 v);

        NEKO_EXPORT void mat3_save(mat3* m, const char* name, Store* s);

        NEKO_EXPORT bool mat3_load(mat3* m, const char* name, mat3 d, Store* s);

)

#define luamat3(m00, m01, m02, m10, m11, m12, m20, m21, m22) (mat3{.m = {{(m00), (m01), (m02)}, {(m10), (m11), (m12)}, {(m20), (m21), (m22)}}})

NEKO_SCRIPT(
        bbox,

        typedef struct BBox BBox;
        struct BBox {
            vec2 min;
            vec2 max;
        };

        NEKO_EXPORT BBox bbox(vec2 min, vec2 max);

        NEKO_EXPORT BBox bbox_bound(vec2 a, vec2 b);

        NEKO_EXPORT BBox bbox_merge(BBox a, BBox b);

        NEKO_EXPORT bool bbox_contains(BBox b, vec2 p);

        // 返回 bbox 围绕改造后的盒子
        NEKO_EXPORT BBox bbox_transform(mat3 m, BBox b);

)

NEKO_SCRIPT(
        color,

        typedef struct Color Color;
        struct Color { Scalar r, g, b, a; };

        typedef struct Color256 Color256; struct Color256 {
            union {
                uint8_t rgba[4];
                struct {
                    uint8_t r, g, b, a;
                };
            };
        };

        NEKO_EXPORT Color color(Scalar r, Scalar g, Scalar b, Scalar a);

        NEKO_EXPORT Color color_opaque(Scalar r, Scalar g, Scalar b);

        NEKO_EXPORT Color color_black;

        NEKO_EXPORT Color color_white;

        NEKO_EXPORT Color color_gray;

        NEKO_EXPORT Color color_red;

        NEKO_EXPORT Color color_green;

        NEKO_EXPORT Color color_blue;

        NEKO_EXPORT Color color_clear;  // zero alpha

        NEKO_EXPORT void color_save(Color* c, const char* name, Store* s);

        NEKO_EXPORT bool color_load(Color* c, const char* name, Color d, Store* s);

)

#ifdef __cplusplus
#define color(r, g, b, a) (Color{(r), (g), (b), (a)})
#define color_opaque(r, g, b, a) color(r, g, b, 1)
#define color256(r, g, b, a) (Color256{(r), (g), (b), (a)})
#else
#define color(r, g, b, a) ((Color){(r), (g), (b), (a)})
#define color_opaque(r, g, b, a) color(r, g, b, 1)
#define color256(r, g, b, a) ((Color256){(r), (g), (b), (a)})
#endif

#define NEKO_COLOR_BLACK color256(0, 0, 0, 255)
#define NEKO_COLOR_WHITE color256(255, 255, 255, 255)
#define NEKO_COLOR_RED color256(255, 0, 0, 255)
#define NEKO_COLOR_GREEN color256(0, 255, 0, 255)
#define NEKO_COLOR_BLUE color256(0, 0, 255, 255)
#define NEKO_COLOR_ORANGE color256(255, 100, 0, 255)
#define NEKO_COLOR_YELLOW color256(255, 255, 0, 255)
#define NEKO_COLOR_PURPLE color256(128, 0, 128, 255)
#define NEKO_COLOR_MAROON color256(128, 0, 0, 255)
#define NEKO_COLOR_BROWN color256(165, 42, 42, 255)

inline Color256 color256_alpha(Color256 c, u8 a) { return color256(c.r, c.g, c.b, a); }

/*========================
// NEKO_MATH
========================*/

// Defines
#define neko_pi 3.14159265358979323846264f
#define neko_tau 2.0 * neko_pi
#define neko_e 2.71828182845904523536f  // e
#define neko_epsilon (1e-6)

// 实用
#define neko_v2(...) vec2_ctor(__VA_ARGS__)
#define neko_v3(...) vec3_ctor(__VA_ARGS__)
#define neko_v4(...) vec4_ctor(__VA_ARGS__)

#define neko_v2s(__S) vec2_ctor((__S), (__S))
#define neko_v3s(__S) vec3_ctor((__S), (__S), (__S))
#define neko_v4s(__S) vec4_ctor((__S), (__S), (__S), (__S))

#define neko_v4_xy_v(__X, __Y, __V) vec4_ctor((__X), (__Y), (__V).x, (__V).y)
#define neko_v4_xyz_s(__XYZ, __S) vec4_ctor((__XYZ).x, (__XYZ).y, (__XYZ).z, (__S))

#define NEKO_XAXIS neko_v3(1.f, 0.f, 0.f)
#define NEKO_YAXIS neko_v3(0.f, 1.f, 0.f)
#define NEKO_ZAXIS neko_v3(0.f, 0.f, 1.f)

/*================================================================================
// Useful Common Math Functions
================================================================================*/

#define neko_rad2deg(__R) (float)((__R * 180.0f) / neko_pi)

#define neko_deg2rad(__D) (float)((__D * neko_pi) / 180.0f)

// Interpolation
// Source: https://codeplea.com/simple-interpolation

// Returns v based on t
inline float neko_interp_linear(float a, float b, float t) { return (a + t * (b - a)); }

// Returns t based on v
inline float neko_interp_linear_inv(float a, float b, float v) { return (v - a) / (b - a); }

inline float neko_interp_smoothstep(float a, float b, float t) { return neko_interp_linear(a, b, t * t * (3.0f - 2.0f * t)); }

inline float neko_interp_cosine(float a, float b, float t) { return neko_interp_linear(a, b, (float)-cos(neko_pi * t) * 0.5f + 0.5f); }

inline float neko_interp_acceleration(float a, float b, float t) { return neko_interp_linear(a, b, t * t); }

inline float neko_interp_deceleration(float a, float b, float t) { return neko_interp_linear(a, b, 1.0f - (1.0f - t) * (1.0f - t)); }

inline float neko_round(float val) { return (float)floor(val + 0.5f); }

inline float neko_map_range(float input_start, float input_end, float output_start, float output_end, float val) {
    float slope = (output_end - output_start) / (input_end - input_start);
    return (output_start + (slope * (val - input_start)));
}

// 缓动来自：https://github.com/raysan5/raylib/blob/ea0f6c7a26f3a61f3be542aa8f066ce033766a9f/examples/others/easings.h
inline float neko_ease_cubic_in(float t, float b, float c, float d) {
    t /= d;
    return (c * t * t * t + b);
}

inline float neko_ease_cubic_out(float t, float b, float c, float d) {
    t = t / d - 1.0f;
    return (c * (t * t * t + 1.0f) + b);
}

inline float neko_ease_cubic_in_out(float t, float b, float c, float d) {
    if ((t /= d / 2.0f) < 1.0f) {
        return (c / 2.0f * t * t * t + b);
    }
    t -= 2.0f;
    return (c / 2.0f * (t * t * t + 2.0f) + b);
}

/*================================================================================
// Vec2
================================================================================*/

inline vec2 vec2_ctor(f32 _x, f32 _y) {
    vec2 v;
    v.x = _x;
    v.y = _y;
    return v;
}

inline bool vec2_nan(vec2 v) {
    if (v.x != v.x || v.y != v.y) return true;
    return false;
}

inline bool vec2_equals(vec2 v0, vec2 v1) { return (v0.x == v1.x && v0.y == v1.y); }

inline vec2 vec2_scale(vec2 v, f32 s) { return vec2_ctor(v.x * s, v.y * s); }

inline f32 vec2_dot(vec2 v0, vec2 v1) { return (f32)(v0.x * v1.x + v0.y * v1.y); }

inline f32 vec2_len(vec2 v) { return (f32)sqrt(vec2_dot(v, v)); }

inline vec2 vec2_project_onto(vec2 v0, vec2 v1) {
    f32 dot = vec2_dot(v0, v1);
    f32 len = vec2_dot(v1, v1);

    // Orthogonal, so return v1
    if (len == 0.f) return v1;

    return vec2_scale(v1, dot / len);
}

inline vec2 vec2_norm(vec2 v) {
    f32 len = vec2_len(v);
    return vec2_scale(v, len != 0.f ? 1.0f / vec2_len(v) : 1.f);
}

inline f32 vec2_dist(vec2 a, vec2 b) {
    f32 dx = (a.x - b.x);
    f32 dy = (a.y - b.y);
    return (float)(sqrt(dx * dx + dy * dy));
}

inline f32 vec2_dist2(vec2 a, vec2 b) {
    f32 dx = (a.x - b.x);
    f32 dy = (a.y - b.y);
    return (float)(dx * dx + dy * dy);
}

inline f32 vec2_cross(vec2 a, vec2 b) { return a.x * b.y - a.y * b.x; }

inline f32 vec2_angle(vec2 a, vec2 b) { return (float)acos(vec2_dot(a, b) / (vec2_len(a) * vec2_len(b))); }

inline bool vec2_equal(vec2 a, vec2 b) { return (a.x == b.x && a.y == b.y); }

/*================================================================================
// Vec3
================================================================================*/

typedef struct {
    union {
        f32 xyz[3];
        struct {
            f32 x, y, z;
        };
    };

} vec3_t;

typedef vec3_t vec3;

inline vec3 vec3_ctor(f32 _x, f32 _y, f32 _z) {
    vec3 v;
    v.x = _x;
    v.y = _y;
    v.z = _z;
    return v;
}

inline bool vec3_eq(vec3 v0, vec3 v1) { return (v0.x == v1.x && v0.y == v1.y && v0.z == v1.z); }

inline vec3 vec3_add(vec3 v0, vec3 v1) { return vec3_ctor(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z); }

inline vec3 vec3_sub(vec3 v0, vec3 v1) { return vec3_ctor(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z); }

inline vec3 vec3_mul(vec3 v0, vec3 v1) { return vec3_ctor(v0.x * v1.x, v0.y * v1.y, v0.z * v1.z); }

inline vec3 vec3_div(vec3 v0, vec3 v1) { return vec3_ctor(v0.x / v1.x, v0.y / v1.y, v0.z / v1.z); }

inline vec3 vec3_scale(vec3 v, f32 s) { return vec3_ctor(v.x * s, v.y * s, v.z * s); }

inline vec3 vec3_neg(vec3 v) { return vec3_scale(v, -1.f); }

inline f32 vec3_dot(vec3 v0, vec3 v1) {
    f32 dot = (f32)((v0.x * v1.x) + (v0.y * v1.y) + v0.z * v1.z);
    return dot;
}

inline bool vec3_same_dir(vec3 v0, vec3 v1) { return (vec3_dot(v0, v1) > 0.f); }

inline vec3 vec3_sign(vec3 v) { return (vec3_ctor(v.x < 0.f ? -1.f : v.x > 0.f ? 1.f : 0.f, v.y < 0.f ? -1.f : v.y > 0.f ? 1.f : 0.f, v.z < 0.f ? -1.f : v.z > 0.f ? 1.f : 0.f)); }

inline float vec3_signX(vec3 v) { return (v.x < 0.f ? -1.f : v.x > 0.f ? 1.f : 0.f); }

inline float vec3_signY(vec3 v) { return (v.y < 0.f ? -1.f : v.y > 0.f ? 1.f : 0.f); }

inline float vec3_signZ(vec3 v) { return (v.z < 0.f ? -1.f : v.z > 0.f ? 1.f : 0.f); }

inline f32 vec3_len(vec3 v) { return (f32)sqrt(vec3_dot(v, v)); }

inline f32 vec3_len2(vec3 v) { return (f32)(vec3_dot(v, v)); }

inline vec3 vec3_project_onto(vec3 v0, vec3 v1) {
    f32 dot = vec3_dot(v0, v1);
    f32 len = vec3_dot(v1, v1);

    // Orthogonal, so return v1
    if (len == 0.f) return v1;

    return vec3_scale(v1, dot / len);
}

inline bool vec3_nan(vec3 v) {
    if (v.x != v.x || v.y != v.y || v.z != v.z) return true;
    return false;
}

inline f32 vec3_dist2(vec3 a, vec3 b) {
    f32 dx = (a.x - b.x);
    f32 dy = (a.y - b.y);
    f32 dz = (a.z - b.z);
    return (dx * dx + dy * dy + dz * dz);
}

inline f32 vec3_dist(vec3 a, vec3 b) { return sqrt(vec3_dist2(a, b)); }

inline vec3 vec3_norm(vec3 v) {
    f32 len = vec3_len(v);
    return len == 0.f ? v : vec3_scale(v, 1.f / len);
}

inline vec3 vec3_cross(vec3 v0, vec3 v1) { return vec3_ctor(v0.y * v1.z - v0.z * v1.y, v0.z * v1.x - v0.x * v1.z, v0.x * v1.y - v0.y * v1.x); }

inline void vec3_scale_ip(vec3* vp, f32 s) {
    vp->x *= s;
    vp->y *= s;
    vp->z *= s;
}

inline float vec3_angle_between(vec3 v0, vec3 v1) { return acosf(vec3_dot(v0, v1)); }

inline float vec3_angle_between_signed(vec3 v0, vec3 v1) { return asinf(vec3_len(vec3_cross(v0, v1))); }

inline vec3 vec3_triple_cross_product(vec3 a, vec3 b, vec3 c) { return vec3_sub((vec3_scale(b, vec3_dot(c, a))), (vec3_scale(a, vec3_dot(c, b)))); }

/*================================================================================
// Vec4
================================================================================*/

typedef struct {
    union {
        f32 xyzw[4];
        struct {
            f32 x, y, z, w;
        };
    };
} vec4_t;

typedef vec4_t vec4;

inline vec4 vec4_ctor(f32 _x, f32 _y, f32 _z, f32 _w) {
    vec4 v;
    v.x = _x;
    v.y = _y;
    v.z = _z;
    v.w = _w;
    return v;
}

inline vec4 vec4_add(vec4 v0, vec4 v1) { return vec4_ctor(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z, v0.w + v1.w); }

inline vec4 vec4_sub(vec4 v0, vec4 v1) { return vec4_ctor(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z, v0.w - v1.w); }

inline vec4 vec4_mul(vec4 v0, vec4 v1) { return vec4_ctor(v0.x * v1.x, v0.y * v1.y, v0.z * v1.z, v0.w * v1.w); }

inline vec4 vec4_div(vec4 v0, vec4 v1) { return vec4_ctor(v0.x / v1.x, v0.y / v1.y, v0.z / v1.z, v0.w / v1.w); }

inline vec4 vec4_scale(vec4 v, f32 s) { return vec4_ctor(v.x * s, v.y * s, v.z * s, v.w * s); }

inline f32 vec4_dot(vec4 v0, vec4 v1) { return (f32)(v0.x * v1.x + v0.y * v1.y + v0.z * v1.z + v0.w * v1.w); }

inline f32 vec4_len(vec4 v) { return (f32)sqrt(vec4_dot(v, v)); }

inline vec4 vec4_project_onto(vec4 v0, vec4 v1) {
    f32 dot = vec4_dot(v0, v1);
    f32 len = vec4_dot(v1, v1);

    // Orthogonal, so return v1
    if (len == 0.f) return v1;

    return vec4_scale(v1, dot / len);
}

inline vec4 vec4_norm(vec4 v) { return vec4_scale(v, 1.0f / vec4_len(v)); }

inline f32 vec4_dist(vec4 v0, vec4 v1) {
    f32 dx = (v0.x - v1.x);
    f32 dy = (v0.y - v1.y);
    f32 dz = (v0.z - v1.z);
    f32 dw = (v0.w - v1.w);
    return (float)(sqrt(dx * dx + dy * dy + dz * dz + dw * dw));
}

/*================================================================================
// Useful Vector Functions
================================================================================*/

inline vec3 neko_v4tov3(vec4 v) { return neko_v3(v.x, v.y, v.z); }

inline vec2 neko_v3tov2(vec3 v) { return neko_v2(v.x, v.y); }

inline vec3 neko_v2tov3(vec2 v) { return neko_v3(v.x, v.y, 0.f); }

/*================================================================================
// Mat3x3
================================================================================*/

inline mat3 mat3_diag(float val) {
    mat3 m = NEKO_DEFAULT_VAL();
    m.v[0 + 0 * 3] = val;
    m.v[1 + 1 * 3] = val;
    m.v[2 + 2 * 3] = val;
    return m;
}

inline mat3 mat3_mul(mat3 m0, mat3 m1) {
    mat3 m = NEKO_DEFAULT_VAL();

    for (u32 y = 0; y < 3; ++y) {
        for (u32 x = 0; x < 3; ++x) {
            f32 sum = 0.0f;
            for (u32 e = 0; e < 3; ++e) {
                sum += m0.v[x + e * 3] * m1.v[e + y * 3];
            }
            m.v[x + y * 3] = sum;
        }
    }

    return m;
}

inline vec3 mat3_mul_vec3(mat3 m, vec3 v) { return vec3_ctor(m.v[0] * v.x + m.v[1] * v.y + m.v[2] * v.z, m.v[3] * v.x + m.v[4] * v.y + m.v[5] * v.z, m.v[6] * v.x + m.v[7] * v.y + m.v[8] * v.z); }

inline mat3 mat3_scale(float x, float y, float z) {
    mat3 m = NEKO_DEFAULT_VAL();
    m.v[0] = x;
    m.v[4] = y;
    m.v[8] = z;
    return m;
}

inline mat3 mat3_rotate(float radians, float x, float y, float z) {
    mat3 m = NEKO_DEFAULT_VAL();
    float s = sinf(radians), c = cosf(radians), c1 = 1.f - c;
    float xy = x * y;
    float yz = y * z;
    float zx = z * x;
    float xs = x * s;
    float ys = y * s;
    float zs = z * s;
    m.v[0] = c1 * x * x + c;
    m.v[1] = c1 * xy - zs;
    m.v[2] = c1 * zx + ys;
    m.v[3] = c1 * xy + zs;
    m.v[4] = c1 * y * y + c;
    m.v[5] = c1 * yz - xs;
    m.v[6] = c1 * zx - ys;
    m.v[7] = c1 * yz + xs;
    m.v[8] = c1 * z * z + c;
    return m;
}

inline mat3 mat3_rotatev(float radians, vec3 axis) { return mat3_rotate(radians, axis.x, axis.y, axis.z); }

// Turn quaternion into mat3
inline mat3 mat3_rotateq(vec4 q) {
    mat3 m = NEKO_DEFAULT_VAL();
    float x2 = q.x * q.x, y2 = q.y * q.y, z2 = q.z * q.z, w2 = q.w * q.w;
    float xz = q.x * q.z, xy = q.x * q.y, yz = q.y * q.z, wz = q.w * q.z, wy = q.w * q.y, wx = q.w * q.x;
    m.v[0] = 1 - 2 * (y2 + z2);
    m.v[1] = 2 * (xy + wz);
    m.v[2] = 2 * (xz - wy);
    m.v[3] = 2 * (xy - wz);
    m.v[4] = 1 - 2 * (x2 + z2);
    m.v[5] = 2 * (yz + wx);
    m.v[6] = 2 * (xz + wy);
    m.v[7] = 2 * (yz - wx);
    m.v[8] = 1 - 2 * (x2 + y2);
    return m;
}

inline mat3 mat3_rsq(vec4 q, vec3 s) {
    mat3 mr = mat3_rotateq(q);
    mat3 ms = mat3_scale(s.x, s.y, s.z);
    return mat3_mul(mr, ms);
}

inline mat3 mat3_inverse(mat3 m) {
    mat3 r = NEKO_DEFAULT_VAL();

    double det = (double)(m.v[0 * 3 + 0] * (m.v[1 * 3 + 1] * m.v[2 * 3 + 2] - m.v[2 * 3 + 1] * m.v[1 * 3 + 2]) - m.v[0 * 3 + 1] * (m.v[1 * 3 + 0] * m.v[2 * 3 + 2] - m.v[1 * 3 + 2] * m.v[2 * 3 + 0]) +
                          m.v[0 * 3 + 2] * (m.v[1 * 3 + 0] * m.v[2 * 3 + 1] - m.v[1 * 3 + 1] * m.v[2 * 3 + 0]));

    double inv_det = det ? 1.0 / det : 0.0;

    r.v[0 * 3 + 0] = (m.v[1 * 3 + 1] * m.v[2 * 3 + 2] - m.v[2 * 3 + 1] * m.v[1 * 3 + 2]) * inv_det;
    r.v[0 * 3 + 1] = (m.v[0 * 3 + 2] * m.v[2 * 3 + 1] - m.v[0 * 3 + 1] * m.v[2 * 3 + 2]) * inv_det;
    r.v[0 * 3 + 2] = (m.v[0 * 3 + 1] * m.v[1 * 3 + 2] - m.v[0 * 3 + 2] * m.v[1 * 3 + 1]) * inv_det;
    r.v[1 * 3 + 0] = (m.v[1 * 3 + 2] * m.v[2 * 3 + 0] - m.v[1 * 3 + 0] * m.v[2 * 3 + 2]) * inv_det;
    r.v[1 * 3 + 1] = (m.v[0 * 3 + 0] * m.v[2 * 3 + 2] - m.v[0 * 3 + 2] * m.v[2 * 3 + 0]) * inv_det;
    r.v[1 * 3 + 2] = (m.v[1 * 3 + 0] * m.v[0 * 3 + 2] - m.v[0 * 3 + 0] * m.v[1 * 3 + 2]) * inv_det;
    r.v[2 * 3 + 0] = (m.v[1 * 3 + 0] * m.v[2 * 3 + 1] - m.v[2 * 3 + 0] * m.v[1 * 3 + 1]) * inv_det;
    r.v[2 * 3 + 1] = (m.v[2 * 3 + 0] * m.v[0 * 3 + 1] - m.v[0 * 3 + 0] * m.v[2 * 3 + 1]) * inv_det;
    r.v[2 * 3 + 2] = (m.v[0 * 3 + 0] * m.v[1 * 3 + 1] - m.v[1 * 3 + 0] * m.v[0 * 3 + 1]) * inv_det;

    return r;
}

/*================================================================================
// Mat4x4
================================================================================*/

/*
    Matrices are stored in linear, contiguous memory and assume a column-major ordering.
*/

typedef struct mat4 {
    union {
        vec4 rows[4];
        float m[4][4];
        float elements[16];
        struct {
            vec4 right, up, dir, position;
        } v;
    };
} mat4_t;

typedef mat4_t mat4;

inline mat4 mat4_diag(f32 val) {
    mat4 m;
    memset(m.elements, 0, sizeof(m.elements));
    m.elements[0 + 0 * 4] = val;
    m.elements[1 + 1 * 4] = val;
    m.elements[2 + 2 * 4] = val;
    m.elements[3 + 3 * 4] = val;
    return m;
}

#define mat4_identity() mat4_diag(1.0f)

inline mat4 mat4_ctor() {
    mat4 mat = NEKO_DEFAULT_VAL();
    return mat;
}

inline mat4 mat4_elem(const float* elements) {
    mat4 mat = mat4_ctor();
    memcpy(mat.elements, elements, sizeof(f32) * 16);
    return mat;
}

inline mat4 mat4_mul(mat4 m0, mat4 m1) {
    mat4 m_res = mat4_ctor();
    for (u32 y = 0; y < 4; ++y) {
        for (u32 x = 0; x < 4; ++x) {
            f32 sum = 0.0f;
            for (u32 e = 0; e < 4; ++e) {
                sum += m0.elements[x + e * 4] * m1.elements[e + y * 4];
            }
            m_res.elements[x + y * 4] = sum;
        }
    }

    return m_res;
}

inline vec4 mat4_mulv(mat4 m, vec4 v) {
    vec4 dest = {};
    for (int i = 0; i < 4; i++) {
        dest.xyzw[i] = m.m[i][0] * v.xyzw[0] + m.m[i][1] * v.xyzw[1] + m.m[i][2] * v.xyzw[2] + m.m[i][3] * v.xyzw[3];
    }
    return dest;
}

inline mat4 mat4_mul_list(uint32_t count, ...) {
    va_list ap;
    mat4 m = mat4_identity();
    va_start(ap, count);
    for (uint32_t i = 0; i < count; ++i) {
        m = mat4_mul(m, va_arg(ap, mat4));
    }
    va_end(ap);
    return m;
}

inline void mat4_set_elements(mat4* m, float* elements, uint32_t count) {
    for (u32 i = 0; i < count; ++i) {
        m->elements[i] = elements[i];
    }
}

inline mat4 mat4_ortho_norm(const mat4* m) {
    mat4 r = *m;
    r.v.right = vec4_norm(r.v.right);
    r.v.up = vec4_norm(r.v.up);
    r.v.dir = vec4_norm(r.v.dir);
    return r;
}

inline mat4 mat4_transpose(mat4 m) {
    mat4 t = mat4_identity();

    // First row
    t.elements[0 * 4 + 0] = m.elements[0 * 4 + 0];
    t.elements[1 * 4 + 0] = m.elements[0 * 4 + 1];
    t.elements[2 * 4 + 0] = m.elements[0 * 4 + 2];
    t.elements[3 * 4 + 0] = m.elements[0 * 4 + 3];

    // Second row
    t.elements[0 * 4 + 1] = m.elements[1 * 4 + 0];
    t.elements[1 * 4 + 1] = m.elements[1 * 4 + 1];
    t.elements[2 * 4 + 1] = m.elements[1 * 4 + 2];
    t.elements[3 * 4 + 1] = m.elements[1 * 4 + 3];

    // Third row
    t.elements[0 * 4 + 2] = m.elements[2 * 4 + 0];
    t.elements[1 * 4 + 2] = m.elements[2 * 4 + 1];
    t.elements[2 * 4 + 2] = m.elements[2 * 4 + 2];
    t.elements[3 * 4 + 2] = m.elements[2 * 4 + 3];

    // Fourth row
    t.elements[0 * 4 + 3] = m.elements[3 * 4 + 0];
    t.elements[1 * 4 + 3] = m.elements[3 * 4 + 1];
    t.elements[2 * 4 + 3] = m.elements[3 * 4 + 2];
    t.elements[3 * 4 + 3] = m.elements[3 * 4 + 3];

    return t;
}

inline mat4 mat4_inverse(mat4 m) {
    mat4 res = mat4_identity();

    f32 temp[16];

    temp[0] = m.elements[5] * m.elements[10] * m.elements[15] - m.elements[5] * m.elements[11] * m.elements[14] - m.elements[9] * m.elements[6] * m.elements[15] +
              m.elements[9] * m.elements[7] * m.elements[14] + m.elements[13] * m.elements[6] * m.elements[11] - m.elements[13] * m.elements[7] * m.elements[10];

    temp[4] = -m.elements[4] * m.elements[10] * m.elements[15] + m.elements[4] * m.elements[11] * m.elements[14] + m.elements[8] * m.elements[6] * m.elements[15] -
              m.elements[8] * m.elements[7] * m.elements[14] - m.elements[12] * m.elements[6] * m.elements[11] + m.elements[12] * m.elements[7] * m.elements[10];

    temp[8] = m.elements[4] * m.elements[9] * m.elements[15] - m.elements[4] * m.elements[11] * m.elements[13] - m.elements[8] * m.elements[5] * m.elements[15] +
              m.elements[8] * m.elements[7] * m.elements[13] + m.elements[12] * m.elements[5] * m.elements[11] - m.elements[12] * m.elements[7] * m.elements[9];

    temp[12] = -m.elements[4] * m.elements[9] * m.elements[14] + m.elements[4] * m.elements[10] * m.elements[13] + m.elements[8] * m.elements[5] * m.elements[14] -
               m.elements[8] * m.elements[6] * m.elements[13] - m.elements[12] * m.elements[5] * m.elements[10] + m.elements[12] * m.elements[6] * m.elements[9];

    temp[1] = -m.elements[1] * m.elements[10] * m.elements[15] + m.elements[1] * m.elements[11] * m.elements[14] + m.elements[9] * m.elements[2] * m.elements[15] -
              m.elements[9] * m.elements[3] * m.elements[14] - m.elements[13] * m.elements[2] * m.elements[11] + m.elements[13] * m.elements[3] * m.elements[10];

    temp[5] = m.elements[0] * m.elements[10] * m.elements[15] - m.elements[0] * m.elements[11] * m.elements[14] - m.elements[8] * m.elements[2] * m.elements[15] +
              m.elements[8] * m.elements[3] * m.elements[14] + m.elements[12] * m.elements[2] * m.elements[11] - m.elements[12] * m.elements[3] * m.elements[10];

    temp[9] = -m.elements[0] * m.elements[9] * m.elements[15] + m.elements[0] * m.elements[11] * m.elements[13] + m.elements[8] * m.elements[1] * m.elements[15] -
              m.elements[8] * m.elements[3] * m.elements[13] - m.elements[12] * m.elements[1] * m.elements[11] + m.elements[12] * m.elements[3] * m.elements[9];

    temp[13] = m.elements[0] * m.elements[9] * m.elements[14] - m.elements[0] * m.elements[10] * m.elements[13] - m.elements[8] * m.elements[1] * m.elements[14] +
               m.elements[8] * m.elements[2] * m.elements[13] + m.elements[12] * m.elements[1] * m.elements[10] - m.elements[12] * m.elements[2] * m.elements[9];

    temp[2] = m.elements[1] * m.elements[6] * m.elements[15] - m.elements[1] * m.elements[7] * m.elements[14] - m.elements[5] * m.elements[2] * m.elements[15] +
              m.elements[5] * m.elements[3] * m.elements[14] + m.elements[13] * m.elements[2] * m.elements[7] - m.elements[13] * m.elements[3] * m.elements[6];

    temp[6] = -m.elements[0] * m.elements[6] * m.elements[15] + m.elements[0] * m.elements[7] * m.elements[14] + m.elements[4] * m.elements[2] * m.elements[15] -
              m.elements[4] * m.elements[3] * m.elements[14] - m.elements[12] * m.elements[2] * m.elements[7] + m.elements[12] * m.elements[3] * m.elements[6];

    temp[10] = m.elements[0] * m.elements[5] * m.elements[15] - m.elements[0] * m.elements[7] * m.elements[13] - m.elements[4] * m.elements[1] * m.elements[15] +
               m.elements[4] * m.elements[3] * m.elements[13] + m.elements[12] * m.elements[1] * m.elements[7] - m.elements[12] * m.elements[3] * m.elements[5];

    temp[14] = -m.elements[0] * m.elements[5] * m.elements[14] + m.elements[0] * m.elements[6] * m.elements[13] + m.elements[4] * m.elements[1] * m.elements[14] -
               m.elements[4] * m.elements[2] * m.elements[13] - m.elements[12] * m.elements[1] * m.elements[6] + m.elements[12] * m.elements[2] * m.elements[5];

    temp[3] = -m.elements[1] * m.elements[6] * m.elements[11] + m.elements[1] * m.elements[7] * m.elements[10] + m.elements[5] * m.elements[2] * m.elements[11] -
              m.elements[5] * m.elements[3] * m.elements[10] - m.elements[9] * m.elements[2] * m.elements[7] + m.elements[9] * m.elements[3] * m.elements[6];

    temp[7] = m.elements[0] * m.elements[6] * m.elements[11] - m.elements[0] * m.elements[7] * m.elements[10] - m.elements[4] * m.elements[2] * m.elements[11] +
              m.elements[4] * m.elements[3] * m.elements[10] + m.elements[8] * m.elements[2] * m.elements[7] - m.elements[8] * m.elements[3] * m.elements[6];

    temp[11] = -m.elements[0] * m.elements[5] * m.elements[11] + m.elements[0] * m.elements[7] * m.elements[9] + m.elements[4] * m.elements[1] * m.elements[11] -
               m.elements[4] * m.elements[3] * m.elements[9] - m.elements[8] * m.elements[1] * m.elements[7] + m.elements[8] * m.elements[3] * m.elements[5];

    temp[15] = m.elements[0] * m.elements[5] * m.elements[10] - m.elements[0] * m.elements[6] * m.elements[9] - m.elements[4] * m.elements[1] * m.elements[10] +
               m.elements[4] * m.elements[2] * m.elements[9] + m.elements[8] * m.elements[1] * m.elements[6] - m.elements[8] * m.elements[2] * m.elements[5];

    float determinant = m.elements[0] * temp[0] + m.elements[1] * temp[4] + m.elements[2] * temp[8] + m.elements[3] * temp[12];
    determinant = 1.0f / determinant;

    for (int i = 0; i < 4 * 4; i++) res.elements[i] = (float)(temp[i] * (float)determinant);

    return res;
}

// f32 l : left
// f32 r : right
// f32 b : bottom
// f32 t : top
// f32 n : near
// f32 f : far
inline mat4 mat4_ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) {
    mat4 m_res = mat4_identity();

    // Main diagonal
    m_res.elements[0 + 0 * 4] = 2.0f / (r - l);
    m_res.elements[1 + 1 * 4] = 2.0f / (t - b);
    m_res.elements[2 + 2 * 4] = -2.0f / (f - n);

    // Last column
    m_res.elements[0 + 3 * 4] = -(r + l) / (r - l);
    m_res.elements[1 + 3 * 4] = -(t + b) / (t - b);
    m_res.elements[2 + 3 * 4] = -(f + n) / (f - n);

    return m_res;
}

inline mat4 mat4_perspective(f32 fov, f32 asp_ratio, f32 n, f32 f) {
    // Zero matrix
    mat4 m_res = mat4_ctor();

    f32 q = 1.0f / (float)tan(neko_deg2rad(0.5f * fov));
    f32 a = q / asp_ratio;
    f32 b = (n + f) / (n - f);
    f32 c = (2.0f * n * f) / (n - f);

    m_res.elements[0 + 0 * 4] = a;
    m_res.elements[1 + 1 * 4] = q;
    m_res.elements[2 + 2 * 4] = b;
    m_res.elements[2 + 3 * 4] = c;
    m_res.elements[3 + 2 * 4] = -1.0f;

    return m_res;
}

inline mat4 mat4_translatev(const vec3 v) {
    mat4 m_res = mat4_identity();

    m_res.elements[0 + 4 * 3] = v.x;
    m_res.elements[1 + 4 * 3] = v.y;
    m_res.elements[2 + 4 * 3] = v.z;

    return m_res;
}

inline mat4 mat4_translate(float x, float y, float z) { return mat4_translatev(neko_v3(x, y, z)); }

inline mat4 mat4_scalev(const vec3 v) {
    mat4 m_res = mat4_identity();
    m_res.elements[0 + 0 * 4] = v.x;
    m_res.elements[1 + 1 * 4] = v.y;
    m_res.elements[2 + 2 * 4] = v.z;
    return m_res;
}

inline mat4 mat4_scale(float x, float y, float z) { return (mat4_scalev(neko_v3(x, y, z))); }

// Assumes normalized axis
inline mat4 mat4_rotatev(float angle, vec3 axis) {
    mat4 m_res = mat4_identity();

    float a = angle;
    float c = (float)cos(a);
    float s = (float)sin(a);

    vec3 naxis = vec3_norm(axis);
    float x = naxis.x;
    float y = naxis.y;
    float z = naxis.z;

    // First column
    m_res.elements[0 + 0 * 4] = x * x * (1 - c) + c;
    m_res.elements[1 + 0 * 4] = x * y * (1 - c) + z * s;
    m_res.elements[2 + 0 * 4] = x * z * (1 - c) - y * s;

    // Second column
    m_res.elements[0 + 1 * 4] = x * y * (1 - c) - z * s;
    m_res.elements[1 + 1 * 4] = y * y * (1 - c) + c;
    m_res.elements[2 + 1 * 4] = y * z * (1 - c) + x * s;

    // Third column
    m_res.elements[0 + 2 * 4] = x * z * (1 - c) + y * s;
    m_res.elements[1 + 2 * 4] = y * z * (1 - c) - x * s;
    m_res.elements[2 + 2 * 4] = z * z * (1 - c) + c;

    return m_res;
}

inline mat4 mat4_rotate(float angle, float x, float y, float z) { return mat4_rotatev(angle, neko_v3(x, y, z)); }

inline mat4 mat4_look_at(vec3 position, vec3 target, vec3 up) {
    vec3 f = vec3_norm(vec3_sub(target, position));
    vec3 s = vec3_norm(vec3_cross(f, up));
    vec3 u = vec3_cross(s, f);

    mat4 m_res = mat4_identity();
    m_res.elements[0 * 4 + 0] = s.x;
    m_res.elements[1 * 4 + 0] = s.y;
    m_res.elements[2 * 4 + 0] = s.z;

    m_res.elements[0 * 4 + 1] = u.x;
    m_res.elements[1 * 4 + 1] = u.y;
    m_res.elements[2 * 4 + 1] = u.z;

    m_res.elements[0 * 4 + 2] = -f.x;
    m_res.elements[1 * 4 + 2] = -f.y;
    m_res.elements[2 * 4 + 2] = -f.z;

    m_res.elements[3 * 4 + 0] = -vec3_dot(s, position);
    ;
    m_res.elements[3 * 4 + 1] = -vec3_dot(u, position);
    m_res.elements[3 * 4 + 2] = vec3_dot(f, position);

    return m_res;
}

// Modified from github.com/CedricGuillemet/ImGuizmo/blob/master/ImGuizmo.cpp

inline void mat4_decompose(const mat4* m, float* translation, float* rotation, float* scale) {
    mat4 mat = *m;

    scale[0] = vec4_len(mat.v.right);
    scale[1] = vec4_len(mat.v.up);
    scale[2] = vec4_len(mat.v.dir);

    mat = mat4_ortho_norm(&mat);

    rotation[0] = neko_rad2deg(atan2f(mat.m[1][2], mat.m[2][2]));
    rotation[1] = neko_rad2deg(atan2f(-mat.m[0][2], sqrtf(mat.m[1][2] * mat.m[1][2] + mat.m[2][2] * mat.m[2][2])));
    rotation[2] = neko_rad2deg(atan2f(mat.m[0][1], mat.m[0][0]));

    translation[0] = mat.v.position.x;
    translation[1] = mat.v.position.y;
    translation[2] = mat.v.position.z;
}

// Modified from github.com/CedricGuillemet/ImGuizmo/blob/master/ImGuizmo.cpp

inline mat4 mat4_recompose(const float* translation, const float* rotation, const float* scale) {
    mat4 mat = mat4_identity();

    vec3 direction_unary[3] = {NEKO_XAXIS, NEKO_YAXIS, NEKO_ZAXIS};

    mat4 rot[3] = {mat4_identity(), mat4_identity(), mat4_identity()};
    for (uint32_t i = 0; i < 3; ++i) {
        rot[i] = mat4_rotatev(neko_deg2rad(rotation[i]), direction_unary[i]);
    }

    mat = mat4_mul_list(3, rot[2], rot[1], rot[0]);

    float valid_scale[3] = NEKO_DEFAULT_VAL();
    for (uint32_t i = 0; i < 3; ++i) {
        valid_scale[i] = fabsf(scale[i]) < neko_epsilon ? 0.001f : scale[i];
    }

    mat.v.right = vec4_scale(mat.v.right, valid_scale[0]);
    mat.v.up = vec4_scale(mat.v.up, valid_scale[1]);
    mat.v.dir = vec4_scale(mat.v.dir, valid_scale[2]);
    mat.v.position = neko_v4(translation[0], translation[1], translation[2], 1.f);

    return mat;
}

inline vec4 mat4_mul_vec4(mat4 m, vec4 v) {
    return vec4_ctor(m.elements[0 + 4 * 0] * v.x + m.elements[0 + 4 * 1] * v.y + m.elements[0 + 4 * 2] * v.z + m.elements[0 + 4 * 3] * v.w,
                     m.elements[1 + 4 * 0] * v.x + m.elements[1 + 4 * 1] * v.y + m.elements[1 + 4 * 2] * v.z + m.elements[1 + 4 * 3] * v.w,
                     m.elements[2 + 4 * 0] * v.x + m.elements[2 + 4 * 1] * v.y + m.elements[2 + 4 * 2] * v.z + m.elements[2 + 4 * 3] * v.w,
                     m.elements[3 + 4 * 0] * v.x + m.elements[3 + 4 * 1] * v.y + m.elements[3 + 4 * 2] * v.z + m.elements[3 + 4 * 3] * v.w);
}

inline vec3 mat4_mul_vec3(mat4 m, vec3 v) { return neko_v4tov3(mat4_mul_vec4(m, neko_v4_xyz_s(v, 1.f))); }

// AABBs
/*
    min is top left of rect,
    max is bottom right
*/

typedef struct neko_aabb_t {
    vec2 min;
    vec2 max;
} neko_aabb_t;

// Collision Resolution: Minimum Translation Vector
inline vec2 neko_aabb_aabb_mtv(neko_aabb_t* a0, neko_aabb_t* a1) {
    vec2 diff = neko_v2(a0->min.x - a1->min.x, a0->min.y - a1->min.y);

    f32 l, r, b, t;
    vec2 mtv = neko_v2(0.f, 0.f);

    l = a1->min.x - a0->max.x;
    r = a1->max.x - a0->min.x;
    b = a1->min.y - a0->max.y;
    t = a1->max.y - a0->min.y;

    mtv.x = fabsf(l) > r ? r : l;
    mtv.y = fabsf(b) > t ? t : b;

    if (fabsf(mtv.x) <= fabsf(mtv.y)) {
        mtv.y = 0.f;
    } else {
        mtv.x = 0.f;
    }

    return mtv;
}

// 2D AABB collision detection (rect. vs. rect.)
inline bool neko_aabb_vs_aabb(neko_aabb_t* a, neko_aabb_t* b) {
    if (a->max.x > b->min.x && a->max.y > b->min.y && a->min.x < b->max.x && a->min.y < b->max.y) {
        return true;
    }

    return false;
}

typedef struct rect_t {
    float x, y, w, h;
} rect_t;

#define neko_rect(...) rect_ctor(__VA_ARGS__)

inline rect_t rect_ctor(f32 _x, f32 _y, f32 _w, f32 _h) {
    rect_t v;
    v.x = _x;
    v.y = _y;
    v.w = _w;
    v.h = _h;
    return v;
}

#endif