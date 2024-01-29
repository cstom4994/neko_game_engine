
#ifndef NEKO_ASEPRITE_H
#define NEKO_ASEPRITE_H

#include "engine/neko.h"

typedef struct ase_t ase_t;

NEKO_API_DECL ase_t* neko_aseprite_load_from_file(const char* path);
NEKO_API_DECL ase_t* neko_aseprite_load_from_memory(const void* memory, int size);
NEKO_API_DECL void neko_aseprite_free(ase_t* aseprite);
NEKO_API_DECL void neko_aseprite_default_blend_bind(ase_t* aseprite);  // 默认图块混合管线

#define __NEKO_ASEPRITE_MAX_LAYERS (64)
#define __NEKO_ASEPRITE_MAX_SLICES (128)
#define __NEKO_ASEPRITE_MAX_PALETTE_ENTRIES (1024)
#define __NEKO_ASEPRITE_MAX_TAGS (256)

typedef struct ase_frame_t ase_frame_t;
typedef struct ase_layer_t ase_layer_t;
typedef struct ase_cel_t ase_cel_t;
typedef struct ase_tag_t ase_tag_t;
typedef struct ase_slice_t ase_slice_t;
typedef struct ase_palette_entry_t ase_palette_entry_t;
typedef struct ase_palette_t ase_palette_t;
typedef struct ase_udata_t ase_udata_t;
typedef struct ase_cel_extra_chunk_t ase_cel_extra_chunk_t;
typedef struct ase_color_profile_t ase_color_profile_t;
typedef struct ase_fixed_t ase_fixed_t;
typedef struct ase_cel_extra_chunk_t ase_cel_extra_chunk_t;

struct ase_fixed_t {
    u16 a;
    u16 b;
};

struct ase_udata_t {
    int has_color;
    neko_color_t color;
    int has_text;
    const char* text;
};

typedef enum ase_layer_flags_t {
    NEKO_ASE_LAYER_FLAGS_VISIBLE = 0x01,
    NEKO_ASE_LAYER_FLAGS_EDITABLE = 0x02,
    NEKO_ASE_LAYER_FLAGS_LOCK_MOVEMENT = 0x04,
    NEKO_ASE_LAYER_FLAGS_BACKGROUND = 0x08,
    NEKO_ASE_LAYER_FLAGS_PREFER_LINKED_CELS = 0x10,
    NEKO_ASE_LAYER_FLAGS_COLLAPSED = 0x20,
    NEKO_ASE_LAYER_FLAGS_REFERENCE = 0x40,
} ase_layer_flags_t;

typedef enum ase_layer_type_t {
    NEKO_ASE_LAYER_TYPE_NORMAL,
    NEKO_ASE_LAYER_TYPE_GROUP,
    NEKO_ASE_LAYER_TYPE_TILEMAP,
} ase_layer_type_t;

struct ase_layer_t {
    ase_layer_flags_t flags;
    ase_layer_type_t type;
    const char* name;
    ase_layer_t* parent;
    float opacity;
    ase_udata_t udata;
};

struct ase_cel_extra_chunk_t {
    int precise_bounds_are_set;
    ase_fixed_t precise_x;
    ase_fixed_t precise_y;
    ase_fixed_t w, h;
};

struct ase_cel_t {
    ase_layer_t* layer;
    void* cel_pixels;  // 图块的pixels数据是唯一的
    int w, h;
    int x, y;
    float opacity;
    int is_linked;
    u16 linked_frame_index;
    int has_extra;
    ase_cel_extra_chunk_t extra;
    ase_udata_t udata;
};

struct ase_frame_t {
    ase_t* ase;
    int duration_milliseconds;
    int pixel_count;
    neko_color_t* pixels[__NEKO_ASEPRITE_MAX_LAYERS];  // 支持每层一个pixels数据
    int cel_count;
    ase_cel_t cels[__NEKO_ASEPRITE_MAX_LAYERS];
};

typedef enum ase_animation_direction_t {
    NEKO_ASE_ANIMATION_DIRECTION_FORWARDS,
    NEKO_ASE_ANIMATION_DIRECTION_BACKWORDS,
    NEKO_ASE_ANIMATION_DIRECTION_PINGPONG,
} ase_animation_direction_t;

struct ase_tag_t {
    int from_frame;
    int to_frame;
    ase_animation_direction_t loop_animation_direction;
    u8 r, g, b;
    const char* name;
    ase_udata_t udata;
};

struct ase_slice_t {
    const char* name;
    int frame_number;
    int origin_x;
    int origin_y;
    int w, h;

    int has_center_as_9_slice;
    int center_x;
    int center_y;
    int center_w;
    int center_h;

    int has_pivot;
    int pivot_x;
    int pivot_y;

    ase_udata_t udata;
};

struct ase_palette_entry_t {
    neko_color_t color;
    const char* color_name;
};

