#ifndef NEKO_GRAPHICS_H
#define NEKO_GRAPHICS_H

#include <map>

#include "engine/common/neko_containers.h"
#include "engine/common/neko_hash.h"
#include "engine/common/neko_str.h"
#include "engine/common/neko_types.h"
#include "engine/graphics/neko_camera.h"
#include "engine/graphics/neko_fontcache.h"
#include "engine/math/neko_math.h"
#include "engine/serialize/neko_byte_buffer.h"

// Forward Decls
struct neko_material_i;
struct neko_uniform_block_i;
struct neko_quad_batch_i;
struct neko_material_t;
struct neko_quad_batch_t;
struct neko_camera_t;

typedef enum neko_shader_program_type { neko_vertex_program = 0, neko_fragment_program, neko_geometry_program, neko_compute_program } neko_shader_program_type;

typedef enum neko_blend_equation_type {
    neko_blend_equation_add,
    neko_blend_equation_subtract,
    neko_blend_equation_reverse_subtract,
    neko_blend_equation_min,
    neko_blend_equation_max
} neko_blend_equation_type;

typedef enum neko_winding_order_type { neko_winding_order_cw, neko_winding_order_ccw } neko_winding_order_type;

typedef enum neko_face_culling_type { neko_face_culling_disabled, neko_face_culling_front, neko_face_culling_back, neko_face_culling_front_and_back } neko_face_culling_type;

typedef enum neko_blend_mode_type {
    neko_blend_mode_disabled,
    neko_blend_mode_zero,
    neko_blend_mode_one,
    neko_blend_mode_src_color,
    neko_blend_mode_one_minus_src_color,
    neko_blend_mode_dst_color,
    neko_blend_mode_one_minus_dst_color,
    neko_blend_mode_src_alpha,
    neko_blend_mode_one_minus_src_alpha,
    neko_blend_mode_dst_alpha,
    neko_blend_mode_one_minus_dst_alpha,
    neko_blend_mode_constant_color,
    neko_blend_mode_one_minus_constant_color,
    neko_blend_mode_constant_alpha,
    neko_blend_mode_one_minus_constant_alpha,
    neko_blend_mode_src_alpha_saturate
} neko_blend_mode_type;

typedef enum neko_shader_language_type { neko_glsl = 0x00 } neko_shader_language_type;

typedef enum neko_uniform_type {
    neko_uniform_type_float = 0,
    neko_uniform_type_int,
    neko_uniform_type_vec2,
    neko_uniform_type_vec3,
    neko_uniform_type_vec4,
    neko_uniform_type_mat4,
    neko_uniform_type_sampler2d
} neko_uniform_type;

// Want to give a vertex buffer layout description (to set attributes)
typedef enum neko_vertex_attribute_type {
    neko_vertex_attribute_float4 = 0,
    neko_vertex_attribute_float3,
    neko_vertex_attribute_float2,
    neko_vertex_attribute_float,
    neko_vertex_attribute_uint4,
    neko_vertex_attribute_uint3,
    neko_vertex_attribute_uint2,
    neko_vertex_attribute_uint,
    neko_vertex_attribute_byte4,
    neko_vertex_attribute_byte3,
    neko_vertex_attribute_byte2,
    neko_vertex_attribute_byte,
} neko_vertex_attribute_type;

/*================
// Resource Decls
=================*/

typedef struct neko_uniform_t {
    neko_uniform_type type;
    u32 location;
} neko_uniform_t;

// typedef neko_resource(neko_uniform) neko_resource_uniform;

// Hash table := key: u64, val: neko_resource_uniform
// neko_hash_table_decl(u64, neko_uniform_t, neko_hash_u64, neko_hash_key_comp_std_type);

// Don't want to pass this around...
typedef struct neko_shader_t {
    u32 program_id;
} neko_shader_t;

typedef struct neko_index_buffer_t {
    u32 ibo;
} neko_index_buffer_t;

typedef struct neko_vertex_buffer_t {
    u32 vbo;
    u32 vao;  // Not sure if I need to do this as well, but we will for now...
} neko_vertex_buffer_t;

typedef struct neko_render_target_t {
    u32 tex_id;
} neko_render_target_t;

typedef struct neko_frame_buffer_t {
    u32 fbo;
} neko_frame_buffer_t;

typedef struct neko_vertex_attribute_layout_desc_t {
    neko_dyn_array(neko_vertex_attribute_type) attributes;
} neko_vertex_attribute_layout_desc_t;

// From on: https://gist.github.com/fairlight1337/4935ae72bcbcc1ba5c72
extern void neko_rgb_to_hsv(u8 r, u8 g, u8 b, f32* h, f32* s, f32* v);
extern void neko_hsv_to_rgb(f32 h, f32 s, f32 v, u8* r, u8* g, u8* b);

/*================
// Texture
=================*/

typedef enum neko_texture_format { neko_texture_format_rgba8, neko_texture_format_rgb8, neko_texture_format_rgba16f, neko_texture_format_a8, neko_texture_format_r8 } neko_texture_format;

typedef enum neko_texture_wrapping { neko_repeat, neko_mirrored_repeat, neko_clamp_to_edge, neko_clamp_to_border } neko_texture_wrapping;

typedef enum neko_texture_filtering { neko_nearest, neko_linear } neko_texture_filtering;

typedef struct neko_texture_parameter_desc {
    neko_texture_wrapping texture_wrap_s;
    neko_texture_wrapping texture_wrap_t;
    neko_texture_filtering min_filter;
    neko_texture_filtering mag_filter;
    neko_texture_filtering mipmap_filter;
    f32 border_color[4];
    bool generate_mips;
    neko_texture_format texture_format;
    void* data;
    u32 width;
    u32 height;
    u32 num_comps;  // 单像素颜色分量 rgba=4
    bool flip_vertically_on_load;
} neko_texture_parameter_desc;

typedef neko_texture_parameter_desc neko_texture_parameter_desc_t;

typedef struct neko_texture_t {
    u16 width;
    u16 height;
    u32 id;
    u32 num_comps;
    neko_texture_format texture_format;
} neko_texture_t;

/*=====================
// Uniform Block
======================*/

neko_hash_table_decl(u64, neko_uniform_t, neko_hash_u64, neko_hash_key_comp_std_type);

typedef struct neko_uniform_block_t {
    neko_byte_buffer_t data;
    neko_hash_table(u64, u32) offset_lookup_table;
    neko_hash_table(u64, neko_uniform_t) uniforms;
    u32 count;
} neko_uniform_block_t;

neko_slot_array_decl(neko_uniform_block_t);
neko_resource_cache_decl(neko_uniform_block_t);

#define __neko_uniform_data_block_uniform_decl(T, ...) \
    typedef struct neko_uniform_data_block_##T##_t {   \
        __VA_ARGS__                                    \
    } neko_uniform_data_block_##T##_t;

__neko_uniform_data_block_uniform_decl(texture_sampler, u32 data; u32 slot;);
__neko_uniform_data_block_uniform_decl(mat4, neko_mat4 data;);
__neko_uniform_data_block_uniform_decl(vec4, neko_vec4 data;);
__neko_uniform_data_block_uniform_decl(vec3, neko_vec3 data;);
__neko_uniform_data_block_uniform_decl(vec2, neko_vec2 data;);
__neko_uniform_data_block_uniform_decl(float, f32 data;);
__neko_uniform_data_block_uniform_decl(int, s32 data;);

#define neko_uniform_block_type(T) neko_uniform_data_block_##T##_t

extern neko_resource(neko_uniform_block_t) __neko_uniform_block_t_new();

extern void __neko_uniform_block_t_set_uniform(neko_resource(neko_uniform_block_t) u_block, neko_uniform_t uniform, const char* name, void* data, usize data_size);
extern void __neko_uniform_block_t_bind_uniforms(neko_command_buffer_t* cb, neko_resource(neko_uniform_block_t) u_block);

// Could hold the data here
typedef struct neko_uniform_block_i {
    neko_resource(neko_uniform_block_t) (*construct)();
    void (*set_uniform)(neko_resource(neko_uniform_block_t) u_block, neko_uniform_t uniform, const char* name, void* data, usize data_size);
    void (*set_uniform_from_shader)(neko_resource(neko_uniform_block_t) u_block_h, neko_shader_t shader, neko_uniform_type type, const char* name, ...);
    void (*bind_uniforms)(neko_command_buffer_t* cb, neko_resource(neko_uniform_block_t) u_block);
    neko_slot_array(neko_uniform_block_t) uniform_blocks;
} neko_uniform_block_i;

extern neko_uniform_block_i __neko_uniform_block_i_new();

/*=====================
// Material
======================*/

typedef struct neko_material_t {
    neko_shader_t shader;
    neko_resource(neko_uniform_block_t) uniforms;
} neko_material_t;

neko_resource_cache_decl(neko_material_t);

extern neko_resource(neko_material_t) neko_material_new(neko_shader_t shader);
extern void __neko_material_i_set_uniform(neko_resource(neko_material_t) mat, neko_uniform_type type, const char* name, void* data);
extern void __neko_material_i_bind_uniforms(neko_command_buffer_t* cb, neko_resource(neko_material_t) mat);

typedef struct neko_material_i {
    neko_resource(neko_material_t) (*construct)(neko_shader_t);
    void (*set_uniform)(neko_resource(neko_material_t) mat, neko_uniform_type, const char* name, void* data);
    void (*bind_uniforms)(neko_command_buffer_t*, neko_resource(neko_material_t));
} neko_material_i;

neko_material_i __neko_material_i_new();

/*================
// Font
=================*/

// This directly maps to stbtt_bakedchar but gets around having to
// explicitly include the library in this header file. Internally, will be able to directly convert between
// the two.
typedef struct neko_baked_char_t {
    u16 x0, y0, x1, y1;  // coordinates of bbox in bitmap
    f32 xoff, yoff, xadvance;
} neko_baked_char_t;

