#pragma once

#include "base/common/base.hpp"

NEKO_SCRIPT(
        color,

        struct Store;

        typedef struct Color Color; struct Color { f32 r, g, b, a; };

        typedef struct Color256 Color256; struct Color256 {
            union {
                uint8_t rgba[4];
                struct {
                    uint8_t r, g, b, a;
                };
            };
        };

        NEKO_EXPORT Color color(f32 r, f32 g, f32 b, f32 a);

        NEKO_EXPORT Color color_opaque(f32 r, f32 g, f32 b);

        NEKO_EXPORT Color color_black;

        NEKO_EXPORT Color color_white;

        NEKO_EXPORT Color color_gray;

        NEKO_EXPORT Color color_red;

        NEKO_EXPORT Color color_green;

        NEKO_EXPORT Color color_blue;

        NEKO_EXPORT Color color_clear;  // zero alpha

)

namespace Neko {

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

}  // namespace Neko
