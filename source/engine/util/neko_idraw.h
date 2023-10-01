

#ifndef NEKO_IDRAW_H
#define NEKO_IDRAW_H

#include "engine/neko_engine.h"

// STB
#include "libs/stb/stb_image.h"
#include "libs/stb/stb_rect_pack.h"
#include "libs/stb/stb_truetype.h"

/*==== Interface ====*/

neko_enum_decl(neko_idraw_matrix_type, NEKO_IDRAW_MATRIX_MODELVIEW, NEKO_IDRAW_MATRIX_PROJECTION);

typedef enum { NEKO_IDRAW_VATTR_POSITION = 0x00, NEKO_IDRAW_VATTR_UV, NEKO_IDRAW_VATTR_COLOR } neko_idraw_vattr_type;

neko_enum_decl(neko_idraw_layout_type,
               NEKO_IDRAW_LAYOUT_VATTR,  // Using vattr type directly
               NEKO_IDRAW_LAYOUT_MESH    // Using asset mesh types indirectly to be converted internally
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
    neko_vec3 position;
    neko_vec2 uv;
    neko_color_t color;
} neko_immediate_vert_t;

typedef struct neko_immediate_cache_t {
    neko_dyn_array(neko_handle(neko_graphics_pipeline_t)) pipelines;
    neko_dyn_array(neko_mat4) modelview;
    neko_dyn_array(neko_mat4) projection;
    neko_dyn_array(neko_idraw_matrix_type) modes;
    neko_vec2 uv;
    neko_color_t color;
    neko_handle(neko_graphics_texture_t) texture;
    neko_idraw_pipeline_state_attr_t pipeline;
} neko_immediate_cache_t;

typedef struct neko_immediate_draw_static_data_t {
    neko_handle(neko_graphics_texture_t) tex_default;
    neko_asset_font_t font_default;
    neko_hash_table(neko_idraw_pipeline_state_attr_t, neko_handle(neko_graphics_pipeline_t)) pipeline_table;
    neko_handle(neko_graphics_uniform_t) uniform;
    neko_handle(neko_graphics_uniform_t) sampler;
    neko_handle(neko_graphics_vertex_buffer_t) vbo;
    neko_handle(neko_graphics_index_buffer_t) ibo;
} neko_immediate_draw_static_data_t;

typedef struct neko_immediate_draw_t {
    neko_byte_buffer_t vertices;
    neko_dyn_array(uint16_t) indices;
    neko_dyn_array(neko_idraw_vattr_type) vattributes;
    neko_immediate_cache_t cache;
    neko_command_buffer_t commands;
    uint32_t window_handle;
    neko_immediate_draw_static_data_t* data;
    uint32_t flags;
} neko_immediate_draw_t;

#ifndef NEKO_NO_SHORT_NAME
typedef neko_immediate_draw_t gsid;
#define neko_idraw_create neko_immediate_draw_new
#define neko_idraw_free neko_immediate_draw_free
#endif

// Create / Init / Shutdown / Free
NEKO_API_DECL neko_immediate_draw_t neko_immediate_draw_new();
NEKO_API_DECL void neko_immediate_draw_free(neko_immediate_draw_t* neko_idraw);
NEKO_API_DECL void neko_immediate_draw_static_data_set(neko_immediate_draw_static_data_t* data);

// Get pipeline from state
NEKO_API_DECL neko_handle(neko_graphics_pipeline_t) neko_idraw_get_pipeline(neko_immediate_draw_t* neko_idraw, neko_idraw_pipeline_state_attr_t state);

// Get default font asset pointer
NEKO_API_DECL neko_asset_font_t* neko_idraw_default_font();

