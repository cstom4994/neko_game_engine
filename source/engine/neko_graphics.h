

#ifndef NEKO_GRAPHICS_H
#define NEKO_GRAPHICS_H

#include "engine/neko.h"
#include "engine/neko_containers.h"

/*=============================
// NEKO_GRAPHICS
=============================*/

// Graphics Pipeline

// Main graphics resources:
// Shader description: vertex, fragment, compute, geometry, tesselation
// Texture Description: texture, depth, render target
// Buffer Description: vertex, index, uniform, frame, pixel
// Pipeline Description: vert-layout, shader, bindables, render states
// Pass Description: render pass, action on render targets (clear, set viewport, etc.)

/* Useful macro for forcing enum decl to be uint32_t type with default = 0x00 for quick init */
#define neko_enum_decl(NAME, ...) typedef enum NAME { _neko_##NAME##_default = 0x0, __VA_ARGS__, _neko_##NAME##_count, _neko_##NAME##_force_u32 = 0x7fffffff } NAME;

#define neko_enum_count(NAME) _neko_##NAME##_count

/* Shader Stage Type */
neko_enum_decl(neko_graphics_shader_stage_type, NEKO_GRAPHICS_SHADER_STAGE_VERTEX, NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT, NEKO_GRAPHICS_SHADER_STAGE_COMPUTE);

/* Winding Order Type */
neko_enum_decl(neko_graphics_winding_order_type, NEKO_GRAPHICS_WINDING_ORDER_CW, NEKO_GRAPHICS_WINDING_ORDER_CCW);

/* Face Culling Type */
neko_enum_decl(neko_graphics_face_culling_type, NEKO_GRAPHICS_FACE_CULLING_FRONT, NEKO_GRAPHICS_FACE_CULLING_BACK, NEKO_GRAPHICS_FACE_CULLING_FRONT_AND_BACK);

/* Blend Equation Type */
neko_enum_decl(neko_graphics_blend_equation_type, NEKO_GRAPHICS_BLEND_EQUATION_ADD, NEKO_GRAPHICS_BLEND_EQUATION_SUBTRACT, NEKO_GRAPHICS_BLEND_EQUATION_REVERSE_SUBTRACT,
               NEKO_GRAPHICS_BLEND_EQUATION_MIN, NEKO_GRAPHICS_BLEND_EQUATION_MAX);

/* Blend Mode Type */
neko_enum_decl(neko_graphics_blend_mode_type, NEKO_GRAPHICS_BLEND_MODE_ZERO, NEKO_GRAPHICS_BLEND_MODE_ONE, NEKO_GRAPHICS_BLEND_MODE_SRC_COLOR, NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_COLOR,
               NEKO_GRAPHICS_BLEND_MODE_DST_COLOR, NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_DST_COLOR, NEKO_GRAPHICS_BLEND_MODE_SRC_ALPHA, NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA,
               NEKO_GRAPHICS_BLEND_MODE_DST_ALPHA, NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_DST_ALPHA, NEKO_GRAPHICS_BLEND_MODE_CONSTANT_COLOR, NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_CONSTANT_COLOR,
               NEKO_GRAPHICS_BLEND_MODE_CONSTANT_ALPHA, NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA);

/* Shader Language Type */
neko_enum_decl(neko_graphics_shader_language_type, NEKO_GRAPHICS_SHADER_LANGUAGE_GLSL);

/* Push Constant Type */
// Really don't want to handle "auto-merging" of data types

/* Uniform Type */
neko_enum_decl(neko_graphics_uniform_type, NEKO_GRAPHICS_UNIFORM_FLOAT, NEKO_GRAPHICS_UNIFORM_INT, NEKO_GRAPHICS_UNIFORM_VEC2, NEKO_GRAPHICS_UNIFORM_VEC3, NEKO_GRAPHICS_UNIFORM_VEC4,
               NEKO_GRAPHICS_UNIFORM_MAT4, NEKO_GRAPHICS_UNIFORM_SAMPLER2D, NEKO_GRAPHICS_UNIFORM_USAMPLER2D, NEKO_GRAPHICS_UNIFORM_SAMPLERCUBE, NEKO_GRAPHICS_UNIFORM_IMAGE2D_RGBA32F,
               NEKO_GRAPHICS_UNIFORM_BLOCK);

/* Uniform Block Usage Type */
neko_enum_decl(neko_graphics_uniform_block_usage_type,
               NEKO_GRAPHICS_UNIFORM_BLOCK_USAGE_STATIC,  // Default of 0x00 is static
               NEKO_GRAPHICS_UNIFORM_BLOCK_USAGE_PUSH_CONSTANT);

/* Sampler Type */
neko_enum_decl(neko_graphics_sampler_type, NEKO_GRAPHICS_SAMPLER_2D);

/* Primitive Type */
neko_enum_decl(neko_graphics_primitive_type, NEKO_GRAPHICS_PRIMITIVE_LINES, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES, NEKO_GRAPHICS_PRIMITIVE_QUADS);

