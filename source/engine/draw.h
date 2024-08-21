#ifndef NEKO_DRAW_H
#define NEKO_DRAW_H

#include "engine/asset.h"
#include "engine/base.h"
#include "engine/gfx.h"

struct DrawDescription {
    float x;
    float y;
    float rotation;

    float sx;  // scale
    float sy;

    float ox;  // offset
    float oy;

    float u0;  // uv coords
    float v0;
    float u1;
    float v1;
};

struct RectDescription {
    float x;
    float y;
    float w;
    float h;

    float rotation;

    float sx;  // scale
    float sy;

    float ox;  // origin
    float oy;
};

#if 0
void renderer_reset();
void renderer_use_sampler(u32 sampler);
void renderer_get_clear_color(float* rgba);
void renderer_set_clear_color(float* rgba);
void renderer_apply_color();
bool renderer_push_color(Color c);
bool renderer_pop_color();
bool renderer_push_matrix();
bool renderer_pop_matrix();
Matrix4 renderer_peek_matrix();
void renderer_set_top_matrix(Matrix4 mat);
void renderer_translate(float x, float y);
void renderer_rotate(float angle);
void renderer_scale(float x, float y);
void renderer_push_quad(Vector4 pos, Vector4 tex);
void renderer_push_xy(float x, float y);

void draw_image(const Image *img, DrawDescription *desc);
void draw_tilemap(const MapLdtk *tm);
void draw_filled_rect(RectDescription *desc);
void draw_line_rect(RectDescription *desc);
void draw_line_circle(float x, float y, float radius);
void draw_line(float x0, float y0, float x1, float y1);
#endif

void draw_sprite(AseSprite* spr, DrawDescription* desc);

struct FontFamily;

struct lua_State;
DrawDescription draw_description_args(lua_State* L, i32 arg_start);
RectDescription rect_description_args(lua_State* L, i32 arg_start);

#endif

#ifndef NEKO_FONT_H
#define NEKO_FONT_H

#include "engine/asset.h"
#include "engine/base.h"
#include "engine/gfx.h"
#include "engine/math.h"

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

float draw_font(FontFamily* font, float size, float x, float y, String text, Color256 col);
float draw_font_wrapped(FontFamily* font, float size, float x, float y, String text, Color256 col, float limit);

void font_init();
void font_draw_all();

int open_mt_font(lua_State* L);

int neko_font_load(lua_State* L);

#endif
