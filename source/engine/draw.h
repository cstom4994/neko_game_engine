#ifndef NEKO_DRAW_H
#define NEKO_DRAW_H

#include "engine/asset.h"
#include "engine/base.hpp"
#include "engine/graphics.h"
#include "engine/sprite.h"
#include "engine/event.h"

// deps
#include "extern/stb_truetype.h"

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

#define NEKO_FONT_BAKED_SIZE 256

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

float draw_font(FontFamily* font, bool draw_in_world, float size, float x, float y, String text, Color256 col);
float draw_font_wrapped(FontFamily* font, bool draw_in_world, float size, float x, float y, String text, Color256 col, float limit);

void font_init();

namespace Neko {

namespace mt_font {
int open_mt_font(lua_State* L);
int neko_font_load(lua_State* L);
}  // namespace mt_font

namespace mt_sprite {
int open_mt_sprite(lua_State* L);
int neko_sprite_load(lua_State* L);
}  // namespace mt_sprite

}  // namespace Neko

typedef struct {
    float position[2];
    float texcoord[2];
} Vertex;

typedef struct batch_renderer {

    // vertex buffer data
    GLuint vao;
    GLuint vbo;
    int vertex_count;
    int vertex_capacity;
    Vertex* vertices;

    float scale;

    GLuint texture;

    bool outline;
    bool glow;
    bool bloom;
    bool trans;
} batch_renderer;

NEKO_API() batch_renderer* batch_init(int vertex_capacity);
NEKO_API() void batch_fini(batch_renderer* batch);
NEKO_API() int batch_update_all(Event evt);
NEKO_API() void batch_draw_all(batch_renderer* batch);

NEKO_API() void batch_flush(batch_renderer* renderer);
NEKO_API() void batch_texture(batch_renderer* renderer, GLuint id);
NEKO_API() void batch_push_vertex(batch_renderer* renderer, float x, float y, float u, float v);

void debug_draw_all();
void debug_draw_init();
void debug_draw_fini();
void debug_draw_add_line(vec2 a, f32 line_width, Color color);
void debug_draw_add_line(vec2 a, vec2 b, f32 line_width, Color color);
void debug_draw_circle(vec2 center, f32 radius, int segment_count, f32 line_width, Color color);

#endif