/* Vertex Atribute Type */
neko_enum_decl(neko_graphics_vertex_attribute_type, NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT4, NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3, NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2,
               NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT, NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT4, NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT3, NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT2,
               NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT, NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4, NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE3, NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE2,
               NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE);

/* Buffer Type */
neko_enum_decl(neko_graphics_buffer_type, NEKO_GRAPHICS_BUFFER_VERTEX, NEKO_GRAPHICS_BUFFER_INDEX, NEKO_GRAPHICS_BUFFER_FRAME, NEKO_GRAPHICS_BUFFER_UNIFORM, NEKO_GRAPHICS_BUFFER_UNIFORM_CONSTANT,
               NEKO_GRAPHICS_BUFFER_SHADER_STORAGE, NEKO_GRAPHICS_BUFFER_SAMPLER);

/* Buffer Usage Type */
neko_enum_decl(neko_graphics_buffer_usage_type, NEKO_GRAPHICS_BUFFER_USAGE_STATIC, NEKO_GRAPHICS_BUFFER_USAGE_STREAM, NEKO_GRAPHICS_BUFFER_USAGE_DYNAMIC);

/* Buffer Update Type */
neko_enum_decl(neko_graphics_buffer_update_type, NEKO_GRAPHICS_BUFFER_UPDATE_RECREATE, NEKO_GRAPHICS_BUFFER_UPDATE_SUBDATA);

neko_enum_decl(neko_graphics_access_type, NEKO_GRAPHICS_ACCESS_READ_ONLY, NEKO_GRAPHICS_ACCESS_WRITE_ONLY, NEKO_GRAPHICS_ACCESS_READ_WRITE);

//=== Texture ===//
typedef enum { NEKO_GRAPHICS_TEXTURE_2D = 0x00, NEKO_GRAPHICS_TEXTURE_CUBEMAP } neko_graphics_texture_type;

typedef enum {
    NEKO_GRAPHICS_TEXTURE_CUBEMAP_POSITIVE_X = 0x00,
    NEKO_GRAPHICS_TEXTURE_CUBEMAP_NEGATIVE_X,
    NEKO_GRAPHICS_TEXTURE_CUBEMAP_POSITIVE_Y,
    NEKO_GRAPHICS_TEXTURE_CUBEMAP_NEGATIVE_Y,
    NEKO_GRAPHICS_TEXTURE_CUBEMAP_POSITIVE_Z,
    NEKO_GRAPHICS_TEXTURE_CUBEMAP_NEGATIVE_Z
} neko_graphics_cubemap_face_type;

neko_enum_decl(neko_graphics_texture_format_type, NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8, NEKO_GRAPHICS_TEXTURE_FORMAT_RGB8, NEKO_GRAPHICS_TEXTURE_FORMAT_RG8, NEKO_GRAPHICS_TEXTURE_FORMAT_R32,
               NEKO_GRAPHICS_TEXTURE_FORMAT_R32F, NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA16F, NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA32F, NEKO_GRAPHICS_TEXTURE_FORMAT_A8, NEKO_GRAPHICS_TEXTURE_FORMAT_R8,
               NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH8, NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH16, NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24, NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F,
               NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24_STENCIL8, NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F_STENCIL8, NEKO_GRAPHICS_TEXTURE_FORMAT_STENCIL8);

neko_enum_decl(neko_graphics_texture_wrapping_type, NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT, NEKO_GRAPHICS_TEXTURE_WRAP_MIRRORED_REPEAT, NEKO_GRAPHICS_TEXTURE_WRAP_CLAMP_TO_EDGE,
               NEKO_GRAPHICS_TEXTURE_WRAP_CLAMP_TO_BORDER);

neko_enum_decl(neko_graphics_texture_filtering_type, NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST, NEKO_GRAPHICS_TEXTURE_FILTER_LINEAR);

//=== Clear ===//
neko_enum_decl(neko_graphics_clear_flag, NEKO_GRAPHICS_CLEAR_COLOR = 0x01, NEKO_GRAPHICS_CLEAR_DEPTH = 0x02, NEKO_GRAPHICS_CLEAR_STENCIL = 0x04, NEKO_GRAPHICS_CLEAR_NONE = 0x08);

#define NEKO_GRAPHICS_CLEAR_ALL NEKO_GRAPHICS_CLEAR_COLOR | NEKO_GRAPHICS_CLEAR_DEPTH | NEKO_GRAPHICS_CLEAR_STENCIL

//=== Bind Type ===//
neko_enum_decl(neko_graphics_bind_type, NEKO_GRAPHICS_BIND_VERTEX_BUFFER, NEKO_GRAPHICS_BIND_INDEX_BUFFER, NEKO_GRAPHICS_BIND_UNIFORM_BUFFER, NEKO_GRAPHICS_BIND_STORAGE_BUFFER,
               NEKO_GRAPHICS_BIND_IMAGE_BUFFER, NEKO_GRAPHICS_BIND_UNIFORM);

