
#ifndef NEKO_BASE_H
#define NEKO_BASE_H

#include "engine/base/base.hpp"

FORMAT_ARGS(1)
inline void neko_panic(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");

    exit(1);
}

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

inline uint8_t wtf8_decode(const char *input, uint32_t *res) {
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

inline size_t wtf8_to_utf16_length(const char *input, size_t length) {
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

inline void wtf8_to_utf16(const char *input, size_t length, wchar_t *output, size_t output_len) {
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

inline uint32_t wtf8_surrogate(const wchar_t *input, bool eof) {
    uint32_t u = input[0];
    if (u >= 0xD800 && u <= 0xDBFF && !eof) {
        uint32_t next = input[1];
        if (next >= 0xDC00 && next <= 0xDFFF) {
            return 0x10000 + ((u - 0xD800) << 10) + (next - 0xDC00);
        }
    }
    return u;
}

inline size_t wtf8_from_utf16_length(const wchar_t *input, size_t length) {
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

inline void wtf8_from_utf16(const wchar_t *input, size_t length, char *output, size_t output_len) {
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

NEKO_FORCE_INLINE u32 neko_string_length(const char *txt) {
    u32 sz = 0;
    while (txt != NULL && txt[sz] != '\0') sz++;
    return sz;
}

#define neko_strlen(str) neko_string_length((const char *)str)

// Expects null terminated strings
NEKO_FORCE_INLINE bool neko_string_compare_equal(const char *txt, const char *cmp) {
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

NEKO_FORCE_INLINE bool neko_string_compare_equal_n(const char *txt, const char *cmp, u32 n) {
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

NEKO_FORCE_INLINE void neko_util_str_to_lower(const char *src, char *buffer, size_t buffer_sz) {
    size_t src_sz = neko_string_length(src);
    size_t len = NEKO_MIN(src_sz, buffer_sz - 1);

    for (u32 i = 0; i < len; ++i) {
        buffer[i] = tolower(src[i]);
    }
    if (len) buffer[len] = '\0';
}

NEKO_FORCE_INLINE bool neko_util_str_is_numeric(const char *str) {
    const char *at = str;
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

NEKO_FORCE_INLINE void neko_util_get_dir_from_file(char *buffer, u32 buffer_size, const char *file_path) {
    u32 str_len = neko_string_length(file_path);
    const char *end = (file_path + str_len);
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

NEKO_FORCE_INLINE void neko_util_string_substring(const char *src, char *dst, size_t sz, u32 start, u32 end) {
    u32 str_len = neko_string_length(src);
    if (end > str_len) {
        end = str_len;
    }
    if (start > str_len) {
        start = str_len;
    }

    const char *at = src + start;
    const char *e = src + end;
    u32 ct = 0;
    while (at && *at != '\0' && at != e) {
        dst[ct] = *at;
        at++;
        ct++;
    }
}

NEKO_FORCE_INLINE void neko_util_string_remove_character(const char *src, char *buffer, u32 buffer_size, char delimiter) {
    u32 ct = 0;
    u32 str_len = neko_string_length(src);
    const char *at = src;
    while (at && *at != '\0' && ct < buffer_size) {
        char c = *at;
        if (c != delimiter) {
            buffer[ct] = c;
            ct++;
        }
        at++;
    }
}

NEKO_FORCE_INLINE void neko_util_string_replace(char *buffer, size_t buffer_sz, const char *replace, char fallback) {
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

NEKO_FORCE_INLINE void neko_util_string_replace_delim(const char *source_str, char *buffer, u32 buffer_size, char delimiter, char replace) {
    u32 str_len = neko_string_length(source_str);
    const char *at = source_str;
    while (at && *at != '\0') {
        char c = *at;
        if (c == delimiter) {
            c = replace;
        }
        buffer[(at - source_str)] = c;
        at++;
    }
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
NEKO_FORCE_INLINE u32 neko_hash_str(const char *str) {
    u32 hash = 5381;
    i32 c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

NEKO_FORCE_INLINE u64 neko_hash_str64(const char *str) {
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

NEKO_FORCE_INLINE bool neko_compare_bytes(void *b0, void *b1, size_t len) { return 0 == memcmp(b0, b1, len); }

// Hash generic bytes using (ripped directly from Sean Barret's stb_ds.h)
#define NEKO_SIZE_T_BITS ((sizeof(size_t)) * 8)
#define NEKO_SIPHASH_C_ROUNDS 1
#define NEKO_SIPHASH_D_ROUNDS 1
#define neko_rotate_left(__V, __N) (((__V) << (__N)) | ((__V) >> (NEKO_SIZE_T_BITS - (__N))))
#define neko_rotate_right(__V, __N) (((__V) >> (__N)) | ((__V) << (NEKO_SIZE_T_BITS - (__N))))

NEKO_FORCE_INLINE size_t neko_hash_siphash_bytes(void *p, size_t len, size_t seed) {
    unsigned char *d = (unsigned char *)p;
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

NEKO_FORCE_INLINE size_t neko_hash_bytes(void *p, size_t len, size_t seed) {
#if 0
  return neko_hash_siphash_bytes(p,len,seed);
#else
    unsigned char *d = (unsigned char *)p;

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

#if 1

// 在内存中存储连续的对象
// 对象可能会在内存中移动 因此不要依赖指向 CArray 中对象的指针
typedef struct CArray CArray;

CArray *array_new_(size_t object_size);  // object_size 是每个元素的大小
void array_free(CArray *arr);
void *array_get(CArray *arr, unsigned int i);
void *array_top(CArray *arr);
unsigned int array_length(CArray *arr);  // 数组中的对象数

// 添加/删除可能会更改开始/结束 因此在迭代时要小心
void *array_begin(CArray *arr);                                           // 指向第一个元素的指针
void *array_end(CArray *arr);                                             // 指向one past last元素的指针
void *array_add(CArray *arr);                                             // 添加新对象, index is length - 1
void array_reset(CArray *arr, unsigned int num);                          // 将数据未定义的对象调整为 num 对象
void array_pop(CArray *arr);                                              // 删除具有最高索引的对象
bool array_quick_remove(CArray *arr, unsigned int i);                     // 快速删除, 可以将其他一些元素交换成 arr[i] 如果是这样 则返回 true
void array_sort(CArray *arr, int (*compar)(const void *, const void *));  // compare 是一个比较器函数

#define array_clear(arr) array_reset(arr, 0)  //
#define array_add_val(type, arr) (*((type *)array_add(arr)))
#define array_top_val(type, arr) (*((type *)array_top(arr)))
#define array_get_val(type, arr, i) (*((type *)array_get(arr, i)))
#define array_new(type) array_new_(sizeof(type))

#define array_foreach(var, arr) for (void *__end = (var = (decltype(var))array_begin(arr), array_end(arr)); var != __end; ++var)

#endif

#include "engine/base/math.hpp"

NEKO_SCRIPT(saveload,

            // 请记住 *_close(...) 当完成以释放资源时

            typedef struct Store Store;

            NEKO_EXPORT Store * store_open();

            NEKO_EXPORT Store * store_open_str(const char *str);

            NEKO_EXPORT const char *store_write_str(Store *s);

            NEKO_EXPORT Store * store_open_file(const char *filename);

            NEKO_EXPORT void store_write_file(Store *s, const char *filename);

            NEKO_EXPORT void store_close(Store *s);

)

// 存储树有助于向后兼容 save/load
bool store_child_save(Store **sp, const char *name, Store *parent);
bool store_child_save_compressed(Store **sp, const char *name, Store *parent);
bool store_child_load(Store **sp, const char *name, Store *parent);
void scalar_save(const Scalar *f, const char *name, Store *s);
bool scalar_load(Scalar *f, const char *name, Scalar d, Store *s);
void uint_save(const unsigned int *u, const char *name, Store *s);
bool uint_load(unsigned int *u, const char *name, unsigned int d, Store *s);
void int_save(const int *i, const char *name, Store *s);
bool int_load(int *i, const char *name, int d, Store *s);

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
        *((int *)val) = e__;            \
    } while (0)

void bool_save(const bool *b, const char *name, Store *s);
bool bool_load(bool *b, const char *name, bool d, Store *s);

void string_save(const char **c, const char *name, Store *s);
bool string_load(char **c, const char *name, const char *d, Store *s);

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

    void *payload;
    u32 payload_size;

    i64 modtime;

    char *file_name;
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

        NEKO_EXPORT size_t neko_capi_vfs_fread(void *dest, size_t size, size_t count, vfs_file *vf);

        NEKO_EXPORT int neko_capi_vfs_fseek(vfs_file *vf, u64 of, int whence);

        NEKO_EXPORT u64 neko_capi_vfs_ftell(vfs_file * vf);

        NEKO_EXPORT vfs_file neko_capi_vfs_fopen(const_str path);

        NEKO_EXPORT int neko_capi_vfs_fclose(vfs_file *vf);

        NEKO_EXPORT int neko_capi_vfs_fscanf(vfs_file *vf, const char *format, ...);

        NEKO_EXPORT bool neko_capi_vfs_file_exists(const_str fsname, const_str filepath);

        NEKO_EXPORT const_str neko_capi_vfs_read_file(const_str fsname, const_str filepath, size_t *size);

)

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

#endif

#if !defined(NEKO_BASE_HPP)
#define NEKO_BASE_HPP

#include "engine/base/arena.hpp"
#include "engine/base/array.hpp"
#include "engine/base/hashmap.hpp"
#include "engine/base/mem.hpp"
#include "engine/base/mutex.hpp"
#include "engine/base/queue.hpp"
#include "engine/base/string.hpp"
#include "engine/base/util.hpp"

/*=============================
//
=============================*/

// #include <intrin.h>

#include <stdint.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

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

namespace detail {
template <typename... Args>
constexpr std::size_t va_count(Args &&...) {
    return sizeof...(Args);
}
}  // namespace detail

#define neko_macro_overload(fun, a, ...)                                               \
    do {                                                                               \
        if (const bool a_ = (a); a_)                                                   \
            [&](auto &&...args) {                                                      \
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

// 一种向任何指针添加字节偏移量的可移植且安全的方法
// https://stackoverflow.com/questions/15934111/portable-and-safe-way-to-add-byte-offset-to-any-pointer
template <typename T>
inline void neko_addoffset(std::ptrdiff_t offset, T *&ptr) {
    if (!ptr) return;
    ptr = (T *)((unsigned char *)ptr + offset);
}

template <typename T>
inline T *neko_addoffset_r(std::ptrdiff_t offset, T *ptr) {
    if (!ptr) return nullptr;
    return (T *)((unsigned char *)ptr + offset);
}

struct lua_State;

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
    return (typename cpp_remove_reference<T>::type &&)arg;
}

template <typename T>
using initializer_list = std::initializer_list<T>;

template <typename T>
using function = std::function<T>;

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
concept concept_is_pair = requires(T t) {
    t.first;
    t.second;
};

template <class T>
struct is_pair : public std::false_type {};

template <class T1, class T2>
struct is_pair<std::pair<T1, T2>> : public std::true_type {};

template <class>
inline constexpr bool always_false = false;

}  // namespace neko

#define NEKO_VA_UNPACK(...) __VA_ARGS__  // 用于解包括号 带逗号的宏参数需要它

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

static inline std::string fs_normalize_path(const std::string &path, char delimiter = '/') {
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

}  // namespace neko

namespace neko::wtf8 {
std::wstring u2w(std::string_view str) noexcept;
std::string w2u(std::wstring_view wstr) noexcept;
}  // namespace neko::wtf8

template <typename F>
struct function_traits : public function_traits<decltype(&F::operator())> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType (ClassType::*)(Args...) const> {
    using return_type = ReturnType;
    using pointer = ReturnType (*)(Args...);
    using std_function = std::function<ReturnType(Args...)>;
};

template <typename F>
typename function_traits<F>::std_function to_function(F &lambda) {
    return typename function_traits<F>::std_function(lambda);
}

namespace neko {

struct format_str {
    constexpr format_str(const char *str) noexcept : str(str) {}
    const char *str;
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
struct lastFnType<function<T2(T1)>> {
    using type = T1;
};

template <typename T1, typename T2>
function<T1(T2)> func_combine(function<T1(T2)> conv) {
    return conv;
}

template <typename T1, typename T2, typename T3, typename... Fn>
auto func_combine(function<T1(T2)> conv1, function<T2(T3)> conv2, Fn... fn) -> function<T1(typename lastFnType<function<T2(T3)>, Fn...>::type)> {
    using In = typename lastFnType<function<T2(T3)>, Fn...>::type;

    return [=](In const &in) { return conv1(func_combine(conv2, fn...)(in)); };
}

template <typename T>
struct tuple_size;

template <typename... Args>
struct tuple_size<std::tuple<Args...>> {
    static constexpr std::size_t value = sizeof...(Args);
};

template <typename T>
constexpr std::size_t tuple_size_v = tuple_size<T>::value;

template <typename T, std::size_t N>
constexpr bool is_pointer_to_const_char(T (&)[N]) {
    return std::is_same_v<const char, T>;
}

template <typename T>
constexpr bool is_pointer_to_const_char(T &&) {
    return std::is_same_v<const char *, T>;
}

template <typename T>
struct is_vector : std::false_type {};

template <typename T, typename Alloc>
struct is_vector<std::vector<T, Alloc>> : std::true_type {};

}  // namespace neko

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
struct is_mappish_impl<T, void_t<typename T::key_type, typename T::mapped_type, decltype(std::declval<T &>()[std::declval<const typename T::key_type &>()])>> : std::true_type {};
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

inline uint64_t hash_fnv(const void *data, int size) {
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

constexpr hash_value hash(char const *input) { return hash_fnv(input); }
constexpr hash_value hash(const std::string &input) { return hash_fnv(input.c_str()); }

template <typename ForwardIterator, typename SpaceDetector>
constexpr ForwardIterator find_terminating_word(ForwardIterator begin, ForwardIterator end, SpaceDetector &&is_space_pred) {
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

template <std::size_t N>
constexpr auto BitSet(const char (&str)[N]) {
    std::bitset<N - 1> bits;  // N - 1 是因为包含 null 终止符
    for (std::size_t i = 0; i < N - 1; ++i) {
        bits.set(i, (str[i] == '1'));
    }
    return bits;
}

template <std::size_t N>
constexpr auto BitSet(const char *str) {
    std::bitset<N> bits;
    std::size_t len = std::strlen(str);  // 计算字符串长度
    for (std::size_t i = 0; i < len && i < bits.size(); ++i) {
        bits.set(i, (str[i] == '1'));
    }
    return bits;
}

}  // namespace neko

typedef struct engine_cfg_t {
    bool show_editor;
    bool show_demo_window;
    bool show_gui;
    bool shader_inspect;
    bool hello_ai_shit;
    bool vsync;
    bool is_hotfix;

    String title;
    String game_proxy;
    String default_font;

    bool hot_reload;
    bool startup_load_scripts;
    bool fullscreen;

    bool debug_on;

    f32 reload_interval;
    f32 swap_interval;
    f32 target_fps;

    i32 batch_vertex_capacity;

    bool dump_allocs_detailed;

    f32 width;
    f32 height;

    f32 bg[3];  //
} neko_client_cvar_t;

void neko_cvar_gui(neko_client_cvar_t &cvar);

namespace neko {}  // namespace neko

#endif

#ifndef NEKO_REFL_HPP
#define NEKO_REFL_HPP

#include <cstddef>
#include <functional>
#include <span>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

namespace neko::reflection {

struct Type;

class Any {
    Type *type;    // type info, similar to vtable
    void *data;    // pointer to the data
    uint8_t flag;  // special flag

public:
    Any() : type(nullptr), data(nullptr), flag(0) {}

    Any(Type *type, void *data) : type(type), data(data), flag(0B00000001) {}

    Any(const Any &other);
    Any(Any &&other);
    ~Any();

    template <typename T>
    Any(T &&value);  // box value to Any

    template <typename T>
    T &cast();  // unbox Any to value

    Type *GetType() const { return type; }  // get type info

    Any invoke(std::string_view name, std::span<Any> args);  // call method

    void foreach (const std::function<void(std::string_view, Any &)> &fn);  // iterate fields
};

struct Type {
    std::string_view name;        // type name
    void (*destroy)(void *);      // destructor
    void *(*copy)(const void *);  // copy constructor
    void *(*move)(void *);        // move constructor

    using Field = std::pair<Type *, std::size_t>;          // type and offset
    using Method = Any (*)(void *, std::span<Any>);        // method
    std::unordered_map<std::string_view, Field> fields;    // field info
    std::unordered_map<std::string_view, Method> methods;  // method info
};

template <typename T>
Type *type_of();  // type_of<T> returns type info of T

template <typename T>
T &Any::cast() {
    if (type != type_of<T>()) {
        throw std::runtime_error{"type mismatch"};
    }
    return *static_cast<T *>(data);
}

template <typename T>
struct member_fn_traits;

template <typename R, typename C, typename... Args>
struct member_fn_traits<R (C::*)(Args...)> {
    using return_type = R;
    using class_type = C;
    using args_type = std::tuple<Args...>;
};

template <auto ptr>
auto *type_ensure() {
    using traits = member_fn_traits<decltype(ptr)>;
    using class_type = typename traits::class_type;
    using result_type = typename traits::return_type;
    using args_type = typename traits::args_type;

    return +[](void *object, std::span<Any> args) -> Any {
        auto self = static_cast<class_type *>(object);
        return [=]<std::size_t... Is>(std::index_sequence<Is...>) {
            if constexpr (std::is_void_v<result_type>) {
                (self->*ptr)(args[Is].cast<std::tuple_element_t<Is, args_type>>()...);
                return Any{};
            } else {
                return Any{(self->*ptr)(args[Is].cast<std::tuple_element_t<Is, args_type>>()...)};
            }
        }(std::make_index_sequence<std::tuple_size_v<args_type>>{});
    };
}

// template <typename T>
// Type* type_of() {
//     static Type type;
//     type.name = typeid(T).name();
//     type.destroy = [](void* obj) { delete static_cast<T*>(obj); };
//     type.copy = [](const void* obj) { return (void*)(new T(*static_cast<const T*>(obj))); };
//     type.move = [](void* obj) { return (void*)(new T(std::move(*static_cast<T*>(obj)))); };
//     return &type;
// }

inline Any::Any(const Any &other) {
    type = other.type;
    data = type->copy(other.data);
    flag = 0;
}

inline Any::Any(Any &&other) {
    type = other.type;
    data = type->move(other.data);
    flag = 0;
}

template <typename T>
Any::Any(T &&value) {
    type = type_of<std::decay_t<T>>();
    data = new std::decay_t<T>(std::forward<T>(value));
    flag = 0;
}

inline Any::~Any() {
    if (!(flag & 0B00000001) && data && type) {
        type->destroy(data);
    }
}

inline void Any::foreach (const std::function<void(std::string_view, Any &)> &fn) {
    for (auto &[name, field] : type->fields) {
        Any any = Any{field.first, static_cast<char *>(data) + field.second};
        fn(name, any);
    }
}

inline Any Any::invoke(std::string_view name, std::span<Any> args) {
    auto it = type->methods.find(name);
    if (it == type->methods.end()) {
        throw std::runtime_error{"method not found"};
    }
    return it->second(data, args);
}

}  // namespace neko::reflection

#define REGISTER_TYPE_DF(C, ...)                                                                   \
    namespace neko::reflection {                                                                   \
    template <>                                                                                    \
    Type *type_of<C>() {                                                                           \
        static Type type;                                                                          \
        type.name = #C;                                                                            \
        type.destroy = [](void *obj) { delete static_cast<C *>(obj); };                            \
        type.copy = [](const void *obj) { return (void *)(new C(*static_cast<const C *>(obj))); }; \
        type.move = [](void *obj) { return (void *)(new C(std::move(*static_cast<C *>(obj)))); };  \
        __VA_ARGS__                                                                                \
        return &type;                                                                              \
    };                                                                                             \
    }

#endif  // NEKO_ENGINE_NEKO_REFL_HPP
