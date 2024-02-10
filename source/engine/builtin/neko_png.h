

#ifndef NEKO_PNG_H

#define NEKO_PNG_ATLAS_MUST_FIT 1              // 如果输入图像不适合 则从NEKO_PNG_MAKE_ATLAS返回错误
#define NEKO_PNG_ATLAS_FLIP_Y_AXIS_FOR_UV 1    // 翻转输出UV坐标的y
#define NEKO_PNG_ATLAS_EMPTY_COLOR 0x000000FF  // 纹理贴图集中空白区域的填充颜色(RGBA)

#include <limits.h>
#include <stdint.h>

typedef struct neko_png_pixel_t neko_png_pixel_t;
typedef struct neko_png_image_t neko_png_image_t;
typedef struct neko_png_indexed_image_t neko_png_indexed_image_t;
typedef struct neko_png_atlas_image_t neko_png_atlas_image_t;

// 在任何函数出现错误的情况下阅读此内容
extern const char* neko_png_error_reason;

// 成功时返回1 失败时返回0
int neko_png_inflate(void* in, int in_bytes, void* out, int out_bytes);
int neko_png_save(const char* file_name, const neko_png_image_t* img);

typedef struct neko_png_saved_png_t {
    int size;
    void* data;
} neko_png_saved_png_t;

neko_png_saved_png_t neko_png_save_to_memory(const neko_png_image_t* img);

neko_png_image_t neko_png_make_atlas(int atlasWidth, int atlasHeight, const neko_png_image_t* pngs, int png_count, neko_png_atlas_image_t* imgs_out);

int neko_png_default_save_atlas(const char* out_path_image, const char* out_path_atlas_txt, const neko_png_image_t* atlas, const neko_png_atlas_image_t* imgs, int img_count, const char** names);

neko_png_image_t neko_png_load(const char* file_name);
neko_png_image_t neko_png_load_mem(const void* png_data, int png_length);
neko_png_image_t neko_png_load_blank(int w, int h);
void neko_png_free(neko_png_image_t* img);
void neko_png_flip_image_horizontal(neko_png_image_t* img);

void neko_png_load_wh(const void* png_data, int png_length, int* w, int* h);

neko_png_indexed_image_t neko_png_load_indexed(const char* file_name);
neko_png_indexed_image_t neko_png_load_indexed_mem(const void* png_data, int png_length);
void neko_png_free_indexed(neko_png_indexed_image_t* img);

neko_png_image_t neko_png_depallete_indexed_image(neko_png_indexed_image_t* img);

// 对像素进行预处理 将图像数据转换为预乘的Alpha格式
// http://www.essentialmath.com/GDC2015/VanVerth_Jim_DoingMathwRGB.pdf
void neko_png_premultiply(neko_png_image_t* img);

struct neko_png_pixel_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct neko_png_image_t {
    int w;
    int h;
    neko_png_pixel_t* pix;
};

struct neko_png_indexed_image_t {
    int w;
    int h;
    uint8_t* pix;
    uint8_t palette_len;
    neko_png_pixel_t palette[256];
};

struct neko_png_atlas_image_t {
    int img_index;     // index into the `imgs` array
    int w, h;          // pixel w/h of original image
    float minx, miny;  // u coordinate
    float maxx, maxy;  // v coordinate
    int fit;           // non-zero if image fit and was placed into the atlas
};

#define NEKO_PNG_H
#endif

#ifdef NEKO_PNG_IMPLEMENTATION
#ifndef NEKO_PNG_IMPLEMENTATION_ONCE
#define NEKO_PNG_IMPLEMENTATION_ONCE

#include <stdlib.h>

#if !defined(NEKO_PNG_ASSERT)
#include <assert.h>
#define NEKO_PNG_ASSERT assert
#endif

static neko_png_pixel_t neko_png_make_pixel_a(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    neko_png_pixel_t p;
    p.r = r;
    p.g = g;
    p.b = b;
    p.a = a;
    return p;
}

static neko_png_pixel_t neko_png_make_pixel(uint8_t r, uint8_t g, uint8_t b) {
    neko_png_pixel_t p;
    p.r = r;
    p.g = g;
    p.b = b;
    p.a = 0xFF;
    return p;
}

const char* neko_png_error_reason;

#define NEKO_PNG_FAIL()    \
    do {                   \
        goto neko_png_err; \
    } while (0)

#define NEKO_PNG_CHECK(X, Y)           \
    do {                               \
        if (!(X)) {                    \
            neko_png_error_reason = Y; \
            NEKO_PNG_FAIL();           \
        }                              \
    } while (0)

#define NEKO_PNG_CALL(X)             \
    do {                             \
        if (!(X)) goto neko_png_err; \
    } while (0)

#define NEKO_PNG_LOOKUP_BITS 9
#define NEKO_PNG_LOOKUP_COUNT (1 << NEKO_PNG_LOOKUP_BITS)
#define NEKO_PNG_LOOKUP_MASK (NEKO_PNG_LOOKUP_COUNT - 1)
#define NEKO_PNG_DEFLATE_MAX_BITLEN 15

// DEFLATE tables from RFC 1951
uint8_t neko_png_fixed_table[288 + 32] = {
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
};                                                                                                                                                                                           // 3.2.6
uint8_t neko_png_permutation_order[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};                                                                                 // 3.2.7
uint8_t neko_png_len_extra_bits[29 + 2] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 0, 0};                                                     // 3.2.5
uint32_t neko_png_len_base[29 + 2] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};                              // 3.2.5
uint8_t neko_png_dist_extra_bits[30 + 2] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 0, 0};                                         // 3.2.5
uint32_t neko_png_dist_base[30 + 2] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577, 0, 0};  // 3.2.5

typedef struct neko_png_state_t {
    uint64_t bits;
    int count;
    uint32_t* words;
    int word_count;
    int word_index;
    int bits_left;

    int final_word_available;
    uint32_t final_word;

    char* out;
    char* out_end;
    char* begin;

    uint16_t lookup[NEKO_PNG_LOOKUP_COUNT];
    uint32_t lit[288];
    uint32_t dst[32];
    uint32_t len[19];
    uint32_t nlit;
    uint32_t ndst;
    uint32_t nlen;
} neko_png_state_t;

static int neko_png_would_overflow(neko_png_state_t* s, int num_bits) { return (s->bits_left + s->count) - num_bits < 0; }

static char* neko_png_ptr(neko_png_state_t* s) {
    NEKO_PNG_ASSERT(!(s->bits_left & 7));
    return (char*)(s->words + s->word_index) - (s->count / 8);
}

static uint64_t neko_png_peak_bits(neko_png_state_t* s, int num_bits_to_read) {
    if (s->count < num_bits_to_read) {
        if (s->word_index < s->word_count) {
            uint32_t word = s->words[s->word_index++];
            s->bits |= (uint64_t)word << s->count;
            s->count += 32;
            NEKO_PNG_ASSERT(s->word_index <= s->word_count);
        }

        else if (s->final_word_available) {
            uint32_t word = s->final_word;
            s->bits |= (uint64_t)word << s->count;
            s->count += s->bits_left;
            s->final_word_available = 0;
        }
    }

    return s->bits;
}