struct ase_palette_t {
    int entry_count;
    ase_palette_entry_t entries[__NEKO_ASEPRITE_MAX_PALETTE_ENTRIES];
};

typedef enum ase_color_profile_type_t {
    NEKO_ASE_COLOR_PROFILE_TYPE_NONE,
    NEKO_ASE_COLOR_PROFILE_TYPE_SRGB,
    NEKO_ASE_COLOR_PROFILE_TYPE_EMBEDDED_ICC,
} ase_color_profile_type_t;

struct ase_color_profile_t {
    ase_color_profile_type_t type;
    int use_fixed_gamma;
    ase_fixed_t gamma;
    u32 icc_profile_data_length;
    void* icc_profile_data;
};

typedef enum ase_mode_t { NEKO_ASE_MODE_RGBA, NEKO_ASE_MODE_GRAYSCALE, NEKO_ASE_MODE_INDEXED } ase_mode_t;

struct ase_t {
    ase_mode_t mode;
    int w, h;
    int transparent_palette_entry_index;
    int number_of_colors;
    int pixel_w;
    int pixel_h;
    int grid_x;
    int grid_y;
    int grid_w;
    int grid_h;
    int has_color_profile;
    ase_color_profile_t color_profile;
    ase_palette_t palette;

    int layer_count;
    ase_layer_t layers[__NEKO_ASEPRITE_MAX_LAYERS];

    int frame_count;
    ase_frame_t* frames;

    int tag_count;
    ase_tag_t tags[__NEKO_ASEPRITE_MAX_TAGS];

    int slice_count;
    ase_slice_t slices[__NEKO_ASEPRITE_MAX_SLICES];
};

neko_inline int s_mul_un8(int a, int b) {
    int t = (a * b) + 0x80;
    return (((t >> 8) + t) >> 8);
}

neko_inline neko_color_t s_blend(neko_color_t src, neko_color_t dst, u8 opacity) {
    src.a = (u8)s_mul_un8(src.a, opacity);
    int a = src.a + dst.a - s_mul_un8(src.a, dst.a);
    int r, g, b;
    if (a == 0) {
        r = g = b = 0;
    } else {
        r = dst.r + (src.r - dst.r) * src.a / a;
        g = dst.g + (src.g - dst.g) * src.a / a;
        b = dst.b + (src.b - dst.b) * src.a / a;
    }
    neko_color_t ret = {(u8)r, (u8)g, (u8)b, (u8)a};
    return ret;
}

neko_inline neko_color_t s_color(ase_t* ase, void* src, int index) {
    neko_color_t result;
    if (ase->mode == NEKO_ASE_MODE_RGBA) {
        result = ((neko_color_t*)src)[index];
    } else if (ase->mode == NEKO_ASE_MODE_GRAYSCALE) {
        u8 saturation = ((u8*)src)[index * 2];
        u8 a = ((u8*)src)[index * 2 + 1];
        result.r = result.g = result.b = saturation;
        result.a = a;
    } else {
        neko_assert(ase->mode == NEKO_ASE_MODE_INDEXED);
        u8 palette_index = ((u8*)src)[index];
        if (palette_index == ase->transparent_palette_entry_index) {
            result = neko_color_ctor(0, 0, 0, 0);
        } else {
            result = ase->palette.entries[palette_index].color;
        }
    }
    return result;
}

#endif  // NEKO_ASEPRITE_H

#if defined(NEKO_IMPL)

neko_global const_str s_error_file = NULL;  // 正在解析的文件的文件路径 如果来自内存则为 NULL
neko_global const_str s_error_reason;       // 用于捕获 DEFLATE 解析期间的错误

#define __NEKO_ASEPRITE_FAIL() \
    do {                       \
        goto ase_err;          \
    } while (0)

#define __NEKO_ASEPRITE_CHECK(X, Y) \
    do {                            \
        if (!(X)) {                 \
            s_error_reason = Y;     \
            __NEKO_ASEPRITE_FAIL(); \
        }                           \
    } while (0)

#define __NEKO_ASEPRITE_CALL(X) \
    do {                        \
        if (!(X)) goto ase_err; \
    } while (0)

#define __NEKO_ASEPRITE_DEFLATE_MAX_BITLEN 15

// DEFLATE tables from RFC 1951
neko_global u8 s_fixed_table[288 + 32] = {
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
};                                                                                                                                                                                           // 3.2.6
neko_global u8 s_permutation_order[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};                                                                                 // 3.2.7
neko_global u8 s_len_extra_bits[29 + 2] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 0, 0};                                                     // 3.2.5
neko_global u32 s_len_base[29 + 2] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};                              // 3.2.5
neko_global u8 s_dist_extra_bits[30 + 2] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 0, 0};                                         // 3.2.5
neko_global u32 s_dist_base[30 + 2] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577, 0, 0};  // 3.2.5