/* Depth Function Type */
neko_enum_decl(neko_graphics_depth_func_type,  // Default value of 0x00 means depth is disabled
               NEKO_GRAPHICS_DEPTH_FUNC_NEVER, NEKO_GRAPHICS_DEPTH_FUNC_LESS, NEKO_GRAPHICS_DEPTH_FUNC_EQUAL, NEKO_GRAPHICS_DEPTH_FUNC_LEQUAL, NEKO_GRAPHICS_DEPTH_FUNC_GREATER,
               NEKO_GRAPHICS_DEPTH_FUNC_NOTEQUAL, NEKO_GRAPHICS_DEPTH_FUNC_GEQUAL, NEKO_GRAPHICS_DEPTH_FUNC_ALWAYS);

neko_enum_decl(neko_graphics_depth_mask_type,  // Default value 0x00 means depth writing enabled
               NEKO_GRAPHICS_DEPTH_MASK_ENABLED, NEKO_GRAPHICS_DEPTH_MASK_DISABLED);

/* Stencil Function Type */
neko_enum_decl(neko_graphics_stencil_func_type,
               NEKO_GRAPHICS_STENCIL_FUNC_NEVER,  // Default value of 0x00 means stencil is disabled
               NEKO_GRAPHICS_STENCIL_FUNC_LESS, NEKO_GRAPHICS_STENCIL_FUNC_EQUAL, NEKO_GRAPHICS_STENCIL_FUNC_LEQUAL, NEKO_GRAPHICS_STENCIL_FUNC_GREATER, NEKO_GRAPHICS_STENCIL_FUNC_NOTEQUAL,
               NEKO_GRAPHICS_STENCIL_FUNC_GEQUAL, NEKO_GRAPHICS_STENCIL_FUNC_ALWAYS);

/* Stencil Op Type */
neko_enum_decl(neko_graphics_stencil_op_type,  // Default value of 0x00 means keep is used
               NEKO_GRAPHICS_STENCIL_OP_KEEP, NEKO_GRAPHICS_STENCIL_OP_ZERO, NEKO_GRAPHICS_STENCIL_OP_REPLACE, NEKO_GRAPHICS_STENCIL_OP_INCR, NEKO_GRAPHICS_STENCIL_OP_INCR_WRAP,
               NEKO_GRAPHICS_STENCIL_OP_DECR, NEKO_GRAPHICS_STENCIL_OP_DECR_WRAP, NEKO_GRAPHICS_STENCIL_OP_INVERT);

/* Internal Graphics Resource Handles */
neko_handle_decl(neko_graphics_shader_t);
neko_handle_decl(neko_graphics_texture_t);
neko_handle_decl(neko_graphics_vertex_buffer_t);
neko_handle_decl(neko_graphics_index_buffer_t);
neko_handle_decl(neko_graphics_uniform_buffer_t);
neko_handle_decl(neko_graphics_storage_buffer_t);
neko_handle_decl(neko_graphics_framebuffer_t);
neko_handle_decl(neko_graphics_uniform_t);
neko_handle_decl(neko_graphics_renderpass_t);
neko_handle_decl(neko_graphics_pipeline_t);

/* Graphics Shader Source Desc */
typedef struct neko_graphics_shader_source_desc_t {
    neko_graphics_shader_stage_type type;  // Shader stage type (vertex, fragment, tesselation, geometry, compute)
    const char* source;                    // Source for shader
} neko_graphics_shader_source_desc_t;

/* Graphics Shader Desc */
typedef struct neko_graphics_shader_desc_t {
    neko_graphics_shader_source_desc_t* sources;  // Array of shader source descriptions
    size_t size;                                  // Size in bytes of shader source desc array
    char name[64];                                // Optional (for logging and debugging mainly)
} neko_graphics_shader_desc_t;

#define NEKO_GRAPHICS_TEXTURE_DATA_MAX 6

/* Graphics Texture Desc */
typedef struct neko_graphics_texture_desc_t {
    neko_graphics_texture_type type;
    uint32_t width;                                   // Width of texture in texels
    uint32_t height;                                  // Height of texture in texels
    uint32_t depth;                                   // Depth of texture
    void* data[NEKO_GRAPHICS_TEXTURE_DATA_MAX];       // Texture data to upload (can be null)
    neko_graphics_texture_format_type format;         // Format of texture data (rgba32, rgba8, rgba32f, r8, depth32f, etc...)
    neko_graphics_texture_wrapping_type wrap_s;       // Wrapping type for s axis of texture
    neko_graphics_texture_wrapping_type wrap_t;       // Wrapping type for t axis of texture
    neko_graphics_texture_wrapping_type wrap_r;       // Wrapping type for r axis of texture
    neko_graphics_texture_filtering_type min_filter;  // Minification filter for texture
    neko_graphics_texture_filtering_type mag_filter;  // Magnification filter for texture
    neko_graphics_texture_filtering_type mip_filter;  // Mip filter for texture
    neko_vec2 offset;                                 // Offset for updates
    uint32_t num_mips;                                // Number of mips to generate (default 0 is disable mip generation)
    struct {
        uint32_t x;       // X offset in texels to start read
        uint32_t y;       // Y offset in texels to start read
        uint32_t width;   // Width in texels for texture
        uint32_t height;  // Height in texels for texture
        size_t size;      // Size in bytes for data to be read
    } read;
    uint16_t flip_y;  // Whether or not y is flipped
} neko_graphics_texture_desc_t;