// This makes this tricky, because it would be nice to have a caching system for internal resources.
typedef struct neko_font_t {
    void* font_info;
    neko_baked_char_t glyphs[96];
    neko_texture_t texture;
} neko_font_t;

neko_resource_cache_decl(neko_font_t);

using neko_font_index = ve_font_id;

/*================
// Pipeline State
================*/

typedef enum neko_matrix_mode { neko_matrix_model, neko_matrix_vp } neko_matrix_mode;

typedef enum neko_draw_mode { neko_lines, neko_triangles, neko_quads } neko_draw_mode;

typedef enum neko_pipeline_state_attr_type {
    neko_blend_func_src,
    neko_blend_func_dst,
    neko_blend_equation,
    neko_depth_enabled,
    neko_winding_order,
    neko_face_culling,
    neko_viewport,
    neko_view_scissor,
    neko_shader
} neko_pipeline_state_attr_type;

typedef struct neko_pipeline_shader_stage_desc_t {
    neko_shader_program_type type;
    neko_shader_language_type language;
    const char* source;
} neko_pipeline_shader_stage_desc_t;

typedef struct neko_viewport_state_desc_t {
    f32 x, y, width, height;
} neko_viewport_state_desc_t;

typedef struct neko_view_scissor_state_desc_t {
    f32 x, y, width, height;
} neko_view_scissor_state_desc_t;

#define neko_clear_none 0x00
#define neko_clear_color 0x01
#define neko_clear_depth 0x02
#define neko_clear_stencil 0x04
#define neko_clear_all neko_clear_color | neko_clear_depth | neko_clear_stencil

// Used to describe render state being created.
typedef struct neko_render_pipeline_state_desc_t {
    neko_blend_mode_type blend_func_src;
    neko_blend_mode_type blend_func_dst;
    neko_blend_equation_type blend_equation;
    b32 depth_enabled;
    neko_winding_order_type winding_order;
    neko_face_culling_type face_culling;
    neko_draw_mode draw_mode;
    neko_vec4 viewport;
    neko_vec4 view_scissor;
    neko_vertex_buffer_t vbo;
    neko_index_buffer_t ibo;
    neko_frame_buffer_t fbo;
    neko_vec4 clear_color;
    u32 clear_bit;
    neko_shader_t shader;
} neko_render_pipeline_state_desc_t;

// Pipeline state (this will be opaque in future, I believe)
typedef struct neko_render_pipeline_state_t {
    neko_render_pipeline_state_desc_t desc;
    neko_resource(neko_uniform_block_t) uniforms;
} neko_render_pipeline_state_t;

neko_declare_resource_type(neko_render_pipeline_state_t);
neko_slot_array_decl(neko_render_pipeline_state_t);

extern neko_render_pipeline_state_desc_t neko_render_pipeline_state_desc_default();
extern neko_resource(neko_render_pipeline_state_t) neko_construct_render_pipeline_state(neko_render_pipeline_state_desc_t desc);

/*
    neko_resource(neko_render_pipeline_state_t) state = gfx->construct_pipeline_state(desc);

    // Rendering loop
    gfx->set_pipeline_state(cb, state);
    gfx->set_pipeline_clear_color(cb, color);
    gfx->set_pipeline_clear_bit(cb, bit);
    gfx->set_pipeline_state_uniform(cb, "u_vp", neko_uniform_type_mat4, model_mat); // for now, not worried if this is possible in the future for vulkan
    gfx->set_pipeline_state_uniform(cb, "u_model", neko_uniform_type_mat4, vp_mat);
    gfx->draw_instanced(cb, ...);

    gfx->set_pipeline_state(cb, ...);
        ...
    gfx->flush_pipeline_state(cb);

    gfx->submit_command_buffer(cb);

*/

/*==========================
// Immediate Mode Rendering
==========================*/

typedef enum neko_graphics_immediate_mode { neko_immediate_mode_2d, neko_immediate_mode_3d } neko_graphics_immediate_mode;

typedef struct neko_graphics_immediate_draw_i {
    // Main begin/end drawing functions
    void (*begin_drawing)(neko_command_buffer_t* cb);
    void (*end_drawing)(neko_command_buffer_t* cb);

    void (*clear)(neko_command_buffer_t* cb, f32 r, f32 g, f32 b, f32 a);

    // Begin new shape declaration
    void (*begin)(neko_command_buffer_t* cb, neko_draw_mode mode);
    void (*end)(neko_command_buffer_t* cb);

    // Force draw vert buffer flush command
    void (*flush)(neko_command_buffer_t* cb);

    neko_resource(neko_render_pipeline_state_t) (*default_pipeline_state)(neko_graphics_immediate_mode mode);

    // State
    void (*push_state)(neko_command_buffer_t* cb, neko_resource(neko_render_pipeline_state_t) state);
    void (*pop_state)(neko_command_buffer_t* cb);
    void (*push_state_attr)(neko_command_buffer_t* cb, neko_pipeline_state_attr_type type, ...);
    void (*pop_state_attr)(neko_command_buffer_t* cb);

    neko_camera_t (*begin_3d)(neko_command_buffer_t* cb);
    void (*end_3d)(neko_command_buffer_t* cb);
    neko_camera_t (*begin_2d)(neko_command_buffer_t* cb);
    void (*end_2d)(neko_command_buffer_t* cb);

    // Vertex attribute ops
    void (*enable_texture_2d)(neko_command_buffer_t* cb, u32 id);
    void (*disable_texture_2d)(neko_command_buffer_t* cb);
    void (*texcoord_2f)(neko_command_buffer_t* cb, f32 s, f32 t);
    void (*texcoord_2fv)(neko_command_buffer_t* cb, neko_vec2 v);
    void (*color_ub)(neko_command_buffer_t* cb, u8 r, u8 g, u8 b, u8 a);
    void (*color_ubv)(neko_command_buffer_t* cb, neko_color_t c);
    void (*color_4f)(neko_command_buffer_t* cb, f32 r, f32 g, f32 b, f32 a);
    void (*color_4fv)(neko_command_buffer_t* cb, neko_vec4 v);
    void (*vertex_3f)(neko_command_buffer_t* cb, f32 x, f32 y, f32 z);
    void (*vertex_3fv)(neko_command_buffer_t* cb, neko_vec3 v);
    void (*vertex_2f)(neko_command_buffer_t* cb, f32 x, f32 y);
    void (*vertex_2fv)(neko_command_buffer_t* cb, neko_vec2 v);

    // Matrices
    void (*push_matrix)(neko_command_buffer_t* cb, neko_matrix_mode mode);
    void (*pop_matrix)(neko_command_buffer_t* cb);
    void (*mat_mul)(neko_command_buffer_t* cb, neko_mat4 m);
    void (*mat_rotatef)(neko_command_buffer_t* cb, f32 rad, f32 x, f32 y, f32 z);
    void (*mat_rotatev)(neko_command_buffer_t* cb, f32 rad, neko_vec3 v);
    void (*mat_rotateq)(neko_command_buffer_t* cb, neko_quat q);
    void (*mat_transf)(neko_command_buffer_t* cb, f32 x, f32 y, f32 z);
    void (*mat_transv)(neko_command_buffer_t* cb, neko_vec3);
    void (*mat_scalef)(neko_command_buffer_t* cb, f32 x, f32 y, f32 z);
    void (*mat_scalev)(neko_command_buffer_t* cb, neko_vec3);
    void (*mat_mul_vqs)(neko_command_buffer_t* cb, neko_vqs xform);

    // Rect
    void (*draw_rect)(neko_command_buffer_t* cb, f32 x0, f32 y0, f32 x1, f32 y1, neko_color_t color);
    void (*draw_rectv)(neko_command_buffer_t* cb, neko_vec2 a, neko_vec2 b, neko_color_t color);
    void (*draw_rect_textured)(neko_command_buffer_t* cb, neko_vec2 a, neko_vec2 b, u32 tex_id, neko_color_t color);
    void (*draw_rect_textured_ext)(neko_command_buffer_t* cb, f32 x0, f32 y0, f32 x1, f32 y1, f32 u0, f32 v0, f32 u1, f32 v1, u32 tex_id, neko_color_t color);
    void (*draw_rect_lines)(neko_command_buffer_t* cb, f32 x0, f32 y0, f32 x1, f32 y1, neko_color_t color);

    // Circle
    void (*draw_circle)(neko_command_buffer_t* cb, neko_vec2 center, f32 radius, s32 segments, neko_color_t color);
    void (*draw_circle_sector)(neko_command_buffer_t* cb, neko_vec2 center, f32 radius, s32 start_angle, s32 end_angle, s32 segments, neko_color_t color);

    // Triangle
    void (*draw_triangle)(neko_command_buffer_t* cb, neko_vec2 a, neko_vec2 b, neko_vec2 c, neko_color_t color);
    void (*draw_triangle_ext)(neko_command_buffer_t* cb, neko_vec3 a, neko_vec3 b, neko_vec3 c, neko_mat4 xform, neko_color_t color);

    // Line
    void (*draw_line)(neko_command_buffer_t* cb, f32 x0, f32 y0, f32 x1, f32 y1, neko_color_t color);
    void (*draw_line_3d)(neko_command_buffer_t* cb, neko_vec3 s, neko_vec3 e, neko_color_t color);
    void (*draw_line_ext)(neko_command_buffer_t* cb, neko_vec2 s, neko_vec2 e, f32 thickness, neko_color_t color);

    // Box
    void (*draw_box)(neko_command_buffer_t* cb, neko_vec3 origin, neko_vec3 half_extents, neko_color_t color);
    void (*draw_box_vqs)(neko_command_buffer_t* cb, neko_vqs xform, neko_color_t color);
    void (*draw_box_textured)(neko_command_buffer_t* cb, neko_vec3 origin, neko_vec3 half_extents, u32 tex_id, neko_color_t color);
    void (*draw_box_textured_vqs)(neko_command_buffer_t* cb, neko_vqs xform, u32 tex_id, neko_color_t color);
    void (*draw_box_lines)(neko_command_buffer_t* cb, neko_vec3 origin, neko_vec3 half_extents, neko_color_t color);
    void (*draw_box_lines_vqs)(neko_command_buffer_t* cb, neko_vqs xform, neko_color_t color);

    // Sphere
    void (*draw_sphere)(neko_command_buffer_t* cb, neko_vec3 center, f32 radius, neko_color_t color);
    void (*draw_sphere_lines)(neko_command_buffer_t* cb, neko_vec3 center, f32 radius, neko_color_t color);
    void (*draw_sphere_lines_vqs)(neko_command_buffer_t* cb, neko_vqs xform, neko_color_t color);

    // Text
    void (*draw_text)(neko_command_buffer_t* cb, f32 x, f32 y, const char* text, neko_color_t color);
    void (*draw_text_ext)(neko_command_buffer_t* cb, f32 x, f32 y, const char* text, neko_resource(neko_font_t) ft, neko_color_t color);

    // Camera
    void (*push_camera)(neko_command_buffer_t* cb, neko_camera_t camera);
    void (*pop_camera)(neko_command_buffer_t* cb);

    // Submit
    void (*submit)(neko_command_buffer_t* cb);
} neko_graphics_immediate_draw_i;