typedef struct deflate_t {
    u64 bits;
    int count;
    u32* words;
    int word_count;
    int word_index;
    int bits_left;

    int final_word_available;
    u32 final_word;

    char* out;
    char* out_end;
    char* begin;

    u32 lit[288];
    u32 dst[32];
    u32 len[19];
    u32 nlit;
    u32 ndst;
    u32 nlen;
} deflate_t;

neko_global int s_would_overflow(deflate_t* s, int num_bits) { return (s->bits_left + s->count) - num_bits < 0; }

neko_global char* s_ptr(deflate_t* s) {
    neko_assert(!(s->bits_left & 7));
    return (char*)(s->words + s->word_index) - (s->count / 8);
}

neko_global u64 s_peak_bits(deflate_t* s, int num_bits_to_read) {
    if (s->count < num_bits_to_read) {
        if (s->word_index < s->word_count) {
            u32 word = s->words[s->word_index++];
            s->bits |= (u64)word << s->count;
            s->count += 32;
            neko_assert(s->word_index <= s->word_count);
        }

        else if (s->final_word_available) {
            u32 word = s->final_word;
            s->bits |= (u64)word << s->count;
            s->count += s->bits_left;
            s->final_word_available = 0;
        }
    }

    return s->bits;
}

neko_global u32 s_consume_bits(deflate_t* s, int num_bits_to_read) {
    neko_assert(s->count >= num_bits_to_read);
    u32 bits = (u32)(s->bits & (((u64)1 << num_bits_to_read) - 1));
    s->bits >>= num_bits_to_read;
    s->count -= num_bits_to_read;
    s->bits_left -= num_bits_to_read;
    return bits;
}

neko_global u32 s_read_bits(deflate_t* s, int num_bits_to_read) {
    neko_assert(num_bits_to_read <= 32);
    neko_assert(num_bits_to_read >= 0);
    neko_assert(s->bits_left > 0);
    neko_assert(s->count <= 64);
    neko_assert(!s_would_overflow(s, num_bits_to_read));
    s_peak_bits(s, num_bits_to_read);
    u32 bits = s_consume_bits(s, num_bits_to_read);
    return bits;
}

neko_global u32 s_rev16(u32 a) {
    a = ((a & 0xAAAA) >> 1) | ((a & 0x5555) << 1);
    a = ((a & 0xCCCC) >> 2) | ((a & 0x3333) << 2);
    a = ((a & 0xF0F0) >> 4) | ((a & 0x0F0F) << 4);
    a = ((a & 0xFF00) >> 8) | ((a & 0x00FF) << 8);
    return a;
}

// RFC 1951 section 3.2.2
neko_global u32 s_build(deflate_t* s, u32* tree, u8* lens, int sym_count) {
    int n, codes[16], first[16], counts[16] = {0};
    neko_unused(s);

    // Frequency count
    for (n = 0; n < sym_count; n++) counts[lens[n]]++;

    // Distribute codes
    counts[0] = codes[0] = first[0] = 0;
    for (n = 1; n <= 15; ++n) {
        codes[n] = (codes[n - 1] + counts[n - 1]) << 1;
        first[n] = first[n - 1] + counts[n - 1];
    }

    for (int i = 0; i < sym_count; ++i) {
        u8 len = lens[i];

        if (len != 0) {
            neko_assert(len < 16);
            u32 code = (u32)codes[len]++;
            u32 slot = (u32)first[len]++;
            tree[slot] = (code << (32 - (u32)len)) | (i << 4) | len;
        }
    }

    return (u32)first[15];
}

neko_global int s_stored(deflate_t* s) {
    char* p;

    // 3.2.3
    // skip any remaining bits in current partially processed byte
    s_read_bits(s, s->count & 7);

    // 3.2.4
    // read LEN and NLEN, should complement each other
    u16 LEN = (u16)s_read_bits(s, 16);
    u16 NLEN = (u16)s_read_bits(s, 16);
    __NEKO_ASEPRITE_CHECK(LEN == (u16)(~NLEN), "Failed to find LEN and NLEN as complements within stored (uncompressed) stream.");
    __NEKO_ASEPRITE_CHECK(s->bits_left / 8 <= (int)LEN, "Stored block extends beyond end of input stream.");
    p = s_ptr(s);
    memcpy(s->out, p, LEN);
    s->out += LEN;
    return 1;

ase_err:
    return 0;
}

// 3.2.6
neko_global int s_fixed(deflate_t* s) {
    s->nlit = s_build(s, s->lit, s_fixed_table, 288);
    s->ndst = s_build(0, s->dst, s_fixed_table + 288, 32);
    return 1;
}

neko_global int s_decode(deflate_t* s, u32* tree, int hi) {
    u64 bits = s_peak_bits(s, 16);
    u32 search = (s_rev16((u32)bits) << 16) | 0xFFFF;
    int lo = 0;
    while (lo < hi) {
        int guess = (lo + hi) >> 1;
        if (search < tree[guess])
            hi = guess;
        else
            lo = guess + 1;
    }

    u32 key = tree[lo - 1];
    u32 len = (32 - (key & 0xF));
    neko_assert((search >> len) == (key >> len));

    s_consume_bits(s, key & 0xF);
    return (key >> 4) & 0xFFF;
}