static uint32_t neko_png_consume_bits(neko_png_state_t* s, int num_bits_to_read) {
    NEKO_PNG_ASSERT(s->count >= num_bits_to_read);
    uint32_t bits = s->bits & (((uint64_t)1 << num_bits_to_read) - 1);
    s->bits >>= num_bits_to_read;
    s->count -= num_bits_to_read;
    s->bits_left -= num_bits_to_read;
    return bits;
}

static uint32_t neko_png_read_bits(neko_png_state_t* s, int num_bits_to_read) {
    NEKO_PNG_ASSERT(num_bits_to_read <= 32);
    NEKO_PNG_ASSERT(num_bits_to_read >= 0);
    NEKO_PNG_ASSERT(s->bits_left > 0);
    NEKO_PNG_ASSERT(s->count <= 64);
    NEKO_PNG_ASSERT(!neko_png_would_overflow(s, num_bits_to_read));
    neko_png_peak_bits(s, num_bits_to_read);
    uint32_t bits = neko_png_consume_bits(s, num_bits_to_read);
    return bits;
}

static char* neko_png_read_file_to_memory(const char* path, int* size) {
    char* data = 0;
    FILE* fp = fopen(path, "rb");
    int sizeNum = 0;

    if (fp) {
        fseek(fp, 0, SEEK_END);
        sizeNum = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        data = (char*)malloc(sizeNum + 1);
        fread(data, sizeNum, 1, fp);
        data[sizeNum] = 0;
        fclose(fp);
    }

    if (size) *size = sizeNum;
    return data;
}

static uint32_t neko_png_rev16(uint32_t a) {
    a = ((a & 0xAAAA) >> 1) | ((a & 0x5555) << 1);
    a = ((a & 0xCCCC) >> 2) | ((a & 0x3333) << 2);
    a = ((a & 0xF0F0) >> 4) | ((a & 0x0F0F) << 4);
    a = ((a & 0xFF00) >> 8) | ((a & 0x00FF) << 8);
    return a;
}

// RFC 1951 section 3.2.2
static int neko_png_build(neko_png_state_t* s, uint32_t* tree, uint8_t* lens, int sym_count) {
    int n, codes[16], first[16], counts[16] = {0};

    // Frequency count
    for (n = 0; n < sym_count; n++) counts[lens[n]]++;

    // Distribute codes
    counts[0] = codes[0] = first[0] = 0;
    for (n = 1; n <= 15; ++n) {
        codes[n] = (codes[n - 1] + counts[n - 1]) << 1;
        first[n] = first[n - 1] + counts[n - 1];
    }

    if (s) memset(s->lookup, 0, sizeof(s->lookup));
    for (int i = 0; i < sym_count; ++i) {
        int len = lens[i];

        if (len != 0) {
            NEKO_PNG_ASSERT(len < 16);
            uint32_t code = codes[len]++;
            uint32_t slot = first[len]++;
            tree[slot] = (code << (32 - len)) | (i << 4) | len;

            if (s && len <= NEKO_PNG_LOOKUP_BITS) {
                int j = neko_png_rev16(code) >> (16 - len);
                while (j < (1 << NEKO_PNG_LOOKUP_BITS)) {
                    s->lookup[j] = (uint16_t)((len << NEKO_PNG_LOOKUP_BITS) | i);
                    j += (1 << len);
                }
            }
        }
    }

    int max_index = first[15];
    return max_index;
}

static int neko_png_stored(neko_png_state_t* s) {
    char* p;

    // 3.2.3
    // skip any remaining bits in current partially processed byte
    neko_png_read_bits(s, s->count & 7);

    // 3.2.4
    // read LEN and NLEN, should complement each other
    uint16_t LEN = (uint16_t)neko_png_read_bits(s, 16);
    uint16_t NLEN = (uint16_t)neko_png_read_bits(s, 16);
    NEKO_PNG_CHECK(LEN == (uint16_t)(~NLEN), "Failed to find LEN and NLEN as complements within stored (uncompressed) stream.");
    NEKO_PNG_CHECK(s->bits_left / 8 <= (int)LEN, "Stored block extends beyond end of input stream.");
    p = neko_png_ptr(s);
    memcpy(s->out, p, LEN);
    s->out += LEN;
    return 1;

neko_png_err:
    return 0;
}

// 3.2.6
static int neko_png_fixed(neko_png_state_t* s) {
    s->nlit = neko_png_build(s, s->lit, neko_png_fixed_table, 288);
    s->ndst = neko_png_build(0, s->dst, neko_png_fixed_table + 288, 32);
    return 1;
}

static int neko_png_decode(neko_png_state_t* s, uint32_t* tree, int hi) {
    uint64_t bits = neko_png_peak_bits(s, 16);
    uint32_t search = (neko_png_rev16((uint32_t)bits) << 16) | 0xFFFF;
    int lo = 0;
    while (lo < hi) {
        int guess = (lo + hi) >> 1;
        if (search < tree[guess])
            hi = guess;
        else
            lo = guess + 1;
    }

    uint32_t key = tree[lo - 1];
    uint32_t len = (32 - (key & 0xF));
    NEKO_PNG_ASSERT((search >> len) == (key >> len));

    int code = neko_png_consume_bits(s, key & 0xF);
    (void)code;
    return (key >> 4) & 0xFFF;
}

// 3.2.7
static int neko_png_dynamic(neko_png_state_t* s) {
    uint8_t lenlens[19] = {0};

    int nlit = 257 + neko_png_read_bits(s, 5);
    int ndst = 1 + neko_png_read_bits(s, 5);
    int nlen = 4 + neko_png_read_bits(s, 4);

    for (int i = 0; i < nlen; ++i) lenlens[neko_png_permutation_order[i]] = (uint8_t)neko_png_read_bits(s, 3);

    // Build the tree for decoding code lengths
    s->nlen = neko_png_build(0, s->len, lenlens, 19);
    uint8_t lens[288 + 32];

    for (int n = 0; n < nlit + ndst;) {
        int sym = neko_png_decode(s, s->len, s->nlen);
        switch (sym) {
            case 16:
                for (int i = 3 + neko_png_read_bits(s, 2); i; --i, ++n) lens[n] = lens[n - 1];
                break;
            case 17:
                for (int i = 3 + neko_png_read_bits(s, 3); i; --i, ++n) lens[n] = 0;
                break;
            case 18:
                for (int i = 11 + neko_png_read_bits(s, 7); i; --i, ++n) lens[n] = 0;
                break;
            default:
                lens[n++] = (uint8_t)sym;
                break;
        }
    }

    s->nlit = neko_png_build(s, s->lit, lens, nlit);
    s->ndst = neko_png_build(0, s->dst, lens + nlit, ndst);
    return 1;
}