/*================
// Graphics API
=================*/

/*
    neko_render_pipeline_desc_t desc;
    desc.shader = ...;
    desc.depth_enabled = ...;
    desc.winding_order = ...;

    // Has to be done on main thread
    neko_resource(neko_render_pipeline_state_t) state = gfx->construct_render_pipeline(desc);   // Construct new render pipeline with uniform block to use

    gfx->set_pipeline_state(cb, state);
    gfx->set_pipeline_shader_uniform(cb, ...);
    gfx->draw_indexed();

    gfx->set_pipeline_state(cb, state);
    gfx->set_pipeline_shader_uniform(cb, ...);
*/

// This will not be abstracted, however the renderer absolutely could be
typedef struct neko_graphics_i {
    /*============================================================
    // Graphics Initilization / De-Initialization
    ============================================================*/
    neko_result (*init)(struct neko_graphics_i*);
    neko_result (*shutdown)(struct neko_graphics_i*);
    neko_result (*pre_update)(struct neko_graphics_i*);
    neko_result (*post_update)(struct neko_graphics_i*);

    /*============================================================
    // Graphics Command Buffer Ops
    ============================================================*/

    // Sets internal pipeline state to be used for rendering
    void (*set_render_pipeline)(neko_command_buffer_t* cb, neko_resource(neko_render_pipeline_state_t) state_h);

    // Sets value of uniform in block for bound pipeline state (could be individual uniform or block type)
    void (*set_render_pipeline_uniform)(neko_command_buffer_t* cb, const char* name, neko_uniform_type type, ...);

    void (*set_pipeline_state)(neko_command_buffer_t* cb, neko_resource(neko_render_pipeline_state_t) state);
    void (*reset_command_buffer)(neko_command_buffer_t*);
    void (*set_depth_enabled)(neko_command_buffer_t*, b32);
    void (*set_winding_order)(neko_command_buffer_t*, neko_winding_order_type);
    void (*set_face_culling)(neko_command_buffer_t*, neko_face_culling_type);
    void (*set_blend_mode)(neko_command_buffer_t*, neko_blend_mode_type, neko_blend_mode_type);
    void (*set_blend_equation)(neko_command_buffer_t*, neko_blend_equation_type);
    void (*set_viewport)(neko_command_buffer_t*, u32 x, u32 y, u32 width, u32 height);
    void (*set_view_scissor)(neko_command_buffer_t*, u32 x, u32 y, u32 width, u32 height);
    void (*bind_frame_buffer)(neko_command_buffer_t*, neko_frame_buffer_t);
    void (*set_frame_buffer_attachment)(neko_command_buffer_t*, neko_texture_t tex, u32 idx);
    void (*unbind_frame_buffer)(neko_command_buffer_t*);
    void (*bind_shader)(neko_command_buffer_t*, neko_shader_t);
    void (*bind_uniform)(neko_command_buffer_t*, neko_uniform_t, void*);
    void (*bind_uniform_mat4)(neko_command_buffer_t*, neko_uniform_t, neko_mat4);
    void (*bind_vertex_buffer)(neko_command_buffer_t*, neko_vertex_buffer_t);
    void (*bind_index_buffer)(neko_command_buffer_t*, neko_index_buffer_t);
    void (*bind_texture)(neko_command_buffer_t*, neko_uniform_t, neko_texture_t, u32 slot);
    void (*bind_texture_id)(neko_command_buffer_t*, neko_uniform_t, u32 id, u32 slot);
    void (*update_vertex_data)(neko_command_buffer_t*, neko_vertex_buffer_t, void*, usize);
    void (*update_index_data)(neko_command_buffer_t*, neko_index_buffer_t, void*, usize);
    void (*set_view_clear)(neko_command_buffer_t*, f32*);
    void (*draw)(neko_command_buffer_t*, u32 start, u32 count);
    void (*draw_indexed)(neko_command_buffer_t*, u32 count, u32 offset);
    void (*submit_command_buffer)(neko_command_buffer_t*);
    // void (* set_uniform_buffer_sub_data)(neko_command_buffer_t*, neko_resource(neko_uniform_buffer), void*, usize);

    // Need to think about materials a bit, because they're assets, not raw resources.
    void (*bind_material_uniforms)(neko_command_buffer_t*, neko_resource(neko_material_t));
    void (*bind_material_shader)(neko_command_buffer_t*, neko_resource(neko_material_t));

    /*============================================================
    // Graphics Resource Construction
    ============================================================*/
    neko_vertex_buffer_t (*construct_vertex_buffer)(neko_vertex_attribute_type*, usize, void*, usize);
    neko_shader_t (*construct_shader)(const char* vert_src, const char* frag_src);
    neko_uniform_t (*construct_uniform)(neko_shader_t, const char* uniform_name, neko_uniform_type);
    neko_render_target_t (*construct_render_target)(neko_texture_parameter_desc);
    neko_frame_buffer_t (*construct_frame_buffer)(neko_texture_t);

    // Raw texture data loading from raw resource
    void* (*load_texture_data_from_file)(const char* file_path, b32 flip_vertically_on_load, neko_texture_format, s32* width, s32* height, s32* num_comps);

    // Will construct texture resource and let user free data...for no
    neko_texture_t (*construct_texture)(neko_texture_parameter_desc);
    neko_texture_t (*construct_texture_from_file)(const char* file_path, neko_texture_parameter_desc* t_desc);
    neko_index_buffer_t (*construct_index_buffer)(void*, usize);

    // These need to be placed into a particular cache for fonts (slot array) which then can be responsible for handling data.
    neko_resource(neko_font_t) (*construct_font_from_file)(const char* file_path, f32 point_size);

    // Used for creating render pipeline state and returns opaque handle
    neko_resource(neko_render_pipeline_state_t) (*construct_render_pipeline_state)(neko_render_pipeline_state_desc_t desc);

    /*============================================================
    // Graphics Default Resources
    ============================================================*/
    neko_resource(neko_font_t) (*default_font)();

    /*============================================================
    // Graphics Resource Free Ops
    ============================================================*/
    void (*free_vertex_buffer)(neko_vertex_buffer_t);
    void (*free_index_buffer)(neko_index_buffer_t);
    void (*free_shader)(neko_shader_t);
    // void (* free_uniform_buffer)(neko_resource(neko_uniform_buffer));

    /*============================================================
    // Graphics Update Ops
    ============================================================*/
    void (*update_vertex_buffer_data)(neko_vertex_buffer_t, void*, usize);
    void (*update_texture_data)(neko_texture_t*, neko_texture_parameter_desc);

    void (*set_material_uniform)(neko_resource(neko_material_t), neko_uniform_type, const char*, void*);  // Generic method for setting uniform data
    void (*set_material_uniform_mat4)(neko_resource(neko_material_t), const char*, neko_mat4);
    void (*set_material_uniform_vec4)(neko_resource(neko_material_t), const char*, neko_vec4);
    void (*set_material_uniform_vec3)(neko_resource(neko_material_t), const char*, neko_vec3);
    void (*set_material_uniform_vec2)(neko_resource(neko_material_t), const char*, neko_vec2);
    void (*set_material_uniform_float)(neko_resource(neko_material_t), const char*, f32);
    void (*set_material_uniform_int)(neko_resource(neko_material_t), const char*, s32);
    void (*set_material_uniform_sampler2d)(neko_resource(neko_material_t), const char*, neko_texture_t, u32);

    void (*quad_batch_begin)(struct neko_quad_batch_t*);
    void (*quad_batch_add)(struct neko_quad_batch_t*, void*);
    void (*quad_batch_end)(struct neko_quad_batch_t*);
    void (*quad_batch_submit)(neko_command_buffer_t*, struct neko_quad_batch_t*);

    /*============================================================
    // Graphics Immediate Mode Debug Rendering Ops
    ============================================================*/
    neko_graphics_immediate_draw_i immediate;

    /*============================================================
    // Graphics Utility Functions
    ============================================================*/
    u32 (*get_byte_size_of_vertex_attribute)(neko_vertex_attribute_type type);
    u32 (*calculate_vertex_size_in_bytes)(neko_vertex_attribute_type* layout_data, u32 count);
    neko_vec2 (*text_dimensions)(const char* text, neko_resource(neko_font_t) ft);
    bool (*make_screenshot)(const neko_string& filename, u32 width, u32 height, u32 dst_width, u32 dst_height);

    /*============================================================
    // Fontcache
    ============================================================*/
    void (*fontcache_create)(void);
    void (*fontcache_destroy)(void);
    void (*fontcache_draw)(void);
    neko_font_index (*fontcache_load)(const void* data, size_t data_size, f32 font_size);

    void (*fontcache_push)(const std::string& text, const neko_font_index font, const neko_vec2 pos);
    void (*fontcache_push_x_y)(const std::string& text, const neko_font_index font, const f32 x, const f32 y);

    /*============================================================
    // Shader Methods
    ============================================================*/
    neko_shader_t* (*neko_shader_create)(const neko_string& name, const neko_string& vert, const neko_string& frag);
    neko_shader_t* (*neko_shader_get)(const neko_string& name);
    neko_hashmap<neko_shader_t>* (*neko_shader_internal_list)();

    // Internal Render Data (API specific)
    void* data;  // 实际上是 opengl_render_data_t

    neko_slot_array(neko_render_pipeline_state_t) render_pipelines;

    // Cache pools for various resources
    neko_resource_cache(neko_font_t) font_cache;
    neko_resource_cache(neko_material_t) material_cache;
    neko_resource_cache(neko_uniform_block_t) uniform_block_cache;

    // neko_resouce_cache(neko_texture_t)           texture_cache;
    // neko_resource_cache(neko_shader_t)           shader_cache;
    // neko_resource_cache(neko_vertex_buffer_t)    vertex_buffer_cache;
    // neko_resource_cache(neko_index_buffer_t)     index_buffer_cache;
    // neko_resource(neko_render_pipeline_state_t) render_pipeline_cache;       // Hold internal caches for everything

    // Utility APIs
    struct neko_material_i* material_i;
    struct neko_uniform_block_i* uniform_i;
    struct neko_quad_batch_i* quad_batch_i;
} neko_graphics_i;

