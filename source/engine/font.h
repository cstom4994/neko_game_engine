#ifndef NEKO_FONT_H
#define NEKO_FONT_H

#include "engine/base.h"
#include "engine/batch.h"
#include "engine/math.h"
#include "engine/texture.h"

// deps
#include <stb_truetype.h>

#define NEKO_FONT_BAKED_SIZE 128

struct FontRange {
    stbtt_bakedchar chars[NEKO_FONT_BAKED_SIZE];
    AssetTexture tex;
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

struct idraw_t;
float draw_font(idraw_t* idraw, FontFamily* font, float size, float x, float y, String text, Color256 col);
float draw_font_wrapped(idraw_t* idraw, FontFamily* font, float size, float x, float y, String text, Color256 col, float limit);

void font_init();
void font_draw_all();

int open_mt_font(lua_State* L);

int neko_font_load(lua_State* L);

#endif