/* Graphics Uniform Layout Desc */
typedef struct neko_graphics_uniform_layout_desc_t {
    neko_graphics_uniform_type type;  // Type of field
    char fname[64];                   // Name of field (required for implicit APIs, like OpenGL/ES)
    uint32_t count;                   // Count variable (used for arrays such as glUniformXXXv)
} neko_graphics_uniform_layout_desc_t;

/* Graphics Uniform Desc */
typedef struct neko_graphics_uniform_desc_t {
    neko_graphics_shader_stage_type stage;
    char name[64];                                // Name of uniform (required for OpenGL/ES, WebGL)
    neko_graphics_uniform_layout_desc_t* layout;  // Layout array for uniform data
    size_t layout_size;                           // Size of uniform data in bytes
} neko_graphics_uniform_desc_t;

typedef struct neko_graphics_buffer_update_desc_t {
    neko_graphics_buffer_update_type type;
    size_t offset;
} neko_graphics_buffer_update_desc_t;

/* Graphics Buffer Desc General */
typedef struct neko_graphics_buffer_base_desc_t {
    void* data;
    size_t size;
    neko_graphics_buffer_usage_type usage;
} neko_graphics_buffer_base_desc_t;

typedef struct neko_graphics_vertex_buffer_desc_t {
    void* data;
    size_t size;
    neko_graphics_buffer_usage_type usage;
    neko_graphics_buffer_update_desc_t update;
} neko_graphics_vertex_buffer_desc_t;

typedef neko_graphics_vertex_buffer_desc_t neko_graphics_index_buffer_desc_t;

typedef struct neko_graphics_uniform_buffer_desc_t {
    void* data;
    size_t size;
    neko_graphics_buffer_usage_type usage;
    const char* name;
    neko_graphics_shader_stage_type stage;
    neko_graphics_buffer_update_desc_t update;
} neko_graphics_uniform_buffer_desc_t;

typedef struct neko_graphics_storage_buffer_desc_t {
    void* data;
    size_t size;
    char name[64];
    neko_graphics_buffer_usage_type usage;
    neko_graphics_access_type access;
    neko_graphics_buffer_update_desc_t update;
} neko_graphics_storage_buffer_desc_t;

typedef struct neko_graphics_framebuffer_desc_t {
    void* data;
} neko_graphics_framebuffer_desc_t;

/* Graphics Clear Action */
typedef struct neko_graphics_clear_action_t {
    neko_graphics_clear_flag flag;  // Flag to be set (clear color, clear depth, clear stencil, clear all)
    union {
        float color[4];   // Clear color value
        float depth;      // Clear depth value
        int32_t stencil;  // Clear stencil value
    };
} neko_graphics_clear_action_t;

/* Graphics Clear Desc */
typedef struct neko_graphics_clear_desc_t {
    neko_graphics_clear_action_t* actions;  // Clear action array
    size_t size;                            // Size
} neko_graphics_clear_desc_t;

/* Graphics Render Pass Desc */
typedef struct neko_graphics_renderpass_desc_t {
    neko_handle(neko_graphics_framebuffer_t) fbo;  // Default is set to invalid for backbuffer
    neko_handle(neko_graphics_texture_t) * color;  // Array of color attachments to be bound (useful for MRT, if supported)
    size_t color_size;                             // Size of color attachment array
    neko_handle(neko_graphics_texture_t) depth;    // Depth attachment to be bound
    neko_handle(neko_graphics_texture_t) stencil;  // Depth attachment to be bound
} neko_graphics_renderpass_desc_t;

/*
    // If you want to write to a color attachment, you have to have a frame buffer attached that isn't the backbuffer
*/

typedef enum neko_graphics_vertex_data_type { NEKO_GRAPHICS_VERTEX_DATA_INTERLEAVED = 0x00, NEKO_GRAPHICS_VERTEX_DATA_NONINTERLEAVED } neko_graphics_vertex_data_type;

typedef struct neko_graphics_bind_vertex_buffer_desc_t {
    neko_handle(neko_graphics_vertex_buffer_t) buffer;
    size_t offset;
    neko_graphics_vertex_data_type data_type;
} neko_graphics_bind_vertex_buffer_desc_t;

typedef struct neko_graphics_bind_index_buffer_desc_t {
    neko_handle(neko_graphics_index_buffer_t) buffer;
} neko_graphics_bind_index_buffer_desc_t;

typedef struct neko_graphics_bind_image_buffer_desc_t {
    neko_handle(neko_graphics_texture_t) tex;
    uint32_t binding;
    neko_graphics_access_type access;
} neko_graphics_bind_image_buffer_desc_t;