// 3.2.3
static int neko_png_block(neko_png_state_t* s) {
    while (1) {
        int symbol = neko_png_decode(s, s->lit, s->nlit);

        if (symbol < 256) {
            NEKO_PNG_CHECK(s->out + 1 <= s->out_end, "Attempted to overwrite out buffer while outputting a symbol.");
            *s->out = (char)symbol;
            s->out += 1;
        }

        else if (symbol > 256) {
            symbol -= 257;
            int length = neko_png_read_bits(s, neko_png_len_extra_bits[symbol]) + neko_png_len_base[symbol];
            int distance_symbol = neko_png_decode(s, s->dst, s->ndst);
            int backwards_distance = neko_png_read_bits(s, neko_png_dist_extra_bits[distance_symbol]) + neko_png_dist_base[distance_symbol];
            NEKO_PNG_CHECK(s->out - backwards_distance >= s->begin, "Attempted to write before out buffer (invalid backwards distance).");
            NEKO_PNG_CHECK(s->out + length <= s->out_end, "Attempted to overwrite out buffer while outputting a string.");
            char* src = s->out - backwards_distance;
            char* dst = s->out;
            s->out += length;

            switch (backwards_distance) {
                case 1:  // very common in images
                    memset(dst, *src, length);
                    break;
                default:
                    while (length--) *dst++ = *src++;
            }
        }

        else
            break;
    }

    return 1;

neko_png_err:
    return 0;
}

// 3.2.3
int neko_png_inflate(void* in, int in_bytes, void* out, int out_bytes) {
    neko_png_state_t* s = (neko_png_state_t*)calloc(1, sizeof(neko_png_state_t));
    s->bits = 0;
    s->count = 0;
    s->word_index = 0;
    s->bits_left = in_bytes * 8;

    // s->words is the in-pointer rounded up to a multiple of 4
    int first_bytes = (int)((((size_t)in + 3) & ~3) - (size_t)in);
    s->words = (uint32_t*)((char*)in + first_bytes);
    s->word_count = (in_bytes - first_bytes) / 4;
    int last_bytes = ((in_bytes - first_bytes) & 3);

    for (int i = 0; i < first_bytes; ++i) s->bits |= (uint64_t)(((uint8_t*)in)[i]) << (i * 8);

    s->final_word_available = last_bytes ? 1 : 0;
    s->final_word = 0;
    for (int i = 0; i < last_bytes; i++) s->final_word |= ((uint8_t*)in)[in_bytes - last_bytes + i] << (i * 8);

    s->count = first_bytes * 8;

    s->out = (char*)out;
    s->out_end = s->out + out_bytes;
    s->begin = (char*)out;

    int count = 0;
    int bfinal;
    do {
        bfinal = neko_png_read_bits(s, 1);
        int btype = neko_png_read_bits(s, 2);

        switch (btype) {
            case 0:
                NEKO_PNG_CALL(neko_png_stored(s));
                break;
            case 1:
                neko_png_fixed(s);
                NEKO_PNG_CALL(neko_png_block(s));
                break;
            case 2:
                neko_png_dynamic(s);
                NEKO_PNG_CALL(neko_png_block(s));
                break;
            case 3:
                NEKO_PNG_CHECK(0, "Detected unknown block type within input stream.");
        }

        ++count;
    } while (!bfinal);

    free(s);
    return 1;

neko_png_err:
    free(s);
    return 0;
}

static uint8_t neko_png_paeth(uint8_t a, uint8_t b, uint8_t c) {
    int p = a + b - c;
    int pa = abs(p - a);
    int pb = abs(p - b);
    int pc = abs(p - c);
    return (pa <= pb && pa <= pc) ? a : (pb <= pc) ? b : c;
}

typedef struct neko_png_save_png_data_t {
    uint32_t crc;
    uint32_t adler;
    uint32_t bits;
    uint32_t prev;
    uint32_t runlen;
    int buflen;
    int bufcap;
    char* buffer;
} neko_png_save_png_data_t;

uint32_t CP_CRC_TABLE[] = {0,          0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
                           0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c, 0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c};

static void neko_png_put8(neko_png_save_png_data_t* s, uint32_t a) {
    if (s->buflen >= s->bufcap) {
        s->bufcap *= 2;
        s->buffer = (char*)realloc(s->buffer, s->bufcap);
    }
    s->buffer[s->buflen++] = a;
    s->crc = (s->crc >> 4) ^ CP_CRC_TABLE[(s->crc & 15) ^ (a & 15)];
    s->crc = (s->crc >> 4) ^ CP_CRC_TABLE[(s->crc & 15) ^ (a >> 4)];
}

static void neko_png_update_adler(neko_png_save_png_data_t* s, uint32_t v) {
    uint32_t s1 = s->adler & 0xFFFF;
    uint32_t s2 = (s->adler >> 16) & 0xFFFF;
    s1 = (s1 + v) % 65521;
    s2 = (s2 + s1) % 65521;
    s->adler = (s2 << 16) + s1;
}

static void neko_png_put32(neko_png_save_png_data_t* s, uint32_t v) {
    neko_png_put8(s, (v >> 24) & 0xFF);
    neko_png_put8(s, (v >> 16) & 0xFF);
    neko_png_put8(s, (v >> 8) & 0xFF);
    neko_png_put8(s, v & 0xFF);
}

static void neko_png_put_bits(neko_png_save_png_data_t* s, uint32_t data, uint32_t bitcount) {
    while (bitcount--) {
        uint32_t prev = s->bits;
        s->bits = (s->bits >> 1) | ((data & 1) << 7);
        data >>= 1;

        if (prev & 1) {
            neko_png_put8(s, s->bits);
            s->bits = 0x80;
        }
    }
}

static void neko_png_put_bitsr(neko_png_save_png_data_t* s, uint32_t data, uint32_t bitcount) {
    while (bitcount--) neko_png_put_bits(s, data >> bitcount, 1);
}

static void neko_png_begin_chunk(neko_png_save_png_data_t* s, const char* id, uint32_t len) {
    neko_png_put32(s, len);
    s->crc = 0xFFFFFFFF;
    neko_png_put8(s, (unsigned char)id[0]);
    neko_png_put8(s, (unsigned char)id[1]);
    neko_png_put8(s, (unsigned char)id[2]);
    neko_png_put8(s, (unsigned char)id[3]);
}

static void neko_png_encode_literal(neko_png_save_png_data_t* s, uint32_t v) {
    // Encode a literal/length using the built-in tables.
    // Could do better with a custom table but whatever.
    if (v < 144)
        neko_png_put_bitsr(s, 0x030 + v - 0, 8);
    else if (v < 256)
        neko_png_put_bitsr(s, 0x190 + v - 144, 9);
    else if (v < 280)
        neko_png_put_bitsr(s, 0x000 + v - 256, 7);
    else
        neko_png_put_bitsr(s, 0x0c0 + v - 280, 8);
}

static void neko_png_encode_len(neko_png_save_png_data_t* s, uint32_t code, uint32_t bits, uint32_t len) {
    neko_png_encode_literal(s, code + (len >> bits));
    neko_png_put_bits(s, len, bits);
    neko_png_put_bits(s, 0, 5);
}

static void neko_png_end_run(neko_png_save_png_data_t* s) {
    s->runlen--;
    neko_png_encode_literal(s, s->prev);

    if (s->runlen >= 67)
        neko_png_encode_len(s, 277, 4, s->runlen - 67);
    else if (s->runlen >= 35)
        neko_png_encode_len(s, 273, 3, s->runlen - 35);
    else if (s->runlen >= 19)
        neko_png_encode_len(s, 269, 2, s->runlen - 19);
    else if (s->runlen >= 11)
        neko_png_encode_len(s, 265, 1, s->runlen - 11);
    else if (s->runlen >= 3)
        neko_png_encode_len(s, 257, 0, s->runlen - 3);
    else
        while (s->runlen--) neko_png_encode_literal(s, s->prev);
}

