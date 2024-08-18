#pragma once

#include "engine/asset.h"
#include "engine/base.h"
#include "engine/gfx.h"

struct DrawDescription {
    float x;
    float y;
    float rotation;

    float sx;  // scale
    float sy;

    float ox;  // origin
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

/*================================================================================
// IDraw
================================================================================*/

neko_enum_decl(neko_idraw_matrix_type, NEKO_IDRAW_MATRIX_MODELVIEW, NEKO_IDRAW_MATRIX_PROJECTION);

typedef enum { NEKO_IDRAW_VATTR_POSITION = 0x00, NEKO_IDRAW_VATTR_UV, NEKO_IDRAW_VATTR_COLOR } neko_idraw_vattr_type;

neko_enum_decl(neko_idraw_layout_type,
               NEKO_IDRAW_LAYOUT_VATTR  // Using vattr type directly
                                        // NEKO_IDRAW_LAYOUT_MESH  // Using asset mesh types indirectly to be converted internally
);

// Need a configurable pipeline matrix
/*
    depth | stencil | face cull | blend | prim
    e/d     e/d         e/d         e/d     l/t

    2 ^ 5 = 32 generated pipeline choices.
*/

enum { NEKO_IDRAW_FLAG_NO_BIND_UNIFORMS = (1 << 0), NEKO_IDRAW_FLAG_NO_BIND_CACHED_PIPELINES = (1 << 1) };

// Hash bytes of state attr struct to get index key for pipeline
typedef struct neko_idraw_pipeline_state_attr_t {
    uint16_t depth_enabled;
    uint16_t stencil_enabled;
    uint16_t blend_enabled;
    uint16_t face_cull_enabled;
    uint16_t prim_type;
} neko_idraw_pipeline_state_attr_t;

typedef struct neko_immediate_vert_t {
    vec3 position;
    vec2 uv;
    Color256 color;
} neko_immediate_vert_t;

typedef struct neko_immediate_cache_t {
    neko_dyn_array(neko_handle(gfx_pipeline_t)) pipelines;
    neko_dyn_array(mat4) modelview;
    neko_dyn_array(mat4) projection;
    neko_dyn_array(neko_idraw_matrix_type) modes;
    vec2 uv;
    Color256 color;
    neko_handle(gfx_texture_t) texture;
    neko_idraw_pipeline_state_attr_t pipeline;
} neko_immediate_cache_t;

typedef struct neko_immediate_draw_static_data_t {
    neko_handle(gfx_texture_t) tex_default;
    neko_hash_table(neko_idraw_pipeline_state_attr_t, neko_handle(gfx_pipeline_t)) pipeline_table;
    neko_handle(gfx_uniform_t) uniform;
    neko_handle(gfx_uniform_t) sampler;
    neko_handle(gfx_vertex_buffer_t) vbo;
    neko_handle(gfx_index_buffer_t) ibo;
} neko_immediate_draw_static_data_t;

typedef struct idraw_t {
    neko_byte_buffer_t vertices;
    neko_dyn_array(uint16_t) indices;
    neko_dyn_array(neko_idraw_vattr_type) vattributes;
    neko_immediate_cache_t cache;
    neko_command_buffer_t commands;
    u32 window_handle;
    neko_immediate_draw_static_data_t* data;
    u32 flags;
} idraw_t;

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
float draw_font(idraw_t* idraw, FontFamily* font, float size, float x, float y, String text, Color256 col);
float draw_font_wrapped(idraw_t* idraw, FontFamily* font, float size, float x, float y, String text, Color256 col, float limit);

struct lua_State;
DrawDescription draw_description_args(lua_State* L, i32 arg_start);
RectDescription rect_description_args(lua_State* L, i32 arg_start);

#ifndef NEKO_IDRAW
#define NEKO_IDRAW

/*================================================================================
// IDraw
================================================================================*/

typedef idraw_t gsid;
#define neko_idraw_create neko_immediate_draw_new
#define neko_idraw_free neko_immediate_draw_free

// Create / Init / Shutdown / Free
idraw_t neko_immediate_draw_new();
void neko_immediate_draw_free(idraw_t* neko_idraw);
void neko_immediate_draw_static_data_set(neko_immediate_draw_static_data_t* data);
void neko_immediate_draw_static_data_free();
neko_immediate_draw_static_data_t* neko_immediate_draw_static_data_get();  // 用于热更新

// Get pipeline from state
neko_handle(gfx_pipeline_t) neko_idraw_get_pipeline(idraw_t* neko_idraw, neko_idraw_pipeline_state_attr_t state);

// Core Vertex Functions
void neko_idraw_begin(idraw_t* neko_idraw, gfx_primitive_type type);
void neko_idraw_end(idraw_t* neko_idraw);
void neko_idraw_tc2f(idraw_t* neko_idraw, f32 u, f32 v);
void neko_idraw_tc2fv(idraw_t* neko_idraw, vec2 uv);
void neko_idraw_c4ub(idraw_t* neko_idraw, u8 r, u8 g, u8 b, u8 a);
void neko_idraw_c4ubv(idraw_t* neko_idraw, Color256 c);
void neko_idraw_v2f(idraw_t* neko_idraw, f32 x, f32 y);
void neko_idraw_v2fv(idraw_t* neko_idraw, vec2 v);
void neko_idraw_v3f(idraw_t* neko_idraw, f32 x, f32 y, f32 z);
void neko_idraw_v3fv(idraw_t* neko_idraw, vec3 v);
void neko_idraw_flush(idraw_t* neko_idraw);
void neko_idraw_texture(idraw_t* neko_idraw, neko_handle(gfx_texture_t) texture);

// neko_hsv_t neko_rgb_to_hsv(Color256 c);
f32 neko_hue_dist(f32 h1, f32 h2);

void neko_idraw_rect_textured(idraw_t* neko_idraw, vec2 a, vec2 b, u32 tex_id, Color256 color);
void neko_idraw_rect_textured_ext(idraw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, f32 u0, f32 v0, f32 u1, f32 v1, u32 tex_id, Color256 color);

// Core pipeline functions
void neko_idraw_blend_enabled(idraw_t* neko_idraw, bool enabled);
void neko_idraw_depth_enabled(idraw_t* neko_idraw, bool enabled);
void neko_idraw_stencil_enabled(idraw_t* neko_idraw, bool enabled);
void neko_idraw_face_cull_enabled(idraw_t* neko_idraw, bool enabled);
void neko_idraw_defaults(idraw_t* neko_idraw);
void neko_idraw_pipeline_set(idraw_t* neko_idraw,
                             neko_handle(gfx_pipeline_t) pipeline);                         // Binds custom user pipeline, sets flag NEKO_IDRAW_FLAG_NO_BIND_CACHED_PIPELINES
void neko_idraw_vattr_list(idraw_t* neko_idraw, neko_idraw_vattr_type* layout, size_t sz);  // Sets user vertex attribute list for custom bound pipeline
// void neko_idraw_vattr_list_mesh(idraw_t* neko_idraw, neko_asset_mesh_layout_t* layout,
//                                 size_t sz);  // Same as above but uses mesh layout to determine which vertex attributes to bind and in what order

// View/Scissor commands
void neko_idraw_set_view_scissor(idraw_t* neko_idraw, u32 x, u32 y, u32 w, u32 h);

// Final Submit / Merge
void neko_idraw_draw(idraw_t* neko_idraw, neko_command_buffer_t* cb);
void neko_idraw_renderpass_submit(idraw_t* neko_idraw, neko_command_buffer_t* cb, vec4 viewport, Color256 clear_color);
void neko_idraw_renderpass_submit_ex(idraw_t* neko_idraw, neko_command_buffer_t* cb, vec4 viewport, gfx_clear_action_t action);

// Core Matrix Functions
void neko_idraw_push_matrix(idraw_t* neko_idraw, neko_idraw_matrix_type type);
void neko_idraw_push_matrix_ex(idraw_t* neko_idraw, neko_idraw_matrix_type type, bool flush);
void neko_idraw_pop_matrix(idraw_t* neko_idraw);
void neko_idraw_pop_matrix_ex(idraw_t* neko_idraw, bool flush);
//  void neko_idraw_matrix_mode(idraw_t* neko_idraw, neko_idraw_matrix_type type);
void neko_idraw_load_matrix(idraw_t* neko_idraw, mat4 m);
void neko_idraw_mul_matrix(idraw_t* neko_idraw, mat4 m);
void neko_idraw_perspective(idraw_t* neko_idraw, f32 fov, f32 aspect, f32 near, f32 far);
void neko_idraw_ortho(idraw_t* neko_idraw, f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
void neko_idraw_rotatef(idraw_t* neko_idraw, f32 angle, f32 x, f32 y, f32 z);
void neko_idraw_rotatev(idraw_t* neko_idraw, f32 angle, vec3 v);
void neko_idraw_translatef(idraw_t* neko_idraw, f32 x, f32 y, f32 z);
void neko_idraw_translatev(idraw_t* neko_idraw, vec3 v);
void neko_idraw_scalef(idraw_t* neko_idraw, f32 x, f32 y, f32 z);

// Camera Utils
void neko_idraw_camera(idraw_t* neko_idraw, mat4 m);
void neko_idraw_camera2d(idraw_t* neko_idraw, u32 width, u32 height);
void neko_idraw_camera2d_ex(idraw_t* neko_idraw, f32 l, f32 r, f32 t, f32 b);
void neko_idraw_camera3d(idraw_t* neko_idraw, u32 width, u32 height);

// Primitive Drawing Util
void neko_idraw_triangle(idraw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, f32 x2, f32 y2, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type);
void neko_idraw_trianglev(idraw_t* neko_idraw, vec2 a, vec2 b, vec2 c, Color256 color, gfx_primitive_type type);
void neko_idraw_trianglex(idraw_t* neko_idraw, f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2, f32 u0, f32 v0, f32 u1, f32 v1, f32 u2, f32 v2, u8 r, u8 g, u8 b, u8 a,
                          gfx_primitive_type type);
void neko_idraw_trianglevx(idraw_t* neko_idraw, vec3 v0, vec3 v1, vec3 v2, vec2 uv0, vec2 uv1, vec2 uv2, Color256 color, gfx_primitive_type type);
void neko_idraw_trianglevxmc(idraw_t* neko_idraw, vec3 v0, vec3 v1, vec3 v2, vec2 uv0, vec2 uv1, vec2 uv2, Color256 c0, Color256 c1, Color256 c2, gfx_primitive_type type);
void neko_idraw_line(idraw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, u8 r, u8 g, u8 b, u8 a);
void neko_idraw_linev(idraw_t* neko_idraw, vec2 v0, vec2 v1, Color256 c);
void neko_idraw_line3D(idraw_t* neko_idraw, f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1, u8 r, u8 g, u8 b, u8 a);
void neko_idraw_line3Dv(idraw_t* neko_idraw, vec3 s, vec3 e, Color256 color);
void neko_idraw_line3Dmc(idraw_t* neko_idraw, f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1, u8 r0, u8 g0, u8 b0, u8 a0, u8 r1, u8 g1, u8 b1, u8 a1);

// Shape Drawing Util
void neko_idraw_rect(idraw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type);
void neko_idraw_rectv(idraw_t* neko_idraw, vec2 bl, vec2 tr, Color256 color, gfx_primitive_type type);
void neko_idraw_rectx(idraw_t* neko_idraw, f32 l, f32 b, f32 r, f32 t, f32 u0, f32 v0, f32 u1, f32 v1, u8 _r, u8 _g, u8 _b, u8 _a, gfx_primitive_type type);
void neko_idraw_rectvx(idraw_t* neko_idraw, vec2 bl, vec2 tr, vec2 uv0, vec2 uv1, Color256 color, gfx_primitive_type type);
void neko_idraw_rectvd(idraw_t* neko_idraw, vec2 xy, vec2 wh, vec2 uv0, vec2 uv1, Color256 color, gfx_primitive_type type);
void neko_idraw_rect3Dv(idraw_t* neko_idraw, vec3 min, vec3 max, vec2 uv0, vec2 uv1, Color256 color, gfx_primitive_type type);
void neko_idraw_rect3Dvd(idraw_t* neko_idraw, vec3 xyz, vec3 whd, vec2 uv0, vec2 uv1, Color256 c, gfx_primitive_type type);
void neko_idraw_circle(idraw_t* neko_idraw, f32 cx, f32 cy, f32 radius, int32_t segments, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type);
void neko_idraw_circlevx(idraw_t* neko_idraw, vec3 c, f32 radius, int32_t segments, Color256 color, gfx_primitive_type type);
void neko_idraw_circle_sector(idraw_t* neko_idraw, f32 cx, f32 cy, f32 radius, int32_t start_angle, int32_t end_angle, int32_t segments, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type);
void neko_idraw_circle_sectorvx(idraw_t* neko_idraw, vec3 c, f32 radius, int32_t start_angle, int32_t end_angle, int32_t segments, Color256 color, gfx_primitive_type type);
void neko_idraw_arc(idraw_t* neko_idraw, f32 cx, f32 cy, f32 radius_inner, f32 radius_outer, f32 start_angle, f32 end_angle, int32_t segments, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type);
void neko_idraw_box(idraw_t* neko_idraw, f32 x0, f32 y0, f32 z0, f32 hx, f32 hy, f32 hz, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type);
void neko_idraw_sphere(idraw_t* neko_idraw, f32 cx, f32 cy, f32 cz, f32 radius, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type);
void neko_idraw_bezier(idraw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, u8 r, u8 g, u8 b, u8 a);
void neko_idraw_cylinder(idraw_t* neko_idraw, f32 x, f32 y, f32 z, f32 r_top, f32 r_bottom, f32 height, int32_t sides, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type);
void neko_idraw_cone(idraw_t* neko_idraw, f32 x, f32 y, f32 z, f32 radius, f32 height, int32_t sides, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type);

// Text Drawing Util
void neko_idraw_text(idraw_t* neko_idraw, f32 x, f32 y, const char* text, FontFamily* fp, bool flip_vertical, Color256 col);

#endif
