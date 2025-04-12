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

    stbtt_aligned_quad quad(u32* img, float* x, float* y, float size, i32 ch, float xscale = 1.0f);
    float width(float size, String text);
};

FontFamily* neko_default_font();

float draw_font(FontFamily* font, bool draw_in_world, float size, float x, float y, String text, Color256 col, f32 scale = 1.0f);
float draw_font_wrapped(FontFamily* font, bool draw_in_world, float size, float x, float y, String text, Color256 col, float limit, f32 scale = 1.0f);

class Font : public SingletonClass<Font> {
public:
    Asset font_shader{};
    GLuint font_vbo, font_vao;

public:
    void font_init();
};

typedef struct {
    float position[2];
    float texcoord[2];
} BatchVertex;

struct batch_renderer {

    // vertex buffer data
    GLuint vao;
    GLuint vbo;
    int vertex_count;
    int vertex_capacity;
    BatchVertex* vertices;

    float scale;

    GLuint texture_id;

    bool outline;
    bool glow;
    bool bloom;
    bool trans;
};

class Batch : public SingletonClass<Batch> {
private:
    batch_renderer* batch;
    Asset shader_asset{};

public:
    void batch_init(int vertex_capacity);
    void batch_fini();
    int batch_update_all(Event evt);
    void batch_draw_all();

    void batch_flush();
    void batch_texture(GLuint id);
    void batch_push_vertex(float x, float y, float u, float v);

    inline batch_renderer* GetBatch() { return batch; }
};

#define MAX_VERTS 3 * 2048 * 16

struct LineVertex {
    union {
        struct {
            vec3 pos;
            float width;
        };
        vec4 pos_width;
    };
    Color col;
};

struct DebugRenderer {
    Asset lines_shader = {};

    GLuint program_id;
    GLuint vao;
    GLuint vbo;

    GLuint view;
    GLuint viewport_size;
    GLuint aa_radius;

    GLuint pos_width;
    GLuint col;

    u32 buf_cap;
    u32 buf_len;
    LineVertex* buf;
};

class DebugDraw : public SingletonClass<DebugDraw> {
private:
    DebugRenderer* debug_renderer;

public:
    void debug_draw_all();
    void debug_draw_init();
    void debug_draw_fini();

    void debug_draw_add_point(vec2 a, f32 line_width, Color color);         // 只添加一个顶点
    void debug_draw_add_line(vec2 a, vec2 b, f32 line_width, Color color);  // 添加线段
    void debug_draw_circle(vec2 center, f32 radius, int segment_count, f32 line_width, Color color);
    void debug_draw_aabb(vec2 min, vec2 max, f32 line_width, Color color);
    void debug_draw_capsule(vec2 a, vec2 b, f32 radius, u32 segment_count, f32 line_width, Color color);
    void debug_draw_half_circle(vec2 center, f32 radius, vec2 direction, u32 segment_count, f32 line_width, Color color);
    void debug_draw_manifold(vec2* points, u32 point_count, f32 line_width, Color color);

private:
    u32 draw_line_update(const void* data, i32 n_elems, i32 elem_size);
};

#endif