/*===============================
// Graphics Default Functionality
===============================*/

extern neko_texture_parameter_desc neko_texture_parameter_desc_default();
extern void* neko_load_texture_data_from_file(const char* path, b32 flip_vertically_on_load);
extern neko_resource(neko_font_t) __neko_construct_font_from_file(const char* path, f32 point_size);
extern neko_vec2 __neko_text_dimensions(const char* text, neko_resource(neko_font_t) ft);
extern neko_resource(neko_font_t) __neko_construct_default_font();
extern neko_resource(neko_font_t) __neko_get_default_font();

/*===============================
// Graphics User Provided Funcs
===============================*/

extern struct neko_graphics_i* __neko_graphics_construct();

/*===============================
// Immediate Mode Drawing
===============================*/
void __neko_draw_line_3d_ext(neko_command_buffer_t* cb, neko_vec3 start, neko_vec3 end, neko_vec3 normal, f32 thickness, neko_color_t color);
void __neko_draw_line_2d_ext(neko_command_buffer_t* cb, neko_vec2 start, neko_vec2 end, f32 thickness, neko_color_t color);
void __neko_draw_line_3d(neko_command_buffer_t* cb, neko_vec3 s, neko_vec3 e, neko_color_t);
void __neko_draw_line_2d(neko_command_buffer_t* cb, f32 x0, f32 y0, f32 x1, f32 y1, neko_color_t);
void __neko_draw_triangle_3d(neko_command_buffer_t* cb, neko_vec3 a, neko_vec3 b, neko_vec3 c, neko_color_t color);
void __neko_draw_triangle_3d_ext(neko_command_buffer_t* cb, neko_vec3 a, neko_vec3 b, neko_vec3 c, neko_mat4 m, neko_color_t color);
void __neko_draw_triangle_2d(neko_command_buffer_t* cb, neko_vec2 a, neko_vec2 b, neko_vec2 c, neko_color_t color);
void __neko_draw_circle(neko_command_buffer_t* cb, neko_vec2 center, f32 radius, s32 segments, neko_color_t color);
void __neko_draw_circle_sector(neko_command_buffer_t* cb, neko_vec2 center, f32 radius, s32 start_angle, s32 end_angle, s32 segments, neko_color_t color);
void __neko_draw_rect_2d(neko_command_buffer_t* cb, f32 x0, f32 y0, f32 x1, f32 y1, neko_color_t color);
void __neko_draw_rect_2dv(neko_command_buffer_t* cb, neko_vec2 a, neko_vec2 b, neko_color_t color);
void __neko_draw_rect_2d_textured(neko_command_buffer_t* cb, neko_vec2 a, neko_vec2 b, u32 texture_id, neko_color_t color);
void __neko_draw_rect_2d_textured_ext(neko_command_buffer_t* cb, f32 x0, f32 y0, f32 x1, f32 y1, f32 u0, f32 v0, f32 u1, f32 v1, u32 tex_id, neko_color_t color);
void __neko_draw_rect_2d_lines(neko_command_buffer_t* cb, f32 x0, f32 y0, f32 x1, f32 y1, neko_color_t color);
void __neko_draw_box(neko_command_buffer_t* cb, neko_vec3 origin, neko_vec3 half_extents, neko_color_t color);
void __neko_draw_box_textured(neko_command_buffer_t* cb, neko_vec3 origin, neko_vec3 half_extents, u32 tex_id, neko_color_t color);
void __neko_draw_box_vqs(neko_command_buffer_t* cb, neko_vqs xform, neko_color_t color);
void __neko_draw_box_textured_vqs(neko_command_buffer_t* cb, neko_vqs xform, u32 tex_id, neko_color_t color);
void __neko_draw_box_lines(neko_command_buffer_t* cb, neko_vec3 origin, neko_vec3 half_extents, neko_color_t color);
void __neko_draw_box_lines_vqs(neko_command_buffer_t* cb, neko_vqs xform, neko_color_t color);
void __neko_draw_sphere(neko_command_buffer_t* cb, neko_vec3 center, f32 radius, neko_color_t color);
void __neko_draw_sphere_lines(neko_command_buffer_t* cb, neko_vec3 center, f32 radius, neko_color_t color);
void __neko_draw_sphere_lines_vqs(neko_command_buffer_t* cb, neko_vqs xform, neko_color_t color);
void __neko_draw_text(neko_command_buffer_t* cb, f32 x, f32 y, const char* text, neko_color_t color);
void __neko_draw_text_ext(neko_command_buffer_t* cb, f32 x, f32 y, const char* text, neko_resource(neko_font_t) ft, neko_color_t color);
void __neko_push_camera(neko_command_buffer_t* cb, neko_camera_t camera);
void __neko_pop_camera(neko_command_buffer_t* cb);
void __neko_mat_rotatef(neko_command_buffer_t* cb, f32 rad, f32 x, f32 y, f32 z);
void __neko_mat_rotatev(neko_command_buffer_t* cb, f32 rad, neko_vec3 v);
void __neko_mat_rotateq(neko_command_buffer_t* cb, neko_quat q);
void __neko_mat_transf(neko_command_buffer_t* cb, f32 x, f32 y, f32 z);
void __neko_mat_transv(neko_command_buffer_t* cb, neko_vec3 v);
void __neko_mat_scalef(neko_command_buffer_t* cb, f32 x, f32 y, f32 z);
void __neko_mat_scalev(neko_command_buffer_t* cb, neko_vec3 v);
void __neko_mat_mul_vqs(neko_command_buffer_t* cb, neko_vqs xform);
neko_camera_t __neko_begin_3d(neko_command_buffer_t* cb);
void __neko_end_3d(neko_command_buffer_t* cb);
neko_camera_t __neko_begin_2d(neko_command_buffer_t* cb);
void __neko_end_2d(neko_command_buffer_t* cb);