static void neko_png_encode_byte(neko_png_save_png_data_t* s, uint8_t v) {
    neko_png_update_adler(s, v);

    // Simple RLE compression. We could do better by doing a search
    // to find matches, but this works pretty well TBH.
    if (s->prev == v && s->runlen < 115)
        s->runlen++;

    else {
        if (s->runlen) neko_png_end_run(s);

        s->prev = v;
        s->runlen = 1;
    }
}

static void neko_png_save_header(neko_png_save_png_data_t* s, neko_png_image_t* img) {
    const unsigned char* hdr = (const unsigned char*)"\211PNG\r\n\032\n";
    for (int i = 0; i < 8; ++i) {
        neko_png_put8(s, *hdr++);
    }
    neko_png_begin_chunk(s, "IHDR", 13);
    neko_png_put32(s, img->w);
    neko_png_put32(s, img->h);
    neko_png_put8(s, 8);  // bit depth
    neko_png_put8(s, 6);  // RGBA
    neko_png_put8(s, 0);  // compression (deflate)
    neko_png_put8(s, 0);  // filter (standard)
    neko_png_put8(s, 0);  // interlace off
    neko_png_put32(s, ~s->crc);
}

static void neko_png_save_data(neko_png_save_png_data_t* s, neko_png_image_t* img, long dataPos, long* dataSize) {
    neko_png_begin_chunk(s, "IDAT", 0);
    neko_png_put8(s, 0x08);      // zlib compression method
    neko_png_put8(s, 0x1D);      // zlib compression flags
    neko_png_put_bits(s, 3, 3);  // zlib last block + fixed dictionary

    for (int y = 0; y < img->h; ++y) {
        neko_png_pixel_t* row = &img->pix[y * img->w];
        neko_png_pixel_t prev = neko_png_make_pixel_a(0, 0, 0, 0);

        neko_png_encode_byte(s, 1);  // sub filter
        for (int x = 0; x < img->w; ++x) {
            neko_png_encode_byte(s, row[x].r - prev.r);
            neko_png_encode_byte(s, row[x].g - prev.g);
            neko_png_encode_byte(s, row[x].b - prev.b);
            neko_png_encode_byte(s, row[x].a - prev.a);
            prev = row[x];
        }
    }

    neko_png_end_run(s);
    neko_png_encode_literal(s, 256);  // terminator
    while (s->bits != 0x80) neko_png_put_bits(s, 0, 1);
    neko_png_put32(s, s->adler);
    *dataSize = (s->buflen - dataPos) - 8;
    neko_png_put32(s, ~s->crc);
}

neko_png_saved_png_t neko_png_save_to_memory(const neko_png_image_t* img) {
    neko_png_saved_png_t result = {0};
    neko_png_save_png_data_t s = {0};
    long dataPos, dataSize, fileSize;
    if (!img) return result;

    s.adler = 1;
    s.bits = 0x80;
    s.prev = 0xFFFF;
    s.bufcap = 1024;
    s.buffer = (char*)malloc(1024);

    neko_png_save_header(&s, (neko_png_image_t*)img);
    dataPos = s.buflen;
    neko_png_save_data(&s, (neko_png_image_t*)img, dataPos, &dataSize);

    // End chunk.
    neko_png_begin_chunk(&s, "IEND", 0);
    neko_png_put32(&s, ~s.crc);

    // Write back payload size.
    fileSize = s.buflen;
    s.buflen = dataPos;
    neko_png_put32(&s, dataSize);

    result.size = fileSize;
    result.data = s.buffer;
    return result;
}

int neko_png_save(const char* file_name, const neko_png_image_t* img) {
    neko_png_saved_png_t s;
    long err;
    FILE* fp = fopen(file_name, "wb");
    if (!fp) return 1;
    s = neko_png_save_to_memory(img);
    fwrite(s.data, s.size, 1, fp);
    err = ferror(fp);
    fclose(fp);
    free(s.data);
    return !err;
}

typedef struct neko_png_raw_png_t {
    const uint8_t* p;
    const uint8_t* end;
} neko_png_raw_png_t;

static uint32_t neko_png_make32(const uint8_t* s) { return (s[0] << 24) | (s[1] << 16) | (s[2] << 8) | s[3]; }

static const uint8_t* neko_png_chunk(neko_png_raw_png_t* png, const char* chunk, uint32_t minlen) {
    uint32_t len = neko_png_make32(png->p);
    const uint8_t* start = png->p;

    if (!memcmp(start + 4, chunk, 4) && len >= minlen) {
        int offset = len + 12;

        if (png->p + offset <= png->end) {
            png->p += offset;
            return start + 8;
        }
    }

    return 0;
}

static const uint8_t* neko_png_find(neko_png_raw_png_t* png, const char* chunk, uint32_t minlen) {
    const uint8_t* start;
    while (png->p < png->end) {
        uint32_t len = neko_png_make32(png->p);
        start = png->p;
        png->p += len + 12;

        if (!memcmp(start + 4, chunk, 4) && len >= minlen && png->p <= png->end) return start + 8;
    }

    return 0;
}

static int neko_png_unfilter(int w, int h, int bpp, uint8_t* raw) {
    int len = w * bpp;
    uint8_t* prev;
    int x;

    if (h > 0) {
#define FILTER_LOOP_FIRST(A)                 \
    for (x = bpp; x < len; x++) raw[x] += A; \
    break
        switch (*raw++) {
            case 0:
                break;
            case 1:
                FILTER_LOOP_FIRST(raw[x - bpp]);
            case 2:
                break;
            case 3:
                FILTER_LOOP_FIRST(raw[x - bpp] / 2);
            case 4:
                FILTER_LOOP_FIRST(neko_png_paeth(raw[x - bpp], 0, 0));
            default:
                return 0;
        }
#undef FILTER_LOOP_FIRST
    }

    prev = raw;
    raw += len;

    for (int y = 1; y < h; y++, prev = raw, raw += len) {
#define FILTER_LOOP(A, B)                  \
    for (x = 0; x < bpp; x++) raw[x] += A; \
    for (; x < len; x++) raw[x] += B;      \
    break
        switch (*raw++) {
            case 0:
                break;
            case 1:
                FILTER_LOOP(0, raw[x - bpp]);
            case 2:
                FILTER_LOOP(prev[x], prev[x]);
            case 3:
                FILTER_LOOP(prev[x] / 2, (raw[x - bpp] + prev[x]) / 2);
            case 4:
                FILTER_LOOP(prev[x], neko_png_paeth(raw[x - bpp], prev[x], prev[x - bpp]));
            default:
                return 0;
        }
#undef FILTER_LOOP
    }

    return 1;
}

