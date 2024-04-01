

#ifndef NEKO_IDRAW_H
#define NEKO_IDRAW_H

#include "engine/neko_engine.h"
#include "engine/neko_asset.h"

/*================================================================================
// Camera
================================================================================*/

typedef enum neko_projection_type { NEKO_PROJECTION_TYPE_ORTHOGRAPHIC, NEKO_PROJECTION_TYPE_PERSPECTIVE } neko_projection_type;

typedef struct neko_camera_t {
    neko_vqs transform;
    f32 fov;
    f32 aspect_ratio;
    f32 near_plane;
    f32 far_plane;
    f32 ortho_scale;
    neko_projection_type proj_type;
} neko_camera_t;

NEKO_API_DECL neko_camera_t neko_camera_default();
NEKO_API_DECL neko_camera_t neko_camera_perspective();
NEKO_API_DECL neko_mat4 neko_camera_get_view(const neko_camera_t* cam);
NEKO_API_DECL neko_mat4 neko_camera_get_proj(const neko_camera_t* cam, s32 view_width, s32 view_height);
NEKO_API_DECL neko_mat4 neko_camera_get_view_projection(const neko_camera_t* cam, s32 view_width, s32 view_height);
NEKO_API_DECL neko_vec3 neko_camera_forward(const neko_camera_t* cam);
NEKO_API_DECL neko_vec3 neko_camera_backward(const neko_camera_t* cam);
NEKO_API_DECL neko_vec3 neko_camera_up(const neko_camera_t* cam);
NEKO_API_DECL neko_vec3 neko_camera_down(const neko_camera_t* cam);
NEKO_API_DECL neko_vec3 neko_camera_right(const neko_camera_t* cam);
NEKO_API_DECL neko_vec3 neko_camera_left(const neko_camera_t* cam);
NEKO_API_DECL neko_vec3 neko_camera_screen_to_world(const neko_camera_t* cam, neko_vec3 coords, s32 view_x, s32 view_y, s32 view_width, s32 view_height);
NEKO_API_DECL neko_vec3 neko_camera_world_to_screen(const neko_camera_t* cam, neko_vec3 coords, s32 view_width, s32 view_height);
NEKO_API_DECL void neko_camera_offset_orientation(neko_camera_t* cam, f32 yaw, f32 picth);

/*================================================================================
// IDraw
================================================================================*/

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
    neko_asset_ascii_font_t font_default;  // Idraw font
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
    u32 window_handle;
    neko_immediate_draw_static_data_t* data;
    u32 flags;
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
NEKO_API_DECL neko_immediate_draw_static_data_t* neko_immediate_draw_static_data_get();  // 用于热更新

// Get pipeline from state
NEKO_API_DECL neko_handle(neko_graphics_pipeline_t) neko_idraw_get_pipeline(neko_immediate_draw_t* neko_idraw, neko_idraw_pipeline_state_attr_t state);

// Get default font asset pointer
NEKO_API_DECL neko_asset_ascii_font_t* neko_idraw_default_font();