// Core Vertex Functions
NEKO_API_DECL void neko_idraw_begin(neko_immediate_draw_t* neko_idraw, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_end(neko_immediate_draw_t* neko_idraw);
NEKO_API_DECL void neko_idraw_tc2f(neko_immediate_draw_t* neko_idraw, float u, float v);
NEKO_API_DECL void neko_idraw_tc2fv(neko_immediate_draw_t* neko_idraw, neko_vec2 uv);
NEKO_API_DECL void neko_idraw_c4ub(neko_immediate_draw_t* neko_idraw, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
NEKO_API_DECL void neko_idraw_c4ubv(neko_immediate_draw_t* neko_idraw, neko_color_t c);
NEKO_API_DECL void neko_idraw_v2f(neko_immediate_draw_t* neko_idraw, float x, float y);
NEKO_API_DECL void neko_idraw_v2fv(neko_immediate_draw_t* neko_idraw, neko_vec2 v);
NEKO_API_DECL void neko_idraw_v3f(neko_immediate_draw_t* neko_idraw, float x, float y, float z);
NEKO_API_DECL void neko_idraw_v3fv(neko_immediate_draw_t* neko_idraw, neko_vec3 v);
NEKO_API_DECL void neko_idraw_flush(neko_immediate_draw_t* neko_idraw);
NEKO_API_DECL void neko_idraw_texture(neko_immediate_draw_t* neko_idraw, neko_handle(neko_graphics_texture_t) texture);

NEKO_API_DECL void neko_idraw_rect_2d_textured_ext(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, f32 u0, f32 v0, f32 u1, f32 v1, u32 tex_id, neko_color_t color);

// Core pipeline functions
NEKO_API_DECL void neko_idraw_blend_enabled(neko_immediate_draw_t* neko_idraw, bool enabled);
NEKO_API_DECL void neko_idraw_depth_enabled(neko_immediate_draw_t* neko_idraw, bool enabled);
NEKO_API_DECL void neko_idraw_stencil_enabled(neko_immediate_draw_t* neko_idraw, bool enabled);
NEKO_API_DECL void neko_idraw_face_cull_enabled(neko_immediate_draw_t* neko_idraw, bool enabled);
NEKO_API_DECL void neko_idraw_defaults(neko_immediate_draw_t* neko_idraw);
NEKO_API_DECL void neko_idraw_pipeline_set(neko_immediate_draw_t* neko_idraw,
                                           neko_handle(neko_graphics_pipeline_t) pipeline);  // Binds custom user pipeline, sets flag NEKO_IDRAW_FLAG_NO_BIND_CACHED_PIPELINES
NEKO_API_DECL void neko_idraw_vattr_list(neko_immediate_draw_t* neko_idraw, neko_idraw_vattr_type* layout, size_t sz);  // Sets user vertex attribute list for custom bound pipeline
NEKO_API_DECL void neko_idraw_vattr_list_mesh(neko_immediate_draw_t* neko_idraw, neko_asset_mesh_layout_t* layout,
                                              size_t sz);  // Same as above but uses mesh layout to determine which vertex attributes to bind and in what order

// View/Scissor commands
NEKO_API_DECL void neko_idraw_set_view_scissor(neko_immediate_draw_t* neko_idraw, uint32_t x, uint32_t y, uint32_t w, uint32_t h);

// Final Submit / Merge
NEKO_API_DECL void neko_idraw_draw(neko_immediate_draw_t* neko_idraw, neko_command_buffer_t* cb);
NEKO_API_DECL void neko_idraw_renderpass_submit(neko_immediate_draw_t* neko_idraw, neko_command_buffer_t* cb, neko_vec4 viewport, neko_color_t clear_color);
NEKO_API_DECL void neko_idraw_renderpass_submit_ex(neko_immediate_draw_t* neko_idraw, neko_command_buffer_t* cb, neko_vec4 viewport, neko_graphics_clear_action_t* action);

// Core Matrix Functions
NEKO_API_DECL void neko_idraw_push_matrix(neko_immediate_draw_t* neko_idraw, neko_idraw_matrix_type type);
NEKO_API_DECL void neko_idraw_push_matrix_ex(neko_immediate_draw_t* neko_idraw, neko_idraw_matrix_type type, bool flush);
NEKO_API_DECL void neko_idraw_pop_matrix(neko_immediate_draw_t* neko_idraw);
NEKO_API_DECL void neko_idraw_pop_matrix_ex(neko_immediate_draw_t* neko_idraw, bool flush);
NEKO_API_DECL void neko_idraw_matrix_mode(neko_immediate_draw_t* neko_idraw, neko_idraw_matrix_type type);
NEKO_API_DECL void neko_idraw_load_matrix(neko_immediate_draw_t* neko_idraw, neko_mat4 m);
NEKO_API_DECL void neko_idraw_mul_matrix(neko_immediate_draw_t* neko_idraw, neko_mat4 m);
NEKO_API_DECL void neko_idraw_perspective(neko_immediate_draw_t* neko_idraw, float fov, float aspect, float near, float far);
NEKO_API_DECL void neko_idraw_ortho(neko_immediate_draw_t* neko_idraw, float left, float right, float bottom, float top, float near, float far);
NEKO_API_DECL void neko_idraw_rotatef(neko_immediate_draw_t* neko_idraw, float angle, float x, float y, float z);
NEKO_API_DECL void neko_idraw_rotatev(neko_immediate_draw_t* neko_idraw, float angle, neko_vec3 v);
NEKO_API_DECL void neko_idraw_translatef(neko_immediate_draw_t* neko_idraw, float x, float y, float z);
NEKO_API_DECL void neko_idraw_translatev(neko_immediate_draw_t* neko_idraw, neko_vec3 v);
NEKO_API_DECL void neko_idraw_scalef(neko_immediate_draw_t* neko_idraw, float x, float y, float z);

// Camera Utils
NEKO_API_DECL void neko_idraw_camera(neko_immediate_draw_t* neko_idraw, neko_camera_t* cam, uint32_t width, uint32_t height);
NEKO_API_DECL void neko_idraw_camera2D(neko_immediate_draw_t* neko_idraw, uint32_t width, uint32_t height);
NEKO_API_DECL void neko_idraw_camera3D(neko_immediate_draw_t* neko_idraw, uint32_t width, uint32_t height);

// Primitive Drawing Util
NEKO_API_DECL void neko_idraw_triangle(neko_immediate_draw_t* neko_idraw, float x0, float y0, float x1, float y1, float x2, float y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a,
                                       neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_trianglev(neko_immediate_draw_t* neko_idraw, neko_vec2 a, neko_vec2 b, neko_vec2 c, neko_color_t color, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_trianglex(neko_immediate_draw_t* neko_idraw, float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2, float u0, float v0, float u1,
                                        float v1, float u2, float v2, uint8_t r, uint8_t g, uint8_t b, uint8_t a, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_trianglevx(neko_immediate_draw_t* neko_idraw, neko_vec3 v0, neko_vec3 v1, neko_vec3 v2, neko_vec2 uv0, neko_vec2 uv1, neko_vec2 uv2, neko_color_t color,
                                         neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_trianglevxmc(neko_immediate_draw_t* neko_idraw, neko_vec3 v0, neko_vec3 v1, neko_vec3 v2, neko_vec2 uv0, neko_vec2 uv1, neko_vec2 uv2, neko_color_t c0, neko_color_t c1,
                                           neko_color_t c2, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_line(neko_immediate_draw_t* neko_idraw, float x0, float y0, float x1, float y1, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
NEKO_API_DECL void neko_idraw_linev(neko_immediate_draw_t* neko_idraw, neko_vec2 v0, neko_vec2 v1, neko_color_t c);
NEKO_API_DECL void neko_idraw_line3D(neko_immediate_draw_t* neko_idraw, float x0, float y0, float z0, float x1, float y1, float z1, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
NEKO_API_DECL void neko_idraw_line3Dv(neko_immediate_draw_t* neko_idraw, neko_vec3 s, neko_vec3 e, neko_color_t color);
NEKO_API_DECL void neko_idraw_line3Dmc(neko_immediate_draw_t* neko_idraw, float x0, float y0, float z0, float x1, float y1, float z1, uint8_t r0, uint8_t g0, uint8_t b0, uint8_t a0, uint8_t r1,
                                       uint8_t g1, uint8_t b1, uint8_t a1);

// Shape Drawing Util
NEKO_API_DECL void neko_idraw_rect(neko_immediate_draw_t* neko_idraw, float x0, float y0, float x1, float y1, uint8_t r, uint8_t g, uint8_t b, uint8_t a, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_rectv(neko_immediate_draw_t* neko_idraw, neko_vec2 bl, neko_vec2 tr, neko_color_t color, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_rectx(neko_immediate_draw_t* neko_idraw, float l, float b, float r, float t, float u0, float v0, float u1, float v1, uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a,
                                    neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_rectvx(neko_immediate_draw_t* neko_idraw, neko_vec2 bl, neko_vec2 tr, neko_vec2 uv0, neko_vec2 uv1, neko_color_t color, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_rectvd(neko_immediate_draw_t* neko_idraw, neko_vec2 xy, neko_vec2 wh, neko_vec2 uv0, neko_vec2 uv1, neko_color_t color, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_rect3Dv(neko_immediate_draw_t* neko_idraw, neko_vec3 min, neko_vec3 max, neko_vec2 uv0, neko_vec2 uv1, neko_color_t color, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_rect3Dvd(neko_immediate_draw_t* neko_idraw, neko_vec3 xyz, neko_vec3 whd, neko_vec2 uv0, neko_vec2 uv1, neko_color_t c, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_circle(neko_immediate_draw_t* neko_idraw, float cx, float cy, float radius, int32_t segments, uint8_t r, uint8_t g, uint8_t b, uint8_t a,
                                     neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_circlevx(neko_immediate_draw_t* neko_idraw, neko_vec3 c, float radius, int32_t segments, neko_color_t color, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_circle_sector(neko_immediate_draw_t* neko_idraw, float cx, float cy, float radius, int32_t start_angle, int32_t end_angle, int32_t segments, uint8_t r, uint8_t g,
                                            uint8_t b, uint8_t a, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_circle_sectorvx(neko_immediate_draw_t* neko_idraw, neko_vec3 c, float radius, int32_t start_angle, int32_t end_angle, int32_t segments, neko_color_t color,
                                              neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_arc(neko_immediate_draw_t* neko_idraw, float cx, float cy, float radius_inner, float radius_outer, float start_angle, float end_angle, int32_t segments, uint8_t r,
                                  uint8_t g, uint8_t b, uint8_t a, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_box(neko_immediate_draw_t* neko_idraw, float x0, float y0, float z0, float hx, float hy, float hz, uint8_t r, uint8_t g, uint8_t b, uint8_t a,
                                  neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_sphere(neko_immediate_draw_t* neko_idraw, float cx, float cy, float cz, float radius, uint8_t r, uint8_t g, uint8_t b, uint8_t a, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_bezier(neko_immediate_draw_t* neko_idraw, float x0, float y0, float x1, float y1, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
NEKO_API_DECL void neko_idraw_cylinder(neko_immediate_draw_t* neko_idraw, float x, float y, float z, float r_top, float r_bottom, float height, int32_t sides, uint8_t r, uint8_t g, uint8_t b,
                                       uint8_t a, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_cone(neko_immediate_draw_t* neko_idraw, float x, float y, float z, float radius, float height, int32_t sides, uint8_t r, uint8_t g, uint8_t b, uint8_t a,
                                   neko_graphics_primitive_type type);

// Draw planes/poly groups

// Text Drawing Util
NEKO_API_DECL void neko_idraw_text(neko_immediate_draw_t* neko_idraw, float x, float y, const char* text, const neko_asset_font_t* fp, bool32_t flip_vertical, uint8_t r, uint8_t g, uint8_t b,
                                   uint8_t a);

// Paths
/*
NEKO_API_DECL void neko_idraw_path_begin(neko_immediate_draw_t* neko_idraw);
NEKO_API_DECL void neko_idraw_path_end(neko_immediate_draw_t* neko_idraw);
NEKO_API_DECL void neko_idraw_path_moveto(neko_immediate_draw_t* neko_idraw, float x, float y, float z);
NEKO_API_DECL void neko_idraw_path_lineto(neko_immediate_draw_t* neko_idraw, float x, float y, float z);
NEKO_API_DECL void neko_idraw_path_arcto(neko_immediate_draw_t* neko_idraw, float x, float y, float z);
*/

// Private Internal Utilities (Not user facing)
NEKO_API_DECL const char* __neko_internal_GetDefaultCompressedFontDataTTFBase85();
NEKO_API_DECL void __neko_internal_Decode85(const unsigned char* src, unsigned char* dst);
NEKO_API_DECL unsigned int __neko_internal_Decode85Byte(char c);

NEKO_API_DECL unsigned int neko_decompress_length(const unsigned char* input);
NEKO_API_DECL unsigned int neko_decompress(unsigned char* output, unsigned char* input, unsigned int length);

#endif  // NEKO_IDRAW_H