typedef struct neko_graphics_bind_uniform_buffer_desc_t {
    neko_handle(neko_graphics_uniform_buffer_t) buffer;
    uint32_t binding;
    struct {
        size_t offset;  // Specify an offset for ranged binds.
        size_t size;    // Specify size for ranged binds.
    } range;
} neko_graphics_bind_uniform_buffer_desc_t;

typedef struct neko_graphics_bind_storage_buffer_desc_t {
    neko_handle(neko_graphics_storage_buffer_t) buffer;
    uint32_t binding;
} neko_graphics_bind_storage_buffer_desc_t;

typedef struct neko_graphics_bind_uniform_desc_t {
    neko_handle(neko_graphics_uniform_t) uniform;
    void* data;
    uint32_t binding;  // Base binding for samplers?
} neko_graphics_bind_uniform_desc_t;

/* Graphics Binding Desc */
typedef struct neko_graphics_bind_desc_t {
    struct {
        neko_graphics_bind_vertex_buffer_desc_t* desc;  // Array of vertex buffer declarations (NULL by default)
        size_t size;                                    // Size of array in bytes (optional if only one)
    } vertex_buffers;

    struct {
        neko_graphics_bind_index_buffer_desc_t* desc;  // Array of index buffer declarations (NULL by default)
        size_t size;                                   // Size of array in bytes (optional if only one)
    } index_buffers;

    struct {
        neko_graphics_bind_uniform_buffer_desc_t* desc;  // Array of uniform buffer declarations (NULL by default)
        size_t size;                                     // Size of array in bytes (optional if only one)
    } uniform_buffers;

    struct {
        neko_graphics_bind_uniform_desc_t* desc;  // Array of uniform declarations (NULL by default)
        size_t size;                              // Size of array in bytes (optional if one)
    } uniforms;

    struct {
        neko_graphics_bind_image_buffer_desc_t* desc;
        size_t size;
    } image_buffers;

    struct {
        neko_graphics_bind_storage_buffer_desc_t* desc;
        size_t size;
    } storage_buffers;

} neko_graphics_bind_desc_t;

/* Graphics Blend State Desc */
typedef struct neko_graphics_blend_state_desc_t {
    neko_graphics_blend_equation_type func;  // Equation function to use for blend ops
    neko_graphics_blend_mode_type src;       // Source blend mode
    neko_graphics_blend_mode_type dst;       // Destination blend mode
} neko_graphics_blend_state_desc_t;

/* Graphics Depth State Desc */
typedef struct neko_graphics_depth_state_desc_t {
    neko_graphics_depth_func_type func;  // Function to set for depth test
    neko_graphics_depth_mask_type mask;  // Whether or not writing is enabled/disabled
} neko_graphics_depth_state_desc_t;

/* Graphics Stencil State Desc */
typedef struct neko_graphics_stencil_state_desc_t {
    neko_graphics_stencil_func_type func;  // Function to set for stencil test
    uint32_t ref;                          // Specifies reference val for stencil test
    uint32_t comp_mask;                    // Specifies mask that is ANDed with both ref val and stored stencil val
    uint32_t write_mask;                   // Specifies mask that is ANDed with both ref val and stored stencil val
    neko_graphics_stencil_op_type sfail;   // Action to take when stencil test fails
    neko_graphics_stencil_op_type dpfail;  // Action to take when stencil test passes but depth test fails
    neko_graphics_stencil_op_type dppass;  // Action to take when both stencil test passes and either depth passes or is not enabled
} neko_graphics_stencil_state_desc_t;

/* Graphics Raster State Desc */
typedef struct neko_graphics_raster_state_desc_t {
    neko_graphics_face_culling_type face_culling;    // Face culling mode to be used (front, back, front and back)
    neko_graphics_winding_order_type winding_order;  // Winding order mode to be used (ccw, cw)
    neko_graphics_primitive_type primitive;          // Primitive type for drawing (lines, quads, triangles, triangle strip)
    neko_handle(neko_graphics_shader_t) shader;      // Shader to bind and use (might be in bindables later on, not sure)
    size_t index_buffer_element_size;                // Element size of index buffer (used for parsing internal data)
} neko_graphics_raster_state_desc_t;

/* Graphics Compute State Desc */
typedef struct neko_graphics_compute_state_desc_t {
    neko_handle(neko_graphics_shader_t) shader;  // Compute shader to bind
} neko_graphics_compute_state_desc_t;

/* Graphics Vertex Attribute Desc */
typedef struct neko_graphics_vertex_attribute_desc_t {
    char name[64];                               // Attribute name (required for lower versions of OpenGL and ES)
    neko_graphics_vertex_attribute_type format;  // Format for vertex attribute
    size_t stride;                               // Total stride of vertex layout (optional, calculated by default)
    size_t offset;                               // Offset of this vertex from base pointer of data (optional, calaculated by default)
    size_t divisor;                              // Used for instancing. (optional, default = 0x00 for no instancing)
    uint32_t buffer_idx;                         // Vertex buffer to use (optional, default = 0x00)
} neko_graphics_vertex_attribute_desc_t;