// 3.2.7
neko_global int s_dynamic(deflate_t* s) {
    u8 lenlens[19] = {0};

    u32 nlit = 257 + s_read_bits(s, 5);
    u32 ndst = 1 + s_read_bits(s, 5);
    u32 nlen = 4 + s_read_bits(s, 4);

    for (u32 i = 0; i < nlen; ++i) lenlens[s_permutation_order[i]] = (u8)s_read_bits(s, 3);

    // Build the tree for decoding code lengths
    s->nlen = s_build(0, s->len, lenlens, 19);
    u8 lens[288 + 32];

    for (u32 n = 0; n < nlit + ndst;) {
        int sym = s_decode(s, s->len, (int)s->nlen);
        switch (sym) {
            case 16:
                for (u32 i = 3 + s_read_bits(s, 2); i; --i, ++n) lens[n] = lens[n - 1];
                break;
            case 17:
                for (u32 i = 3 + s_read_bits(s, 3); i; --i, ++n) lens[n] = 0;
                break;
            case 18:
                for (u32 i = 11 + s_read_bits(s, 7); i; --i, ++n) lens[n] = 0;
                break;
            default:
                lens[n++] = (u8)sym;
                break;
        }
    }

    s->nlit = s_build(s, s->lit, lens, (int)nlit);
    s->ndst = s_build(0, s->dst, lens + nlit, (int)ndst);
    return 1;
}

// 3.2.3
neko_global int s_block(deflate_t* s) {
    while (1) {
        int symbol = s_decode(s, s->lit, (int)s->nlit);

        if (symbol < 256) {
            __NEKO_ASEPRITE_CHECK(s->out + 1 <= s->out_end, "Attempted to overwrite out buffer while outputting a symbol.");
            *s->out = (char)symbol;
            s->out += 1;
        }

        else if (symbol > 256) {
            symbol -= 257;
            u32 length = s_read_bits(s, (int)(s_len_extra_bits[symbol])) + s_len_base[symbol];
            int distance_symbol = s_decode(s, s->dst, (int)s->ndst);
            u32 backwards_distance = s_read_bits(s, s_dist_extra_bits[distance_symbol]) + s_dist_base[distance_symbol];
            __NEKO_ASEPRITE_CHECK(s->out - backwards_distance >= s->begin, "Attempted to write before out buffer (invalid backwards distance).");
            __NEKO_ASEPRITE_CHECK(s->out + length <= s->out_end, "Attempted to overwrite out buffer while outputting a string.");
            char* src = s->out - backwards_distance;
            char* dst = s->out;
            s->out += length;

            switch (backwards_distance) {
                case 1:  // very common in images
                    memset(dst, *src, (size_t)length);
                    break;
                default:
                    while (length--) *dst++ = *src++;
            }
        }

        else
            break;
    }

    return 1;

ase_err:
    return 0;
}

// 3.2.3
neko_global int s_inflate(const void* in, int in_bytes, void* out, int out_bytes) {
    deflate_t* s = (deflate_t*)neko_safe_malloc(sizeof(deflate_t));
    s->bits = 0;
    s->count = 0;
    s->word_index = 0;
    s->bits_left = in_bytes * 8;

    // s->words is the in-pointer rounded up to a multiple of 4
    int first_bytes = (int)((((size_t)in + 3) & ~3) - (size_t)in);
    s->words = (u32*)((char*)in + first_bytes);
    s->word_count = (in_bytes - first_bytes) / 4;
    int last_bytes = ((in_bytes - first_bytes) & 3);

    for (int i = 0; i < first_bytes; ++i) s->bits |= (u64)(((u8*)in)[i]) << (i * 8);

    s->final_word_available = last_bytes ? 1 : 0;
    s->final_word = 0;
    for (int i = 0; i < last_bytes; i++) s->final_word |= ((u8*)in)[in_bytes - last_bytes + i] << (i * 8);

    s->count = first_bytes * 8;

    s->out = (char*)out;
    s->out_end = s->out + out_bytes;
    s->begin = (char*)out;

    int count = 0;
    u32 bfinal;
    do {
        bfinal = s_read_bits(s, 1);
        u32 btype = s_read_bits(s, 2);

        switch (btype) {
            case 0:
                __NEKO_ASEPRITE_CALL(s_stored(s));
                break;
            case 1:
                s_fixed(s);
                __NEKO_ASEPRITE_CALL(s_block(s));
                break;
            case 2:
                s_dynamic(s);
                __NEKO_ASEPRITE_CALL(s_block(s));
                break;
            case 3:
                __NEKO_ASEPRITE_CHECK(0, "Detected unknown block type within input stream.");
        }

        ++count;
    } while (!bfinal);

    neko_safe_free(s);
    return 1;

ase_err:
    neko_safe_free(s);
    return 0;
}