/*
    What are the responsibilities here?
    The graphics subsystem will construct a renderer then be responsible for holding graphics scenes,
    dispatching calls to the renderer?
    Or should there just be a renderer? To make it simple?
    Want to load resources from file using the graphics subsystem, however that is totally dependent on what
    graphics API is being used (for instanced, a shader in a OGL vs. DX)

    Want some concept of "render passes" that can be ordered with custom jobs the user or renderer can define
    Does this require a simple job system before this can be implemented? Does a job system require a better memory
    management system? (I don't think so for the latter)

    Graphics subsystem holds raw resources, such as :
        * texture ids
        * vertex buffers
        * index buffers
        * uniform buffers
        * uniform handles
        * shader handles
        * render texture handle
        * frame buffer handle

    Asset subsystem will hold engine level assets that will use raw resources from various other subsystems:
        * mesh
        * texture
        * shader

    neko_graphics_i* gfx = neko_engine_subsystem()->ctx.graphics;

    // Will load texture into VRAM, will not store texture data in RAM,
    //  but will keep texture handle in slot array in graphics subsystem
    neko_resource_handle texture = gfx->load_texture("path/to/image.extension");

    // Load shader
    neko_resource_handle shader = gfx->load_shader(neko_compute, "path/to/shader.extension");
    neko_resource_handle shader = gfx->load_shader(neko_default, "path/to/vertex_shader.extension", "path/to/fragment_shader.extension");
    neko_resource_handle shader = gfx->load_shader(neko_geometry, "path/to/vertex_shader.extension", "path/to/fragment_shader.extension", "path/to/geo_shader.extension");

    // Create render target resource
    neko_resource_handle render_target = gfx->create_render_target(width, height, neko_render_target_flags);

    // For render passes, not sure what this could look like...


// // Uniform buffer objects are better...just push down to opengl 3.3
// neko_render_pipeline_state_t state = gfx->construct_render_pipeline_state();     // By default, constructing a NEW pipeline state will construct a new ubo
// state.draw_mode = mode;
// state.vertex_buffer = vbo;
// state.index_buffer = ibo;
// state.params = params;
// state.uniform_block = block;

// // Actual render loop example
// {
//  gfx->set_state(cb, state);
//  {
//      gfx->set_state_uniform(cb, "uniform_name", neko_uniform_type_mat4, mat);
//      gfx->set_state_uniform(cb, "uniform_name", neko_uniform_type_mat4, mat);
//      gfx->set_state_draw_mode(cb, mode);

//      neko_for_range_i(buffers)
//      {
//          gfx->set_vertex_buffer(cb, buffers[i].vbo);
//          gfx->set_index_buffer(cb, buffers[i].ibo);

//          // Draw sets all state parameters currently set then submits draw call (inefficient)
//          gfx->draw_indexed(cb, ...);
//      }

//      // What does this do? It just submits immediate vertex data to the backend.
//      // Depending on the shader set, this *might* not render correctly.
//      // Requires a very particular setup.
//      // So unless these push/pop states, can't guarantee that they'll always work.
//  }
//  gfx->flush_state(cb);
    neko_pipeline_state_params_t params = {0};
    params.depth_enabled = false;
    params.face_culling = ...;
    params.winding_order = ...;
    params.vertex_buffer = ...;
    params.index_buffer = ...;
    params.shader = ...;
    params.clear_bit = ...;
    params.clear_color = ...;
    params.frame_buffer = ...;

    gfx->set_pipeline_state_params(cb, params);

    // This is assuming a bound pipeline state
    rapi.setGraphicsPipeline(pipelineStates[entryIdx], cb); -> binds relevant state
    rapi.setGpuParams(gpuParams[entryIdx], cb);
    rapi.setVertexBuffers(0, &vertexBuffers[entryIdx], 1, cb);
    rapi.setIndexBuffer(indexBuffers[entryIdx], cb);
    rapi.setVertexDeclaration(vertexDeclarations[entryIdx], cb);
    rapi.setDrawOperation(DOT_TRIANGLE_LIST, cb);
    rapi.drawIndexed(0, numIndices[entryIdx], 0, numVertices[entryIdx], 0, cb);

    // Render Loop

    // Bind state, which will include making clears, setting up frame buffer, binding uniforms, etc.
    // I don't understand. What's the difference between this and that and this and that and...
    gfx->set_pipeline_state(cb, state);
    {
        neko_for_range_i(neko_dyn_array_size(models))
        {
            // Issues change flag for pipeline (to rebind on next draw call submitted?)
            gfx->bind_vertex_buffer(cb, models[i].vbo);
            gfx->bind_index_buffer(cb, models[i].ibo, cb);
            gfx->set_vertex_decl(cb, decl); // Interesting...
            gfx->set_draw_operation(cb, mode);
            gfx->draw_indexed(cb, 0, indices, 0, verts, 0);
            gfx->bind_pipeline_state_uniform(cb, state, "u_model", neko_uniform_type_mat4, models[i].model_mtx); (will set data and bind, simple)
            gfx->draw();
        }
    }
    gfx->set_pipeline_state(cb, 0); // Reset to default state

    neko_graphics_immediate_draw_i* imm = &gfx->immediate_i;
    imm->begin_drawing(cb, );

    neko_resource(neko_render_pipeline_state_t) state = gfx->construct_render_pipeline_state();
    gfx->set_pipeline_state_params(cb, params);
    gfx->set_pipeline_state_uniform(cb, state, name, type);

    gfx->set_pipeline_state_uniform(cb, state, name, type); -> should this then be done before? is it even necessary?
    gfx->set_pipeline_state(cb, state);     // Just sets internally? Or actively binds all uniforms/state/etc.

    gfx->set_pipeline_state_uniform(cb, state, name, type);
    gfx->bind_pipeline_state(cb, state);



    neko_resource_handle




    From: http://www.gijskaerts.com/wordpress/?p=112
    Handles are essentially strongly typed integers. You can implement these in various ways: You could wrap an integer type into a struct, or define an enum class with a sized integer backing. The
important thing here is that they’re strongly typed; you don’t want to be able to assign a handle representing a descriptor table to a handle representing a 2D texture, for example.

    // An example of a strongly typed 32-bit handle using an enum class
    // Note: A typedef or using statement won't work here as this won't provide strong typing

    enum class Tex2DHandle : uint32 { Invalid = 0xFFFFFFFF };
    // The same handle type using the struct approach

    struct Tex2DHandle { uint32 m_value; };

    // Recommending NOT making typdefs for resource handles and instead strongly typing them to ensure correctness

    // Could still use a macro to define these types in a "generic" way

    #define neko_resource(type)\
        neko_resource_#type

    #define neko_declare_resource_type(type)\
        typedef struct neko_resource(type) {\
    \       u32 id;\
        } neko_resource(type);\

    neko_declare_resource_type(neko_texture);
    neko_declare_resource_type(neko_shader);
    ...

    void main()
    {
        // Get handle to texture resource (strongly typed)
        neko_resource(neko_texture) texture = gfx->construct_texture(...);
    }


    // Example of what a render packet could look like. Again, this is up to you!
    struct RenderPacket
    {
      PipelineState m_pipelineState;
      VertexBufferView m_vertexBuffers[8];
      IndexBufferView m_indexBuffer;
      ShaderResourceHandle m_shaderResources[16];
      PrimitiveTopology m_topology;
      ...
    };
    // Example draw operation using a render packet
    void DrawIndexed(CommandBuffer& cbuffer, const RenderPacket& packet);


    void Draw(CommandBuffer& cb, const RenderPacket& packet);
    void DrawIndex(CommandBuffer& cb, const RenderPacket& packet);
    void DrawIndirect(CommandBuffer& cb, const RenderPacket& packet, BufferHandle argsBuffer);
    void Dispatch(CommandBuffer& cb, uint32 x, uint32 y, uint32 z);
    void CopyResource(commandBuffer& cb, BufferHandle src, BufferHandle dest);
    #if defined (SOME_PLATFORM)
    void ExoticOperationOnlySupportedOnSomePlatform(CommandBuffer& cb);
    #endif

    //////////////////////////////////////////////////////////////////////////////////////////////
    // Example of a graphics pipeline state with a fragment + vertex shader, and enabled blending
    //////////////////////////////////////////////////////////////////////////////////////////////

    // Vertex program GLSL source
    const char* vertProgSrc = R"(
        layout (binding = 0, std140) uniform GUIParams
        {
            mat4 gWorldTransform;
            float gInvViewportWidth;
            float gInvViewportHeight;
            vec4 gTint;
        };

        layout (location = 0) in vec3 bs_position;
        layout (location = 1) in vec2 bs_texcoord0;

        layout (location = 0) out vec2 texcoord0;

        out gl_PerVertex
        {
            vec4 gl_Position;
        };

        void main()
        {
            vec4 tfrmdPos = gWorldTransform * vec4(bs_position.xy, 0, 1);

            float tfrmdX = -1.0f + (tfrmdPos.x * gInvViewportWidth);
            float tfrmdY = 1.0f - (tfrmdPos.y * gInvViewportHeight);

            gl_Position = vec4(tfrmdX, tfrmdY, 0, 1);
            texcoord0 = bs_texcoord0;
        }
    )";

    // Fragment program GLSL source
    const char* fragProgSrc = R"(
        layout (binding = 0, std140) uniform GUIParams
        {
            mat4 gWorldTransform;
            float gInvViewportWidth;
            float gInvViewportHeight;
            vec4 gTint;
        };

        layout (binding = 1) uniform sampler2D gMainTexture;

        layout (location = 0) in vec2 texcoord0;
        layout (location = 0) out vec4 fragColor;

        void main()
        {
            vec4 color = texture2D(gMainTexture, texcoord0.st);
            fragColor = color * gTint;
        }
    )";

    // Descriptor structures used for creating the GPU programs
    GPU_PROGRAM_DESC vertProgDesc;
    vertProgDesc.type = GPT_VERTEX_PROGRAM;
    vertProgDesc.entryPoint = "main";
    vertProgDesc.language = "GLSL";
    vertProgDesc.source = vertProgSrc;

    GPU_PROGRAM_DESC fragProgDesc;
    fragProgDesc.type = GPT_FRAGMENT_PROGRAM;
    fragProgDesc.entryPoint = "main";
    fragProgDesc.language = "GLSL";
    fragProgDesc.source = fragProgSrc;

    // Descriptor structures used for setting blend and depth-stencil states
    BLEND_STATE_DESC blendDesc;
    blendDesc.renderTargetDesc[0].blendEnable = true;
    blendDesc.renderTargetDesc[0].renderTargetWriteMask = 0b0111; // RGB, don't write to alpha
    blendDesc.renderTargetDesc[0].blendOp = BO_ADD;
    blendDesc.renderTargetDesc[0].srcBlend = BF_SOURCE_ALPHA;
    blendDesc.renderTargetDesc[0].dstBlend = BF_INV_SOURCE_ALPHA;

    DEPTH_STENCIL_STATE_DESC depthStencilDesc;
    depthStencilDesc.depthWriteEnable = false;
    depthStencilDesc.depthReadEnable = false;

    // Create pipeline state descriptor
    PIPELINE_STATE_DESC pipelineDesc;
    pipelineDesc.blendState = BlendState::create(blendDesc);
    pipelineDesc.depthStencilState = DepthStencilState::create(depthStencilDesc);
    pipelineDesc.vertexProgram = GpuProgram::create(vertProgDesc);
    pipelineDesc.fragmentProgram = GpuProgram::create(fragProgDesc);

    // And finally, create the pipeline
    SPtr<GraphicsPipelineState> pipelineState = GraphicsPipelineState::create(pipelineDesc);

    //////////////////////////////////////////////////////////////////////////////////////////////
    // Creating and setting GPU parameters for the pipeline we created above
    //////////////////////////////////////////////////////////////////////////////////////////////

    // Create a container object to hold the parameters for all GPU programs in the pipeline state
    SPtr<GpuParams> gpuParams = GpuParams::create(pipelineState);

    // Create a structure that will hold our uniform block variables
    struct UniformBlock
    {
        Matrix4 gWorldTransform;
        float gInvViewportWidth;
        float gInvViewportHeight;
        Color gTint;
    };

    // Fill out the uniform block variables
    UniformBlock uniformBlock;
    uniformBlock.gWorldTransform = Matrix4::IDENTITY;
    uniformBlock.gInvViewportWidth = 1.0f / 1920.0f;
    uniformBlock.gInvViewportHeight = 1.0f / 1080.0f;
    uniformBlock.gTint = Color::White;

    // Create a uniform block buffer for holding the uniform variables
    SPtr<GpuParamBlockBuffer> uniformBuffer = GpuParamBlockBuffer::create(sizeof(UniformBlock));
    uniformBuffer->write(0, &uniformBlock, sizeof(uniformBuffer));

    // Assign the uniform buffer to set 0, binding 0
    gpuParams->setParamBlockBuffer(0, 0, uniformBuffer);

    // Import a texture to assign to the gMainTexture parameter
    HTexture texture = gImporter().import<Texture>("myTexture.png");

    // Assign the texture to set 0, binding 1
    gpuParams->setTexture(0, 1, texture);

    // Bind the GPU parameters for use
    RenderAPI::setGpuParams(gpuParams);

    //////////////////////////////////////////////////////////////////////////////////////////////
    // Example for executing 8000 draw calls distributed over eight threads
    //////////////////////////////////////////////////////////////////////////////////////////////

    // Retrieve the core thread's render API interface
    RenderAPI& rapi = RenderAPI::instance();

    // Create eight command buffers we'll use for parallel command submission
    SPtr<CommandBuffer> commandBuffers[8];
    for (UINT32 i = 0; i < 8; i++)
        commandBuffers[i] = CommandBuffer::create(GQT_GRAPHICS); // Command buffers running on the graphics queue

    // Worker that queues 1000 different draw calls on a command buffer with the provided index
    // (For simplicity, assuming you have created relevant pipeline states, GPU parameters, index/vertex buffers
    // and vertex declarations earlier)
    auto renderWorker = [&](UINT32 idx)
    {
        SPtr<CommandBuffer> cb = commandBuffers[idx];
        for(UINT32 i = 0; i < 1000; i++)
        {
            UINT32 entryIdx = idx * 1000 + i;

            rapi.setGraphicsPipeline(pipelineStates[entryIdx], cb);
            rapi.setGpuParams(gpuParams[entryIdx], cb);
            rapi.setVertexBuffers(0, &vertexBuffers[entryIdx], 1, cb);
            rapi.setIndexBuffer(indexBuffers[entryIdx], cb);
            rapi.setVertexDeclaration(vertexDeclarations[entryIdx], cb);
            rapi.setDrawOperation(DOT_TRIANGLE_LIST, cb);
            rapi.drawIndexed(0, numIndices[entryIdx], 0, numVertices[entryIdx], 0, cb);
        }
    };

    // Run all of our worker threads
    SPtr<Task> tasks[8];
    for(UINT32 i = 0; i < 8; i++)
    {
        tasks[i] = Task::create("Render", std::bind(&renderWorker, i));
        TaskScheduler::instance().addTask(tasks[i]);
    }

    // Block this thread until all workers are done populating the command buffers
    for (UINT32 i = 0; i < 8; i++)
        tasks[i]->wait();

    // Submit all the command buffers for execution
    for (UINT32 i = 0; i < 8; i++)
        rapi.submitCommandBuffer(commandBuffers[i]);


    Command queue -> Command Buffers -> Commands

    Have command buffers for graphics tasks, compute tasks, transfer tasks

    typedef neko_graphics_i
    {
        neko_resource_handle(neko_texture) (* load_texture_from_file)(const char* path);
    } neko_graphics_i;

    Graphics resources are entirely render backend dependent

    What would a simple forward shader look like?

    // Load shader source from file
    const char* vs = platform->load_from_file("shader.vs");
    const char* fs = platform->load_from_file("shader.fs");

    neko_resource(neko_shader) shader = gfx->load_shader(neko_render, vs, fs);

    // Simple triangle
    f32 vert_data[] =
    {
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f
    };

    u32 index_data[] =
    {
        0, 1, 2, 2, 3, 0
    };

    typedef enum neko_uniform_type
    {
        neko_uniform_type_float;
        neko_uniform_type_int;
        neko_uniform_type_vec2;
        neko_uniform_type_vec3;
        neko_uniform_type_vec4;
        neko_uniform_type_mat4;
        neko_uniform_type_texture_2d;
        neko_uniform_type_texture_3d;
    } neko_uniform_type;

    typedef struct neko_uniform_desc
    {
        neko_uniform_type type;
        neko_resource(neko_uniform) uniform_handle;
    } neko_uniform_desc;

    typedef struct neko_uniform_buffer
    {
        neko_dyn_array(neko_uniform_desc) uniforms;
    } neko_uniform_buffer;

    typedef struct neko_pipeline_state_desc
    {
        neko_resource(neko_pipeline_state)  shader;
        neko_resource(neko_vertex_buffer)   vertex_buffer;
        neko_resource(neko_index_buffer)        index_buffer;
        neko_resource(neko_uniform_buffer)  uniform_buffer;
    } neko_pipeline_state_desc;

    neko_resource(neko_vertex_buffer) vertex_buffer = gfx->create_vertex_buffer(&vert_data, 4);
    neko_resource(neko_index_buffer) index_buffer = gfx->create_index_buffer(&index_data, 6);

    // Not sure how to create a uniform buffer object though...or how I would like to create individual uniforms?
    neko_resource(neko_uniform) projMatrixHandle = gfx->create_uniform_handle(shader, "projMatrix");
    neko_resource(neko_uniform) viewMatrixHandle = gfx->create_uniform_handle(shader, "viewMatrix");
    neko_resource(neko_uniform) modelMatrixHandle = gfx->create_uniform_handle(shader, "modelMatrix");

    neko_graphics_params_desc g_desc = neko_default_val();
    g_desc.blend_state              = neko_blend_state_default;
    g_desc.frame_buffer             = neko_back_buffer;
    g_desc.stencil_state            = neko_stencil_default;

    // Create a pipeline state object (as a resource?)
    neko_pipeline_state_desc p_desc     = neko_default_val();
    p_desc.shader                   = shader;
    p_desc.vertex_buffer            = vertex_buffer;
    p_desc.index_buffer             = index_buffer;

    // Create pipeline state object passing in descriptor and get resource handle back
    neko_resource(neko_pipeline_state) ps = gfx->create_pipeline_state(p_desc);

    // Another possible api for setting parameters for the state object
    neko_resource(neko_pipeline_state) pso = gfx->create_pipeline_state();
    gfx->set_pipeline_state_shader(pso, shader);
    gfx->set_pipeline_state_vertex_buffer(pso, vertex_buffer);
    gfx->set_pipeline_state_index_buffer(pso, index_buffer);

    // Construct command buffer
    neko_render_command_buffer cb = gfx->create_command_buffer(neko_render);        // The memory for these command buffers will be recycled internally

    // Set pipeline state
    gfx->set_pipeline_state(&cb, pso);

    // Set global graphics params for pass (whatever these are, usually blend states)
    gfx->set_gfx_params(&cb, gparams);

    // Bind uniforms? These should be held within the gpu params? Or should I allow individual uniform binding such as this?
    // Not sure about these...since they don't really make sense in explicit graphcis apis
    gfx->bind_uniform(&cb, proj_mat_handle, neko_uniform_type_mat4, &proj_mat)
    gfx->bind_uniform(&cb, view_mat_handle, neko_uniform_type_mat4, &view_mat);
    gfx->bind_uniform(&cb, model_mat_handle, neko_uniform_type_mat4, &model_mat);
    gfx->bind_uniform(&cb, texture_handle, neko_uniform_type_sampler, NULL);

    // Draw
    gfx->draw_indexed(&cb, 0, num_indices, 0, num_verts);

    // Submit buffer
    gfx->submit_command_buffer(&cb);

    // Free buffer
    gfx->free_command_buffer(&cb);

*/