static void neko_png_convert(int bpp, int w, int h, uint8_t* src, neko_png_pixel_t* dst) {
    for (int y = 0; y < h; y++) {
        // skip filter byte
        src++;

        for (int x = 0; x < w; x++, src += bpp) {
            switch (bpp) {
                case 1:
                    *dst++ = neko_png_make_pixel(src[0], src[0], src[0]);
                    break;
                case 2:
                    *dst++ = neko_png_make_pixel_a(src[0], src[0], src[0], src[1]);
                    break;
                case 3:
                    *dst++ = neko_png_make_pixel(src[0], src[1], src[2]);
                    break;
                case 4:
                    *dst++ = neko_png_make_pixel_a(src[0], src[1], src[2], src[3]);
                    break;
            }
        }
    }
}

// http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html#C.tRNS
static uint8_t neko_png_get_alpha_for_indexed_image(int index, const uint8_t* trns, uint32_t trns_len) {
    if (!trns)
        return 255;
    else if ((uint32_t)index >= trns_len)
        return 255;
    else
        return trns[index];
}

static void neko_png_depalette(int w, int h, uint8_t* src, neko_png_pixel_t* dst, const uint8_t* plte, const uint8_t* trns, uint32_t trns_len) {
    for (int y = 0; y < h; ++y) {
        // skip filter byte
        ++src;

        for (int x = 0; x < w; ++x, ++src) {
            int c = *src;
            uint8_t r = plte[c * 3];
            uint8_t g = plte[c * 3 + 1];
            uint8_t b = plte[c * 3 + 2];
            uint8_t a = neko_png_get_alpha_for_indexed_image(c, trns, trns_len);
            *dst++ = neko_png_make_pixel_a(r, g, b, a);
        }
    }
}

static uint32_t neko_png_get_chunk_byte_length(const uint8_t* chunk) { return neko_png_make32(chunk - 8); }

static int neko_png_out_size(neko_png_image_t* img, int bpp) { return (img->w + 1) * img->h * bpp; }

neko_png_image_t neko_png_load_mem(const void* png_data, int png_length) {
    const char* sig = "\211PNG\r\n\032\n";
    const uint8_t *ihdr, *first, *plte, *trns;
    int bit_depth, color_type, bpp, w, h, pix_bytes;
    int compression, filter, interlace;
    int datalen, offset;
    uint8_t* out;
    neko_png_image_t img = {0};
    uint8_t* data = 0;
    neko_png_raw_png_t png;
    png.p = (uint8_t*)png_data;
    png.end = (uint8_t*)png_data + png_length;

    NEKO_PNG_CHECK(!memcmp(png.p, sig, 8), "incorrect file signature (is this a png file?)");
    png.p += 8;

    ihdr = neko_png_chunk(&png, "IHDR", 13);
    NEKO_PNG_CHECK(ihdr, "unable to find IHDR chunk");
    bit_depth = ihdr[8];
    color_type = ihdr[9];
    NEKO_PNG_CHECK(bit_depth == 8, "only bit-depth of 8 is supported");

    switch (color_type) {
        case 0:
            bpp = 1;
            break;  // greyscale
        case 2:
            bpp = 3;
            break;  // RGB
        case 3:
            bpp = 1;
            break;  // paletted
        case 4:
            bpp = 2;
            break;  // grey+alpha
        case 6:
            bpp = 4;
            break;  // RGBA
        default:
            NEKO_PNG_CHECK(0, "unknown color type");
    }

    // +1 for filter byte (which is dumb! just stick this at file header...)
    w = neko_png_make32(ihdr) + 1;
    h = neko_png_make32(ihdr + 4);
    NEKO_PNG_CHECK(w >= 1, "invalid IHDR chunk found, image width was less than 1");
    NEKO_PNG_CHECK(h >= 1, "invalid IHDR chunk found, image height was less than 1");
    NEKO_PNG_CHECK((int64_t)w * h * sizeof(neko_png_pixel_t) < INT_MAX, "image too large");
    pix_bytes = w * h * sizeof(neko_png_pixel_t);
    img.w = w - 1;
    img.h = h;
    img.pix = (neko_png_pixel_t*)malloc(pix_bytes);
    NEKO_PNG_CHECK(img.pix, "unable to allocate raw image space");

    compression = ihdr[10];
    filter = ihdr[11];
    interlace = ihdr[12];
    NEKO_PNG_CHECK(!compression, "only standard compression DEFLATE is supported");
    NEKO_PNG_CHECK(!filter, "only standard adaptive filtering is supported");
    NEKO_PNG_CHECK(!interlace, "interlacing is not supported");

    // PLTE must come before any IDAT chunk
    first = png.p;
    plte = neko_png_find(&png, "PLTE", 0);
    if (!plte)
        png.p = first;
    else
        first = png.p;

    // tRNS can come after PLTE
    trns = neko_png_find(&png, "tRNS", 0);
    if (!trns)
        png.p = first;
    else
        first = png.p;

    // Compute length of the DEFLATE stream through IDAT chunk data sizes
    datalen = 0;
    for (const uint8_t* idat = neko_png_find(&png, "IDAT", 0); idat; idat = neko_png_chunk(&png, "IDAT", 0)) {
        uint32_t len = neko_png_get_chunk_byte_length(idat);
        datalen += len;
    }

    // Copy in IDAT chunk data sections to form the compressed DEFLATE stream
    png.p = first;
    data = (uint8_t*)malloc(datalen);
    offset = 0;
    for (const uint8_t* idat = neko_png_find(&png, "IDAT", 0); idat; idat = neko_png_chunk(&png, "IDAT", 0)) {
        uint32_t len = neko_png_get_chunk_byte_length(idat);
        memcpy(data + offset, idat, len);
        offset += len;
    }

    // check for proper zlib structure in DEFLATE stream
    NEKO_PNG_CHECK(data && datalen >= 6, "corrupt zlib structure in DEFLATE stream");
    NEKO_PNG_CHECK((data[0] & 0x0f) == 0x08, "only zlib compression method (RFC 1950) is supported");
    NEKO_PNG_CHECK((data[0] & 0xf0) <= 0x70, "innapropriate window size detected");
    NEKO_PNG_CHECK(!(data[1] & 0x20), "preset dictionary is present and not supported");

    // check for integer overflow
    NEKO_PNG_CHECK(neko_png_out_size(&img, 4) >= 1, "invalid image size found");
    NEKO_PNG_CHECK(neko_png_out_size(&img, bpp) >= 1, "invalid image size found");

    out = (uint8_t*)img.pix + neko_png_out_size(&img, 4) - neko_png_out_size(&img, bpp);
    NEKO_PNG_CHECK(neko_png_inflate(data + 2, datalen - 6, out, pix_bytes), "DEFLATE algorithm failed");
    NEKO_PNG_CHECK(neko_png_unfilter(img.w, img.h, bpp, out), "invalid filter byte found");

    if (color_type == 3) {
        NEKO_PNG_CHECK(plte, "color type of indexed requires a PLTE chunk");
        uint32_t trns_len = trns ? neko_png_get_chunk_byte_length(trns) : 0;
        neko_png_depalette(img.w, img.h, out, img.pix, plte, trns, trns_len);
    } else
        neko_png_convert(bpp, img.w, img.h, out, img.pix);

    free(data);
    return img;

neko_png_err:
    free(data);
    free(img.pix);
    img.pix = 0;

    return img;
}

