#ifndef NEKO_DRAW_H
#define NEKO_DRAW_H

#include "engine/asset.h"
#include "engine/base.h"
#include "engine/graphics.h"

// deps
#include <stb_truetype.h>

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

NEKO_SCRIPT(sprite,

            NEKO_EXPORT void sprite_set_atlas(const char* filename);

            NEKO_EXPORT const char* sprite_get_atlas();

            NEKO_EXPORT void sprite_add(Entity ent);

            NEKO_EXPORT void sprite_remove(Entity ent);

            NEKO_EXPORT bool sprite_has(Entity ent);

            // size to draw in world units, centered at transform position
            NEKO_EXPORT void sprite_set_size(Entity ent, vec2 size);

            NEKO_EXPORT vec2 sprite_get_size(Entity ent);

            // bottom left corner of atlas region in pixels
            NEKO_EXPORT void sprite_set_texcell(Entity ent, vec2 texcell);

            NEKO_EXPORT vec2 sprite_get_texcell(Entity ent);

            // size of atlas region in pixels
            NEKO_EXPORT void sprite_set_texsize(Entity ent, vec2 texsize);

            NEKO_EXPORT vec2 sprite_get_texsize(Entity ent);

            // lower depth drawn on top
            NEKO_EXPORT void sprite_set_depth(Entity ent, int depth);

            NEKO_EXPORT int sprite_get_depth(Entity ent);

)

void sprite_init();
void sprite_fini();
void sprite_update_all();
void sprite_draw_all();
void sprite_save_all(Store* s);
void sprite_load_all(Store* s);

int open_mt_sprite(lua_State* L);
int neko_sprite_load(lua_State* L);

#endif
