

#ifndef NEKO_COMMON_H
#define NEKO_COMMON_H

#include "neko.h"

struct neko_hashtable_t {
    void* memctx;
    int count;
    int item_size;

    struct slot_t {
        u32 key_hash;
        int item_index;
        int base_count;
    }* slots;
    int slot_capacity;

    u64* items_key;
    int* items_slot;
    void* items_data;
    int item_capacity;

    void* swap_temp;
};

typedef struct neko_hashtable_t neko_hashtable_t;

NEKO_API_DECL void hashtable_init(neko_hashtable_t* table, int item_size, int initial_capacity, void* memctx);
NEKO_API_DECL void hashtable_term(neko_hashtable_t* table);
NEKO_API_DECL void* hashtable_insert(neko_hashtable_t* table, u64 key, void const* item);
NEKO_API_DECL void hashtable_remove(neko_hashtable_t* table, u64 key);
NEKO_API_DECL void hashtable_clear(neko_hashtable_t* table);
NEKO_API_DECL void* hashtable_find(neko_hashtable_t const* table, u64 key);
NEKO_API_DECL int hashtable_count(neko_hashtable_t const* table);
NEKO_API_DECL void* hashtable_items(neko_hashtable_t const* table);
NEKO_API_DECL u64 const* hashtable_keys(neko_hashtable_t const* table);
NEKO_API_DECL void hashtable_swap(neko_hashtable_t* table, int index_a, int index_b);

/*================================================================================
// Utils
================================================================================*/

NEKO_INLINE u32 neko_abs(s32 v) {
    unsigned int r;
    int const mask = v >> sizeof(int) * CHAR_BIT - 1;
    r = (v + mask) ^ mask;
    return r;
}

// 将UTF-8编码字符转换为Unicode
NEKO_FORCE_INLINE u32 neko_utf8_to_unicode(const_str utf8, s32* bytes_read) {
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

NEKO_FORCE_INLINE bool neko_token_is_end_of_line(char c) { return (c == '\n' || c == '\r'); }
NEKO_FORCE_INLINE bool neko_token_char_is_white_space(char c) { return (c == '\t' || c == ' ' || neko_token_is_end_of_line(c)); }
NEKO_FORCE_INLINE bool neko_token_char_is_alpha(char c) { return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')); }
NEKO_FORCE_INLINE bool neko_token_char_is_numeric(char c) { return (c >= '0' && c <= '9'); }

NEKO_FORCE_INLINE u32 neko_darken_color(u32 color, f32 brightness) {
    s32 a = (color >> 24) & 0xFF;
    s32 r = (s32)(((color >> 16) & 0xFF) * brightness);
    s32 g = (s32)(((color >> 8) & 0xFF) * brightness);
    s32 b = (s32)((color & 0xFF) * brightness);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

NEKO_FORCE_INLINE void neko_tex_flip_vertically(int width, int height, u8* data) {
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
NEKO_API_DECL int64_t neko_rand_gen_long(neko_mt_rand_t* rand);
NEKO_API_DECL double neko_rand_gen(neko_mt_rand_t* rand);
NEKO_API_DECL double neko_rand_gen_range(neko_mt_rand_t* rand, double min, double max);
NEKO_API_DECL int64_t neko_rand_gen_range_long(neko_mt_rand_t* rand, int32_t min, int32_t max);
NEKO_API_DECL neko_color_t neko_rand_gen_color(neko_mt_rand_t* rand);

// xorshf32随机数算法
// http://www.iro.umontreal.ca/~lecuyer/myftp/papers/xorshift.pdf
// xyzw -> [0, 2^32 - 1]

NEKO_INLINE u32 neko_rand_xorshf32(void) {

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
NEKO_INLINE f32 neko_rand_xor() {
    f32 r = (f32)neko_rand_xorshf32();
    r /= neko_rand_xorshf32_max;
    r = 2.0f * r - 1.0f;
    return r;
}

NEKO_INLINE f32 neko_rand_range_xor(f32 lo, f32 hi) {
    f32 r = (f32)neko_rand_xorshf32();
    r /= neko_rand_xorshf32_max;
    r = (hi - lo) * r + lo;
    return r;
}

#endif