// Core Vertex Functions
NEKO_API_DECL void neko_idraw_begin(neko_immediate_draw_t* neko_idraw, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_end(neko_immediate_draw_t* neko_idraw);
NEKO_API_DECL void neko_idraw_tc2f(neko_immediate_draw_t* neko_idraw, f32 u, f32 v);
NEKO_API_DECL void neko_idraw_tc2fv(neko_immediate_draw_t* neko_idraw, neko_vec2 uv);
NEKO_API_DECL void neko_idraw_c4ub(neko_immediate_draw_t* neko_idraw, u8 r, u8 g, u8 b, u8 a);
NEKO_API_DECL void neko_idraw_c4ubv(neko_immediate_draw_t* neko_idraw, neko_color_t c);
NEKO_API_DECL void neko_idraw_v2f(neko_immediate_draw_t* neko_idraw, f32 x, f32 y);
NEKO_API_DECL void neko_idraw_v2fv(neko_immediate_draw_t* neko_idraw, neko_vec2 v);
NEKO_API_DECL void neko_idraw_v3f(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z);
NEKO_API_DECL void neko_idraw_v3fv(neko_immediate_draw_t* neko_idraw, neko_vec3 v);
NEKO_API_DECL void neko_idraw_flush(neko_immediate_draw_t* neko_idraw);
NEKO_API_DECL void neko_idraw_texture(neko_immediate_draw_t* neko_idraw, neko_handle(neko_graphics_texture_t) texture);

NEKO_API_DECL neko_hsv_t neko_rgb_to_hsv(neko_color_t c);
NEKO_API_DECL f32 neko_hue_dist(f32 h1, f32 h2);

NEKO_API_DECL void neko_idraw_rect_textured(neko_immediate_draw_t* neko_idraw, neko_vec2 a, neko_vec2 b, u32 tex_id, neko_color_t color);
NEKO_API_DECL void neko_idraw_rect_textured_ext(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, f32 u0, f32 v0, f32 u1, f32 v1, u32 tex_id, neko_color_t color);

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
NEKO_API_DECL void neko_idraw_set_view_scissor(neko_immediate_draw_t* neko_idraw, u32 x, u32 y, u32 w, u32 h);

// Final Submit / Merge
NEKO_API_DECL void neko_idraw_draw(neko_immediate_draw_t* neko_idraw, neko_command_buffer_t* cb);
NEKO_API_DECL void neko_idraw_renderpass_submit(neko_immediate_draw_t* neko_idraw, neko_command_buffer_t* cb, neko_vec4 viewport, neko_color_t clear_color);
NEKO_API_DECL void neko_idraw_renderpass_submit_ex(neko_immediate_draw_t* neko_idraw, neko_command_buffer_t* cb, neko_vec4 viewport, neko_graphics_clear_action_t action);

// Core Matrix Functions
NEKO_API_DECL void neko_idraw_push_matrix(neko_immediate_draw_t* neko_idraw, neko_idraw_matrix_type type);
NEKO_API_DECL void neko_idraw_push_matrix_ex(neko_immediate_draw_t* neko_idraw, neko_idraw_matrix_type type, bool flush);
NEKO_API_DECL void neko_idraw_pop_matrix(neko_immediate_draw_t* neko_idraw);
NEKO_API_DECL void neko_idraw_pop_matrix_ex(neko_immediate_draw_t* neko_idraw, bool flush);
// NEKO_API_DECL void neko_idraw_matrix_mode(neko_immediate_draw_t* neko_idraw, neko_idraw_matrix_type type);
NEKO_API_DECL void neko_idraw_load_matrix(neko_immediate_draw_t* neko_idraw, neko_mat4 m);
NEKO_API_DECL void neko_idraw_mul_matrix(neko_immediate_draw_t* neko_idraw, neko_mat4 m);
NEKO_API_DECL void neko_idraw_perspective(neko_immediate_draw_t* neko_idraw, f32 fov, f32 aspect, f32 near, f32 far);
NEKO_API_DECL void neko_idraw_ortho(neko_immediate_draw_t* neko_idraw, f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
NEKO_API_DECL void neko_idraw_rotatef(neko_immediate_draw_t* neko_idraw, f32 angle, f32 x, f32 y, f32 z);
NEKO_API_DECL void neko_idraw_rotatev(neko_immediate_draw_t* neko_idraw, f32 angle, neko_vec3 v);
NEKO_API_DECL void neko_idraw_translatef(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z);
NEKO_API_DECL void neko_idraw_translatev(neko_immediate_draw_t* neko_idraw, neko_vec3 v);
NEKO_API_DECL void neko_idraw_scalef(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z);

// Camera Utils
NEKO_API_DECL void neko_idraw_camera(neko_immediate_draw_t* neko_idraw, neko_camera_t* cam, u32 width, u32 height);
NEKO_API_DECL void neko_idraw_camera2d(neko_immediate_draw_t* neko_idraw, u32 width, u32 height);
NEKO_API_DECL void neko_idraw_camera2d_ex(neko_immediate_draw_t* neko_idraw, f32 l, f32 r, f32 t, f32 b);
NEKO_API_DECL void neko_idraw_camera3d(neko_immediate_draw_t* neko_idraw, u32 width, u32 height);

// Primitive Drawing Util
NEKO_API_DECL void neko_idraw_triangle(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, f32 x2, f32 y2, u8 r, u8 g, u8 b, u8 a, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_trianglev(neko_immediate_draw_t* neko_idraw, neko_vec2 a, neko_vec2 b, neko_vec2 c, neko_color_t color, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_trianglex(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2, f32 u0, f32 v0, f32 u1, f32 v1, f32 u2, f32 v2, u8 r,
                                        u8 g, u8 b, u8 a, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_trianglevx(neko_immediate_draw_t* neko_idraw, neko_vec3 v0, neko_vec3 v1, neko_vec3 v2, neko_vec2 uv0, neko_vec2 uv1, neko_vec2 uv2, neko_color_t color,
                                         neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_trianglevxmc(neko_immediate_draw_t* neko_idraw, neko_vec3 v0, neko_vec3 v1, neko_vec3 v2, neko_vec2 uv0, neko_vec2 uv1, neko_vec2 uv2, neko_color_t c0, neko_color_t c1,
                                           neko_color_t c2, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_line(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, u8 r, u8 g, u8 b, u8 a);
NEKO_API_DECL void neko_idraw_linev(neko_immediate_draw_t* neko_idraw, neko_vec2 v0, neko_vec2 v1, neko_color_t c);
NEKO_API_DECL void neko_idraw_line3D(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1, u8 r, u8 g, u8 b, u8 a);
NEKO_API_DECL void neko_idraw_line3Dv(neko_immediate_draw_t* neko_idraw, neko_vec3 s, neko_vec3 e, neko_color_t color);
NEKO_API_DECL void neko_idraw_line3Dmc(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1, u8 r0, u8 g0, u8 b0, u8 a0, u8 r1, u8 g1, u8 b1, u8 a1);

// Shape Drawing Util
NEKO_API_DECL void neko_idraw_rect(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, u8 r, u8 g, u8 b, u8 a, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_rectv(neko_immediate_draw_t* neko_idraw, neko_vec2 bl, neko_vec2 tr, neko_color_t color, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_rectx(neko_immediate_draw_t* neko_idraw, f32 l, f32 b, f32 r, f32 t, f32 u0, f32 v0, f32 u1, f32 v1, u8 _r, u8 _g, u8 _b, u8 _a, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_rectvx(neko_immediate_draw_t* neko_idraw, neko_vec2 bl, neko_vec2 tr, neko_vec2 uv0, neko_vec2 uv1, neko_color_t color, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_rectvd(neko_immediate_draw_t* neko_idraw, neko_vec2 xy, neko_vec2 wh, neko_vec2 uv0, neko_vec2 uv1, neko_color_t color, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_rect3Dv(neko_immediate_draw_t* neko_idraw, neko_vec3 min, neko_vec3 max, neko_vec2 uv0, neko_vec2 uv1, neko_color_t color, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_rect3Dvd(neko_immediate_draw_t* neko_idraw, neko_vec3 xyz, neko_vec3 whd, neko_vec2 uv0, neko_vec2 uv1, neko_color_t c, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_circle(neko_immediate_draw_t* neko_idraw, f32 cx, f32 cy, f32 radius, int32_t segments, u8 r, u8 g, u8 b, u8 a, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_circlevx(neko_immediate_draw_t* neko_idraw, neko_vec3 c, f32 radius, int32_t segments, neko_color_t color, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_circle_sector(neko_immediate_draw_t* neko_idraw, f32 cx, f32 cy, f32 radius, int32_t start_angle, int32_t end_angle, int32_t segments, u8 r, u8 g, u8 b, u8 a,
                                            neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_circle_sectorvx(neko_immediate_draw_t* neko_idraw, neko_vec3 c, f32 radius, int32_t start_angle, int32_t end_angle, int32_t segments, neko_color_t color,
                                              neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_arc(neko_immediate_draw_t* neko_idraw, f32 cx, f32 cy, f32 radius_inner, f32 radius_outer, f32 start_angle, f32 end_angle, int32_t segments, u8 r, u8 g, u8 b, u8 a,
                                  neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_box(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 z0, f32 hx, f32 hy, f32 hz, u8 r, u8 g, u8 b, u8 a, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_sphere(neko_immediate_draw_t* neko_idraw, f32 cx, f32 cy, f32 cz, f32 radius, u8 r, u8 g, u8 b, u8 a, neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_bezier(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, u8 r, u8 g, u8 b, u8 a);
NEKO_API_DECL void neko_idraw_cylinder(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z, f32 r_top, f32 r_bottom, f32 height, int32_t sides, u8 r, u8 g, u8 b, u8 a,
                                       neko_graphics_primitive_type type);
NEKO_API_DECL void neko_idraw_cone(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z, f32 radius, f32 height, int32_t sides, u8 r, u8 g, u8 b, u8 a, neko_graphics_primitive_type type);

// Draw planes/poly groups

// Text Drawing Util
NEKO_API_DECL void neko_idraw_text(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, const char* text, const neko_asset_ascii_font_t* fp, bool32_t flip_vertical, neko_color_t col);

// Paths
/*
NEKO_API_DECL void neko_idraw_path_begin(neko_immediate_draw_t* neko_idraw);
NEKO_API_DECL void neko_idraw_path_end(neko_immediate_draw_t* neko_idraw);
NEKO_API_DECL void neko_idraw_path_moveto(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z);
NEKO_API_DECL void neko_idraw_path_lineto(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z);
NEKO_API_DECL void neko_idraw_path_arcto(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z);
*/

// Private Internal Utilities (Not user facing)
NEKO_API_DECL const char* __neko_internal_GetDefaultCompressedFontDataTTFBase85();
NEKO_API_DECL void __neko_internal_Decode85(const unsigned char* src, unsigned char* dst);
NEKO_API_DECL unsigned int __neko_internal_Decode85Byte(char c);

NEKO_API_DECL unsigned int neko_decompress_length(const unsigned char* input);
NEKO_API_DECL unsigned int neko_decompress(unsigned char* output, unsigned char* input, unsigned int length);

#define NEKO_GFXT_HNDL void*  // Default handle will just be a pointer to the data itself
#define NEKO_GFXT_TEX_COORD_MAX 4
#define NEKO_GFXT_COLOR_MAX 4
#define NEKO_GFXT_JOINT_MAX 4
#define NEKO_GFXT_WEIGHT_MAX 4
#define NEKO_GFXT_CUSTOM_UINT_MAX 4
#define NEKO_GFXT_INCLUDE_DIR_MAX 8
#define NEKO_GFXT_UNIFORM_VIEW_MATRIX "U_VIEW_MTX"
#define NEKO_GFXT_UNIFORM_PROJECTION_MATRIX "U_PROJECTION_MTX"
#define NEKO_GFXT_UNIFORM_VIEW_PROJECTION_MATRIX "U_VIEW_PROJECTION_MTX"
#define NEKO_GFXT_UNIFORM_MODEL_MATRIX "U_MODEL_MTX"
#define NEKO_GFXT_UNIFORM_INVERSE_MODEL_MATRIX "U_INVERSE_MODEL_MTX"
#define NEKO_GFXT_UNIFORM_VIEW_WORLD_POSITION "U_VIEW_WORLD_POSITION"
#define NEKO_GFXT_UNIFORM_MODEL_VIEW_PROJECTION_MATRIX "U_MVP_MTX"
#define NEKO_GFXT_UNIFORM_TIME "U_TIME"

typedef void* (*neko_gfxt_raw_data_func)(NEKO_GFXT_HNDL hndl, void* user_data);

#define NEKO_GFXT_RAW_DATA(FUNC_DESC, T) ((T*)(FUNC_DESC)->func((FUNC_DESC)->hndl, (FUNC_DESC)->user_data))

typedef struct neko_gfxt_raw_data_func_desc_t {
    NEKO_GFXT_HNDL hndl;           // Handle used for retrieving data.
    neko_gfxt_raw_data_func func;  // User defined function for pipeline data retrieval
    void* user_data;               // Optional user data for function
} neko_gfxt_raw_data_func_desc_t;

//=== Uniforms/Uniform blocks ===//
typedef struct neko_gfxt_uniform_desc_t {
    char name[64];                          // the_name of uniform (for binding to shader)
    neko_graphics_uniform_type type;        // Type of uniform: NEKO_GRAPHICS_UNIFORM_VEC2, NEKO_GRAPHICS_UNIFORM_VEC3, etc.
    uint32_t binding;                       // Binding for this uniform in shader
    neko_graphics_shader_stage_type stage;  // Shader stage for this uniform
    neko_graphics_access_type access_type;  // Access type for this uniform (compute only)
} neko_gfxt_uniform_desc_t;

typedef struct neko_gfxt_uniform_t {
    neko_handle(neko_graphics_uniform_t) hndl;  // Graphics handle resource for actual uniform
    uint32_t offset;                            // Individual offset for this uniform in material byte buffer data
    uint32_t binding;                           // Binding for this uniform
    size_t size;                                // Size of this uniform data in bytes
    neko_graphics_uniform_type type;            // Type of this uniform
    neko_graphics_access_type access_type;      // Access type of uniform (compute only)
} neko_gfxt_uniform_t;

typedef struct neko_gfxt_uniform_block_desc_t {
    neko_gfxt_uniform_desc_t* layout;  // Layout for all uniform data for this block to hold
    size_t size;                       // Size of layout in bytes
} neko_gfxt_uniform_block_desc_t;

typedef struct neko_gfxt_uniform_block_lookup_key_t {
    char name[64];
} neko_gfxt_uniform_block_lookup_key_t;

typedef struct neko_gfxt_uniform_block_t {
    neko_dyn_array(neko_gfxt_uniform_t) uniforms;  // Raw uniform handle array
    neko_hash_table(uint64_t, uint32_t) lookup;    // Index lookup table (used for byte buffer offsets in material uni. data)
    size_t size;                                   // Total size of material data for entire block
} neko_gfxt_uniform_block_t;

//=== Texture ===//
typedef neko_handle(neko_graphics_texture_t) neko_gfxt_texture_t;

//=== Mesh ===//
typedef neko_asset_mesh_attribute_type neko_gfxt_mesh_attribute_type;
typedef neko_asset_mesh_layout_t neko_gfxt_mesh_layout_t;

/*
    typedef struct
    {
        union
        {
            void* interleave;
            struct
            {
                void* positions;
                void* normals;
                void* tangents;
                void* tex_coords[TEX_COORD_MAX];
                void* joints[JOINT_MAX];
                void* weights[WEIGHT_MAX];
            } non_interleave;
        } vertex;
        size_t vertex_size;
        void* indices;
    } neko_gfxt_mesh_primitive_data_t;
*/

typedef struct {
    void* data;
    size_t size;
} neko_gfxt_mesh_vertex_attribute_t;

typedef struct {
    neko_gfxt_mesh_vertex_attribute_t positions;  // All position data
    neko_gfxt_mesh_vertex_attribute_t normals;
    neko_gfxt_mesh_vertex_attribute_t tangents;
    neko_gfxt_mesh_vertex_attribute_t tex_coords[NEKO_GFXT_TEX_COORD_MAX];
    neko_gfxt_mesh_vertex_attribute_t colors[NEKO_GFXT_COLOR_MAX];
    neko_gfxt_mesh_vertex_attribute_t joints[NEKO_GFXT_JOINT_MAX];
    neko_gfxt_mesh_vertex_attribute_t weights[NEKO_GFXT_WEIGHT_MAX];
    neko_gfxt_mesh_vertex_attribute_t custom_uint[NEKO_GFXT_CUSTOM_UINT_MAX];
    neko_gfxt_mesh_vertex_attribute_t indices;
    uint32_t count;  // Total count of indices
} neko_gfxt_mesh_vertex_data_t;

// Structured/packed raw mesh data
typedef struct neko_gfxt_mesh_raw_data_t {
    neko_dyn_array(neko_gfxt_mesh_vertex_data_t) primitives;  // All primitive data
} neko_gfxt_mesh_raw_data_t;

typedef struct neko_gfxt_mesh_import_options_t {
    neko_gfxt_mesh_layout_t* layout;   // Mesh attribute layout array
    size_t size;                       // Size of mesh attribute layout array in bytes
    size_t index_buffer_element_size;  // Size of index data size in bytes
} neko_gfxt_mesh_import_options_t;

NEKO_API_DECL void neko_gfxt_mesh_import_options_free(neko_gfxt_mesh_import_options_t* opt);

typedef struct neko_gfxt_mesh_desc_s {
    neko_gfxt_mesh_raw_data_t* meshes;  // Mesh data array
    size_t size;                        // Size of mesh data array in bytes
    b32 keep_data;                      // Whether or not to free data after use
} neko_gfxt_mesh_desc_t;

typedef struct neko_gfxt_vertex_stream_s {
    neko_handle(neko_graphics_vertex_buffer_t) positions;
    neko_handle(neko_graphics_vertex_buffer_t) normals;
    neko_handle(neko_graphics_vertex_buffer_t) tangents;
    neko_handle(neko_graphics_vertex_buffer_t) colors[NEKO_GFXT_COLOR_MAX];
    neko_handle(neko_graphics_vertex_buffer_t) tex_coords[NEKO_GFXT_TEX_COORD_MAX];
    neko_handle(neko_graphics_vertex_buffer_t) joints[NEKO_GFXT_JOINT_MAX];
    neko_handle(neko_graphics_vertex_buffer_t) weights[NEKO_GFXT_WEIGHT_MAX];
    neko_handle(neko_graphics_vertex_buffer_t) custom_uint[NEKO_GFXT_CUSTOM_UINT_MAX];
} neko_gfxt_vertex_stream_t;

typedef struct neko_gfxt_mesh_primitive_s {
    neko_gfxt_vertex_stream_t stream;                   // All vertex data streams
    neko_handle(neko_graphics_index_buffer_t) indices;  // Index buffer
    uint32_t count;                                     // Total number of vertices
} neko_gfxt_mesh_primitive_t;

typedef struct neko_gfxt_mesh_s {
    neko_dyn_array(neko_gfxt_mesh_primitive_t) primitives;
    neko_gfxt_mesh_desc_t desc;
} neko_gfxt_mesh_t;

//=== Pipeline ===//
typedef struct neko_gfxt_pipeline_desc_s {
    neko_graphics_pipeline_desc_t pip_desc;      // Description for constructing pipeline object
    neko_gfxt_uniform_block_desc_t ublock_desc;  // Description for constructing uniform block object
} neko_gfxt_pipeline_desc_t;

typedef struct neko_gfxt_pipeline_s {
    neko_handle(neko_graphics_pipeline_t) hndl;  // Graphics handle resource for actual pipeline
    neko_gfxt_uniform_block_t ublock;            // Uniform block for holding all uniform data
    neko_dyn_array(neko_gfxt_mesh_layout_t) mesh_layout;
    neko_graphics_pipeline_desc_t desc;
} neko_gfxt_pipeline_t;

//=== Material ===//
typedef struct neko_gfxt_material_desc_s {
    neko_gfxt_raw_data_func_desc_t pip_func;  // Description for retrieving raw pipeline pointer data from handle.
} neko_gfxt_material_desc_t;

typedef struct neko_gfxt_material_s {
    neko_gfxt_material_desc_t desc;        // Material description object
    neko_byte_buffer_t uniform_data;       // Byte buffer of actual uniform data to send to GPU
    neko_byte_buffer_t image_buffer_data;  // Image buffer data
} neko_gfxt_material_t;

//=== Renderable ===//
typedef struct neko_gfxt_renderable_desc_s {
    neko_gfxt_raw_data_func_desc_t mesh;      // Description for retrieving raw mesh pointer data from handle.
    neko_gfxt_raw_data_func_desc_t material;  // Description for retrieving raw material pointer data from handle.
} neko_gfxt_renderable_desc_t;

typedef struct neko_gfxt_renderable_s {
    neko_gfxt_renderable_desc_t desc;  // Renderable description object
    neko_mat4 model_matrix;            // Model matrix for renderable
} neko_gfxt_renderable_t;

//=== Graphics scene ===//
typedef struct neko_gfxt_scene_s {
    neko_slot_array(neko_gfxt_renderable_t) renderables;
} neko_gfxt_scene_t;

//==== API =====//

//=== Creation ===//
NEKO_API_DECL neko_gfxt_pipeline_t neko_gfxt_pipeline_create(const neko_gfxt_pipeline_desc_t* desc);
NEKO_API_DECL neko_gfxt_material_t neko_gfxt_material_create(neko_gfxt_material_desc_t* desc);
NEKO_API_DECL neko_gfxt_mesh_t neko_gfxt_mesh_create(const neko_gfxt_mesh_desc_t* desc);
NEKO_API_DECL void neko_gfxt_mesh_update_or_create(neko_gfxt_mesh_t* mesh, const neko_gfxt_mesh_desc_t* desc);
NEKO_API_DECL neko_gfxt_renderable_t neko_gfxt_renderable_create(const neko_gfxt_renderable_desc_t* desc);
NEKO_API_DECL neko_gfxt_uniform_block_t neko_gfxt_uniform_block_create(const neko_gfxt_uniform_block_desc_t* desc);
NEKO_API_DECL neko_gfxt_texture_t neko_gfxt_texture_create(neko_graphics_texture_desc_t* desc);

//=== Destruction ===//
NEKO_API_DECL void neko_gfxt_texture_destroy(neko_gfxt_texture_t* texture);
NEKO_API_DECL void neko_gfxt_material_destroy(neko_gfxt_material_t* material);
NEKO_API_DECL void neko_gfxt_mesh_destroy(neko_gfxt_mesh_t* mesh);
NEKO_API_DECL void neko_gfxt_uniform_block_destroy(neko_gfxt_uniform_block_t* ub);
NEKO_API_DECL void neko_gfxt_pipeline_destroy(neko_gfxt_pipeline_t* pipeline);

//=== Resource Loading ===//
NEKO_API_DECL neko_gfxt_pipeline_t neko_gfxt_pipeline_load_from_file(const char* path);
NEKO_API_DECL neko_gfxt_pipeline_t neko_gfxt_pipeline_load_from_memory(const char* data, size_t sz);
NEKO_API_DECL neko_gfxt_pipeline_t neko_gfxt_pipeline_load_from_memory_ext(const char* data, size_t sz, const char* file_dir);
NEKO_API_DECL neko_gfxt_texture_t neko_gfxt_texture_load_from_file(const char* path, neko_graphics_texture_desc_t* desc, bool flip, bool keep_data);
NEKO_API_DECL neko_gfxt_texture_t neko_gfxt_texture_load_from_memory(const char* data, size_t sz, neko_graphics_texture_desc_t* desc, bool flip, bool keep_data);

//=== Copy ===//
NEKO_API_DECL neko_gfxt_material_t neko_gfxt_material_deep_copy(neko_gfxt_material_t* src);

//=== Pipeline API ===//
NEKO_API_DECL neko_gfxt_uniform_t* neko_gfxt_pipeline_get_uniform(neko_gfxt_pipeline_t* pip, const char* name);

//=== Material API ===//
NEKO_API_DECL void neko_gfxt_material_set_uniform(neko_gfxt_material_t* mat, const char* name, const void* data);
NEKO_API_DECL void neko_gfxt_material_bind(neko_command_buffer_t* cb, neko_gfxt_material_t* mat);
NEKO_API_DECL void neko_gfxt_material_bind_pipeline(neko_command_buffer_t* cb, neko_gfxt_material_t* mat);
NEKO_API_DECL void neko_gfxt_material_bind_uniforms(neko_command_buffer_t* cb, neko_gfxt_material_t* mat);
NEKO_API_DECL neko_gfxt_pipeline_t* neko_gfxt_material_get_pipeline(neko_gfxt_material_t* mat);

//=== Mesh API ===//
NEKO_API_DECL void neko_gfxt_mesh_draw_pipeline(neko_command_buffer_t* cb, neko_gfxt_mesh_t* mesh, neko_gfxt_pipeline_t* pip);
NEKO_API_DECL void neko_gfxt_mesh_draw_material(neko_command_buffer_t* cb, neko_gfxt_mesh_t* mesh, neko_gfxt_material_t* mat);
NEKO_API_DECL void neko_gfxt_mesh_draw_materials(neko_command_buffer_t* cb, neko_gfxt_mesh_t* mesh, neko_gfxt_material_t** mats, size_t mats_size);
NEKO_API_DECL void neko_gfxt_mesh_draw_layout(neko_command_buffer_t* cb, neko_gfxt_mesh_t* mesh, neko_gfxt_mesh_layout_t* layout, size_t layout_size);
NEKO_API_DECL neko_gfxt_mesh_t neko_gfxt_mesh_load_from_file(const char* file, neko_gfxt_mesh_import_options_t* options);
// NEKO_API_DECL bool neko_gfxt_load_gltf_data_from_file(const char* path, neko_gfxt_mesh_import_options_t* options, neko_gfxt_mesh_raw_data_t** out, uint32_t* mesh_count);

// Util API
NEKO_API_DECL void* neko_gfxt_raw_data_default_impl(NEKO_GFXT_HNDL hndl, void* user_data);

// Mesh Generation API
NEKO_API_DECL neko_gfxt_mesh_t neko_gfxt_mesh_unit_quad_generate(neko_gfxt_mesh_import_options_t* options);
NEKO_API_DECL neko_handle(neko_graphics_texture_t) neko_gfxt_texture_generate_default();

// ECS component
typedef struct neko_gfxt_renderer {
    neko_gfxt_pipeline_t pip;
    neko_gfxt_material_t mat;
    neko_gfxt_mesh_t mesh;
    neko_gfxt_texture_t texture;
} neko_gfxt_renderer;

#endif  // NEKO_IDRAW_H