// GPU_PROGRAM_DESC vertProgDesc;
// vertProgDesc.type = GPT_VERTEX_PROGRAM;
// vertProgDesc.entryPoint = "main";
// vertProgDesc.language = "GLSL";
// vertProgDesc.source = vertProgSrc;

// // Descriptor structures used for setting blend and depth-stencil states
// BLEND_STATE_DESC blendDesc;
// blendDesc.renderTargetDesc[0].blendEnable = true;
// blendDesc.renderTargetDesc[0].renderTargetWriteMask = 0b0111; // RGB, don't write to alpha
// blendDesc.renderTargetDesc[0].blendOp = BO_ADD;
// blendDesc.renderTargetDesc[0].srcBlend = BF_SOURCE_ALPHA;
// blendDesc.renderTargetDesc[0].dstBlend = BF_INV_SOURCE_ALPHA;

// DEPTH_STENCIL_STATE_DESC depthStencilDesc;
// depthStencilDesc.depthWriteEnable = false;
// depthStencilDesc.depthReadEnable = false;

// typedef struct neko_render_pipeline_state_desc
// {
//  neko_blend_state            blend_state;
//  neko_depth_stencil_state    depth_stencil_state;
//  neko_shader_program         vertex_program;
//  neko_shader_program         fragment_program;
// } neko_pipeline_state_desc;

// Something simple to get something on the fucking screen! Then I can continue to shape it.