/* Graphics Vertex Layout Desc */
typedef struct neko_graphics_vertex_layout_desc_t {
    neko_graphics_vertex_attribute_desc_t* attrs;  // Vertex attribute array
    size_t size;                                   // Size in bytes of vertex attribute array
} neko_graphics_vertex_layout_desc_t;

/* Graphics Pipeline Desc */
typedef struct neko_graphics_pipeline_desc_t {
    neko_graphics_blend_state_desc_t blend;      // Blend state desc for pipeline
    neko_graphics_depth_state_desc_t depth;      // Depth state desc for pipeline
    neko_graphics_raster_state_desc_t raster;    // Raster state desc for pipeline
    neko_graphics_stencil_state_desc_t stencil;  // Stencil state desc for pipeline
    neko_graphics_compute_state_desc_t compute;  // Compute state desc for pipeline
    neko_graphics_vertex_layout_desc_t layout;   // Vertex layout desc for pipeline
} neko_graphics_pipeline_desc_t;

/* Graphics Draw Desc */
typedef struct neko_graphics_draw_desc_t {
    uint32_t start;
    uint32_t count;
    uint32_t instances;
    uint32_t base_vertex;
    struct {
        uint32_t start;
        uint32_t end;
    } range;
} neko_graphics_draw_desc_t;

neko_inline neko_handle(neko_graphics_renderpass_t) __neko_renderpass_default_impl() {
    neko_handle(neko_graphics_renderpass_t) hndl = neko_default_val();
    return hndl;
}

// Convenience define for default render pass to back buffer
#define NEKO_GRAPHICS_RENDER_PASS_DEFAULT __neko_renderpass_default_impl()

typedef struct neko_graphics_info_t {
    uint32_t major_version;
    uint32_t minor_version;
    uint32_t max_texture_units;
    struct {
        b32 available;
        uint32_t max_work_group_count[3];
        uint32_t max_work_group_size[3];
        uint32_t max_work_group_invocations;
    } compute;
} neko_graphics_info_t;

// fontcache

typedef int64_t ve_font_id;
typedef int32_t ve_codepoint;
typedef int32_t ve_glyph;

typedef ve_font_id neko_font_index;

/*==========================
// Graphics Interface
==========================*/

typedef struct neko_graphics_t {
    void* user_data;            // For internal use
    neko_graphics_info_t info;  // Used for querying by user for features
    struct {

        // Create
        neko_handle(neko_graphics_texture_t) (*texture_create)(const neko_graphics_texture_desc_t* desc);
        neko_handle(neko_graphics_uniform_t) (*uniform_create)(const neko_graphics_uniform_desc_t* desc);
        neko_handle(neko_graphics_shader_t) (*shader_create)(const neko_graphics_shader_desc_t* desc);
        neko_handle(neko_graphics_vertex_buffer_t) (*vertex_buffer_create)(const neko_graphics_vertex_buffer_desc_t* desc);
        neko_handle(neko_graphics_index_buffer_t) (*index_buffer_create)(const neko_graphics_index_buffer_desc_t* desc);
        neko_handle(neko_graphics_uniform_buffer_t) (*uniform_buffer_create)(const neko_graphics_uniform_buffer_desc_t* desc);
        neko_handle(neko_graphics_storage_buffer_t) (*storage_buffer_create)(const neko_graphics_storage_buffer_desc_t* desc);
        neko_handle(neko_graphics_framebuffer_t) (*framebuffer_create)(const neko_graphics_framebuffer_desc_t* desc);
        neko_handle(neko_graphics_renderpass_t) (*renderpass_create)(const neko_graphics_renderpass_desc_t* desc);
        neko_handle(neko_graphics_pipeline_t) (*pipeline_create)(const neko_graphics_pipeline_desc_t* desc);

        // Destroy
        void (*texture_destroy)(neko_handle(neko_graphics_texture_t) hndl);
        void (*uniform_destroy)(neko_handle(neko_graphics_uniform_t) hndl);
        void (*shader_destroy)(neko_handle(neko_graphics_shader_t) hndl);
        void (*vertex_buffer_destroy)(neko_handle(neko_graphics_vertex_buffer_t) hndl);
        void (*index_buffer_destroy)(neko_handle(neko_graphics_index_buffer_t) hndl);
        void (*uniform_buffer_destroy)(neko_handle(neko_graphics_uniform_buffer_t) hndl);
        void (*storage_buffer_destroy)(neko_handle(neko_graphics_storage_buffer_t) hndl);
        void (*framebuffer_destroy)(neko_handle(neko_graphics_framebuffer_t) hndl);
        void (*renderpass_destroy)(neko_handle(neko_graphics_renderpass_t) hndl);
        void (*pipeline_destroy)(neko_handle(neko_graphics_pipeline_t) hndl);

        // Resource Updates (main thread only)
        void (*vertex_buffer_update)(neko_handle(neko_graphics_vertex_buffer_t) hndl, neko_graphics_vertex_buffer_desc_t* desc);
        void (*index_buffer_update)(neko_handle(neko_graphics_index_buffer_t) hndl, neko_graphics_index_buffer_desc_t* desc);
        void (*storage_buffer_update)(neko_handle(neko_graphics_storage_buffer_t) hndl, neko_graphics_storage_buffer_desc_t* desc);
        void (*texture_update)(neko_handle(neko_graphics_texture_t) hndl, neko_graphics_texture_desc_t* desc);
        void (*texture_read)(neko_handle(neko_graphics_texture_t) hndl, neko_graphics_texture_desc_t* desc);

        // Submission (Main Thread)
        void (*command_buffer_submit)(neko_command_buffer_t* cb);

        /*============================================================
        // Fontcache
        ============================================================*/
        void (*fontcache_create)(void);
        void (*fontcache_destroy)(void);
        void (*fontcache_draw)(void);
        neko_font_index (*fontcache_load)(const void* data, size_t data_size, f32 font_size);

        void (*fontcache_push)(const char* text, const neko_font_index font, const neko_vec2 pos);
        void (*fontcache_push_x_y)(const char* text, const neko_font_index font, const f32 x, const f32 y);

    } api;  // Interface for stable access across .dll boundaries

} neko_graphics_t;