typedef struct ase_state_t {
    u8* in;
    u8* end;
} ase_state_t;

neko_global u8 s_read_uint8(ase_state_t* s) {
    neko_assert(s->in <= s->end + sizeof(u8));
    u8** p = &s->in;
    u8 value = **p;
    ++(*p);
    return value;
}

neko_global u16 s_read_uint16(ase_state_t* s) {
    neko_assert(s->in <= s->end + sizeof(u16));
    u8** p = &s->in;
    u16 value;
    value = (*p)[0];
    value |= (((u16)((*p)[1])) << 8);
    *p += 2;
    return value;
}

neko_global ase_fixed_t s_read_fixed(ase_state_t* s) {
    ase_fixed_t value;
    value.a = s_read_uint16(s);
    value.b = s_read_uint16(s);
    return value;
}

neko_global u32 s_read_uint32(ase_state_t* s) {
    neko_assert(s->in <= s->end + sizeof(u32));
    u8** p = &s->in;
    u32 value;
    value = (*p)[0];
    value |= (((u32)((*p)[1])) << 8);
    value |= (((u32)((*p)[2])) << 16);
    value |= (((u32)((*p)[3])) << 24);
    *p += 4;
    return value;
}

neko_global s16 s_read_int16(ase_state_t* s) { return (s16)s_read_uint16(s); }
neko_global s16 s_read_int32(ase_state_t* s) { return (s32)s_read_uint32(s); }

neko_global const char* s_read_string(ase_state_t* s) {
    int len = (int)s_read_uint16(s);
    char* bytes = (char*)neko_safe_malloc(len + 1);
    for (int i = 0; i < len; ++i) {
        bytes[i] = (char)s_read_uint8(s);
    }
    bytes[len] = 0;
    return bytes;
}

neko_global void s_skip(ase_state_t* ase, int num_bytes) {
    neko_assert(ase->in <= ase->end + num_bytes);
    ase->in += num_bytes;
}

neko_global char* s_fopen(const char* path, int* size) {
    char* data = 0;
    FILE* fp = fopen(path, "rb");
    int sz = 0;

    if (fp) {
        fseek(fp, 0, SEEK_END);
        sz = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        data = (char*)neko_safe_malloc(sz + 1);
        fread(data, sz, 1, fp);
        data[sz] = 0;
        fclose(fp);
    }

    if (size) *size = sz;
    return data;
}

ase_t* neko_aseprite_load_from_file(const char* path) {
    s_error_file = path;
    int sz;
    void* file = s_fopen(path, &sz);
    if (!file) {
        neko_log_warning("unable to find map file %s", s_error_file ? s_error_file : "MEMORY");
        return NULL;
    }
    ase_t* aseprite = neko_aseprite_load_from_memory(file, sz);
    neko_safe_free(file);
    s_error_file = NULL;
    return aseprite;
}

