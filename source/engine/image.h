
#ifndef NEKO_IMAGE_H
#define NEKO_IMAGE_H

#include "engine/base.h"
#include "engine/prelude.h"
#include "engine/texture.h"

// deps
#include <stb_truetype.h>

#if 0

struct FontRange {
    stbtt_bakedchar chars[256];
    Image image;
};

struct FontQuad {
    stbtt_aligned_quad quad;
};

struct FontFamily {
    String ttf;
    HashMap<FontRange> ranges;
    StringBuilder sb;

    bool load(String filepath);
    void load_default();
    void trash();

    stbtt_aligned_quad quad(u32 *img, float *x, float *y, float size, i32 ch);
    float width(float size, String text);
};

struct AtlasImage {
    float u0, v0, u1, v1;
    float width;
    float height;
    Image img;
};

struct Atlas {
    HashMap<AtlasImage> by_name;
    Image img;

    bool load(String filepath, bool generate_mips);
    void trash();
    AtlasImage *get(String name);
};

#endif

#endif