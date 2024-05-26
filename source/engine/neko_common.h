

#ifndef NEKO_COMMON_H
#define NEKO_COMMON_H

#include "neko.h"

NEKO_API_DECL unsigned char* neko_base64_encode(unsigned char* str);
NEKO_API_DECL unsigned char* neko_base64_decode(unsigned char* code);

#ifndef NEKO_SEXPR
#define NEKO_SEXPR

// neko_s_expression

enum neko_sexpr_types {
    NEKO_SEXPR_TYPE_NONE,
    NEKO_SEXPR_TYPE_INT,
    NEKO_SEXPR_TYPE_FLOAT,
    NEKO_SEXPR_TYPE_STRING,
    NEKO_SEXPR_TYPE_ARRAY,  // 数组包括原始数组和具名节点
};

typedef struct neko_sexpr_data_entry_s {
    enum neko_sexpr_types type;
    union {
        s32 i;
        f32 f;
        char* str;
        struct neko_sexpr_data_entry_s* node;
    };

    s32 is_str_allocated;

    //////////////////
    // NEKO_SEXPR_TYPE_ARRAY stuff
    s32 node_len;
    s32 is_node_allocated;
    s32 len_is_fixed;

    // for pretty serialisation
    s32 new_line_at_start;
    s32 new_line_at_end;
    s32 new_line_at_end_of_subsequent_elements;
} neko_sexpr_data_entry_t;

typedef neko_sexpr_data_entry_t neko_snode_t;

#define neko_sexpr_i(__i) \
    { .type = NEKO_SEXPR_TYPE_INT, .i = __i }

#define neko_sexpr_f(__f) \
    { .type = NEKO_SEXPR_TYPE_FLOAT, .f = __f }

#define neko_sexpr_s(__s) \
    { .type = NEKO_SEXPR_TYPE_STRING, .str = __s }

#define neko_sexpr_a(__count, ...)                                                                   \
    {                                                                                                \
        .type = NEKO_SEXPR_TYPE_ARRAY, .node_len = __count, .node = (neko_snode_t[]) { __VA_ARGS__ } \
    }

#define neko_sexpr_new_a(__count, ...)                                                                                                            \
    {                                                                                                                                             \
        .new_line_at_end_of_subsequent_elements = 1, .type = NEKO_SEXPR_TYPE_ARRAY, .node_len = __count, .node = (neko_snode_t[]) { __VA_ARGS__ } \
    }

#define neko_sexpr_tagged_i(__name, __i)                                                                                   \
    {                                                                                                                      \
        .type = NEKO_SEXPR_TYPE_ARRAY, .node_len = 2, .node = (neko_snode_t[]) { neko_sexpr_s(__name), neko_sexpr_i(__i) } \
    }

#define neko_sexpr_tagged_f(__name, __f)                                                                                   \
    {                                                                                                                      \
        .type = NEKO_SEXPR_TYPE_ARRAY, .node_len = 2, .node = (neko_snode_t[]) { neko_sexpr_s(__name), neko_sexpr_f(__f) } \
    }

#define neko_sexpr_tagged_s(__name, __s)                                                                                   \
    {                                                                                                                      \
        .type = NEKO_SEXPR_TYPE_ARRAY, .node_len = 2, .node = (neko_snode_t[]) { neko_sexpr_s(__name), neko_sexpr_s(__s) } \
    }

#define neko_sexpr_tagged_a(__name, __count, ...) neko_sexpr_a(__count + 1, neko_sexpr_s(__name), __VA_ARGS__)

#define neko_sexpr_tagged_new_a(__name, __count, ...) neko_sexpr_new_a(__count + 1, neko_sexpr_s(__name), __VA_ARGS__)

extern s32 neko_sexpr_node_fits_format(neko_snode_t* node, neko_snode_t* fmt);

extern neko_snode_t* neko_sexpr_node_get_value(neko_snode_t* node, s32* len);  // 返回 node->node + 1 如果 node->node 是字符串 (判定为具名节点)
extern neko_snode_t* neko_sexpr_node_get_tagged(neko_snode_t* node, const_str tag);
extern neko_snode_t* neko_sexpr_node_get_index(neko_snode_t* node, s32 index);

extern s32 neko_sexpr_write_to_file(char* filename, const neko_snode_t node);

extern neko_snode_t neko_sexpr_parse_file(char* filename);
extern neko_snode_t neko_sexpr_parse_memory(char* mem, s32 sz);

extern void neko_sexpr_free_node(neko_snode_t* node);
extern neko_snode_t neko_sexpr_dup_node(neko_snode_t* node, s32 free_old_node);

#endif

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
NEKO_INLINE u32 neko_utf8_to_unicode(const_str utf8, s32* bytes_read) {
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
NEKO_FORCE_INLINE
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
NEKO_FORCE_INLINE
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

NEKO_FORCE_INLINE
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

NEKO_INLINE b8 neko_token_is_end_of_line(char c) { return (c == '\n' || c == '\r'); }
NEKO_INLINE b8 neko_token_char_is_white_space(char c) { return (c == '\t' || c == ' ' || neko_token_is_end_of_line(c)); }
NEKO_INLINE b8 neko_token_char_is_alpha(char c) { return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')); }
NEKO_INLINE b8 neko_token_char_is_numeric(char c) { return (c >= '0' && c <= '9'); }

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

NEKO_INLINE u32 neko_darken_color(u32 color, f32 brightness) {
    s32 a = (color >> 24) & 0xFF;
    s32 r = (s32)(((color >> 16) & 0xFF) * brightness);
    s32 g = (s32)(((color >> 8) & 0xFF) * brightness);
    s32 b = (s32)((color & 0xFF) * brightness);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

NEKO_INLINE void neko_tex_flip_vertically(int width, int height, u8* data) {
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

#endif