/*==========================
// Graphics API
==========================*/

#define neko_graphics() neko_ctx()->graphics

// Graphics Interface Creation / Initialization / Shutdown / Destruction
NEKO_API_DECL neko_graphics_t* neko_graphics_create();
NEKO_API_DECL void neko_graphics_destroy(neko_graphics_t* graphics);
NEKO_API_DECL void neko_graphics_init(neko_graphics_t* graphics);
NEKO_API_DECL void neko_graphics_shutdown(neko_graphics_t* graphics);

// Graphics Info Object Query
NEKO_API_DECL neko_graphics_info_t* neko_graphics_info();

// Resource Creation
// Create
NEKO_API_DECL neko_handle(neko_graphics_texture_t) neko_graphics_texture_create(const neko_graphics_texture_desc_t* desc);
NEKO_API_DECL neko_handle(neko_graphics_uniform_t) neko_graphics_uniform_create(const neko_graphics_uniform_desc_t* desc);
NEKO_API_DECL neko_handle(neko_graphics_shader_t) neko_graphics_shader_create(const neko_graphics_shader_desc_t* desc);
NEKO_API_DECL neko_handle(neko_graphics_vertex_buffer_t) neko_graphics_vertex_buffer_create(const neko_graphics_vertex_buffer_desc_t* desc);
NEKO_API_DECL neko_handle(neko_graphics_index_buffer_t) neko_graphics_index_buffer_create(const neko_graphics_index_buffer_desc_t* desc);
NEKO_API_DECL neko_handle(neko_graphics_uniform_buffer_t) neko_graphics_uniform_buffer_create(const neko_graphics_uniform_buffer_desc_t* desc);
NEKO_API_DECL neko_handle(neko_graphics_storage_buffer_t) neko_graphics_storage_buffer_create(const neko_graphics_storage_buffer_desc_t* desc);
NEKO_API_DECL neko_handle(neko_graphics_framebuffer_t) neko_graphics_framebuffer_create(const neko_graphics_framebuffer_desc_t* desc);
NEKO_API_DECL neko_handle(neko_graphics_renderpass_t) neko_graphics_renderpass_create(const neko_graphics_renderpass_desc_t* desc);
NEKO_API_DECL neko_handle(neko_graphics_pipeline_t) neko_graphics_pipeline_create(const neko_graphics_pipeline_desc_t* desc);

// Destroy
NEKO_API_DECL void neko_graphics_texture_destroy(neko_handle(neko_graphics_texture_t) hndl);
NEKO_API_DECL void neko_graphics_uniform_destroy(neko_handle(neko_graphics_uniform_t) hndl);
NEKO_API_DECL void neko_graphics_shader_destroy(neko_handle(neko_graphics_shader_t) hndl);
NEKO_API_DECL void neko_graphics_vertex_buffer_destroy(neko_handle(neko_graphics_vertex_buffer_t) hndl);
NEKO_API_DECL void neko_graphics_index_buffer_destroy(neko_handle(neko_graphics_index_buffer_t) hndl);
NEKO_API_DECL void neko_graphics_uniform_buffer_destroy(neko_handle(neko_graphics_uniform_buffer_t) hndl);
NEKO_API_DECL void neko_graphics_storage_buffer_destroy(neko_handle(neko_graphics_storage_buffer_t) hndl);
NEKO_API_DECL void neko_graphics_framebuffer_destroy(neko_handle(neko_graphics_framebuffer_t) hndl);
NEKO_API_DECL void neko_graphics_renderpass_destroy(neko_handle(neko_graphics_renderpass_t) hndl);
NEKO_API_DECL void neko_graphics_pipeline_destroy(neko_handle(neko_graphics_pipeline_t) hndl);