ase_t* neko_aseprite_load_from_memory(const void* memory, int size) {
    ase_t* ase = (ase_t*)neko_safe_malloc(sizeof(ase_t));
    memset(ase, 0, sizeof(*ase));

    ase_state_t state = {0};
    ase_state_t* s = &state;
    s->in = (u8*)memory;
    s->end = s->in + size;

    s_skip(s, sizeof(u32));  // File size.
    int magic = (int)s_read_uint16(s);
    neko_assert(magic == 0xA5E0);

    ase->frame_count = (int)s_read_uint16(s);
    ase->w = s_read_uint16(s);
    ase->h = s_read_uint16(s);
    u16 bpp = s_read_uint16(s) / 8;
    if (bpp == 4)
        ase->mode = NEKO_ASE_MODE_RGBA;
    else if (bpp == 2)
        ase->mode = NEKO_ASE_MODE_GRAYSCALE;
    else {
        neko_assert(bpp == 1);
        ase->mode = NEKO_ASE_MODE_INDEXED;
    }
    u32 valid_layer_opacity = s_read_uint32(s) & 1;
    int speed = s_read_uint16(s);
    s_skip(s, sizeof(u32) * 2);  // Spec says skip these bytes, as they're zero'd.
    ase->transparent_palette_entry_index = s_read_uint8(s);
    s_skip(s, 3);  // Spec says skip these bytes.
    ase->number_of_colors = (int)s_read_uint16(s);
    ase->pixel_w = (int)s_read_uint8(s);
    ase->pixel_h = (int)s_read_uint8(s);
    ase->grid_x = (int)s_read_int16(s);
    ase->grid_y = (int)s_read_int16(s);
    ase->grid_w = (int)s_read_uint16(s);
    ase->grid_h = (int)s_read_uint16(s);
    s_skip(s, 84);  // For future use (set to zero).

    ase->frames = (ase_frame_t*)neko_safe_malloc((int)(sizeof(ase_frame_t)) * ase->frame_count);
    memset(ase->frames, 0, sizeof(ase_frame_t) * (size_t)ase->frame_count);

    ase_udata_t* last_udata = NULL;
    int was_on_tags = 0;
    int tag_index = 0;

    ase_layer_t* layer_stack[__NEKO_ASEPRITE_MAX_LAYERS];

    // 查看 aseprite 文件标准
    // https://github.com/aseprite/aseprite/blob/main/docs/ase-file-specs.md

    // Parse all chunks in the .aseprite file.
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t* frame = ase->frames + i;
        frame->ase = ase;
        s_skip(s, sizeof(u32));  // Frame size.
        magic = (int)s_read_uint16(s);
        neko_assert(magic == 0xF1FA);
        int chunk_count = (int)s_read_uint16(s);
        frame->duration_milliseconds = s_read_uint16(s);
        if (frame->duration_milliseconds == 0) frame->duration_milliseconds = speed;
        s_skip(s, 2);  // For future use (set to zero).
        u32 new_chunk_count = s_read_uint32(s);
        if (new_chunk_count) chunk_count = (int)new_chunk_count;

        for (int j = 0; j < chunk_count; ++j) {
            u32 chunk_size = s_read_uint32(s);
            u16 chunk_type = s_read_uint16(s);
            chunk_size -= (u32)(sizeof(u32) + sizeof(u16));
            u8* chunk_start = s->in;

            switch (chunk_type) {
                case 0x2004:  // Layer chunk.
                {
                    neko_assert(ase->layer_count < __NEKO_ASEPRITE_MAX_LAYERS);
                    ase_layer_t* layer = ase->layers + ase->layer_count++;
                    layer->flags = (ase_layer_flags_t)s_read_uint16(s);

                    // WORD Layer type
                    //  0 = Normal(image) layer
                    //  1 = Group
                    //  2 = Tilemap
                    layer->type = (ase_layer_type_t)s_read_uint16(s);

                    layer->parent = NULL;
                    int child_level = (int)s_read_uint16(s);
                    layer_stack[child_level] = layer;
                    if (child_level) {
                        layer->parent = layer_stack[child_level - 1];
                    }
                    s_skip(s, sizeof(u16));  // Default layer width in pixels (ignored).
                    s_skip(s, sizeof(u16));  // Default layer height in pixels (ignored).
                    int blend_mode = (int)s_read_uint16(s);
                    if (blend_mode) neko_log_warning("aseprite unknown blend mode encountered.");
                    layer->opacity = s_read_uint8(s) / 255.0f;
                    if (!valid_layer_opacity) layer->opacity = 1.0f;
                    s_skip(s, 3);  // For future use (set to zero).
                    layer->name = s_read_string(s);
                    last_udata = &layer->udata;
                } break;

                case 0x2005:  // Cel chunk.
                {
                    neko_assert(frame->cel_count < __NEKO_ASEPRITE_MAX_LAYERS);
                    ase_cel_t* cel = frame->cels + frame->cel_count++;
                    int layer_index = (int)s_read_uint16(s);
                    cel->layer = ase->layers + layer_index;
                    cel->x = s_read_int16(s);
                    cel->y = s_read_int16(s);
                    cel->opacity = s_read_uint8(s) / 255.0f;
                    int cel_type = (int)s_read_uint16(s);
                    s_skip(s, 7);  // For future (set to zero).
                    switch (cel_type) {
                        case 0:  // Raw cel.
                            cel->w = s_read_uint16(s);
                            cel->h = s_read_uint16(s);
                            cel->cel_pixels = neko_safe_malloc(cel->w * cel->h * bpp);
                            memcpy(cel->cel_pixels, s->in, (size_t)(cel->w * cel->h * bpp));
                            s_skip(s, cel->w * cel->h * bpp);
                            break;

                        case 1:  // Linked cel.
                            cel->is_linked = 1;
                            cel->linked_frame_index = s_read_uint16(s);
                            break;

                        case 2:  // Compressed image cel.
                        {
                            cel->w = s_read_uint16(s);
                            cel->h = s_read_uint16(s);
                            int zlib_byte0 = s_read_uint8(s);
                            int zlib_byte1 = s_read_uint8(s);
                            int deflate_bytes = (int)chunk_size - (int)(s->in - chunk_start);
                            void* pixels = s->in;
                            neko_assert((zlib_byte0 & 0x0F) == 0x08);  // Only zlib compression method (RFC 1950) is supported.
                            neko_assert((zlib_byte0 & 0xF0) <= 0x70);  // Innapropriate window size detected.
                            neko_assert(!(zlib_byte1 & 0x20));         // Preset dictionary is present and not supported.
                            int pixels_sz = cel->w * cel->h * bpp;
                            void* pixels_decompressed = neko_safe_malloc(pixels_sz);
                            int ret = s_inflate(pixels, deflate_bytes, pixels_decompressed, pixels_sz);
                            if (!ret) neko_log_warning(s_error_reason);
                            cel->cel_pixels = pixels_decompressed;
                            s_skip(s, deflate_bytes);
                        } break;
                    }
                    last_udata = &cel->udata;
                } break;

                case 0x2006:  // Cel extra chunk.
                {
                    ase_cel_t* cel = frame->cels + frame->cel_count;
                    cel->has_extra = 1;
                    cel->extra.precise_bounds_are_set = (int)s_read_uint32(s);
                    cel->extra.precise_x = s_read_fixed(s);
                    cel->extra.precise_y = s_read_fixed(s);
                    cel->extra.w = s_read_fixed(s);
                    cel->extra.h = s_read_fixed(s);
                    s_skip(s, 16);  // For future use (set to zero).
                } break;

                case 0x2007:  // Color profile chunk.
                {
                    ase->has_color_profile = 1;
                    ase->color_profile.type = (ase_color_profile_type_t)s_read_uint16(s);
                    ase->color_profile.use_fixed_gamma = (int)s_read_uint16(s) & 1;
                    ase->color_profile.gamma = s_read_fixed(s);
                    s_skip(s, 8);  // For future use (set to zero).
                    if (ase->color_profile.type == NEKO_ASE_COLOR_PROFILE_TYPE_EMBEDDED_ICC) {
                        // Use the embedded ICC profile.
                        ase->color_profile.icc_profile_data_length = s_read_uint32(s);
                        ase->color_profile.icc_profile_data = neko_safe_malloc(ase->color_profile.icc_profile_data_length);
                        memcpy(ase->color_profile.icc_profile_data, s->in, ase->color_profile.icc_profile_data_length);
                        s->in += ase->color_profile.icc_profile_data_length;
                    }
                } break;

                case 0x2018:  // Tags chunk.
                {
                    ase->tag_count = (int)s_read_uint16(s);
                    s_skip(s, 8);  // For future (set to zero).
                    neko_assert(ase->tag_count < __NEKO_ASEPRITE_MAX_TAGS);
                    for (int k = 0; k < ase->tag_count; ++k) {
                        ase_tag_t tag;
                        tag.from_frame = (int)s_read_uint16(s);
                        tag.to_frame = (int)s_read_uint16(s);
                        tag.loop_animation_direction = (ase_animation_direction_t)s_read_uint8(s);
                        s_skip(s, 8);  // For future (set to zero).
                        tag.r = s_read_uint8(s);
                        tag.g = s_read_uint8(s);
                        tag.b = s_read_uint8(s);
                        s_skip(s, 1);  // Extra byte (zero).
                        tag.name = s_read_string(s);
                        ase->tags[k] = tag;
                    }
                    was_on_tags = 1;
                } break;

                case 0x2019:  // Palette chunk.
                {
                    ase->palette.entry_count = (int)s_read_uint32(s);
                    neko_assert(ase->palette.entry_count <= __NEKO_ASEPRITE_MAX_PALETTE_ENTRIES);
                    int first_index = (int)s_read_uint32(s);
                    int last_index = (int)s_read_uint32(s);
                    s_skip(s, 8);  // For future (set to zero).
                    for (int k = first_index; k <= last_index; ++k) {
                        int has_name = s_read_uint16(s);
                        ase_palette_entry_t entry;
                        entry.color.r = s_read_uint8(s);
                        entry.color.g = s_read_uint8(s);
                        entry.color.b = s_read_uint8(s);
                        entry.color.a = s_read_uint8(s);
                        if (has_name) {
                            entry.color_name = s_read_string(s);
                        } else {
                            entry.color_name = NULL;
                        }
                        ase->palette.entries[k] = entry;
                    }
                } break;

                case 0x2020:  // Udata chunk.
                {
                    neko_assert(last_udata || was_on_tags);
                    if (was_on_tags && !last_udata) {
                        neko_assert(tag_index < ase->tag_count);
                        last_udata = &ase->tags[tag_index++].udata;
                    }
                    int flags = (int)s_read_uint32(s);
                    if (flags & 1) {
                        last_udata->has_text = 1;
                        last_udata->text = s_read_string(s);
                    }
                    if (flags & 2) {
                        last_udata->color.r = s_read_uint8(s);
                        last_udata->color.g = s_read_uint8(s);
                        last_udata->color.b = s_read_uint8(s);
                        last_udata->color.a = s_read_uint8(s);
                    }
                    last_udata = NULL;
                } break;

                case 0x2022:  // Slice chunk.
                {
                    int slice_count = (int)s_read_uint32(s);
                    int flags = (int)s_read_uint32(s);
                    s_skip(s, sizeof(u32));  // Reserved.
                    const char* name = s_read_string(s);
                    for (int k = 0; k < (int)slice_count; ++k) {
                        ase_slice_t slice = {0};
                        slice.name = name;
                        slice.frame_number = (int)s_read_uint32(s);
                        slice.origin_x = (int)s_read_int32(s);
                        slice.origin_y = (int)s_read_int32(s);
                        slice.w = (int)s_read_uint32(s);
                        slice.h = (int)s_read_uint32(s);
                        if (flags & 1) {
                            // It's a 9-patches slice.
                            slice.has_center_as_9_slice = 1;
                            slice.center_x = (int)s_read_int32(s);
                            slice.center_y = (int)s_read_int32(s);
                            slice.center_w = (int)s_read_uint32(s);
                            slice.center_h = (int)s_read_uint32(s);
                        } else if (flags & 2) {
                            // Has pivot information.
                            slice.has_pivot = 1;
                            slice.pivot_x = (int)s_read_int32(s);
                            slice.pivot_y = (int)s_read_int32(s);
                        }
                        neko_assert(ase->slice_count < __NEKO_ASEPRITE_MAX_SLICES);
                        ase->slices[ase->slice_count++] = slice;
                        last_udata = &ase->slices[ase->slice_count - 1].udata;
                    }
                } break;

                default:
                    s_skip(s, (int)chunk_size);
                    break;
            }

            u32 size_read = (u32)(s->in - chunk_start);
            neko_assert(size_read == chunk_size);
        }
    }

    return ase;
}

