

#ifndef NEKO_COMMON_H
#define NEKO_COMMON_H

#include "neko_prelude.h"

/*================================================================================
// Utils
================================================================================*/

// 将UTF-8编码字符转换为Unicode
NEKO_FORCE_INLINE u32 neko_utf8_to_unicode(const_str utf8, i32* bytes_read) {
    u32 unicode = 0;
    i32 len = 0;
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

    for (i32 i = 1; i < len; i++) {
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

NEKO_FORCE_INLINE u32 neko_darken_color(u32 color, f32 brightness) {
    i32 a = (color >> 24) & 0xFF;
    i32 r = (i32)(((color >> 16) & 0xFF) * brightness);
    i32 g = (i32)(((color >> 8) & 0xFF) * brightness);
    i32 b = (i32)((color & 0xFF) * brightness);
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

NEKO_FORCE_INLINE bool neko_token_is_end_of_line(char c) { return (c == '\n' || c == '\r'); }
NEKO_FORCE_INLINE bool neko_token_char_is_white_space(char c) { return (c == '\t' || c == ' ' || neko_token_is_end_of_line(c)); }
NEKO_FORCE_INLINE bool neko_token_char_is_alpha(char c) { return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')); }
NEKO_FORCE_INLINE bool neko_token_char_is_numeric(char c) { return (c >= '0' && c <= '9'); }

#endif