neko_png_image_t neko_png_load_blank(int w, int h) {
    neko_png_image_t img;
    img.w = w;
    img.h = h;
    img.pix = (neko_png_pixel_t*)malloc(w * h * sizeof(neko_png_pixel_t));
    return img;
}

neko_png_image_t neko_png_load(const char* file_name) {
    neko_png_image_t img = {0};
    int len;
    void* data = neko_png_read_file_to_memory(file_name, &len);
    if (!data) return img;
    img = neko_png_load_mem(data, len);
    free(data);
    return img;
}

void neko_png_free(neko_png_image_t* img) {
    free(img->pix);
    img->pix = 0;
    img->w = img->h = 0;
}

void neko_png_flip_image_horizontal(neko_png_image_t* img) {
    neko_png_pixel_t* pix = img->pix;
    int w = img->w;
    int h = img->h;
    int flips = h / 2;
    for (int i = 0; i < flips; ++i) {
        neko_png_pixel_t* a = pix + w * i;
        neko_png_pixel_t* b = pix + w * (h - i - 1);
        for (int j = 0; j < w; ++j) {
            neko_png_pixel_t t = *a;
            *a = *b;
            *b = t;
            ++a;
            ++b;
        }
    }
}

void neko_png_load_wh(const void* png_data, int png_length, int* w_out, int* h_out) {
    const char* sig = "\211PNG\r\n\032\n";
    const uint8_t* ihdr;
    neko_png_raw_png_t png;
    int w, h;
    png.p = (uint8_t*)png_data;
    png.end = (uint8_t*)png_data + png_length;

    if (w_out) *w_out = 0;
    if (h_out) *h_out = 0;

    NEKO_PNG_CHECK(!memcmp(png.p, sig, 8), "incorrect file signature (is this a png file?)");
    png.p += 8;

    ihdr = neko_png_chunk(&png, "IHDR", 13);
    NEKO_PNG_CHECK(ihdr, "unable to find IHDR chunk");

    // +1 for filter byte (which is dumb! just stick this at file header...)
    w = neko_png_make32(ihdr) + 1;
    h = neko_png_make32(ihdr + 4);
    if (w_out) *w_out = w - 1;
    if (h_out) *h_out = h;

neko_png_err:;
}

neko_png_indexed_image_t neko_png_load_indexed(const char* file_name) {
    neko_png_indexed_image_t img = {0};
    int len;
    void* data = neko_png_read_file_to_memory(file_name, &len);
    if (!data) return img;
    img = neko_png_load_indexed_mem(data, len);
    free(data);
    return img;
}

static void neko_png_unpack_indexed_rows(int w, int h, uint8_t* src, uint8_t* dst) {
    for (int y = 0; y < h; ++y) {
        // skip filter byte
        ++src;

        for (int x = 0; x < w; ++x, ++src) {
            *dst++ = *src;
        }
    }
}

void neko_png_unpack_palette(neko_png_pixel_t* dst, const uint8_t* plte, int plte_len, const uint8_t* trns, int trns_len) {
    for (int i = 0; i < plte_len * 3; i += 3) {
        unsigned char r = plte[i];
        unsigned char g = plte[i + 1];
        unsigned char b = plte[i + 2];
        unsigned char a = neko_png_get_alpha_for_indexed_image(i / 3, trns, trns_len);
        neko_png_pixel_t p = neko_png_make_pixel_a(r, g, b, a);
        *dst++ = p;
    }
}

neko_png_indexed_image_t neko_png_load_indexed_mem(const void* png_data, int png_length) {
    const char* sig = "\211PNG\r\n\032\n";
    const uint8_t *ihdr, *first, *plte, *trns;
    int bit_depth, color_type, bpp, w, h, pix_bytes;
    int compression, filter, interlace;
    int datalen, offset;
    int plte_len;
    uint8_t* out;
    neko_png_indexed_image_t img = {0};
    uint8_t* data = 0;
    neko_png_raw_png_t png;
    png.p = (uint8_t*)png_data;
    png.end = (uint8_t*)png_data + png_length;

    NEKO_PNG_CHECK(!memcmp(png.p, sig, 8), "incorrect file signature (is this a png file?)");
    png.p += 8;

    ihdr = neko_png_chunk(&png, "IHDR", 13);
    NEKO_PNG_CHECK(ihdr, "unable to find IHDR chunk");
    bit_depth = ihdr[8];
    color_type = ihdr[9];
    bpp = 1;  // bytes per pixel
    NEKO_PNG_CHECK(bit_depth == 8, "only bit-depth of 8 is supported");
    NEKO_PNG_CHECK(color_type == 3, "only indexed png images (images with a palette) are valid for neko_png_load_indexed_png_mem");

    // +1 for filter byte (which is dumb! just stick this at file header...)
    w = neko_png_make32(ihdr) + 1;
    h = neko_png_make32(ihdr + 4);
    NEKO_PNG_CHECK((int64_t)w * h * sizeof(uint8_t) < INT_MAX, "image too large");
    pix_bytes = w * h * sizeof(uint8_t);
    img.w = w - 1;
    img.h = h;
    img.pix = (uint8_t*)malloc(pix_bytes);
    NEKO_PNG_CHECK(img.pix, "unable to allocate raw image space");

    compression = ihdr[10];
    filter = ihdr[11];
    interlace = ihdr[12];
    NEKO_PNG_CHECK(!compression, "only standard compression DEFLATE is supported");
    NEKO_PNG_CHECK(!filter, "only standard adaptive filtering is supported");
    NEKO_PNG_CHECK(!interlace, "interlacing is not supported");

    // PLTE must come before any IDAT chunk
    first = png.p;
    plte = neko_png_find(&png, "PLTE", 0);
    if (!plte)
        png.p = first;
    else
        first = png.p;

    // tRNS can come after PLTE
    trns = neko_png_find(&png, "tRNS", 0);
    if (!trns)
        png.p = first;
    else
        first = png.p;

    // Compute length of the DEFLATE stream through IDAT chunk data sizes
    datalen = 0;
    for (const uint8_t* idat = neko_png_find(&png, "IDAT", 0); idat; idat = neko_png_chunk(&png, "IDAT", 0)) {
        uint32_t len = neko_png_get_chunk_byte_length(idat);
        datalen += len;
    }

    // Copy in IDAT chunk data sections to form the compressed DEFLATE stream
    png.p = first;
    data = (uint8_t*)malloc(datalen);
    offset = 0;
    for (const uint8_t* idat = neko_png_find(&png, "IDAT", 0); idat; idat = neko_png_chunk(&png, "IDAT", 0)) {
        uint32_t len = neko_png_get_chunk_byte_length(idat);
        memcpy(data + offset, idat, len);
        offset += len;
    }

    // check for proper zlib structure in DEFLATE stream
    NEKO_PNG_CHECK(data && datalen >= 6, "corrupt zlib structure in DEFLATE stream");
    NEKO_PNG_CHECK((data[0] & 0x0f) == 0x08, "only zlib compression method (RFC 1950) is supported");
    NEKO_PNG_CHECK((data[0] & 0xf0) <= 0x70, "innapropriate window size detected");
    NEKO_PNG_CHECK(!(data[1] & 0x20), "preset dictionary is present and not supported");

    out = img.pix;
    NEKO_PNG_CHECK(neko_png_inflate(data + 2, datalen - 6, out, pix_bytes), "DEFLATE algorithm failed");
    NEKO_PNG_CHECK(neko_png_unfilter(img.w, img.h, bpp, out), "invalid filter byte found");
    neko_png_unpack_indexed_rows(img.w, img.h, out, img.pix);

    plte_len = neko_png_get_chunk_byte_length(plte) / 3;
    neko_png_unpack_palette(img.palette, plte, plte_len, trns, neko_png_get_chunk_byte_length(trns));
    img.palette_len = (uint8_t)plte_len;

    free(data);
    return img;

neko_png_err:
    free(data);
    free(img.pix);
    img.pix = 0;

    return img;
}