void neko_aseprite_default_blend_bind(ase_t* ase) {

    neko_assert(ase);
    neko_assert(ase->frame_count);

    // 为了方便起见，将所有单元像素混合到各自的帧中
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t* frame = ase->frames + i;
        frame->pixels[0] = (neko_color_t*)neko_safe_malloc((size_t)(sizeof(neko_color_t)) * ase->w * ase->h);
        memset(frame->pixels[0], 0, sizeof(neko_color_t) * (size_t)ase->w * (size_t)ase->h);
        neko_color_t* dst = frame->pixels[0];

        // neko_println_debug("frame: %d cel_count: %d", i, frame->cel_count);

        for (int j = 0; j < frame->cel_count; ++j) {  //

            ase_cel_t* cel = frame->cels + j;

            if (!(cel->layer->flags & NEKO_ASE_LAYER_FLAGS_VISIBLE) || (cel->layer->parent && !(cel->layer->parent->flags & NEKO_ASE_LAYER_FLAGS_VISIBLE))) {
                continue;
            }

            while (cel->is_linked) {
                ase_frame_t* frame = ase->frames + cel->linked_frame_index;
                int found = 0;
                for (int k = 0; k < frame->cel_count; ++k) {
                    if (frame->cels[k].layer == cel->layer) {
                        cel = frame->cels + k;
                        found = 1;
                        break;
                    }
                }
                neko_assert(found);
            }
            void* src = cel->cel_pixels;
            u8 opacity = (u8)(cel->opacity * cel->layer->opacity * 255.0f);
            int cx = cel->x;
            int cy = cel->y;
            int cw = cel->w;
            int ch = cel->h;
            int cl = -neko_min(cx, 0);
            int ct = -neko_min(cy, 0);
            int dl = neko_max(cx, 0);
            int dt = neko_max(cy, 0);
            int dr = neko_min(ase->w, cw + cx);
            int db = neko_min(ase->h, ch + cy);
            int aw = ase->w;
            for (int dx = dl, sx = cl; dx < dr; dx++, sx++) {
                for (int dy = dt, sy = ct; dy < db; dy++, sy++) {
                    int dst_index = aw * dy + dx;
                    neko_color_t src_color = s_color(ase, src, cw * sy + sx);
                    neko_color_t dst_color = dst[dst_index];
                    neko_color_t result = s_blend(src_color, dst_color, opacity);
                    dst[dst_index] = result;
                }
            }
        }
    }
}