// Resource Updates (main thread only)
NEKO_API_DECL void neko_graphics_vertex_buffer_update(neko_handle(neko_graphics_vertex_buffer_t) hndl, neko_graphics_vertex_buffer_desc_t* desc);
NEKO_API_DECL void neko_graphics_index_buffer_update(neko_handle(neko_graphics_index_buffer_t) hndl, neko_graphics_index_buffer_desc_t* desc);
NEKO_API_DECL void neko_graphics_storage_buffer_update(neko_handle(neko_graphics_storage_buffer_t) hndl, neko_graphics_storage_buffer_desc_t* desc);
NEKO_API_DECL void neko_graphics_texture_update(neko_handle(neko_graphics_texture_t) hndl, neko_graphics_texture_desc_t* desc);
NEKO_API_DECL void neko_graphics_texture_read(neko_handle(neko_graphics_texture_t) hndl, neko_graphics_texture_desc_t* desc);

// Resource Queries
NEKO_API_DECL void neko_graphics_pipeline_desc_query(neko_handle(neko_graphics_pipeline_t) hndl, neko_graphics_pipeline_desc_t* out);
NEKO_API_DECL void neko_graphics_texture_desc_query(neko_handle(neko_graphics_texture_t) hndl, neko_graphics_texture_desc_t* out);
NEKO_API_DECL size_t neko_graphics_uniform_size_query(neko_handle(neko_graphics_uniform_t) hndl);

// Resource In-Flight Update
NEKO_API_DECL void neko_graphics_texture_request_update(neko_command_buffer_t* cb, neko_handle(neko_graphics_texture_t) hndl, neko_graphics_texture_desc_t* desc);
NEKO_API_DECL void neko_graphics_vertex_buffer_request_update(neko_command_buffer_t* cb, neko_handle(neko_graphics_vertex_buffer_t) hndl, neko_graphics_vertex_buffer_desc_t* desc);
NEKO_API_DECL void neko_graphics_index_buffer_request_update(neko_command_buffer_t* cb, neko_handle(neko_graphics_index_buffer_t) hndl, neko_graphics_index_buffer_desc_t* desc);
NEKO_API_DECL void neko_graphics_uniform_buffer_request_update(neko_command_buffer_t* cb, neko_handle(neko_graphics_uniform_buffer_t) hndl, neko_graphics_uniform_buffer_desc_t* desc);
NEKO_API_DECL void neko_graphics_storage_buffer_request_update(neko_command_buffer_t* cb, neko_handle(neko_graphics_storage_buffer_t) hndl, neko_graphics_storage_buffer_desc_t* desc);

// Pipeline / Pass / Bind / Draw
NEKO_API_DECL void neko_graphics_renderpass_begin(neko_command_buffer_t* cb, neko_handle(neko_graphics_renderpass_t) hndl);
NEKO_API_DECL void neko_graphics_renderpass_end(neko_command_buffer_t* cb);
NEKO_API_DECL void neko_graphics_set_viewport(neko_command_buffer_t* cb, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
NEKO_API_DECL void neko_graphics_set_view_scissor(neko_command_buffer_t* cb, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
NEKO_API_DECL void neko_graphics_clear(neko_command_buffer_t* cb, neko_graphics_clear_desc_t* desc);
NEKO_API_DECL void neko_graphics_pipeline_bind(neko_command_buffer_t* cb, neko_handle(neko_graphics_pipeline_t) hndl);
NEKO_API_DECL void neko_graphics_apply_bindings(neko_command_buffer_t* cb, neko_graphics_bind_desc_t* binds);
NEKO_API_DECL void neko_graphics_draw(neko_command_buffer_t* cb, neko_graphics_draw_desc_t* desc);
NEKO_API_DECL void neko_graphics_dispatch_compute(neko_command_buffer_t* cb, uint32_t num_x_groups, uint32_t num_y_groups, uint32_t num_z_groups);

// Submission (Main Thread)
#define neko_graphics_command_buffer_submit(CB) neko_graphics()->api.command_buffer_submit((CB))

#ifndef NEKO_NO_SHORT_NAME

typedef neko_handle(neko_graphics_shader_t) neko_shader_t;
typedef neko_handle(neko_graphics_texture_t) neko_texture_t;
typedef neko_handle(neko_graphics_renderpass_t) neko_renderpass_t;
typedef neko_handle(neko_graphics_framebuffer_t) neko_framebuffer_t;
typedef neko_handle(neko_graphics_pipeline_t) neko_pipeline_t;
typedef neko_handle(neko_graphics_vertex_buffer_t) neko_vbo_t;
typedef neko_handle(neko_graphics_index_buffer_t) neko_ibo_t;
typedef neko_handle(neko_graphics_uniform_buffer_t) neko_ubo_t;
typedef neko_handle(neko_graphics_uniform_t) neko_uniform_t;

#endif

#endif