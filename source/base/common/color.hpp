#pragma once

#include "base/common/base.hpp"

#include <stdexcept>
#include <sstream>
#include <iomanip>

struct Color {
    f32 r, g, b, a;
};

struct Color256 {
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

inline u32 color256_from_rgb(Color rgb) {
    i8 r = (i8)(rgb.r * 255.0);
    i8 g = (i8)(rgb.g * 255.0);
    i8 b = (i8)(rgb.b * 255.0);

    return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

inline Color color256_to_rgb(u32 color) {
    int r = (color >> 16) & 0xFF;
    int g = (color >> 8) & 0xFF;
    int b = color & 0xFF;

    float rf = (float)r / 255.0f;
    float gf = (float)g / 255.0f;
    float bf = (float)b / 255.0f;

    return Color{rf, gf, bf, 1.0f};
}

inline Color256 ParseHexColor(const std::string& hex) {
    if (hex.empty() || hex[0] != '#') {
        throw std::invalid_argument("Invalid hex color string: must start with '#'");
    }

    std::string hex_code = hex.substr(1);

    if (hex_code.length() == 8) {
        uint32_t value;
        std::istringstream(hex_code) >> std::hex >> value;
        return Color256{
                static_cast<uint8_t>((value >> 24) & 0xFF),  // r
                static_cast<uint8_t>((value >> 16) & 0xFF),  // g
                static_cast<uint8_t>((value >> 8) & 0xFF),   // b
                static_cast<uint8_t>(value & 0xFF)           // a
        };
    } else if (hex_code.length() == 6) {
        uint32_t value;
        std::istringstream(hex_code) >> std::hex >> value;
        return Color256{
                static_cast<uint8_t>((value >> 16) & 0xFF),  // r
                static_cast<uint8_t>((value >> 8) & 0xFF),   // g
                static_cast<uint8_t>(value & 0xFF),          // b
                static_cast<uint8_t>(0xFF)                   // a
        };
    } else {
        throw std::invalid_argument("Invalid hex color string length: must be 6 or 8 characters after '#'");
    }
}

}  // namespace Neko