void neko_aseprite_free(ase_t* ase) {
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t* frame = ase->frames + i;
        neko_safe_free(frame->pixels[0]);
        for (int j = 0; j < frame->cel_count; ++j) {
            ase_cel_t* cel = frame->cels + j;
            neko_safe_free(cel->cel_pixels);
            neko_safe_free((void*)cel->udata.text);
        }
    }
    for (int i = 0; i < ase->layer_count; ++i) {
        ase_layer_t* layer = ase->layers + i;
        neko_safe_free((void*)layer->name);
        neko_safe_free((void*)layer->udata.text);
    }
    for (int i = 0; i < ase->tag_count; ++i) {
        ase_tag_t* tag = ase->tags + i;
        neko_safe_free((void*)tag->name);
    }
    for (int i = 0; i < ase->slice_count; ++i) {
        ase_slice_t* slice = ase->slices + i;
        neko_safe_free((void*)slice->udata.text);
    }
    if (ase->slice_count) {
        neko_safe_free((void*)ase->slices[0].name);
    }
    for (int i = 0; i < ase->palette.entry_count; ++i) {
        neko_safe_free((void*)ase->palette.entries[i].color_name);
    }
    neko_safe_free(ase->color_profile.icc_profile_data);
    neko_safe_free(ase->frames);
    neko_safe_free(ase);
}

#endif
