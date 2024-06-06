

#ifndef NEKO_COMMON_H
#define NEKO_COMMON_H

#include "neko.h"

NEKO_API_DECL const char* neko_base64_encode(const char* str);
NEKO_API_DECL const char* neko_base64_decode(const char* code);

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
    char* text;
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