// // Create pipeline state descriptor
// PIPELINE_STATE_DESC pipelineDesc;
// pipelineDesc.blendState = BlendState::create(blendDesc);
// pipelineDesc.depthStencilState = DepthStencilState::create(depthStencilDesc);
// pipelineDesc.vertexProgram = GpuProgram::create(vertProgDesc);
// pipelineDesc.fragmentProgram = GpuProgram::create(fragProgDesc);

/*
    typedef struct neko_renderer_i
    {
        void (* render)();
    } neko_renderer_i;

    // Some implementation of a renderer?
    struct neko_deferred_renderer
    {
        _base(neko_renderer_i);
    } neko_deferred_renderer;

    // Record INTO the pass, right? Not sure how to architect this worth a damn...
    void GBufferPass(neko_command_buffer* cb)
    {
        // Want some way to order passes as well eventually, if not manually?

        // Set a pipeline state for this pass? Where is that stored?

        // Grab all renderables to be drawn from the scene
    }

    Alright, want some method for allowing the user to create as many render passes as they want, control the order, control dependencies,
    then set this thing up to go as wide as possible

    Some simple update loop for the renderer

    # Can't do deferred render pass if on mobile due to lack of MRT
    What do you need to do a gbuffer render pass?
        - Frame buffer with MRT (this is already making assumptions about the rendering capabilities, btw)
        - blend settings for state
        - depth settings for state

        For each renderable, bind resource descriptors, like program, uniform, etc.
        Sort renderable by hashed key

    // These pipeline states are for renderables...so does that mean when you attach a new material or mesh to an object, you have to
    // recompile its pipeline state?
    neko_resource(neko_pipeline_state)

    void renderer_init()
    {
        // Initialize gpu params, pipeline states, etc.

        // Create pipeline state object passing in descriptor and get resource handle back
        neko_resource(neko_pipeline_state) ps = gfx->create_pipeline_state(p_desc);

        // I suppose each material could have its own pipeline description...
        neko_pipeline_state_desc gbuffer_pipeline_desc;

        // PIPELINE_STATE_DESC pipelineDesc;
        // pipelineDesc.blendState = BlendState::create(blendDesc);
        // pipelineDesc.depthStencilState = DepthStencilState::create(depthStencilDesc);
        // pipelineDesc.vertexProgram = GpuProgram::create(vertProgDesc);
        // pipelineDesc.fragmentProgram = GpuProgram::create(fragProgDesc);

    }

    void renderer_update()
    {
        neko_command_buffer cb_gbuffer = gfx->construct_command_buffer();
        neko_command_buffer cb_lighting = gfx->construct_command_buffer();

        gbuffer_pass();
        lighting_pass();        // Need information from the gbuffer for this pass
    }

    // Just some example of a USER DEFINED specific uniform block to be used for this particular rendering pass
    typedef struct neko_uniform_block
    {
        neko_mat4 proj_mat;
        neko_mat4 view_mat;
    } neko_uniform_block;

    neko_resource(neko_gpu_params) gpu_params;
    neko_resource(neko_render_target) render_target;

    void gbuffer_pass(neko_command_buffer* cb)
    {
        // Fill out the uniform block variables
        neko_uniform_block ub;
        ub.proj_mat = cam->get_projection_matrix();
        ub.view_mat = cam->get_view_matrix();

        // Bind render surface & clear it
        gfx->setRenderTarget(render_target, false, RT_NONE, cb);
        gfx->clearRenderTarget(FBT_COLOR | FBT_DEPTH, Color::Blue, 1, 0, 0xFF, cb);

        // Create a uniform block buffer for holding the uniform variables
        neko_resource(neko_uniform_buffer) uniform_buffer = gfx->create_param_block_buffer(sizeof(neko_uniform_block));
        gfx->write_param_block(0, &ub, sizeof(neko_uniform_block));

        // Grab renderables from scene (not sure who manages this)
        renderables = gfx->get_static_mesh_renderables_from_somewhere();

        // Assign the uniform buffer & texture
        gfx->set_gpu_params_block_buffer(gpu_params, neko_fragment, "params", ub);
        gfx->set_gpu_params_block_buffer(gpu_params, neko_vertex, "params", ub);

        // psuedo code
        for each (renderable in renderables)
        {
            // Bind program for renderable if different from previous
            // Bind required textures for renderable


            // pipeline state object
        }

        // Another possible api for setting parameters for the state object
        neko_resource(neko_pipeline_state) pso = gfx->create_pipeline_state();
        gfx->set_pipeline_state_shader(pso, shader);
        gfx->set_pipeline_state_vertex_buffer(pso, vertex_buffer);
        gfx->set_pipeline_state_index_buffer(pso, index_buffer);

        // Construct command buffer
        neko_render_command_buffer cb = gfx->create_command_buffer(neko_render);        // The memory for these command buffers will be recycled internally

        // Set pipeline state
        gfx->set_pipeline_state(&cb, pso);

        // Set global graphics params for pass (whatever these are, usually blend states)
        gfx->set_gfx_params(&cb, gparams);

        // Bind uniforms? These should be held within the PSO
        // Not sure about these...since they don't really make sense in explicit graphcis apis
        gfx->bind_uniform(&cb, proj_mat_handle, neko_uniform_type_mat4, &proj_mat)
        gfx->bind_uniform(&cb, view_mat_handle, neko_uniform_type_mat4, &view_mat);
        gfx->bind_uniform(&cb, model_mat_handle, neko_uniform_type_mat4, &model_mat);
        gfx->bind_uniform(&cb, texture_handle, neko_uniform_type_sampler, NULL);

        // Draw
        gfx->draw_indexed(&cb, 0, num_indices, 0, num_verts);

        // Submit buffer
        gfx->submit_command_buffer(&cb);
    }

    // Want to give a vertex buffer layout description (to set attributes)
    typedef enum neko_vertex_attribute_type
    {
        neko_attribute_float4,
        neko_attribute_float3,
        neko_attribute_float2,
        neko_attribute_float,
        neko_attribute_uint4,
        neko_attribute_uint3,
        neko_attribute_uint2,
        neko_attribute_uint
    } neko_vertex_attribute_format_type;

    typedef struct neko_vertex_attribute_layout_desc
    {
        usize byte_size;
        neko_dyn_array(neko_vertex_attribute_type) decl;
    } neko_vertex_attribute_layout_desc;

    1---3
    |\  |
    | \ |
    |  \|
    0---2

    f32 vert_data[] =
    {
        // Positions
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f
    };

    f32 index_data[] =
    {
        0, 2, 1, 1, 2, 3
    };

    const char* vs_source = "\
        layout (location = 0) in vec3 neko_position;\
        layout (location = 1) in vec2 neko_texcoord0;\
        layout (location = 0) out vec2 texcoord0;\

        layout (std140) uniform ub_matrices\
        {\
            mat4 g_proj;
            mat4 g_view;\
        };\
\
        void main()\
        {\
            vec4 world_pos = g_proj * g_view * vec4(neko_position.xy, 0, 1);\
            gl_Position = vec4(world_pos.x, world_pos.y, 0, 1);\
            texcoord0 = neko_texcoord0;\
        }";

    const char* neko_source = "\
        layout (std140) uniform ub_matrices\
        {\
            mat4 g_proj;
            mat4 g_view;\
        };
\
        layout (location = 0) in vec2 texcoord0;\
        layout (location = 0) out vec4 fragColor;\
\
        uniform float4 u_color;\
\
        void main()\
        {\
            fragColor = u_color;\
        }\
    )";

    // Use this as our data block to upload every frame
    typedef struct uniform_block
    {
        neko_mat4 proj;
        neko_mat4 view;
    } uniform_block;

    neko_resource(neko_vertex_buffer)       g_vertex_buffer = neko_default_val();
    neko_resource(neko_shader)              g_shader = neko_default_val();
    neko_resource(neko_render_command_buffer) g_cb = neko_default_val();
    neko_resource(neko_uniform_buffer)      g_ub = neko_default_val();
    neko_resource(neko_uniform)             g_uniform = neko_default_val();

    // Should this be set as a callback to the renderer?
    // Should user be in charge of update loop completely?
    void render()
    {
        // Graphics graphics API
        neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;

        // Reset render command buffer
        gfx->reset_command_buffer(g_cb);

        // Bind shader
        gfx->set_shader(g_cb, g_shader);

        // Set data for uniform block
        uniform_block ub = (uniform_block){ cam->projection_matrix(), cam->view_matrix() };

        // Upload data into uniform buffer
        gfx->set_uniform_buffer_sub_data(g_cb, g_ub, &ub, sizeof(uniform_block));

        // Bind constant uniform
        f32 red[4] = { 1.0f, 0.f, 0.f, 1.f };
        gfx->set_uniform(g_cb, g_uniform, &red);

        // Bind vertex buffer
        gfx->set_vertex_buffer(g_cb, g_vertex_buffer);

        // Draw
        gfx->draw(g_cb, 0, 3);

        // Submit/Flush command buffer
        gfx->submit_command_buffer(g_cb);
    }

    void app_init()
    {
        init_graphics_resources();
    }

    void init_graphics_resources()
    {
        neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;

        // Construct layout description format for our mesh data
        neko_vertex_buffer_layout_desc v_desc;
        gfx->push_vertex_attribute(&v_desc, neko_attribute_float3);

        // Construct vertex buffer
        g_vertex_buffer = gfx->construct_buffer(neko_vertex_buffer, v_desc, vert_data, sizeof(vert_data));

        // This is a little ehh...We'll just do it for now (eventually, this will go into a pipeline state object to emulate explicit api)
        g_shader = gfx->construct_shader(neko_render, vs_source, fs_source);

        // Construct command buffer to use for rendering
        g_cb = gfx->construct_command_buffer(neko_render_command_buffer);

        // Construct uniform buffer binding handle
        g_ub = gfx->construct_uniform_buffer(g_shader, "ub_matrices");

        // Construct uniform handle for individual/constant/push uniform
        g_uniform = gfx->construct_uniform(g_shader, "u_color");
    }

    // Example program showing rendering a simple triangle to screen
    void main()
    {
        // Make engine and init...
    }


    // Rendering fonts

    Fonts are NOT a graphics level resource - the texture for the atlas, however, is. Fonts are an asset level resource,
    which might specifically be a game/application layer concept.

    typedef struct neko_font
    {
        // Glyph data for font
        ...

        // Handle to internal texture atlas resource
        neko_resource(neko_texture) atlas;
    } neko_font;

    // Debug drawing (for lines / shapes / primitives / etc.)

    // Should add this into a vertex buffer. Use some internal rendering mechanism for displaying all of this data.
    // It'll get drawn on top of all other passes? Or the user can determine when to submit? I suppose it doesn't really matter...
    // Or, could just draw this as a single draw call? I don't like that option.
    gfx->draw_line(start, end, color);

    ...

    // Could this be what I want?
    gfx->submit_debug_drawing(cb_handle);

    // Want to throw all debug drawing ops into a vertex buffer for a single draw call
    // Want the ability to do 3d rendering as well as 2d rendering for it
    // Want the user to have ability to render these shapes into whatever they want, and then set whatever
    // parameters they want for debug rendering, such as view, projection

    // Internally
    struct debug_drawing_internal_data {
        neko_mat4 view_matrix;
        neko_mat4 proj_matrix;
        neko_resource(neko_vertex_buffer) vbo;
        neko_dyn_array(f32) vertex_data;
    };

    struct debug_properties props = {
        view_matrix,
        proj_matrix
    };

    // I mean, what's the point of this, though? Provide simple utilities for adding vertex data into a buffer
    // for primitive drawing. Abstracting the bore of having to code all of that yourself. A particular shader
    // is needed for drawing, the vertex layout needs to be understood correctly.

    immediate->begin_3d();
    {
    }
    immediate->end_3d();

    gfx->set_state(cb, gfx->immediate_state_3d());
        gfx->draw_line();
    gfx->flush_state();

    gfx->set_debug_properties(cb_handle, props);
        gfx->draw_line(cb_handle, start, end, color);
        gfx->draw_cube(cb_handle, aabb, model_matrix, color);
        gfx->draw_square(cb_handle, origin, extents, color);
    gfx->submit_debug_rendering(cb_handle);

    // Graphics update
    Debug Rendering...

    // Alright, so forget 3d rendering...
    // Let's do simple 2d rendering solely...how do? I can't access a global buffer if I want this to scale...
    // Wait, yes I can...I'm being dumb. In drawing lines, it'll add in the submit phase. This'll work.
    // But...if I want these things to be quads, that's where the lines will be weird. Unless I keep this entire
    // implementation 2d. Or have two different vertex buffers - one for lines, one for quads. Then all
    // debug drawing will come down to two separate calls. I think I'm okay with that...maybe?
    // When would the actual drawing take place? In a command buffer submission? Or at end of frame? What if user
    // wants to render debug data into a particular render target? The user can't control the way the debug drawing looks,
    // but he can control where it is drawn into.

    void graphics_update()
    {
        // Submit all command buffers...

        // Do debug drawing
    }

    What a life to live - working for some woke, multi-national corporation that uses degnerate social issues as a signal solely to
    extract more wealth for itself from everyone else. Don't you dare question it, of course - just produce more goods for the
    leech to sell to mindless consumers, who are unaware just how their participation is feeding into their purposelessness,
    their all-encompassing nihilism and total lack of self ownership. Take back control of your life. Stop being a mindless zombie for
    large corporations and governments that see you as nothing more than meat for the grinder.

    struct neko_render_pass
    {
        neko_command_buffer_t* command_buffer;
    }

    What would a deferred rendering pass look like? Would be implicit, I believe...

    render pipelines

    // Each pipeline state holds a reference to a uniform block
    typedef struct neko_render_pipeline_state_t
    {
    } neko_render_pipeline_state_t;

    // Are they transient? I suppose they could be...

    // Uniform buffer objects are better...just push down to opengl 3.3
    neko_render_pipeline_state_t state = gfx->construct_render_pipeline_state();    // By default, constructing a NEW pipeline state will construct a new ubo
    state.draw_mode = mode;
    state.vertex_buffer = vbo;
    state.index_buffer = ibo;
    state.params = params;
    state.uniform_block = block;

    // Actual render loop example
    {
        gfx->set_state(cb, state);
        {
            gfx->set_state_uniform(cb, "uniform_name", neko_uniform_type_mat4, mat);
            gfx->set_state_uniform(cb, "uniform_name", neko_uniform_type_mat4, mat);
            gfx->set_state_draw_mode(cb, mode);

            neko_for_range_i(buffers)
            {
                gfx->set_vertex_buffer(cb, buffers[i].vbo);
                gfx->set_index_buffer(cb, buffers[i].ibo);

                // Draw sets all state parameters currently set then submits draw call
                gfx->draw_indexed(cb, ...);
            }

            // What does this do? It just submits immediate vertex data to the backend.
            // Depending on the shader set, this *might* not render correctly.
            // Requires a very particular setup.
            // So unless these push/pop states, can't guarantee that they'll always work.
        }
        gfx->flush_state(cb);

        gfx->set_state(cb, state);
        gfx->set_state_uniform(cb); // Affects currently bound state. Could be default state, if none set.
        gfx->set_state_params(cb);
        gfx->draw(cb);
        gfx->flush(cb);

        // Perhaps certain pipeline states could be created?
        // And these states have certain features that can be filled out? Like function pointers?
        // User data as well?

        // Grab copies of default states
        neko_render_pipeline_state_t state_2d = gfx->immediate.render_pipeline_2d();    // Just don't like these being so specific and in the graphics api
        neko_render_pipeline_state_t state_3d = gfx->immediate.render_pipeline_3d();

        // I might need to make a specific rendering pipeline separate to handle this.
        // So have the states, but then create an example that has immediate mode rendering outside of the main graphics library.
        // I think that might be the better way to handle this.

        gfx->set_state(cb, state_2d);
        {
            gfx->draw_line(cb, ...);
            gfx->draw_textured_rect(...);
        }
        gfx->flush(cb);

        // Goofy helper function (creates transient render pipeline state and sets to active state)
        gfx->set_state(cb, gfx->construct_render_pipeline_state_transient());               // Just create a new transient state on the stack with this call
        gfx->set_state_uniform(cb, neko_vp_default_name, neko_default_3d_camera_view_proj());
        gfx->push_matrix(cb, mat_model);
            neko_mat4 mat = neko_mat_mul_list(...);
            gfx->set_state_uniform(cb, neko_model_default_name, mat);
            gfx->draw_line();
        gfx->pop_matrix(cb);
        gfx->draw_line();
        gfx->flush(cb);

        // Now in default state, can draw these things regularly
        gfx->push_state(cb, neko_graphics_default_3d_state());      // Does this create a new state, with a new uniform buffer? Can they be transient?
        {
            neko_mat4_mat = neko_mat_stack(
                4,
                ...,
                ...,
                ...
            );

            // This is then enclosed within the state, so will pop off with it
            gfx->set_state_uniform(cb, neko_default_model_matrix_uniform_name, neko_uniform_type_mat4, mat); // Going to be an issue with shared uniform blocks

            // Of course, these aren't expected to work for all states.
            gfx->draw_line(cb);
            gfx->draw_box(cb);
            gfx->draw_text(cb);
        }
        gfx->pop_state(cb);     // Pop state will flush the immediate vertex buffer

        // Final submit of buffer for render
        gfx->submit_command_buffer(cb);

        // Submitting command buffer also resets ALL default pipeline states
    }

    // Immediate mode rendering api provided (for debugging and "hand holding" mainly)
    graphics_immediate_i* imm = &gfx->immediate_i;
    imm->begin_drawing(cb, neko_immediate_mode_2d);
    {
        imm->push_camera(cb, imm->camera_3d());
        {
            imm->push_matrix(cb, ...);
                imm->mat_transf(cb, ...);
                imm->mat_rotateq(cb, ...);
                imm->draw_box(cb, ...);
                imm->draw_mesh(cb, ...);
            imm->pop_matrix(cb);
        }
        imm->pop_camera(cb);
        imm->draw_line(cb, ...);
    }
    imm->end_drawing(cb);

    // In the backend, this gets called...
    imm->begin_drawing(cb, neko_immediate_mode_2d); -->

    gfx->set_state(cb, gfx->immediate->default_state);
    neko_mat4 vp_mat = {0};
    switch (mode)
    {
        case neko_immediate_mode_2d: vp_mat = gfx->immediate->default_vp_mat_ortho(); break;
        case neko_immediate_mode_3d: vp_mat = gfx->immediate->default_vp_mat_persp(); break;
    }
    gfx->set_state_uniform(cb, gfx->immediate->vp_matrix_name, neko_uniform_type_mat4, gfx->immediate->default_vp_orth_matrix());

    rapi.setGraphicsPipeline(pipelineStates[entryIdx], cb);
    rapi.setGpuParams(gpuParams[entryIdx], cb);
    rapi.setVertexBuffers(0, &vertexBuffers[entryIdx], 1, cb);
    rapi.setIndexBuffer(indexBuffers[entryIdx], cb);
    rapi.setVertexDeclaration(vertexDeclarations[entryIdx], cb);
    rapi.setDrawOperation(DOT_TRIANGLE_LIST, cb);
    rapi.drawIndexed(0, numIndices[entryIdx], 0, numVertices[entryIdx], 0, cb);

    // Setting individual uniforms
    gfx->set_uniform(state.uniform_block, "", val);
*/

#endif  // NEKO_GRAPHICS_SUBSYSTEM_H