void neko_png_free_indexed(neko_png_indexed_image_t* img) {
    free(img->pix);
    img->pix = 0;
    img->w = img->h = 0;
}

neko_png_image_t neko_png_depallete_indexed_image(neko_png_indexed_image_t* img) {
    neko_png_image_t out = {0};
    out.w = img->w;
    out.h = img->h;
    out.pix = (neko_png_pixel_t*)malloc(sizeof(neko_png_pixel_t) * out.w * out.h);

    neko_png_pixel_t* dst = out.pix;
    uint8_t* src = img->pix;

    for (int y = 0; y < out.h; ++y) {
        for (int x = 0; x < out.w; ++x) {
            int index = *src++;
            neko_png_pixel_t p = img->palette[index];
            *dst++ = p;
        }
    }

    return out;
}

typedef struct neko_png_v2i_t {
    int x;
    int y;
} neko_png_v2i_t;

typedef struct neko_png_integer_image_t {
    int img_index;
    neko_png_v2i_t size;
    neko_png_v2i_t min;
    neko_png_v2i_t max;
    int fit;
} neko_png_integer_image_t;

static neko_png_v2i_t neko_png_v2i(int x, int y) {
    neko_png_v2i_t v;
    v.x = x;
    v.y = y;
    return v;
}

static neko_png_v2i_t neko_png_sub(neko_png_v2i_t a, neko_png_v2i_t b) {
    neko_png_v2i_t v;
    v.x = a.x - b.x;
    v.y = a.y - b.y;
    return v;
}

static neko_png_v2i_t neko_png_add(neko_png_v2i_t a, neko_png_v2i_t b) {
    neko_png_v2i_t v;
    v.x = a.x + b.x;
    v.y = a.y + b.y;
    return v;
}

typedef struct neko_png_atlas_node_t {
    neko_png_v2i_t size;
    neko_png_v2i_t min;
    neko_png_v2i_t max;
} neko_png_atlas_node_t;

static neko_png_atlas_node_t* neko_png_best_fit(int sp, const neko_png_image_t* png, neko_png_atlas_node_t* nodes) {
    int bestVolume = INT_MAX;
    neko_png_atlas_node_t* best_node = 0;
    int width = png->w;
    int height = png->h;
    int png_volume = width * height;

    for (int i = 0; i < sp; ++i) {
        neko_png_atlas_node_t* node = nodes + i;
        int can_contain = node->size.x >= width && node->size.y >= height;
        if (can_contain) {
            int node_volume = node->size.x * node->size.y;
            if (node_volume == png_volume) return node;
            if (node_volume < bestVolume) {
                bestVolume = node_volume;
                best_node = node;
            }
        }
    }

    return best_node;
}

static int neko_png_perimeter_pred(neko_png_integer_image_t* a, neko_png_integer_image_t* b) {
    int perimeterA = 2 * (a->size.x + a->size.y);
    int perimeterB = 2 * (b->size.x + b->size.y);
    return perimeterB < perimeterA;
}

void neko_png_premultiply(neko_png_image_t* img) {
    int w = img->w;
    int h = img->h;
    int stride = w * sizeof(neko_png_pixel_t);
    uint8_t* data = (uint8_t*)img->pix;

    for (int i = 0; i < (int)stride * h; i += sizeof(neko_png_pixel_t)) {
        float a = (float)data[i + 3] / 255.0f;
        float r = (float)data[i + 0] / 255.0f;
        float g = (float)data[i + 1] / 255.0f;
        float b = (float)data[i + 2] / 255.0f;
        r *= a;
        g *= a;
        b *= a;
        data[i + 0] = (uint8_t)(r * 255.0f);
        data[i + 1] = (uint8_t)(g * 255.0f);
        data[i + 2] = (uint8_t)(b * 255.0f);
    }
}

static void neko_png_qsort(neko_png_integer_image_t* items, int count) {
    if (count <= 1) return;

    neko_png_integer_image_t pivot = items[count - 1];
    int low = 0;
    for (int i = 0; i < count - 1; ++i) {
        if (neko_png_perimeter_pred(items + i, &pivot)) {
            neko_png_integer_image_t tmp = items[i];
            items[i] = items[low];
            items[low] = tmp;
            low++;
        }
    }

    items[count - 1] = items[low];
    items[low] = pivot;
    neko_png_qsort(items, low);
    neko_png_qsort(items + low + 1, count - 1 - low);
}

static void neko_png_write_pixel(char* mem, long color) {
    mem[0] = (color >> 24) & 0xFF;
    mem[1] = (color >> 16) & 0xFF;
    mem[2] = (color >> 8) & 0xFF;
    mem[3] = (color >> 0) & 0xFF;
}

