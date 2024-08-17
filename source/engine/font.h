#ifndef NEKO_FONT_H
#define NEKO_FONT_H

#include "engine/base.h"
#include "engine/math.h"
#include "engine/texture.h"

// deps
#include <stb_truetype.h>

#define NEKO_FONT_BAKED_SIZE 1024

struct FontRange {
    stbtt_bakedchar chars[NEKO_FONT_BAKED_SIZE];
    Texture tex;
};

struct FontQuad {
    stbtt_aligned_quad quad;
};

struct FontFamily {
    String ttf;
    HashMap<FontRange> ranges;
    StringBuilder sb;

    bool load(String filepath);
    void trash();

    stbtt_aligned_quad quad(u32* img, float* x, float* y, float size, i32 ch);
    float width(float size, String text);
};

FontFamily* neko_default_font();

#endif