neko_png_image_t neko_png_make_atlas(int atlas_width, int atlas_height, const neko_png_image_t* pngs, int png_count, neko_png_atlas_image_t* imgs_out) {
    float w0, h0, div, wTol, hTol;
    int atlas_image_size, atlas_stride, sp;
    void* atlas_pixels = 0;
    int atlas_node_capacity = png_count * 2;
    neko_png_image_t atlas_image;
    neko_png_integer_image_t* images = 0;
    neko_png_atlas_node_t* nodes = 0;

    atlas_image.w = atlas_width;
    atlas_image.h = atlas_height;
    atlas_image.pix = 0;

    NEKO_PNG_CHECK(pngs, "pngs array was NULL");
    NEKO_PNG_CHECK(imgs_out, "imgs_out array was NULL");

    images = (neko_png_integer_image_t*)alloca(sizeof(neko_png_integer_image_t) * png_count);
    nodes = (neko_png_atlas_node_t*)malloc(sizeof(neko_png_atlas_node_t) * atlas_node_capacity);
    NEKO_PNG_CHECK(images, "out of mem");
    NEKO_PNG_CHECK(nodes, "out of mem");

    for (int i = 0; i < png_count; ++i) {
        const neko_png_image_t* png = pngs + i;
        neko_png_integer_image_t* image = images + i;
        image->fit = 0;
        image->size = neko_png_v2i(png->w, png->h);
        image->img_index = i;
    }

    // Sort PNGs from largest to smallest
    neko_png_qsort(images, png_count);

    // stack pointer, the stack is the nodes array which we will
    // allocate nodes from as necessary.
    sp = 1;

    nodes[0].min = neko_png_v2i(0, 0);
    nodes[0].max = neko_png_v2i(atlas_width, atlas_height);
    nodes[0].size = neko_png_v2i(atlas_width, atlas_height);

    // Nodes represent empty space in the atlas. Placing a texture into the
    // atlas involves splitting a node into two smaller pieces (or, if a
    // perfect fit is found, deleting the node).
    for (int i = 0; i < png_count; ++i) {
        neko_png_integer_image_t* image = images + i;
        const neko_png_image_t* png = pngs + image->img_index;
        int width = png->w;
        int height = png->h;
        neko_png_atlas_node_t* best_fit = neko_png_best_fit(sp, png, nodes);
        if (NEKO_PNG_ATLAS_MUST_FIT)
            NEKO_PNG_CHECK(best_fit, "Not enough room to place image in atlas.");
        else if (!best_fit) {
            image->fit = 0;
            continue;
        }

        image->min = best_fit->min;
        image->max = neko_png_add(image->min, image->size);

        if (best_fit->size.x == width && best_fit->size.y == height) {
            neko_png_atlas_node_t* last_node = nodes + --sp;
            *best_fit = *last_node;
            image->fit = 1;

            continue;
        }

        image->fit = 1;

        if (sp == atlas_node_capacity) {
            int new_capacity = atlas_node_capacity * 2;
            neko_png_atlas_node_t* new_nodes = (neko_png_atlas_node_t*)malloc(sizeof(neko_png_atlas_node_t) * new_capacity);
            NEKO_PNG_CHECK(new_nodes, "out of mem");
            memcpy(new_nodes, nodes, sizeof(neko_png_atlas_node_t) * sp);
            free(nodes);
            // best_fit became a dangling pointer, so relocate it
            best_fit = new_nodes + (best_fit - nodes);
            nodes = new_nodes;
            atlas_node_capacity = new_capacity;
        }

        neko_png_atlas_node_t* new_node = nodes + sp++;
        new_node->min = best_fit->min;

        // Split bestFit along x or y, whichever minimizes
        // fragmentation of empty space
        neko_png_v2i_t d = neko_png_sub(best_fit->size, neko_png_v2i(width, height));
        if (d.x < d.y) {
            new_node->size.x = d.x;
            new_node->size.y = height;
            new_node->min.x += width;

            best_fit->size.y = d.y;
            best_fit->min.y += height;
        }

        else {
            new_node->size.x = width;
            new_node->size.y = d.y;
            new_node->min.y += height;

            best_fit->size.x = d.x;
            best_fit->min.x += width;
        }

        new_node->max = neko_png_add(new_node->min, new_node->size);
    }

    // Write the final atlas image, use NEKO_PNG_ATLAS_EMPTY_COLOR as base color
    atlas_stride = atlas_width * sizeof(neko_png_pixel_t);
    atlas_image_size = atlas_width * atlas_height * sizeof(neko_png_pixel_t);
    atlas_pixels = malloc(atlas_image_size);
    NEKO_PNG_CHECK(atlas_pixels, "out of mem");

    for (int i = 0; i < atlas_image_size; i += sizeof(neko_png_pixel_t)) {
        neko_png_write_pixel((char*)atlas_pixels + i, NEKO_PNG_ATLAS_EMPTY_COLOR);
    }

    for (int i = 0; i < png_count; ++i) {
        neko_png_integer_image_t* image = images + i;

        if (image->fit) {
            const neko_png_image_t* png = pngs + image->img_index;
            char* pixels = (char*)png->pix;
            neko_png_v2i_t min = image->min;
            neko_png_v2i_t max = image->max;
            int atlas_offset = min.x * sizeof(neko_png_pixel_t);
            int tex_stride = png->w * sizeof(neko_png_pixel_t);

            for (int row = min.y, y = 0; row < max.y; ++row, ++y) {
                void* row_ptr = (char*)atlas_pixels + (row * atlas_stride + atlas_offset);
                memcpy(row_ptr, pixels + y * tex_stride, tex_stride);
            }
        }
    }

    atlas_image.pix = (neko_png_pixel_t*)atlas_pixels;

    // squeeze UVs inward by 128th of a pixel
    // this prevents atlas bleeding. tune as necessary for good results.
    w0 = 1.0f / (float)(atlas_width);
    h0 = 1.0f / (float)(atlas_height);
    div = 1.0f / 128.0f;
    wTol = w0 * div;
    hTol = h0 * div;

    for (int i = 0; i < png_count; ++i) {
        neko_png_integer_image_t* image = images + i;
        neko_png_atlas_image_t* img_out = imgs_out + i;

        img_out->img_index = image->img_index;
        img_out->w = image->size.x;
        img_out->h = image->size.y;
        img_out->fit = image->fit;

        if (image->fit) {
            neko_png_v2i_t min = image->min;
            neko_png_v2i_t max = image->max;

            float min_x = (float)min.x * w0 + wTol;
            float min_y = (float)min.y * h0 + hTol;
            float max_x = (float)max.x * w0 - wTol;
            float max_y = (float)max.y * h0 - hTol;

            // flip image on y axis
            if (NEKO_PNG_ATLAS_FLIP_Y_AXIS_FOR_UV) {
                float tmp = min_y;
                min_y = max_y;
                max_y = tmp;
            }

            img_out->minx = min_x;
            img_out->miny = min_y;
            img_out->maxx = max_x;
            img_out->maxy = max_y;
        }
    }

    free(nodes);
    return atlas_image;

neko_png_err:
    free(atlas_pixels);
    free(nodes);
    atlas_image.pix = 0;
    return atlas_image;
}

int neko_png_default_save_atlas(const char* out_path_image, const char* out_path_atlas_txt, const neko_png_image_t* atlas, const neko_png_atlas_image_t* imgs, int img_count, const char** names) {
    FILE* fp = fopen(out_path_atlas_txt, "wt");
    NEKO_PNG_CHECK(fp, "unable to open out_path_atlas_txt in neko_png_default_save_atlas");

    fprintf(fp, "%s\n%d\n\n", out_path_image, img_count);

    for (int i = 0; i < img_count; ++i) {
        const neko_png_atlas_image_t* image = imgs + i;
        const char* name = names ? names[image->img_index] : 0;

        if (image->fit) {
            int width = image->w;
            int height = image->h;
            float min_x = image->minx;
            float min_y = image->miny;
            float max_x = image->maxx;
            float max_y = image->maxy;

            if (name)
                fprintf(fp, "{ \"%s\", w = %d, h = %d, u = { %.10f, %.10f }, v = { %.10f, %.10f } }\n", name, width, height, min_x, min_y, max_x, max_y);
            else
                fprintf(fp, "{ w = %d, h = %d, u = { %.10f, %.10f }, v = { %.10f, %.10f } }\n", width, height, min_x, min_y, max_x, max_y);
        }
    }

    // Save atlas image PNG to disk
    NEKO_PNG_CHECK(neko_png_save(out_path_image, atlas), "failed to save atlas image to disk");

neko_png_err:
    fclose(fp);
    return 0;
}

#endif  // NEKO_PNG_IMPLEMENTATION_ONCE
#endif  // NEKO_PNG_IMPLEMENTATION
