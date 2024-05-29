

#ifndef NEKO_RENDER_H
#define NEKO_RENDER_H

#include "engine/neko.h"
#include "engine/neko_math.h"

// OpenGL
#include <glad/glad.h>

/*=============================
// NEKO_GRAPHICS
=============================*/

NEKO_API_DECL const_str __neko_gl_error_string(GLenum const err);
NEKO_API_DECL const_str neko_opengl_string(GLenum e);

#define neko_check_gl_error() neko_render_print_error(__FILE__, __LINE__)
NEKO_API_DECL void neko_render_print_error(const char* file, u32 line);

// OpenGL
#define __neko_gl_state_backup()                                                \
    GLenum last_active_texture;                                                 \
    glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);             \
    glActiveTexture(GL_TEXTURE0);                                               \
    GLuint last_program;                                                        \
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&last_program);                   \
    GLuint last_texture;                                                        \
    glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&last_texture);                \
    GLuint last_array_buffer;                                                   \
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (GLint*)&last_array_buffer);         \
    GLint last_viewport[4];                                                     \
    glGetIntegerv(GL_VIEWPORT, last_viewport);                                  \
    GLint last_scissor_box[4];                                                  \
    glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);                            \
    GLenum last_blend_src_rgb;                                                  \
    glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);               \
    GLenum last_blend_dst_rgb;                                                  \
    glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);               \
    GLenum last_blend_src_alpha;                                                \
    glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);           \
    GLenum last_blend_dst_alpha;                                                \
    glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);           \
    GLenum last_blend_equation_rgb;                                             \
    glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);     \
    GLenum last_blend_equation_alpha;                                           \
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha); \
    GLboolean last_enable_blend = glIsEnabled(GL_BLEND);                        \
    GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);                \
    GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);              \
    GLboolean last_enable_stencil_test = glIsEnabled(GL_STENCIL_TEST);          \
    GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);          \
    GLboolean last_enable_mutisample = glIsEnabled(GL_MULTISAMPLE);             \
    GLboolean last_enable_framebuffer_srgb = glIsEnabled(GL_FRAMEBUFFER_SRGB)

#define __neko_gl_state_restore()                                                                            \
    glUseProgram(last_program);                                                                              \
    glBindTexture(GL_TEXTURE_2D, last_texture);                                                              \
    glActiveTexture(last_active_texture);                                                                    \
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);                                                        \
    glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);                             \
    glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha); \
    if (last_enable_blend)                                                                                   \
        glEnable(GL_BLEND);                                                                                  \
    else                                                                                                     \
        glDisable(GL_BLEND);                                                                                 \
    if (last_enable_cull_face)                                                                               \
        glEnable(GL_CULL_FACE);                                                                              \
    else                                                                                                     \
        glDisable(GL_CULL_FACE);                                                                             \
    if (last_enable_depth_test)                                                                              \
        glEnable(GL_DEPTH_TEST);                                                                             \
    else                                                                                                     \
        glDisable(GL_DEPTH_TEST);                                                                            \
    if (last_enable_stencil_test)                                                                            \
        glEnable(GL_STENCIL_TEST);                                                                           \
    else                                                                                                     \
        glDisable(GL_STENCIL_TEST);                                                                          \
    if (last_enable_scissor_test)                                                                            \
        glEnable(GL_SCISSOR_TEST);                                                                           \
    else                                                                                                     \
        glDisable(GL_SCISSOR_TEST);                                                                          \
    if (last_enable_mutisample)                                                                              \
        glEnable(GL_MULTISAMPLE);                                                                            \
    else                                                                                                     \
        glDisable(GL_MULTISAMPLE);                                                                           \
    if (last_enable_framebuffer_srgb)                                                                        \
        glEnable(GL_FRAMEBUFFER_SRGB);                                                                       \
    else                                                                                                     \
        glDisable(GL_FRAMEBUFFER_SRGB);                                                                      \
    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);    \
    glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3])

// Graphics Pipeline

// Main graphics resources:
// Shader description: vertex, fragment, compute, geometry, tesselation
// Texture Description: texture, depth, render target
// Buffer Description: vertex, index, uniform, frame, pixel
// Pipeline Description: vert-layout, shader, bindables, render states
// Pass Description: render pass, action on render targets (clear, set viewport, etc.)

/* Useful macro for forcing enum decl to be u32 type with default = 0x00 for quick init */
#define neko_enum_decl(NAME, ...) typedef enum NAME { _neko_##NAME##_default = 0x0, __VA_ARGS__, _neko_##NAME##_count, _neko_##NAME##_force_u32 = 0x7fffffff } NAME;

#define neko_enum_count(NAME) _neko_##NAME##_count

/* Shader Stage Type */
neko_enum_decl(neko_render_shader_stage_type, NEKO_RENDER_SHADER_STAGE_VERTEX, NEKO_RENDER_SHADER_STAGE_FRAGMENT, NEKO_RENDER_SHADER_STAGE_COMPUTE);

/* Winding Order Type */
neko_enum_decl(neko_render_winding_order_type, NEKO_RENDER_WINDING_ORDER_CW, NEKO_RENDER_WINDING_ORDER_CCW);

/* Face Culling Type */
neko_enum_decl(neko_render_face_culling_type, NEKO_RENDER_FACE_CULLING_FRONT, NEKO_RENDER_FACE_CULLING_BACK, NEKO_RENDER_FACE_CULLING_FRONT_AND_BACK);

/* Blend Equation Type */
neko_enum_decl(neko_render_blend_equation_type, NEKO_RENDER_BLEND_EQUATION_ADD, NEKO_RENDER_BLEND_EQUATION_SUBTRACT, NEKO_RENDER_BLEND_EQUATION_REVERSE_SUBTRACT, NEKO_RENDER_BLEND_EQUATION_MIN,
               NEKO_RENDER_BLEND_EQUATION_MAX);

/* Blend Mode Type */
neko_enum_decl(neko_render_blend_mode_type, NEKO_RENDER_BLEND_MODE_ZERO, NEKO_RENDER_BLEND_MODE_ONE, NEKO_RENDER_BLEND_MODE_SRC_COLOR, NEKO_RENDER_BLEND_MODE_ONE_MINUS_SRC_COLOR,
               NEKO_RENDER_BLEND_MODE_DST_COLOR, NEKO_RENDER_BLEND_MODE_ONE_MINUS_DST_COLOR, NEKO_RENDER_BLEND_MODE_SRC_ALPHA, NEKO_RENDER_BLEND_MODE_ONE_MINUS_SRC_ALPHA,
               NEKO_RENDER_BLEND_MODE_DST_ALPHA, NEKO_RENDER_BLEND_MODE_ONE_MINUS_DST_ALPHA, NEKO_RENDER_BLEND_MODE_CONSTANT_COLOR, NEKO_RENDER_BLEND_MODE_ONE_MINUS_CONSTANT_COLOR,
               NEKO_RENDER_BLEND_MODE_CONSTANT_ALPHA, NEKO_RENDER_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA);

/* Shader Language Type */
neko_enum_decl(neko_render_shader_language_type, NEKO_RENDER_SHADER_LANGUAGE_GLSL);

/* Push Constant Type */
// Really don't want to handle "auto-merging" of data types

/* Uniform Type */
neko_enum_decl(neko_render_uniform_type, NEKO_RENDER_UNIFORM_FLOAT, NEKO_RENDER_UNIFORM_INT, NEKO_RENDER_UNIFORM_VEC2, NEKO_RENDER_UNIFORM_VEC3, NEKO_RENDER_UNIFORM_VEC4, NEKO_RENDER_UNIFORM_MAT4,
               NEKO_RENDER_UNIFORM_SAMPLER2D, NEKO_RENDER_UNIFORM_USAMPLER2D, NEKO_RENDER_UNIFORM_SAMPLERCUBE, NEKO_RENDER_UNIFORM_IMAGE2D_RGBA32F, NEKO_RENDER_UNIFORM_BLOCK);

/* Uniform Block Usage Type */
neko_enum_decl(neko_render_uniform_block_usage_type,
               NEKO_RENDER_UNIFORM_BLOCK_USAGE_STATIC,  // Default of 0x00 is static
               NEKO_RENDER_UNIFORM_BLOCK_USAGE_PUSH_CONSTANT);

/* Sampler Type */
neko_enum_decl(neko_render_sampler_type, NEKO_RENDER_SAMPLER_2D);

/* Primitive Type */
neko_enum_decl(neko_render_primitive_type, NEKO_RENDER_PRIMITIVE_LINES, NEKO_RENDER_PRIMITIVE_TRIANGLES, NEKO_RENDER_PRIMITIVE_QUADS);

/* Vertex Atribute Type */
neko_enum_decl(neko_render_vertex_attribute_type, NEKO_RENDER_VERTEX_ATTRIBUTE_FLOAT4, NEKO_RENDER_VERTEX_ATTRIBUTE_FLOAT3, NEKO_RENDER_VERTEX_ATTRIBUTE_FLOAT2, NEKO_RENDER_VERTEX_ATTRIBUTE_FLOAT,
               NEKO_RENDER_VERTEX_ATTRIBUTE_UINT4, NEKO_RENDER_VERTEX_ATTRIBUTE_UINT3, NEKO_RENDER_VERTEX_ATTRIBUTE_UINT2, NEKO_RENDER_VERTEX_ATTRIBUTE_UINT, NEKO_RENDER_VERTEX_ATTRIBUTE_BYTE4,
               NEKO_RENDER_VERTEX_ATTRIBUTE_BYTE3, NEKO_RENDER_VERTEX_ATTRIBUTE_BYTE2, NEKO_RENDER_VERTEX_ATTRIBUTE_BYTE);

/* Buffer Type */
neko_enum_decl(neko_render_buffer_type, NEKO_RENDER_BUFFER_VERTEX, NEKO_RENDER_BUFFER_INDEX, NEKO_RENDER_BUFFER_FRAME, NEKO_RENDER_BUFFER_UNIFORM, NEKO_RENDER_BUFFER_UNIFORM_CONSTANT,
               NEKO_RENDER_BUFFER_SHADER_STORAGE, NEKO_RENDER_BUFFER_SAMPLER);

/* Buffer Usage Type */
neko_enum_decl(neko_render_buffer_usage_type, NEKO_RENDER_BUFFER_USAGE_STATIC, NEKO_RENDER_BUFFER_USAGE_STREAM, NEKO_RENDER_BUFFER_USAGE_DYNAMIC);

/* Buffer Update Type */
neko_enum_decl(neko_render_buffer_update_type, NEKO_RENDER_BUFFER_UPDATE_RECREATE, NEKO_RENDER_BUFFER_UPDATE_SUBDATA);

neko_enum_decl(neko_render_access_type, NEKO_RENDER_ACCESS_READ_ONLY, NEKO_RENDER_ACCESS_WRITE_ONLY, NEKO_RENDER_ACCESS_READ_WRITE);

//=== Texture ===//
typedef enum { NEKO_RENDER_TEXTURE_2D = 0x00, NEKO_RENDER_TEXTURE_CUBEMAP } neko_render_texture_type;

typedef enum {
    NEKO_RENDER_TEXTURE_CUBEMAP_POSITIVE_X = 0x00,
    NEKO_RENDER_TEXTURE_CUBEMAP_NEGATIVE_X,
    NEKO_RENDER_TEXTURE_CUBEMAP_POSITIVE_Y,
    NEKO_RENDER_TEXTURE_CUBEMAP_NEGATIVE_Y,
    NEKO_RENDER_TEXTURE_CUBEMAP_POSITIVE_Z,
    NEKO_RENDER_TEXTURE_CUBEMAP_NEGATIVE_Z
} neko_render_cubemap_face_type;

neko_enum_decl(neko_render_texture_format_type, NEKO_RENDER_TEXTURE_FORMAT_RGBA8, NEKO_RENDER_TEXTURE_FORMAT_RGB8, NEKO_RENDER_TEXTURE_FORMAT_RG8, NEKO_RENDER_TEXTURE_FORMAT_R32,
               NEKO_RENDER_TEXTURE_FORMAT_R32F, NEKO_RENDER_TEXTURE_FORMAT_RGBA16F, NEKO_RENDER_TEXTURE_FORMAT_RGBA32F, NEKO_RENDER_TEXTURE_FORMAT_A8, NEKO_RENDER_TEXTURE_FORMAT_R8,
               NEKO_RENDER_TEXTURE_FORMAT_DEPTH8, NEKO_RENDER_TEXTURE_FORMAT_DEPTH16, NEKO_RENDER_TEXTURE_FORMAT_DEPTH24, NEKO_RENDER_TEXTURE_FORMAT_DEPTH32F,
               NEKO_RENDER_TEXTURE_FORMAT_DEPTH24_STENCIL8, NEKO_RENDER_TEXTURE_FORMAT_DEPTH32F_STENCIL8, NEKO_RENDER_TEXTURE_FORMAT_STENCIL8);

neko_enum_decl(neko_render_texture_wrapping_type, NEKO_RENDER_TEXTURE_WRAP_REPEAT, NEKO_RENDER_TEXTURE_WRAP_MIRRORED_REPEAT, NEKO_RENDER_TEXTURE_WRAP_CLAMP_TO_EDGE,
               NEKO_RENDER_TEXTURE_WRAP_CLAMP_TO_BORDER);

neko_enum_decl(neko_render_texture_filtering_type, NEKO_RENDER_TEXTURE_FILTER_NEAREST, NEKO_RENDER_TEXTURE_FILTER_LINEAR);

//=== Clear ===//
neko_enum_decl(neko_render_clear_flag, NEKO_RENDER_CLEAR_COLOR = 0x01, NEKO_RENDER_CLEAR_DEPTH = 0x02, NEKO_RENDER_CLEAR_STENCIL = 0x04, NEKO_RENDER_CLEAR_NONE = 0x08);

#define NEKO_RENDER_CLEAR_ALL NEKO_RENDER_CLEAR_COLOR | NEKO_RENDER_CLEAR_DEPTH | NEKO_RENDER_CLEAR_STENCIL

//=== Bind Type ===//
neko_enum_decl(neko_render_bind_type, NEKO_RENDER_BIND_VERTEX_BUFFER, NEKO_RENDER_BIND_INDEX_BUFFER, NEKO_RENDER_BIND_UNIFORM_BUFFER, NEKO_RENDER_BIND_STORAGE_BUFFER, NEKO_RENDER_BIND_IMAGE_BUFFER,
               NEKO_RENDER_BIND_UNIFORM);

/* Depth Function Type */
neko_enum_decl(neko_render_depth_func_type,  // Default value of 0x00 means depth is disabled
               NEKO_RENDER_DEPTH_FUNC_NEVER, NEKO_RENDER_DEPTH_FUNC_LESS, NEKO_RENDER_DEPTH_FUNC_EQUAL, NEKO_RENDER_DEPTH_FUNC_LEQUAL, NEKO_RENDER_DEPTH_FUNC_GREATER, NEKO_RENDER_DEPTH_FUNC_NOTEQUAL,
               NEKO_RENDER_DEPTH_FUNC_GEQUAL, NEKO_RENDER_DEPTH_FUNC_ALWAYS);

neko_enum_decl(neko_render_depth_mask_type,  // Default value 0x00 means depth writing enabled
               NEKO_RENDER_DEPTH_MASK_ENABLED, NEKO_RENDER_DEPTH_MASK_DISABLED);

/* Stencil Function Type */
neko_enum_decl(neko_render_stencil_func_type,
               NEKO_RENDER_STENCIL_FUNC_NEVER,  // Default value of 0x00 means stencil is disabled
               NEKO_RENDER_STENCIL_FUNC_LESS, NEKO_RENDER_STENCIL_FUNC_EQUAL, NEKO_RENDER_STENCIL_FUNC_LEQUAL, NEKO_RENDER_STENCIL_FUNC_GREATER, NEKO_RENDER_STENCIL_FUNC_NOTEQUAL,
               NEKO_RENDER_STENCIL_FUNC_GEQUAL, NEKO_RENDER_STENCIL_FUNC_ALWAYS);

/* Stencil Op Type */
neko_enum_decl(neko_render_stencil_op_type,  // Default value of 0x00 means keep is used
               NEKO_RENDER_STENCIL_OP_KEEP, NEKO_RENDER_STENCIL_OP_ZERO, NEKO_RENDER_STENCIL_OP_REPLACE, NEKO_RENDER_STENCIL_OP_INCR, NEKO_RENDER_STENCIL_OP_INCR_WRAP, NEKO_RENDER_STENCIL_OP_DECR,
               NEKO_RENDER_STENCIL_OP_DECR_WRAP, NEKO_RENDER_STENCIL_OP_INVERT);

/* Internal Graphics Resource Handles */
neko_handle_decl(neko_render_shader_t);
neko_handle_decl(neko_render_texture_t);
neko_handle_decl(neko_render_vertex_buffer_t);
neko_handle_decl(neko_render_index_buffer_t);
neko_handle_decl(neko_render_uniform_buffer_t);
neko_handle_decl(neko_render_storage_buffer_t);
neko_handle_decl(neko_render_framebuffer_t);
neko_handle_decl(neko_render_uniform_t);
neko_handle_decl(neko_render_renderpass_t);
neko_handle_decl(neko_render_pipeline_t);

/* Graphics Shader Source Desc */
typedef struct neko_render_shader_source_desc_t {
    neko_render_shader_stage_type type;  // Shader stage type (vertex, fragment, tesselation, geometry, compute)
    const char* source;                  // Source for shader
} neko_render_shader_source_desc_t;

/* Graphics Shader Desc */
typedef struct neko_render_shader_desc_t {
    neko_render_shader_source_desc_t* sources;  // Array of shader source descriptions
    size_t size;                                // Size in bytes of shader source desc array
    char name[64];                              // Optional (for logging and debugging mainly)
} neko_render_shader_desc_t;

#define NEKO_RENDER_TEXTURE_DATA_MAX 6

/* Graphics Texture Desc */
typedef struct neko_render_texture_desc_t {
    neko_render_texture_type type;
    u32 width;                                      // Width of texture in texels
    u32 height;                                     // Height of texture in texels
    u32 depth;                                      // Depth of texture
    void* data[NEKO_RENDER_TEXTURE_DATA_MAX];       // Texture data to upload (can be null)
    neko_render_texture_format_type format;         // Format of texture data (rgba32, rgba8, rgba32f, r8, depth32f, etc...)
    neko_render_texture_wrapping_type wrap_s;       // Wrapping type for s axis of texture
    neko_render_texture_wrapping_type wrap_t;       // Wrapping type for t axis of texture
    neko_render_texture_wrapping_type wrap_r;       // Wrapping type for r axis of texture
    neko_render_texture_filtering_type min_filter;  // Minification filter for texture
    neko_render_texture_filtering_type mag_filter;  // Magnification filter for texture
    neko_render_texture_filtering_type mip_filter;  // Mip filter for texture
    neko_vec2 offset;                               // Offset for updates
    u32 num_mips;                                   // Number of mips to generate (default 0 is disable mip generation)
    struct {
        u32 x;        // X offset in texels to start read
        u32 y;        // Y offset in texels to start read
        u32 width;    // Width in texels for texture
        u32 height;   // Height in texels for texture
        size_t size;  // Size in bytes for data to be read
    } read;
    u16 flip_y;  // Whether or not y is flipped
} neko_render_texture_desc_t;

/* Graphics Uniform Layout Desc */
typedef struct neko_render_uniform_layout_desc_t {
    neko_render_uniform_type type;  // Type of field
    char fname[64];                 // The name of field (required for implicit APIs, like OpenGL/ES)
    u32 count;                      // Count variable (used for arrays such as glUniformXXXv)
} neko_render_uniform_layout_desc_t;

/* Graphics Uniform Desc */
typedef struct neko_render_uniform_desc_t {
    neko_render_shader_stage_type stage;
    char name[64];                              // The name of uniform (required for OpenGL/ES, WebGL)
    neko_render_uniform_layout_desc_t* layout;  // Layout array for uniform data
    size_t layout_size;                         // Size of uniform data in bytes
} neko_render_uniform_desc_t;

typedef struct neko_render_buffer_update_desc_t {
    neko_render_buffer_update_type type;
    size_t offset;
} neko_render_buffer_update_desc_t;

/* Graphics Buffer Desc General */
typedef struct neko_render_buffer_base_desc_t {
    void* data;
    size_t size;
    neko_render_buffer_usage_type usage;
} neko_render_buffer_base_desc_t;

typedef struct neko_render_vertex_buffer_desc_t {
    void* data;
    size_t size;
    neko_render_buffer_usage_type usage;
    neko_render_buffer_update_desc_t update;
} neko_render_vertex_buffer_desc_t;

typedef neko_render_vertex_buffer_desc_t neko_render_index_buffer_desc_t;

typedef struct neko_render_uniform_buffer_desc_t {
    void* data;
    size_t size;
    neko_render_buffer_usage_type usage;
    const char* name;
    neko_render_shader_stage_type stage;
    neko_render_buffer_update_desc_t update;
} neko_render_uniform_buffer_desc_t;

typedef struct neko_render_storage_buffer_desc_t {
    void* data;
    size_t size;
    char name[64];
    neko_render_buffer_usage_type usage;
    neko_render_access_type access;
    neko_render_buffer_update_desc_t update;
} neko_render_storage_buffer_desc_t;

typedef struct neko_render_framebuffer_desc_t {
    void* data;
} neko_render_framebuffer_desc_t;

/* Graphics Clear Action */
typedef struct neko_render_clear_action_t {
    neko_render_clear_flag flag;  // Flag to be set (clear color, clear depth, clear stencil, clear all)
    union {
        float color[4];   // Clear color value
        float depth;      // Clear depth value
        int32_t stencil;  // Clear stencil value
    };
} neko_render_clear_action_t;

/* Graphics Clear Desc */
typedef struct neko_render_clear_desc_t {
    neko_render_clear_action_t* actions;  // Clear action
    size_t size;                          // Size
} neko_render_clear_desc_t;

/* Graphics Render Pass Desc */
typedef struct neko_render_renderpass_desc_t {
    neko_handle(neko_render_framebuffer_t) fbo;  // Default is set to invalid for backbuffer
    neko_handle(neko_render_texture_t) * color;  // Array of color attachments to be bound (useful for MRT, if supported)
    size_t color_size;                           // Size of color attachment array
    neko_handle(neko_render_texture_t) depth;    // Depth attachment to be bound
    neko_handle(neko_render_texture_t) stencil;  // Depth attachment to be bound
} neko_render_renderpass_desc_t;

/*
    // If you want to write to a color attachment, you have to have a frame buffer attached that isn't the backbuffer
*/

typedef enum neko_render_vertex_data_type { NEKO_RENDER_VERTEX_DATA_INTERLEAVED = 0x00, NEKO_RENDER_VERTEX_DATA_NONINTERLEAVED } neko_render_vertex_data_type;

typedef struct neko_render_bind_vertex_buffer_desc_t {
    neko_handle(neko_render_vertex_buffer_t) buffer;
    size_t offset;
    neko_render_vertex_data_type data_type;
} neko_render_bind_vertex_buffer_desc_t;

typedef struct neko_render_bind_index_buffer_desc_t {
    neko_handle(neko_render_index_buffer_t) buffer;
} neko_render_bind_index_buffer_desc_t;

typedef struct neko_render_bind_image_buffer_desc_t {
    neko_handle(neko_render_texture_t) tex;
    u32 binding;
    neko_render_access_type access;
} neko_render_bind_image_buffer_desc_t;

typedef struct neko_render_bind_uniform_buffer_desc_t {
    neko_handle(neko_render_uniform_buffer_t) buffer;
    u32 binding;
    struct {
        size_t offset;  // Specify an offset for ranged binds.
        size_t size;    // Specify size for ranged binds.
    } range;
} neko_render_bind_uniform_buffer_desc_t;

typedef struct neko_render_bind_storage_buffer_desc_t {
    neko_handle(neko_render_storage_buffer_t) buffer;
    u32 binding;
} neko_render_bind_storage_buffer_desc_t;

typedef struct neko_render_bind_uniform_desc_t {
    neko_handle(neko_render_uniform_t) uniform;
    void* data;
    u32 binding;  // Base binding for samplers?
} neko_render_bind_uniform_desc_t;

/* Graphics Binding Desc */
typedef struct neko_render_bind_desc_t {
    struct {
        neko_render_bind_vertex_buffer_desc_t* desc;  // Array of vertex buffer declarations (NULL by default)
        size_t size;                                  // Size of array in bytes (optional if only one)
    } vertex_buffers;

    struct {
        neko_render_bind_index_buffer_desc_t* desc;  // Array of index buffer declarations (NULL by default)
        size_t size;                                 // Size of array in bytes (optional if only one)
    } index_buffers;

    struct {
        neko_render_bind_uniform_buffer_desc_t* desc;  // Array of uniform buffer declarations (NULL by default)
        size_t size;                                   // Size of array in bytes (optional if only one)
    } uniform_buffers;

    struct {
        neko_render_bind_uniform_desc_t* desc;  // Array of uniform declarations (NULL by default)
        size_t size;                            // Size of array in bytes (optional if one)
    } uniforms;

    struct {
        neko_render_bind_image_buffer_desc_t* desc;
        size_t size;
    } image_buffers;

    struct {
        neko_render_bind_storage_buffer_desc_t* desc;
        size_t size;
    } storage_buffers;

} neko_render_bind_desc_t;

/* Graphics Blend State Desc */
typedef struct neko_render_blend_state_desc_t {
    neko_render_blend_equation_type func;  // Equation function to use for blend ops
    neko_render_blend_mode_type src;       // Source blend mode
    neko_render_blend_mode_type dst;       // Destination blend mode
} neko_render_blend_state_desc_t;

/* Graphics Depth State Desc */
typedef struct neko_render_depth_state_desc_t {
    neko_render_depth_func_type func;  // Function to set for depth test
    neko_render_depth_mask_type mask;  // Whether or not writing is enabled/disabled
} neko_render_depth_state_desc_t;

/* Graphics Stencil State Desc */
typedef struct neko_render_stencil_state_desc_t {
    neko_render_stencil_func_type func;  // Function to set for stencil test
    u32 ref;                             // Specifies reference val for stencil test
    u32 comp_mask;                       // Specifies mask that is ANDed with both ref val and stored stencil val
    u32 write_mask;                      // Specifies mask that is ANDed with both ref val and stored stencil val
    neko_render_stencil_op_type sfail;   // Action to take when stencil test fails
    neko_render_stencil_op_type dpfail;  // Action to take when stencil test passes but depth test fails
    neko_render_stencil_op_type dppass;  // Action to take when both stencil test passes and either depth passes or is not enabled
} neko_render_stencil_state_desc_t;

/* Graphics Raster State Desc */
typedef struct neko_render_raster_state_desc_t {
    neko_render_face_culling_type face_culling;    // Face culling mode to be used (front, back, front and back)
    neko_render_winding_order_type winding_order;  // Winding order mode to be used (ccw, cw)
    neko_render_primitive_type primitive;          // Primitive type for drawing (lines, quads, triangles, triangle strip)
    neko_handle(neko_render_shader_t) shader;      // Shader to bind and use (might be in bindables later on, not sure)
    size_t index_buffer_element_size;              // Element size of index buffer (used for parsing internal data)
} neko_render_raster_state_desc_t;

/* Graphics Compute State Desc */
typedef struct neko_render_compute_state_desc_t {
    neko_handle(neko_render_shader_t) shader;  // Compute shader to bind
} neko_render_compute_state_desc_t;

/* Graphics Vertex Attribute Desc */
typedef struct neko_render_vertex_attribute_desc_t {
    char name[64];                             // Attribute name (required for lower versions of OpenGL and ES)
    neko_render_vertex_attribute_type format;  // Format for vertex attribute
    size_t stride;                             // Total stride of vertex layout (optional, calculated by default)
    size_t offset;                             // Offset of this vertex from base pointer of data (optional, calaculated by default)
    size_t divisor;                            // Used for instancing. (optional, default = 0x00 for no instancing)
    u32 buffer_idx;                            // Vertex buffer to use (optional, default = 0x00)
} neko_render_vertex_attribute_desc_t;

/* Graphics Vertex Layout Desc */
typedef struct neko_render_vertex_layout_desc_t {
    neko_render_vertex_attribute_desc_t* attrs;  // Vertex attribute array
    size_t size;                                 // Size in bytes of vertex attribute array
} neko_render_vertex_layout_desc_t;

/* Graphics Pipeline Desc */
typedef struct neko_render_pipeline_desc_t {
    neko_render_blend_state_desc_t blend;      // Blend state desc for pipeline
    neko_render_depth_state_desc_t depth;      // Depth state desc for pipeline
    neko_render_raster_state_desc_t raster;    // Raster state desc for pipeline
    neko_render_stencil_state_desc_t stencil;  // Stencil state desc for pipeline
    neko_render_compute_state_desc_t compute;  // Compute state desc for pipeline
    neko_render_vertex_layout_desc_t layout;   // Vertex layout desc for pipeline
} neko_render_pipeline_desc_t;

/* Graphics Draw Desc */
typedef struct neko_render_draw_desc_t {
    u32 start;
    u32 count;
    u32 instances;
    u32 base_vertex;
    struct {
        u32 start;
        u32 end;
    } range;
} neko_render_draw_desc_t;

NEKO_INLINE neko_handle(neko_render_renderpass_t) __neko_renderpass_default_impl() {
    neko_handle(neko_render_renderpass_t) hndl = NEKO_DEFAULT_VAL();
    return hndl;
}

// Convenience define for default render pass to back buffer
#define NEKO_RENDER_RENDER_PASS_DEFAULT __neko_renderpass_default_impl()

typedef struct neko_render_info_t {
    const_str version;
    const_str vendor;
    u32 major_version;
    u32 minor_version;
    u32 max_texture_units;
    struct {
        b32 available;
        u32 max_work_group_count[3];
        u32 max_work_group_size[3];
        u32 max_work_group_invocations;
    } compute;
} neko_render_info_t;

/*==========================
// Graphics Interface
==========================*/

typedef struct neko_render_t {
    void* user_data;          // For internal use
    neko_render_info_t info;  // Used for querying by user for features
    struct {

        // Create
        neko_handle(neko_render_texture_t) (*texture_create)(const neko_render_texture_desc_t desc);
        neko_handle(neko_render_uniform_t) (*uniform_create)(const neko_render_uniform_desc_t desc);
        neko_handle(neko_render_shader_t) (*shader_create)(const neko_render_shader_desc_t desc);
        neko_handle(neko_render_vertex_buffer_t) (*vertex_buffer_create)(const neko_render_vertex_buffer_desc_t desc);
        neko_handle(neko_render_index_buffer_t) (*index_buffer_create)(const neko_render_index_buffer_desc_t desc);
        neko_handle(neko_render_uniform_buffer_t) (*uniform_buffer_create)(const neko_render_uniform_buffer_desc_t desc);
        neko_handle(neko_render_storage_buffer_t) (*storage_buffer_create)(const neko_render_storage_buffer_desc_t desc);
        neko_handle(neko_render_framebuffer_t) (*framebuffer_create)(const neko_render_framebuffer_desc_t desc);
        neko_handle(neko_render_renderpass_t) (*renderpass_create)(const neko_render_renderpass_desc_t desc);
        neko_handle(neko_render_pipeline_t) (*pipeline_create)(const neko_render_pipeline_desc_t desc);

        // Destroy
        void (*texture_destroy)(neko_handle(neko_render_texture_t) hndl);
        void (*uniform_destroy)(neko_handle(neko_render_uniform_t) hndl);
        void (*shader_destroy)(neko_handle(neko_render_shader_t) hndl);
        void (*vertex_buffer_destroy)(neko_handle(neko_render_vertex_buffer_t) hndl);
        void (*index_buffer_destroy)(neko_handle(neko_render_index_buffer_t) hndl);
        void (*uniform_buffer_destroy)(neko_handle(neko_render_uniform_buffer_t) hndl);
        void (*storage_buffer_destroy)(neko_handle(neko_render_storage_buffer_t) hndl);
        void (*framebuffer_destroy)(neko_handle(neko_render_framebuffer_t) hndl);
        void (*renderpass_destroy)(neko_handle(neko_render_renderpass_t) hndl);
        void (*pipeline_destroy)(neko_handle(neko_render_pipeline_t) hndl);

        // Resource Updates (main thread only)
        void (*vertex_buffer_update)(neko_handle(neko_render_vertex_buffer_t) hndl, neko_render_vertex_buffer_desc_t* desc);
        void (*index_buffer_update)(neko_handle(neko_render_index_buffer_t) hndl, neko_render_index_buffer_desc_t* desc);
        void (*storage_buffer_update)(neko_handle(neko_render_storage_buffer_t) hndl, neko_render_storage_buffer_desc_t* desc);
        void (*texture_update)(neko_handle(neko_render_texture_t) hndl, neko_render_texture_desc_t* desc);
        void (*texture_read)(neko_handle(neko_render_texture_t) hndl, neko_render_texture_desc_t* desc);

        // Submission (Main Thread)
        void (*command_buffer_submit)(neko_command_buffer_t* cb);

    } api;
} neko_render_t;

/*==========================
// Graphics API
==========================*/

#define neko_render() neko_ctx()->render

// Graphics Interface Creation / Initialization / Shutdown / Destruction
NEKO_API_DECL neko_render_t* neko_render_create();
NEKO_API_DECL void neko_render_destroy(neko_render_t* render);
NEKO_API_DECL void neko_render_init(neko_render_t* render);
NEKO_API_DECL void neko_render_shutdown(neko_render_t* render);

// Graphics Info Object Query
NEKO_API_DECL neko_render_info_t* neko_render_info();

// recommended to leave this on as long as possible (perhaps until release)
#define NEKO_RENDER_BATCH_DEBUG_CHECKS 1

enum {
    NEKO_RENDER_BATCH_FLOAT,
    NEKO_RENDER_BATCH_INT,
    NEKO_RENDER_BATCH_BOOL,
    NEKO_RENDER_BATCH_SAMPLER,
    NEKO_RENDER_BATCH_UNKNOWN,
};

typedef struct neko_render_batch_vertex_attribute_t {
    const char* name;
    u64 hash;
    u32 size;
    u32 type;
    u32 offset;
    u32 location;
} neko_render_batch_vertex_attribute_t;

#define NEKO_RENDER_BATCH_ATTRIBUTE_MAX_COUNT 16
typedef struct neko_render_batch_vertex_data_t {
    u32 buffer_size;
    u32 vertex_stride;
    u32 primitive;
    u32 usage;

    u32 attribute_count;
    neko_render_batch_vertex_attribute_t attributes[NEKO_RENDER_BATCH_ATTRIBUTE_MAX_COUNT];
} neko_render_batch_vertex_data_t;

// 根据需要调整此项以创建绘制调用顺序
// see: http://realtimecollisiondetection.net/blog/?p=86
typedef struct neko_render_batch_render_internal_state_t {
    union {
        struct {
            int fullscreen : 2;
            int hud : 5;
            int depth : 25;
            int translucency : 32;
        } bits;

        u64 key;
    } u;
} neko_render_batch_render_internal_state_t;

struct neko_render_batch_shader_t;
typedef struct neko_render_batch_shader_t neko_render_batch_shader_t;

typedef struct neko_render_batch_renderable_t {
    neko_render_batch_vertex_data_t data;
    neko_render_batch_shader_t* program;
    neko_render_batch_render_internal_state_t state;
    u32 attribute_count;

    u32 index0;
    u32 index1;
    u32 buffer_number;
    u32 need_new_sync;
    u32 buffer_count;
    u32 buffers[3];
    GLsync fences[3];
} neko_render_batch_renderable_t;

#define NEKO_RENDER_BATCH_UNIFORM_NAME_LENGTH 64
#define NEKO_RENDER_BATCH_UNIFORM_MAX_COUNT 16

typedef struct neko_render_batch_uniform_t {
    char name[NEKO_RENDER_BATCH_UNIFORM_NAME_LENGTH];
    u32 id;
    u64 hash;
    u32 size;
    u32 type;
    u32 location;
} neko_render_batch_uniform_t;

struct neko_render_batch_shader_t {
    u32 program;
    u32 uniform_count;
    neko_render_batch_uniform_t uniforms[NEKO_RENDER_BATCH_UNIFORM_MAX_COUNT];
};

typedef struct neko_render_batch_framebuffer_t {
    u32 fb_id;
    u32 tex_id;
    u32 rb_id;
    u32 quad_id;
    neko_render_batch_shader_t* shader;
    int w, h;
} neko_render_batch_framebuffer_t;

typedef struct {
    u32 vert_count;
    void* verts;
    neko_render_batch_renderable_t* r;
    u32 texture_count;
    u32 textures[8];
} neko_render_batch_draw_call_t;

struct neko_render_batch_context_t;
typedef struct neko_render_batch_context_t neko_render_batch_context_t;
typedef struct neko_render_batch_context_t* neko_render_batch_context_ptr;
typedef struct neko_render_batch_framebuffer_t* neko_render_batch_framebuffer_ptr;

NEKO_API_DECL neko_render_batch_context_t* neko_render_batch_make_ctx(u32 max_draw_calls);
NEKO_API_DECL void neko_render_batch_free(void* ctx);

NEKO_API_DECL void neko_render_batch_make_frame_buffer(neko_render_batch_framebuffer_t* fb, neko_render_batch_shader_t* shader, int w, int h, int use_depth_test);
NEKO_API_DECL void neko_render_batch_free_frame_buffer(neko_render_batch_framebuffer_t* fb);

NEKO_API_DECL void neko_render_batch_make_vertex_data(neko_render_batch_vertex_data_t* vd, u32 buffer_size, u32 primitive, u32 vertex_stride, u32 usage);
NEKO_API_DECL void neko_render_batch_add_attribute(neko_render_batch_vertex_data_t* vd, const char* name, u32 size, u32 type, u32 offset);
NEKO_API_DECL void neko_render_batch_make_renderable(neko_render_batch_renderable_t* r, neko_render_batch_vertex_data_t* vd);

// Must be called after gl_make_renderable
NEKO_API_DECL void neko_render_batch_set_shader(neko_render_batch_renderable_t* r, neko_render_batch_shader_t* s);
NEKO_API_DECL void neko_render_batch_load_shader(neko_render_batch_shader_t* s, const char* vertex, const char* pixel);
NEKO_API_DECL void neko_render_batch_free_shader(neko_render_batch_shader_t* s);

NEKO_API_DECL void neko_render_batch_set_active_shader(neko_render_batch_shader_t* s);
NEKO_API_DECL void neko_render_batch_deactivate_shader();
NEKO_API_DECL void neko_render_batch_send_f32(neko_render_batch_shader_t* s, const char* uniform_name, u32 size, float* floats, u32 count);
NEKO_API_DECL void neko_render_batch_send_matrix(neko_render_batch_shader_t* s, const char* uniform_name, float* floats);
NEKO_API_DECL void neko_render_batch_send_texture(neko_render_batch_shader_t* s, const char* uniform_name, u32 index);

NEKO_API_DECL void neko_render_batch_push_draw_call(void* ctx, neko_render_batch_draw_call_t call);

NEKO_API_DECL void neko_render_batch_flush(void* ctx, neko_render_batch_framebuffer_t* fb, int w, int h);
NEKO_API_DECL int neko_render_batch_draw_call_count(void* ctx);

// 4x4 matrix helper functions
NEKO_API_DECL void neko_render_batch_ortho_2d(float w, float h, float x, float y, float* m);
NEKO_API_DECL void neko_render_batch_perspective(float* m, float y_fov_radians, float aspect, float n, float f);
NEKO_API_DECL void neko_render_batch_mul(float* a, float* b, float* out);  // perform a * b, stores result in out
NEKO_API_DECL void neko_render_batch_identity(float* m);
NEKO_API_DECL void neko_render_batch_copy(float* dst, float* src);

// Resource Creation
// Create
NEKO_API_DECL neko_handle(neko_render_texture_t) neko_render_texture_create(const neko_render_texture_desc_t desc);
NEKO_API_DECL neko_handle(neko_render_uniform_t) neko_render_uniform_create(const neko_render_uniform_desc_t desc);
NEKO_API_DECL neko_handle(neko_render_shader_t) neko_render_shader_create(const neko_render_shader_desc_t desc);
NEKO_API_DECL neko_handle(neko_render_vertex_buffer_t) neko_render_vertex_buffer_create(const neko_render_vertex_buffer_desc_t desc);
NEKO_API_DECL neko_handle(neko_render_index_buffer_t) neko_render_index_buffer_create(const neko_render_index_buffer_desc_t desc);
NEKO_API_DECL neko_handle(neko_render_uniform_buffer_t) neko_render_uniform_buffer_create(const neko_render_uniform_buffer_desc_t desc);
NEKO_API_DECL neko_handle(neko_render_storage_buffer_t) neko_render_storage_buffer_create(const neko_render_storage_buffer_desc_t desc);
NEKO_API_DECL neko_handle(neko_render_framebuffer_t) neko_render_framebuffer_create(const neko_render_framebuffer_desc_t desc);
NEKO_API_DECL neko_handle(neko_render_renderpass_t) neko_render_renderpass_create(const neko_render_renderpass_desc_t desc);
NEKO_API_DECL neko_handle(neko_render_pipeline_t) neko_render_pipeline_create(const neko_render_pipeline_desc_t desc);

// Destroy
NEKO_API_DECL void neko_render_texture_destroy(neko_handle(neko_render_texture_t) hndl);
NEKO_API_DECL void neko_render_uniform_destroy(neko_handle(neko_render_uniform_t) hndl);
NEKO_API_DECL void neko_render_shader_destroy(neko_handle(neko_render_shader_t) hndl);
NEKO_API_DECL void neko_render_vertex_buffer_destroy(neko_handle(neko_render_vertex_buffer_t) hndl);
NEKO_API_DECL void neko_render_index_buffer_destroy(neko_handle(neko_render_index_buffer_t) hndl);
NEKO_API_DECL void neko_render_uniform_buffer_destroy(neko_handle(neko_render_uniform_buffer_t) hndl);
NEKO_API_DECL void neko_render_storage_buffer_destroy(neko_handle(neko_render_storage_buffer_t) hndl);
NEKO_API_DECL void neko_render_framebuffer_destroy(neko_handle(neko_render_framebuffer_t) hndl);
NEKO_API_DECL void neko_render_renderpass_destroy(neko_handle(neko_render_renderpass_t) hndl);
NEKO_API_DECL void neko_render_pipeline_destroy(neko_handle(neko_render_pipeline_t) hndl);

// Resource Updates (main thread only)
NEKO_API_DECL void neko_render_vertex_buffer_update(neko_handle(neko_render_vertex_buffer_t) hndl, neko_render_vertex_buffer_desc_t* desc);
NEKO_API_DECL void neko_render_index_buffer_update(neko_handle(neko_render_index_buffer_t) hndl, neko_render_index_buffer_desc_t* desc);
NEKO_API_DECL void neko_render_storage_buffer_update(neko_handle(neko_render_storage_buffer_t) hndl, neko_render_storage_buffer_desc_t* desc);
NEKO_API_DECL void neko_render_texture_update(neko_handle(neko_render_texture_t) hndl, neko_render_texture_desc_t* desc);
NEKO_API_DECL void neko_render_texture_read(neko_handle(neko_render_texture_t) hndl, neko_render_texture_desc_t* desc);

// Resource Queries
NEKO_API_DECL void neko_render_pipeline_desc_query(neko_handle(neko_render_pipeline_t) hndl, neko_render_pipeline_desc_t* out);
NEKO_API_DECL void neko_render_texture_desc_query(neko_handle(neko_render_texture_t) hndl, neko_render_texture_desc_t* out);
NEKO_API_DECL size_t neko_render_uniform_size_query(neko_handle(neko_render_uniform_t) hndl);

// Resource In-Flight Update
NEKO_API_DECL void neko_render_texture_request_update(neko_command_buffer_t* cb, neko_handle(neko_render_texture_t) hndl, neko_render_texture_desc_t* desc);
NEKO_API_DECL void neko_render_vertex_buffer_request_update(neko_command_buffer_t* cb, neko_handle(neko_render_vertex_buffer_t) hndl, neko_render_vertex_buffer_desc_t* desc);
NEKO_API_DECL void neko_render_index_buffer_request_update(neko_command_buffer_t* cb, neko_handle(neko_render_index_buffer_t) hndl, neko_render_index_buffer_desc_t* desc);
NEKO_API_DECL void neko_render_uniform_buffer_request_update(neko_command_buffer_t* cb, neko_handle(neko_render_uniform_buffer_t) hndl, neko_render_uniform_buffer_desc_t* desc);
NEKO_API_DECL void neko_render_storage_buffer_request_update(neko_command_buffer_t* cb, neko_handle(neko_render_storage_buffer_t) hndl, neko_render_storage_buffer_desc_t* desc);

// Pipeline / Pass / Bind / Draw
NEKO_API_DECL void neko_render_renderpass_begin(neko_command_buffer_t* cb, neko_handle(neko_render_renderpass_t) hndl);
NEKO_API_DECL void neko_render_renderpass_end(neko_command_buffer_t* cb);
NEKO_API_DECL void neko_render_set_viewport(neko_command_buffer_t* cb, u32 x, u32 y, u32 w, u32 h);
NEKO_API_DECL void neko_render_set_view_scissor(neko_command_buffer_t* cb, u32 x, u32 y, u32 w, u32 h);
NEKO_API_DECL void neko_render_clear(neko_command_buffer_t* cb, neko_render_clear_action_t action);
NEKO_API_DECL void neko_render_clear_ex(neko_command_buffer_t* cb, neko_render_clear_desc_t* desc);
NEKO_API_DECL void neko_render_pipeline_bind(neko_command_buffer_t* cb, neko_handle(neko_render_pipeline_t) hndl);
NEKO_API_DECL void neko_render_apply_bindings(neko_command_buffer_t* cb, neko_render_bind_desc_t* binds);
NEKO_API_DECL void neko_render_draw(neko_command_buffer_t* cb, neko_render_draw_desc_t* desc);
NEKO_API_DECL void neko_render_draw_batch(neko_command_buffer_t* cb, neko_render_batch_context_t* ctx, neko_render_batch_framebuffer_t* fb, s32 w, s32 h);
NEKO_API_DECL void neko_render_dispatch_compute(neko_command_buffer_t* cb, u32 num_x_groups, u32 num_y_groups, u32 num_z_groups);

// Macros
#define neko_render_command_buffer_submit(CB) neko_render()->api.command_buffer_submit((CB))

typedef neko_handle(neko_render_shader_t) neko_shader_t;
typedef neko_handle(neko_render_texture_t) neko_texture_t;
typedef neko_handle(neko_render_renderpass_t) neko_renderpass_t;
typedef neko_handle(neko_render_framebuffer_t) neko_framebuffer_t;
typedef neko_handle(neko_render_pipeline_t) neko_pipeline_t;
typedef neko_handle(neko_render_vertex_buffer_t) neko_vbo_t;
typedef neko_handle(neko_render_index_buffer_t) neko_ibo_t;
typedef neko_handle(neko_render_uniform_buffer_t) neko_ubo_t;
typedef neko_handle(neko_render_uniform_t) neko_uniform_t;
typedef neko_handle(neko_render_storage_buffer_t) neko_storage_buffer_t;

/*=============================
// NEKO_OPENGL
=============================*/

typedef enum neko_gl_uniform_type {
    NEKO_GL_UNIFORMTYPE_FLOAT,
    NEKO_GL_UNIFORMTYPE_INT,
    NEKO_GL_UNIFORMTYPE_VEC2,
    NEKO_GL_UNIFORMTYPE_VEC3,
    NEKO_GL_UNIFORMTYPE_VEC4,
    NEKO_GL_UNIFORMTYPE_MAT4,
    NEKO_GL_UNIFORMTYPE_SAMPLER2D,
    NEKO_GL_UNIFORMTYPE_SAMPLERCUBE
} neko_gl_uniform_type;

/* Uniform (stores samplers as well as primitive uniforms) */
typedef struct neko_gl_uniform_t {
    char name[64];              // Name of uniform to find location
    neko_gl_uniform_type type;  // Type of uniform data
    u32 location;               // Location of uniform
    size_t size;                // Total data size of uniform
    u32 sid;                    // Shader id (should probably inverse this, but I don't want to force a map lookup)
    u32 count;                  // Count (used for arrays)
} neko_gl_uniform_t;

// When a user passes in a uniform layout, that handle could then pass to a WHOLE list of uniforms (if describing a struct)
typedef struct neko_gl_uniform_list_t {
    neko_dyn_array(neko_gl_uniform_t) uniforms;  // Individual uniforms in list
    size_t size;                                 // Total size of uniform data
    char name[64];                               // Base name of uniform
} neko_gl_uniform_list_t;

typedef struct neko_gl_uniform_buffer_t {
    char name[64];
    u32 location;
    size_t size;
    u32 ubo;
    u32 sid;
} neko_gl_uniform_buffer_t;

typedef struct neko_gl_storage_buffer_t {
    char name[64];
    u32 buffer;
    s32 access;
    size_t size;
    u32 block_idx;
    u32 location;
} neko_gl_storage_buffer_t;

/* Pipeline */
typedef struct neko_gl_pipeline_t {
    neko_render_blend_state_desc_t blend;
    neko_render_depth_state_desc_t depth;
    neko_render_raster_state_desc_t raster;
    neko_render_stencil_state_desc_t stencil;
    neko_render_compute_state_desc_t compute;
    neko_dyn_array(neko_render_vertex_attribute_desc_t) layout;
} neko_gl_pipeline_t;

/* Render Pass */
typedef struct neko_gl_renderpass_t {
    neko_handle(neko_render_framebuffer_t) fbo;
    neko_dyn_array(neko_handle(neko_render_texture_t)) color;
    neko_handle(neko_render_texture_t) depth;
    neko_handle(neko_render_texture_t) stencil;
} neko_gl_renderpass_t;

/* Shader */
typedef u32 neko_gl_shader_t;

/* Gfx Buffer */
typedef u32 neko_gl_buffer_t;

/* Texture */
typedef struct neko_gl_texture_t {
    u32 id;
    neko_render_texture_desc_t desc;
} neko_gl_texture_t;

typedef struct neko_gl_vertex_buffer_decl_t {
    neko_gl_buffer_t vbo;
    neko_render_vertex_data_type data_type;
    size_t offset;
} neko_gl_vertex_buffer_decl_t;

// 绘制之间的缓存数据
typedef struct neko_gl_data_cache_t {
    neko_gl_buffer_t vao;
    neko_gl_buffer_t ibo;
    size_t ibo_elem_sz;
    neko_dyn_array(neko_gl_vertex_buffer_decl_t) vdecls;
    neko_handle(neko_render_pipeline_t) pipeline;
} neko_gl_data_cache_t;

// 内部 OpenGL 数据
typedef struct neko_gl_data_t {
    neko_slot_array(neko_gl_shader_t) shaders;
    neko_slot_array(neko_gl_texture_t) textures;
    neko_slot_array(neko_gl_buffer_t) vertex_buffers;
    neko_slot_array(neko_gl_uniform_buffer_t) uniform_buffers;
    neko_slot_array(neko_gl_storage_buffer_t) storage_buffers;
    neko_slot_array(neko_gl_buffer_t) index_buffers;
    neko_slot_array(neko_gl_buffer_t) frame_buffers;
    neko_slot_array(neko_gl_uniform_list_t) uniforms;
    neko_slot_array(neko_gl_pipeline_t) pipelines;
    neko_slot_array(neko_gl_renderpass_t) renderpasses;

    // All the required uniform data for strict aliasing.
    struct {
        neko_dyn_array(u32) ui32;
        neko_dyn_array(s32) i32;
        neko_dyn_array(float) flt;
        neko_dyn_array(neko_vec2) vec2;
        neko_dyn_array(neko_vec3) vec3;
        neko_dyn_array(neko_vec4) vec4;
        neko_dyn_array(neko_mat4) mat4;
    } uniform_data;

    // Cached data between draw calls (to minimize state changes)
    neko_gl_data_cache_t cache;

} neko_gl_data_t;

// 内部OpenGL命令缓冲指令
typedef enum neko_opengl_op_code_type {
    NEKO_OPENGL_OP_BEGIN_RENDER_PASS = 0x00,
    NEKO_OPENGL_OP_END_RENDER_PASS,
    NEKO_OPENGL_OP_SET_VIEWPORT,
    NEKO_OPENGL_OP_SET_VIEW_SCISSOR,
    NEKO_OPENGL_OP_CLEAR,
    NEKO_OPENGL_OP_REQUEST_BUFFER_UPDATE,
    NEKO_OPENGL_OP_REQUEST_TEXTURE_UPDATE,
    NEKO_OPENGL_OP_BIND_PIPELINE,
    NEKO_OPENGL_OP_APPLY_BINDINGS,
    NEKO_OPENGL_OP_DISPATCH_COMPUTE,
    NEKO_OPENGL_OP_DRAW,
    NEKO_OPENGL_OP_DRAW_BATCH,
} neko_opengl_op_code_type;

/*=============================
//
=============================*/

NEKO_INLINE u64 generate_texture_handle(void* pixels, int w, int h, void* udata) {
    (void)udata;
    GLuint location;
    glGenTextures(1, &location);
    glBindTexture(GL_TEXTURE_2D, location);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    return (u64)location;
}

NEKO_INLINE void destroy_texture_handle(u64 texture_id, void* udata) {
    (void)udata;
    GLuint id = (GLuint)texture_id;
    glDeleteTextures(1, &id);
}

// Mesh
neko_enum_decl(neko_asset_mesh_attribute_type, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_POSITION, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_NORMAL, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TANGENT,
               NEKO_ASSET_MESH_ATTRIBUTE_TYPE_JOINT, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_WEIGHT, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD, NEKO_ASSET_MESH_ATTRIBUTE_TYPE_COLOR,
               NEKO_ASSET_MESH_ATTRIBUTE_TYPE_UINT);

typedef struct neko_asset_mesh_layout_t {
    neko_asset_mesh_attribute_type type;  // Type of attribute
    u32 idx;                              // Optional index (for joint/weight/texcoord/color)
} neko_asset_mesh_layout_t;

typedef struct neko_asset_mesh_decl_t {
    neko_asset_mesh_layout_t* layout;  // Mesh attribute layout array
    size_t layout_size;                // Size of mesh attribute layout array in bytes
    size_t index_buffer_element_size;  // Size of index data size in bytes
} neko_asset_mesh_decl_t;

typedef struct neko_asset_mesh_primitive_t {
    neko_handle(neko_render_vertex_buffer_t) vbo;
    neko_handle(neko_render_index_buffer_t) ibo;
    u32 count;
} neko_asset_mesh_primitive_t;

typedef struct neko_asset_mesh_t {
    neko_dyn_array(neko_asset_mesh_primitive_t) primitives;
} neko_asset_mesh_t;

// Structured/packed raw mesh data
typedef struct neko_asset_mesh_raw_data_t {
    u32 prim_count;
    size_t* vertex_sizes;
    size_t* index_sizes;
    void** vertices;
    void** indices;
} neko_asset_mesh_raw_data_t;

typedef struct neko_asset_texture_t {
    neko_handle(neko_render_texture_t) hndl;
    neko_render_texture_desc_t desc;
} neko_asset_texture_t;

typedef struct neko_baked_char_t {
    u32 codepoint;
    u16 x0, y0, x1, y1;
    float xoff, yoff, advance;
    u32 width, height;
} neko_baked_char_t;

typedef struct neko_asset_ascii_font_t {
    void* font_info;
    neko_baked_char_t glyphs[96];
    neko_asset_texture_t texture;
    float ascent;
    float descent;
    float line_gap;
} neko_asset_ascii_font_t;

/*==========================
// NEKO_ASSET_TYPES
==========================*/

// Texture

NEKO_API_DECL bool neko_asset_texture_load_from_file(const_str path, void* out, neko_render_texture_desc_t* desc, b32 flip_on_load, b32 keep_data);
NEKO_API_DECL bool neko_asset_texture_load_from_memory(const void* memory, size_t sz, void* out, neko_render_texture_desc_t* desc, b32 flip_on_load, b32 keep_data);

// Font
NEKO_API_DECL bool neko_asset_ascii_font_load_from_file(const_str path, void* out, u32 point_size);
NEKO_API_DECL bool neko_asset_ascii_font_load_from_memory(const void* memory, size_t sz, void* out, u32 point_size);
NEKO_API_DECL neko_vec2 neko_asset_ascii_font_text_dimensions(const neko_asset_ascii_font_t* font, const_str text, s32 len);
NEKO_API_DECL neko_vec2 neko_asset_ascii_font_text_dimensions_ex(const neko_asset_ascii_font_t* fp, const_str text, s32 len, b32 include_past_baseline);
NEKO_API_DECL float neko_asset_ascii_font_max_height(const neko_asset_ascii_font_t* font);

NEKO_API_DECL bool neko_asset_mesh_load_from_file(const_str path, void* out, neko_asset_mesh_decl_t* decl, void* data_out, size_t data_size);
// NEKO_API_DECL bool neko_util_load_gltf_data_from_file(const_str path, neko_asset_mesh_decl_t* decl, neko_asset_mesh_raw_data_t** out, u32* mesh_count);
// NEKO_API_DECL bool neko_util_load_gltf_data_from_memory(const void* memory, size_t sz, neko_asset_mesh_decl_t* decl, neko_asset_mesh_raw_data_t** out, u32* mesh_count);

#ifndef NEKO_IDRAW
#define NEKO_IDRAW

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
    neko_dyn_array(neko_handle(neko_render_pipeline_t)) pipelines;
    neko_dyn_array(neko_mat4) modelview;
    neko_dyn_array(neko_mat4) projection;
    neko_dyn_array(neko_idraw_matrix_type) modes;
    neko_vec2 uv;
    neko_color_t color;
    neko_handle(neko_render_texture_t) texture;
    neko_idraw_pipeline_state_attr_t pipeline;
} neko_immediate_cache_t;

typedef struct neko_immediate_draw_static_data_t {
    neko_handle(neko_render_texture_t) tex_default;
    neko_asset_ascii_font_t font_default;  // Idraw font
    neko_hash_table(neko_idraw_pipeline_state_attr_t, neko_handle(neko_render_pipeline_t)) pipeline_table;
    neko_handle(neko_render_uniform_t) uniform;
    neko_handle(neko_render_uniform_t) sampler;
    neko_handle(neko_render_vertex_buffer_t) vbo;
    neko_handle(neko_render_index_buffer_t) ibo;
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

typedef neko_immediate_draw_t gsid;
#define neko_idraw_create neko_immediate_draw_new
#define neko_idraw_free neko_immediate_draw_free

// Create / Init / Shutdown / Free
NEKO_API_DECL neko_immediate_draw_t neko_immediate_draw_new();
NEKO_API_DECL void neko_immediate_draw_free(neko_immediate_draw_t* neko_idraw);
NEKO_API_DECL void neko_immediate_draw_static_data_set(neko_immediate_draw_static_data_t* data);
NEKO_API_DECL neko_immediate_draw_static_data_t* neko_immediate_draw_static_data_get();  // 用于热更新

// Get pipeline from state
NEKO_API_DECL neko_handle(neko_render_pipeline_t) neko_idraw_get_pipeline(neko_immediate_draw_t* neko_idraw, neko_idraw_pipeline_state_attr_t state);

// Get default font asset pointer
NEKO_API_DECL neko_asset_ascii_font_t* neko_idraw_default_font();

// Core Vertex Functions
NEKO_API_DECL void neko_idraw_begin(neko_immediate_draw_t* neko_idraw, neko_render_primitive_type type);
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
NEKO_API_DECL void neko_idraw_texture(neko_immediate_draw_t* neko_idraw, neko_handle(neko_render_texture_t) texture);

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
                                           neko_handle(neko_render_pipeline_t) pipeline);  // Binds custom user pipeline, sets flag NEKO_IDRAW_FLAG_NO_BIND_CACHED_PIPELINES
NEKO_API_DECL void neko_idraw_vattr_list(neko_immediate_draw_t* neko_idraw, neko_idraw_vattr_type* layout, size_t sz);  // Sets user vertex attribute list for custom bound pipeline
NEKO_API_DECL void neko_idraw_vattr_list_mesh(neko_immediate_draw_t* neko_idraw, neko_asset_mesh_layout_t* layout,
                                              size_t sz);  // Same as above but uses mesh layout to determine which vertex attributes to bind and in what order

// View/Scissor commands
NEKO_API_DECL void neko_idraw_set_view_scissor(neko_immediate_draw_t* neko_idraw, u32 x, u32 y, u32 w, u32 h);

// Final Submit / Merge
NEKO_API_DECL void neko_idraw_draw(neko_immediate_draw_t* neko_idraw, neko_command_buffer_t* cb);
NEKO_API_DECL void neko_idraw_renderpass_submit(neko_immediate_draw_t* neko_idraw, neko_command_buffer_t* cb, neko_vec4 viewport, neko_color_t clear_color);
NEKO_API_DECL void neko_idraw_renderpass_submit_ex(neko_immediate_draw_t* neko_idraw, neko_command_buffer_t* cb, neko_vec4 viewport, neko_render_clear_action_t action);

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
NEKO_API_DECL void neko_idraw_triangle(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, f32 x2, f32 y2, u8 r, u8 g, u8 b, u8 a, neko_render_primitive_type type);
NEKO_API_DECL void neko_idraw_trianglev(neko_immediate_draw_t* neko_idraw, neko_vec2 a, neko_vec2 b, neko_vec2 c, neko_color_t color, neko_render_primitive_type type);
NEKO_API_DECL void neko_idraw_trianglex(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2, f32 u0, f32 v0, f32 u1, f32 v1, f32 u2, f32 v2, u8 r,
                                        u8 g, u8 b, u8 a, neko_render_primitive_type type);
NEKO_API_DECL void neko_idraw_trianglevx(neko_immediate_draw_t* neko_idraw, neko_vec3 v0, neko_vec3 v1, neko_vec3 v2, neko_vec2 uv0, neko_vec2 uv1, neko_vec2 uv2, neko_color_t color,
                                         neko_render_primitive_type type);
NEKO_API_DECL void neko_idraw_trianglevxmc(neko_immediate_draw_t* neko_idraw, neko_vec3 v0, neko_vec3 v1, neko_vec3 v2, neko_vec2 uv0, neko_vec2 uv1, neko_vec2 uv2, neko_color_t c0, neko_color_t c1,
                                           neko_color_t c2, neko_render_primitive_type type);
NEKO_API_DECL void neko_idraw_line(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, u8 r, u8 g, u8 b, u8 a);
NEKO_API_DECL void neko_idraw_linev(neko_immediate_draw_t* neko_idraw, neko_vec2 v0, neko_vec2 v1, neko_color_t c);
NEKO_API_DECL void neko_idraw_line3D(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1, u8 r, u8 g, u8 b, u8 a);
NEKO_API_DECL void neko_idraw_line3Dv(neko_immediate_draw_t* neko_idraw, neko_vec3 s, neko_vec3 e, neko_color_t color);
NEKO_API_DECL void neko_idraw_line3Dmc(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1, u8 r0, u8 g0, u8 b0, u8 a0, u8 r1, u8 g1, u8 b1, u8 a1);

// Shape Drawing Util
NEKO_API_DECL void neko_idraw_rect(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, u8 r, u8 g, u8 b, u8 a, neko_render_primitive_type type);
NEKO_API_DECL void neko_idraw_rectv(neko_immediate_draw_t* neko_idraw, neko_vec2 bl, neko_vec2 tr, neko_color_t color, neko_render_primitive_type type);
NEKO_API_DECL void neko_idraw_rectx(neko_immediate_draw_t* neko_idraw, f32 l, f32 b, f32 r, f32 t, f32 u0, f32 v0, f32 u1, f32 v1, u8 _r, u8 _g, u8 _b, u8 _a, neko_render_primitive_type type);
NEKO_API_DECL void neko_idraw_rectvx(neko_immediate_draw_t* neko_idraw, neko_vec2 bl, neko_vec2 tr, neko_vec2 uv0, neko_vec2 uv1, neko_color_t color, neko_render_primitive_type type);
NEKO_API_DECL void neko_idraw_rectvd(neko_immediate_draw_t* neko_idraw, neko_vec2 xy, neko_vec2 wh, neko_vec2 uv0, neko_vec2 uv1, neko_color_t color, neko_render_primitive_type type);
NEKO_API_DECL void neko_idraw_rect3Dv(neko_immediate_draw_t* neko_idraw, neko_vec3 min, neko_vec3 max, neko_vec2 uv0, neko_vec2 uv1, neko_color_t color, neko_render_primitive_type type);
NEKO_API_DECL void neko_idraw_rect3Dvd(neko_immediate_draw_t* neko_idraw, neko_vec3 xyz, neko_vec3 whd, neko_vec2 uv0, neko_vec2 uv1, neko_color_t c, neko_render_primitive_type type);
NEKO_API_DECL void neko_idraw_circle(neko_immediate_draw_t* neko_idraw, f32 cx, f32 cy, f32 radius, int32_t segments, u8 r, u8 g, u8 b, u8 a, neko_render_primitive_type type);
NEKO_API_DECL void neko_idraw_circlevx(neko_immediate_draw_t* neko_idraw, neko_vec3 c, f32 radius, int32_t segments, neko_color_t color, neko_render_primitive_type type);
NEKO_API_DECL void neko_idraw_circle_sector(neko_immediate_draw_t* neko_idraw, f32 cx, f32 cy, f32 radius, int32_t start_angle, int32_t end_angle, int32_t segments, u8 r, u8 g, u8 b, u8 a,
                                            neko_render_primitive_type type);
NEKO_API_DECL void neko_idraw_circle_sectorvx(neko_immediate_draw_t* neko_idraw, neko_vec3 c, f32 radius, int32_t start_angle, int32_t end_angle, int32_t segments, neko_color_t color,
                                              neko_render_primitive_type type);
NEKO_API_DECL void neko_idraw_arc(neko_immediate_draw_t* neko_idraw, f32 cx, f32 cy, f32 radius_inner, f32 radius_outer, f32 start_angle, f32 end_angle, int32_t segments, u8 r, u8 g, u8 b, u8 a,
                                  neko_render_primitive_type type);
NEKO_API_DECL void neko_idraw_box(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 z0, f32 hx, f32 hy, f32 hz, u8 r, u8 g, u8 b, u8 a, neko_render_primitive_type type);
NEKO_API_DECL void neko_idraw_sphere(neko_immediate_draw_t* neko_idraw, f32 cx, f32 cy, f32 cz, f32 radius, u8 r, u8 g, u8 b, u8 a, neko_render_primitive_type type);
NEKO_API_DECL void neko_idraw_bezier(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, u8 r, u8 g, u8 b, u8 a);
NEKO_API_DECL void neko_idraw_cylinder(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z, f32 r_top, f32 r_bottom, f32 height, int32_t sides, u8 r, u8 g, u8 b, u8 a,
                                       neko_render_primitive_type type);
NEKO_API_DECL void neko_idraw_cone(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z, f32 radius, f32 height, int32_t sides, u8 r, u8 g, u8 b, u8 a, neko_render_primitive_type type);

// Draw planes/poly groups

// Text Drawing Util
NEKO_API_DECL void neko_idraw_text(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, const char* text, const neko_asset_ascii_font_t* fp, b32 flip_vertical, neko_color_t col);

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

typedef void* (*neko_draw_raw_data_func)(NEKO_GFXT_HNDL hndl, void* user_data);

#define NEKO_GFXT_RAW_DATA(FUNC_DESC, T) ((T*)(FUNC_DESC)->func((FUNC_DESC)->hndl, (FUNC_DESC)->user_data))

typedef struct neko_draw_raw_data_func_desc_t {
    NEKO_GFXT_HNDL hndl;           // Handle used for retrieving data.
    neko_draw_raw_data_func func;  // User defined function for pipeline data retrieval
    void* user_data;               // Optional user data for function
} neko_draw_raw_data_func_desc_t;

//=== Uniforms/Uniform blocks ===//
typedef struct neko_draw_uniform_desc_t {
    char name[64];                        // the_name of uniform (for binding to shader)
    neko_render_uniform_type type;        // Type of uniform: NEKO_RENDER_UNIFORM_VEC2, NEKO_RENDER_UNIFORM_VEC3, etc.
    uint32_t binding;                     // Binding for this uniform in shader
    neko_render_shader_stage_type stage;  // Shader stage for this uniform
    neko_render_access_type access_type;  // Access type for this uniform (compute only)
} neko_draw_uniform_desc_t;

typedef struct neko_draw_uniform_t {
    neko_handle(neko_render_uniform_t) hndl;  // Graphics handle resource for actual uniform
    uint32_t offset;                          // Individual offset for this uniform in material byte buffer data
    uint32_t binding;                         // Binding for this uniform
    size_t size;                              // Size of this uniform data in bytes
    neko_render_uniform_type type;            // Type of this uniform
    neko_render_access_type access_type;      // Access type of uniform (compute only)
} neko_draw_uniform_t;

typedef struct neko_draw_uniform_block_desc_t {
    neko_draw_uniform_desc_t* layout;  // Layout for all uniform data for this block to hold
    size_t size;                       // Size of layout in bytes
} neko_draw_uniform_block_desc_t;

typedef struct neko_draw_uniform_block_lookup_key_t {
    char name[64];
} neko_draw_uniform_block_lookup_key_t;

typedef struct neko_draw_uniform_block_t {
    neko_dyn_array(neko_draw_uniform_t) uniforms;  // Raw uniform handle array
    neko_hash_table(uint64_t, uint32_t) lookup;    // Index lookup table (used for byte buffer offsets in material uni. data)
    size_t size;                                   // Total size of material data for entire block
} neko_draw_uniform_block_t;

//=== Texture ===//
typedef neko_handle(neko_render_texture_t) neko_draw_texture_t;

//=== Mesh ===//
typedef neko_asset_mesh_attribute_type neko_draw_mesh_attribute_type;
typedef neko_asset_mesh_layout_t neko_draw_mesh_layout_t;

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
    } neko_draw_mesh_primitive_data_t;
*/

typedef struct {
    void* data;
    size_t size;
} neko_draw_mesh_vertex_attribute_t;

typedef struct {
    neko_draw_mesh_vertex_attribute_t positions;  // All position data
    neko_draw_mesh_vertex_attribute_t normals;
    neko_draw_mesh_vertex_attribute_t tangents;
    neko_draw_mesh_vertex_attribute_t tex_coords[NEKO_GFXT_TEX_COORD_MAX];
    neko_draw_mesh_vertex_attribute_t colors[NEKO_GFXT_COLOR_MAX];
    neko_draw_mesh_vertex_attribute_t joints[NEKO_GFXT_JOINT_MAX];
    neko_draw_mesh_vertex_attribute_t weights[NEKO_GFXT_WEIGHT_MAX];
    neko_draw_mesh_vertex_attribute_t custom_uint[NEKO_GFXT_CUSTOM_UINT_MAX];
    neko_draw_mesh_vertex_attribute_t indices;
    uint32_t count;  // Total count of indices
} neko_draw_mesh_vertex_data_t;

// Structured/packed raw mesh data
typedef struct neko_draw_mesh_raw_data_t {
    neko_dyn_array(neko_draw_mesh_vertex_data_t) primitives;  // All primitive data
} neko_draw_mesh_raw_data_t;

typedef struct neko_draw_mesh_import_options_t {
    neko_draw_mesh_layout_t* layout;   // Mesh attribute layout array
    size_t size;                       // Size of mesh attribute layout array in bytes
    size_t index_buffer_element_size;  // Size of index data size in bytes
} neko_draw_mesh_import_options_t;

NEKO_API_DECL void neko_draw_mesh_import_options_free(neko_draw_mesh_import_options_t* opt);

typedef struct neko_draw_mesh_desc_s {
    neko_draw_mesh_raw_data_t* meshes;  // Mesh data array
    size_t size;                        // Size of mesh data array in bytes
    b32 keep_data;                      // Whether or not to free data after use
} neko_draw_mesh_desc_t;

typedef struct neko_draw_vertex_stream_s {
    neko_handle(neko_render_vertex_buffer_t) positions;
    neko_handle(neko_render_vertex_buffer_t) normals;
    neko_handle(neko_render_vertex_buffer_t) tangents;
    neko_handle(neko_render_vertex_buffer_t) colors[NEKO_GFXT_COLOR_MAX];
    neko_handle(neko_render_vertex_buffer_t) tex_coords[NEKO_GFXT_TEX_COORD_MAX];
    neko_handle(neko_render_vertex_buffer_t) joints[NEKO_GFXT_JOINT_MAX];
    neko_handle(neko_render_vertex_buffer_t) weights[NEKO_GFXT_WEIGHT_MAX];
    neko_handle(neko_render_vertex_buffer_t) custom_uint[NEKO_GFXT_CUSTOM_UINT_MAX];
} neko_draw_vertex_stream_t;

typedef struct neko_draw_mesh_primitive_s {
    neko_draw_vertex_stream_t stream;                 // All vertex data streams
    neko_handle(neko_render_index_buffer_t) indices;  // Index buffer
    uint32_t count;                                   // Total number of vertices
} neko_draw_mesh_primitive_t;

typedef struct neko_draw_mesh_s {
    neko_dyn_array(neko_draw_mesh_primitive_t) primitives;
    neko_draw_mesh_desc_t desc;
} neko_draw_mesh_t;

//=== Pipeline ===//
typedef struct neko_draw_pipeline_desc_s {
    neko_render_pipeline_desc_t pip_desc;        // Description for constructing pipeline object
    neko_draw_uniform_block_desc_t ublock_desc;  // Description for constructing uniform block object
} neko_draw_pipeline_desc_t;

typedef struct neko_draw_pipeline_s {
    neko_handle(neko_render_pipeline_t) hndl;  // Graphics handle resource for actual pipeline
    neko_draw_uniform_block_t ublock;          // Uniform block for holding all uniform data
    neko_dyn_array(neko_draw_mesh_layout_t) mesh_layout;
    neko_render_pipeline_desc_t desc;
} neko_draw_pipeline_t;

//=== Material ===//
typedef struct neko_draw_material_desc_s {
    neko_draw_raw_data_func_desc_t pip_func;  // Description for retrieving raw pipeline pointer data from handle.
} neko_draw_material_desc_t;

typedef struct neko_draw_material_s {
    neko_draw_material_desc_t desc;        // Material description object
    neko_byte_buffer_t uniform_data;       // Byte buffer of actual uniform data to send to GPU
    neko_byte_buffer_t image_buffer_data;  // Image buffer data
} neko_draw_material_t;

//=== Renderable ===//
typedef struct neko_draw_renderable_desc_s {
    neko_draw_raw_data_func_desc_t mesh;      // Description for retrieving raw mesh pointer data from handle.
    neko_draw_raw_data_func_desc_t material;  // Description for retrieving raw material pointer data from handle.
} neko_draw_renderable_desc_t;

typedef struct neko_draw_renderable_s {
    neko_draw_renderable_desc_t desc;  // Renderable description object
    neko_mat4 model_matrix;            // Model matrix for renderable
} neko_draw_renderable_t;

//=== Graphics scene ===//
typedef struct neko_draw_scene_s {
    neko_slot_array(neko_draw_renderable_t) renderables;
} neko_draw_scene_t;

//==== API =====//

//=== Creation ===//
NEKO_API_DECL neko_draw_pipeline_t neko_draw_pipeline_create(const neko_draw_pipeline_desc_t* desc);
NEKO_API_DECL neko_draw_material_t neko_draw_material_create(neko_draw_material_desc_t* desc);
NEKO_API_DECL neko_draw_mesh_t neko_draw_mesh_create(const neko_draw_mesh_desc_t* desc);
NEKO_API_DECL void neko_draw_mesh_update_or_create(neko_draw_mesh_t* mesh, const neko_draw_mesh_desc_t* desc);
NEKO_API_DECL neko_draw_renderable_t neko_draw_renderable_create(const neko_draw_renderable_desc_t* desc);
NEKO_API_DECL neko_draw_uniform_block_t neko_draw_uniform_block_create(const neko_draw_uniform_block_desc_t* desc);
NEKO_API_DECL neko_draw_texture_t neko_draw_texture_create(neko_render_texture_desc_t desc);

//=== Destruction ===//
NEKO_API_DECL void neko_draw_texture_destroy(neko_draw_texture_t* texture);
NEKO_API_DECL void neko_draw_material_destroy(neko_draw_material_t* material);
NEKO_API_DECL void neko_draw_mesh_destroy(neko_draw_mesh_t* mesh);
NEKO_API_DECL void neko_draw_uniform_block_destroy(neko_draw_uniform_block_t* ub);
NEKO_API_DECL void neko_draw_pipeline_destroy(neko_draw_pipeline_t* pipeline);

//=== Resource Loading ===//
NEKO_API_DECL neko_draw_pipeline_t neko_draw_pipeline_load_from_file(const char* path);
NEKO_API_DECL neko_draw_pipeline_t neko_draw_pipeline_load_from_memory(const char* data, size_t sz);
NEKO_API_DECL neko_draw_pipeline_t neko_draw_pipeline_load_from_memory_ext(const char* data, size_t sz, const char* file_dir);
NEKO_API_DECL neko_draw_texture_t neko_draw_texture_load_from_file(const char* path, neko_render_texture_desc_t* desc, bool flip, bool keep_data);
NEKO_API_DECL neko_draw_texture_t neko_draw_texture_load_from_memory(const char* data, size_t sz, neko_render_texture_desc_t* desc, bool flip, bool keep_data);

//=== Copy ===//
NEKO_API_DECL neko_draw_material_t neko_draw_material_deep_copy(neko_draw_material_t* src);

//=== Pipeline API ===//
NEKO_API_DECL neko_draw_uniform_t* neko_draw_pipeline_get_uniform(neko_draw_pipeline_t* pip, const char* name);

//=== Material API ===//
NEKO_API_DECL void neko_draw_material_set_uniform(neko_draw_material_t* mat, const char* name, const void* data);
NEKO_API_DECL void neko_draw_material_bind(neko_command_buffer_t* cb, neko_draw_material_t* mat);
NEKO_API_DECL void neko_draw_material_bind_pipeline(neko_command_buffer_t* cb, neko_draw_material_t* mat);
NEKO_API_DECL void neko_draw_material_bind_uniforms(neko_command_buffer_t* cb, neko_draw_material_t* mat);
NEKO_API_DECL neko_draw_pipeline_t* neko_draw_material_get_pipeline(neko_draw_material_t* mat);

//=== Mesh API ===//
NEKO_API_DECL void neko_draw_mesh_draw_pipeline(neko_command_buffer_t* cb, neko_draw_mesh_t* mesh, neko_draw_pipeline_t* pip);
NEKO_API_DECL void neko_draw_mesh_draw_material(neko_command_buffer_t* cb, neko_draw_mesh_t* mesh, neko_draw_material_t* mat);
NEKO_API_DECL void neko_draw_mesh_draw_materials(neko_command_buffer_t* cb, neko_draw_mesh_t* mesh, neko_draw_material_t** mats, size_t mats_size);
NEKO_API_DECL void neko_draw_mesh_draw_layout(neko_command_buffer_t* cb, neko_draw_mesh_t* mesh, neko_draw_mesh_layout_t* layout, size_t layout_size);
NEKO_API_DECL neko_draw_mesh_t neko_draw_mesh_load_from_file(const char* file, neko_draw_mesh_import_options_t* options);
// NEKO_API_DECL bool neko_draw_load_gltf_data_from_file(const char* path, neko_draw_mesh_import_options_t* options, neko_draw_mesh_raw_data_t** out, uint32_t* mesh_count);

// Util API
NEKO_API_DECL void* neko_draw_raw_data_default_impl(NEKO_GFXT_HNDL hndl, void* user_data);

// Mesh Generation API
NEKO_API_DECL neko_draw_mesh_t neko_draw_mesh_unit_quad_generate(neko_draw_mesh_import_options_t* options);
NEKO_API_DECL neko_handle(neko_render_texture_t) neko_draw_texture_generate_default();

// ECS component
typedef struct neko_draw_renderer {
    neko_draw_pipeline_t pip;
    neko_draw_material_t mat;
    neko_draw_mesh_t mesh;
    neko_draw_texture_t texture;
} neko_draw_renderer;

#endif

#ifndef NEKO_UI
#define NEKO_UI

// Modified from microui(https://github.com/rxi/microui) and ImGui(https://github.com/ocornut/imgui)

#define NEKO_UI_SPLIT_SIZE 2.f
#define NEKO_UI_MAX_CNT 48
#define NEKO_UI_COMMANDLIST_SIZE (256 * 1024)
#define NEKO_UI_ROOTLIST_SIZE 32
#define NEKO_UI_CONTAINERSTACK_SIZE 32
#define NEKO_UI_CLIPSTACK_SIZE 32
#define NEKO_UI_IDSTACK_SIZE 32
#define NEKO_UI_LAYOUTSTACK_SIZE 16
#define NEKO_UI_CONTAINERPOOL_SIZE 48
#define NEKO_UI_TREENODEPOOL_SIZE 48
#define NEKO_UI_NEKO_UI_SPLIT_SIZE 32
#define NEKO_UI_NEKO_UI_TAB_SIZE 32
#define NEKO_UI_MAX_WIDTHS 16
#define NEKO_UI_REAL f32
#define NEKO_UI_REAL_FMT "%.3g"
#define NEKO_UI_SLIDER_FMT "%.2f"
#define NEKO_UI_MAX_FMT 127
#define NEKO_UI_TAB_ITEM_MAX 24
#define NEKO_UI_CLS_SELECTOR_MAX 4

#ifdef __cplusplus
#define neko_ui_widths(...)                       \
    [&]() -> const s32* {                         \
        static s32 temp_widths[] = {__VA_ARGS__}; \
        return temp_widths;                       \
    }()
#else
#define neko_ui_widths(...) \
    (int[]) { __VA_ARGS__ }
#endif

#define neko_ui_stack(T, n) \
    struct {                \
        s32 idx;            \
        T items[n];         \
    }

enum { NEKO_UI_CLIP_PART = 1, NEKO_UI_CLIP_ALL };

enum {
    NEKO_UI_COMMAND_JUMP = 1,
    NEKO_UI_COMMAND_CLIP,
    NEKO_UI_COMMAND_SHAPE,
    NEKO_UI_COMMAND_TEXT,
    NEKO_UI_COMMAND_ICON,
    NEKO_UI_COMMAND_IMAGE,
    NEKO_UI_COMMAND_CUSTOM,
    NEKO_UI_COMMAND_PIPELINE,
    NEKO_UI_COMMAND_UNIFORMS,
    NEKO_UI_COMMAND_MAX
};

enum { NEKO_UI_SHAPE_RECT = 1, NEKO_UI_SHAPE_CIRCLE, NEKO_UI_SHAPE_TRIANGLE, NEKO_UI_SHAPE_LINE };

enum { NEKO_UI_TRANSFORM_WORLD = 0x00, NEKO_UI_TRANSFORM_LOCAL };

enum { NEKO_UI_GIZMO_TRANSLATE = 0x00, NEKO_UI_GIZMO_ROTATE, NEKO_UI_GIZMO_SCALE };

enum {
    NEKO_UI_COLOR_BACKGROUND = 0x00,
    NEKO_UI_COLOR_CONTENT,
    NEKO_UI_COLOR_BORDER,
    NEKO_UI_COLOR_SHADOW,
    NEKO_UI_COLOR_CONTENT_BACKGROUND,
    NEKO_UI_COLOR_CONTENT_SHADOW,
    NEKO_UI_COLOR_CONTENT_BORDER,

    NEKO_UI_COLOR_MAX
};

enum { NEKO_UI_ICON_CLOSE = 1, NEKO_UI_ICON_CHECK, NEKO_UI_ICON_COLLAPSED, NEKO_UI_ICON_EXPANDED, NEKO_UI_ICON_MAX };

enum { NEKO_UI_RES_ACTIVE = (1 << 0), NEKO_UI_RES_SUBMIT = (1 << 1), NEKO_UI_RES_CHANGE = (1 << 2) };

typedef enum neko_ui_alt_drag_mode_type {
    NEKO_UI_ALT_DRAG_QUAD = 0x00,  // Quadrants
    NEKO_UI_ALT_DRAG_NINE,         // Nine splice the window
    NEKO_UI_ALT_DRAG_SINGLE        // Single window drag (controls the width/height, leaving x/y position of window in tact)
} neko_ui_alt_drag_mode_type;

enum {
    NEKO_UI_OPT_NOSTYLESHADOW = (1ULL << 0),
    NEKO_UI_OPT_NOSTYLEBORDER = (1ULL << 1),
    NEKO_UI_OPT_NOINTERACT = (1ULL << 2),
    NEKO_UI_OPT_NOFRAME = (1ULL << 3),
    NEKO_UI_OPT_NORESIZE = (1ULL << 4),
    NEKO_UI_OPT_NOSCROLL = (1ULL << 5),
    NEKO_UI_OPT_NOCLOSE = (1ULL << 6),
    NEKO_UI_OPT_NOTITLE = (1ULL << 7),
    NEKO_UI_OPT_HOLDFOCUS = (1ULL << 8),
    NEKO_UI_OPT_AUTOSIZE = (1ULL << 9),
    NEKO_UI_OPT_POPUP = (1ULL << 10),
    NEKO_UI_OPT_CLOSED = (1ULL << 11),
    NEKO_UI_OPT_EXPANDED = (1ULL << 12),
    NEKO_UI_OPT_NOHOVER = (1ULL << 13),
    NEKO_UI_OPT_FORCESETRECT = (1ULL << 14),
    NEKO_UI_OPT_NOFOCUS = (1ULL << 15),
    NEKO_UI_OPT_FORCEFOCUS = (1ULL << 16),
    NEKO_UI_OPT_NOMOVE = (1ULL << 17),
    NEKO_UI_OPT_NOCLIP = (1ULL << 18),
    NEKO_UI_OPT_NODOCK = (1ULL << 19),
    NEKO_UI_OPT_FULLSCREEN = (1ULL << 20),
    NEKO_UI_OPT_DOCKSPACE = (1ULL << 21),
    NEKO_UI_OPT_NOBRINGTOFRONT = (1ULL << 22),
    NEKO_UI_OPT_LEFTCLICKONLY = (1ULL << 23),
    NEKO_UI_OPT_NOSWITCHSTATE = (1ULL << 24),
    NEKO_UI_OPT_NOBORDER = (1ULL << 25),
    NEKO_UI_OPT_ISCONTENT = (1ULL << 26),
    NEKO_UI_OPT_NOCARET = (1ULL << 27),
    NEKO_UI_OPT_NOSCROLLHORIZONTAL = (1ULL << 28),
    NEKO_UI_OPT_NOSCROLLVERTICAL = (1ULL << 29),
    NEKO_UI_OPT_NOSTYLEBACKGROUND = (1ULL << 30),
    NEKO_UI_OPT_PARSEIDTAGONLY = (1ULL << 31)
};

enum { NEKO_UI_MOUSE_LEFT = (1 << 0), NEKO_UI_MOUSE_RIGHT = (1 << 1), NEKO_UI_MOUSE_MIDDLE = (1 << 2) };

enum { NEKO_UI_KEY_SHIFT = (1 << 0), NEKO_UI_KEY_CTRL = (1 << 1), NEKO_UI_KEY_ALT = (1 << 2), NEKO_UI_KEY_BACKSPACE = (1 << 3), NEKO_UI_KEY_RETURN = (1 << 4) };

#define NEKO_UI_OPT_NOSTYLING (NEKO_UI_OPT_NOSTYLEBORDER | NEKO_UI_OPT_NOSTYLEBACKGROUND | NEKO_UI_OPT_NOSTYLESHADOW)

typedef struct neko_ui_context_t neko_ui_context_t;
typedef u32 neko_ui_id;
typedef NEKO_UI_REAL neko_ui_real;

// Shapes
typedef struct {
    float x, y, w, h;
} neko_ui_rect_t;
typedef struct {
    float radius;
    neko_vec2 center;
} neko_ui_circle_t;
typedef struct {
    neko_vec2 points[3];
} neko_ui_triangle_t;
typedef struct {
    neko_vec2 start;
    neko_vec2 end;
} neko_ui_line_t;

typedef struct {
    neko_ui_id id;
    s32 last_update;
} neko_ui_pool_item_t;

typedef struct {
    s32 type, size;
} neko_ui_basecommand_t;

typedef struct {
    neko_ui_basecommand_t base;
    void* dst;
} neko_ui_jumpcommand_t;

typedef struct {
    neko_ui_basecommand_t base;
    neko_ui_rect_t rect;
} neko_ui_clipcommand_t;

typedef struct {
    neko_ui_basecommand_t base;
    neko_asset_ascii_font_t* font;
    neko_vec2 pos;
    neko_color_t color;
    char str[1];
} neko_ui_textcommand_t;

typedef struct {
    neko_ui_basecommand_t base;
    neko_handle(neko_render_pipeline_t) pipeline;
    neko_idraw_layout_type layout_type;
    void* layout;
    size_t layout_sz;
} neko_ui_pipelinecommand_t;

typedef struct {
    neko_ui_basecommand_t base;
    void* data;
    size_t sz;
} neko_ui_binduniformscommand_t;

typedef struct {
    neko_ui_basecommand_t base;
    neko_ui_rect_t rect;
    neko_handle(neko_render_texture_t) hndl;
    neko_vec4 uvs;
    neko_color_t color;
} neko_ui_imagecommand_t;

struct neko_ui_customcommand_t;

// Draw Callback
typedef void (*neko_ui_draw_callback_t)(neko_ui_context_t* ctx, struct neko_ui_customcommand_t* cmd);

typedef struct neko_ui_customcommand_t {
    neko_ui_basecommand_t base;
    neko_ui_rect_t clip;
    neko_ui_rect_t viewport;
    neko_ui_id hash;
    neko_ui_id hover;
    neko_ui_id focus;
    neko_ui_draw_callback_t cb;
    void* data;
    size_t sz;
} neko_ui_customcommand_t;

typedef struct {
    neko_ui_basecommand_t base;
    u32 type;
    union {
        neko_ui_rect_t rect;
        neko_ui_circle_t circle;
        neko_ui_triangle_t triangle;
        neko_ui_line_t line;
    };
    neko_color_t color;
} neko_ui_shapecommand_t;

// 注意 考虑到如何将异构类型推入字节缓冲区 这是浪费的
typedef union {
    s32 type;
    neko_ui_basecommand_t base;
    neko_ui_jumpcommand_t jump;
    neko_ui_clipcommand_t clip;
    neko_ui_shapecommand_t shape;
    neko_ui_textcommand_t text;
    neko_ui_imagecommand_t image;
    neko_ui_customcommand_t custom;
    neko_ui_pipelinecommand_t pipeline;
    neko_ui_binduniformscommand_t uniforms;
} neko_ui_command_t;

struct neko_ui_context_t;

typedef void (*neko_ui_on_draw_button_callback)(struct neko_ui_context_t* ctx, neko_ui_rect_t rect, neko_ui_id id, bool hovered, bool focused, u64 opt, const char* label, s32 icon);

typedef enum {
    NEKO_UI_LAYOUT_ANCHOR_TOPLEFT = 0x00,
    NEKO_UI_LAYOUT_ANCHOR_TOPCENTER,
    NEKO_UI_LAYOUT_ANCHOR_TOPRIGHT,
    NEKO_UI_LAYOUT_ANCHOR_LEFT,
    NEKO_UI_LAYOUT_ANCHOR_CENTER,
    NEKO_UI_LAYOUT_ANCHOR_RIGHT,
    NEKO_UI_LAYOUT_ANCHOR_BOTTOMLEFT,
    NEKO_UI_LAYOUT_ANCHOR_BOTTOMCENTER,
    NEKO_UI_LAYOUT_ANCHOR_BOTTOMRIGHT
} neko_ui_layout_anchor_type;

typedef enum { NEKO_UI_ALIGN_START = 0x00, NEKO_UI_ALIGN_CENTER, NEKO_UI_ALIGN_END } neko_ui_align_type;

typedef enum { NEKO_UI_JUSTIFY_START = 0x00, NEKO_UI_JUSTIFY_CENTER, NEKO_UI_JUSTIFY_END } neko_ui_justification_type;

typedef enum { NEKO_UI_DIRECTION_COLUMN = 0x00, NEKO_UI_DIRECTION_ROW, NEKO_UI_DIRECTION_COLUMN_REVERSE, NEKO_UI_DIRECTION_ROW_REVERSE } neko_ui_direction;

typedef struct neko_ui_layout_t {
    neko_ui_rect_t body;
    neko_ui_rect_t next;
    neko_vec2 position;
    neko_vec2 size;
    neko_vec2 max;
    s32 padding[4];
    s32 widths[NEKO_UI_MAX_WIDTHS];
    s32 items;
    s32 item_index;
    s32 next_row;
    s32 next_type;
    s32 indent;

    // flex direction / justification / alignment
    s32 direction;
    s32 justify_content;
    s32 align_content;

} neko_ui_layout_t;

// Forward decl.
struct neko_ui_container_t;

typedef enum neko_ui_split_node_type { NEKO_UI_SPLIT_NODE_CONTAINER = 0x00, NEKO_UI_SPLIT_NODE_SPLIT } neko_ui_split_node_type;

enum { NEKO_UI_SPLIT_NODE_CHILD = 0x00, NEKO_UI_SPLIT_NODE_PARENT };

typedef struct neko_ui_split_node_t {
    neko_ui_split_node_type type;
    union {
        u32 split;
        struct neko_ui_container_t* container;
    };
} neko_ui_split_node_t;

typedef enum neko_ui_split_type { NEKO_UI_SPLIT_LEFT = 0x00, NEKO_UI_SPLIT_RIGHT, NEKO_UI_SPLIT_TOP, NEKO_UI_SPLIT_BOTTOM, NEKO_UI_SPLIT_TAB } neko_ui_split_type;

typedef struct neko_ui_split_t {
    neko_ui_split_type type;  // NEKO_UI_SPLIT_LEFT, NEKO_UI_SPLIT_RIGHT, NEKO_UI_SPLIT_TAB, NEKO_UI_SPLIT_BOTTOM, NEKO_UI_SPLIT_TOP
    float ratio;              // Split ratio between children [0.f, 1.f], (left node = ratio), right node = (1.f - ratio)
    neko_ui_rect_t rect;
    neko_ui_rect_t prev_rect;
    neko_ui_split_node_t children[2];
    u32 parent;
    u32 id;
    u32 zindex;
    s32 frame;
} neko_ui_split_t;

typedef enum neko_ui_window_flags {
    NEKO_UI_WINDOW_FLAGS_VISIBLE = (1 << 0),     //
    NEKO_UI_WINDOW_FLAGS_FIRST_INIT = (1 << 1),  //
    NEKO_UI_WINDOW_FLAGS_PUSH_ID = (1 << 2)      //
} neko_ui_window_flags;

// Equidistantly sized tabs, based on rect of window
typedef struct neko_ui_tab_item_t {
    neko_ui_id tab_bar;
    u32 zindex;  // Sorting index in tab bar
    void* data;  // User set data pointer for this item
    u32 idx;     // Internal index
} neko_ui_tab_item_t;

typedef struct neko_ui_tab_bar_t {
    neko_ui_tab_item_t items[NEKO_UI_TAB_ITEM_MAX];
    u32 size;             // Current number of items in tab bar
    neko_ui_rect_t rect;  // Cached sized for tab bar
    u32 focus;            // Focused item in tab bar
} neko_ui_tab_bar_t;

typedef struct neko_ui_container_t {
    neko_ui_command_t *head, *tail;
    neko_ui_rect_t rect;
    neko_ui_rect_t body;
    neko_vec2 content_size;
    neko_vec2 scroll;
    s32 zindex;
    s32 open;
    neko_ui_id id;
    neko_ui_id split;  // If container is docked, then will have owning split to get sizing (0x00 for NULL)
    u32 tab_bar;
    u32 tab_item;
    struct neko_ui_container_t* parent;  // Owning parent (for tabbing)
    u64 opt;
    u32 frame;
    u32 visible;
    s32 flags;
    char name[256];
} neko_ui_container_t;

typedef enum {
    NEKO_UI_ELEMENT_STATE_NEG = -1,
    NEKO_UI_ELEMENT_STATE_DEFAULT = 0x00,
    NEKO_UI_ELEMENT_STATE_HOVER,
    NEKO_UI_ELEMENT_STATE_FOCUS,
    NEKO_UI_ELEMENT_STATE_COUNT,
    NEKO_UI_ELEMENT_STATE_ON_HOVER,
    NEKO_UI_ELEMENT_STATE_ON_FOCUS,
    NEKO_UI_ELEMENT_STATE_OFF_HOVER,
    NEKO_UI_ELEMENT_STATE_OFF_FOCUS
} neko_ui_element_state;

typedef enum neko_ui_element_type {
    NEKO_UI_ELEMENT_CONTAINER = 0x00,
    NEKO_UI_ELEMENT_LABEL,
    NEKO_UI_ELEMENT_TEXT,
    NEKO_UI_ELEMENT_PANEL,
    NEKO_UI_ELEMENT_INPUT,
    NEKO_UI_ELEMENT_BUTTON,
    NEKO_UI_ELEMENT_SCROLL,
    NEKO_UI_ELEMENT_IMAGE,
    NEKO_UI_ELEMENT_COUNT
} neko_ui_element_type;

typedef enum { NEKO_UI_PADDING_LEFT = 0x00, NEKO_UI_PADDING_RIGHT, NEKO_UI_PADDING_TOP, NEKO_UI_PADDING_BOTTOM } neko_ui_padding_type;

typedef enum { NEKO_UI_MARGIN_LEFT = 0x00, NEKO_UI_MARGIN_RIGHT, NEKO_UI_MARGIN_TOP, NEKO_UI_MARGIN_BOTTOM } neko_ui_margin_type;

typedef enum {

    // Width/Height
    NEKO_UI_STYLE_WIDTH = 0x00,
    NEKO_UI_STYLE_HEIGHT,

    // Padding
    NEKO_UI_STYLE_PADDING,
    NEKO_UI_STYLE_PADDING_LEFT,
    NEKO_UI_STYLE_PADDING_RIGHT,
    NEKO_UI_STYLE_PADDING_TOP,
    NEKO_UI_STYLE_PADDING_BOTTOM,

    NEKO_UI_STYLE_MARGIN,  // Can set margin for all at once, if -1.f then will assume 'auto' to simulate standard css
    NEKO_UI_STYLE_MARGIN_LEFT,
    NEKO_UI_STYLE_MARGIN_RIGHT,
    NEKO_UI_STYLE_MARGIN_TOP,
    NEKO_UI_STYLE_MARGIN_BOTTOM,

    // Border Radius
    NEKO_UI_STYLE_BORDER_RADIUS,
    NEKO_UI_STYLE_BORDER_RADIUS_LEFT,
    NEKO_UI_STYLE_BORDER_RADIUS_RIGHT,
    NEKO_UI_STYLE_BORDER_RADIUS_TOP,
    NEKO_UI_STYLE_BORDER_RADIUS_BOTTOM,

    // Border Width
    NEKO_UI_STYLE_BORDER_WIDTH,
    NEKO_UI_STYLE_BORDER_WIDTH_LEFT,
    NEKO_UI_STYLE_BORDER_WIDTH_RIGHT,
    NEKO_UI_STYLE_BORDER_WIDTH_TOP,
    NEKO_UI_STYLE_BORDER_WIDTH_BOTTOM,

    // Text
    NEKO_UI_STYLE_TEXT_ALIGN,

    // Flex
    NEKO_UI_STYLE_DIRECTION,
    NEKO_UI_STYLE_ALIGN_CONTENT,
    NEKO_UI_STYLE_JUSTIFY_CONTENT,  // Justify runs parallel to direction (ex. for row, left to right)

    // Shadow
    NEKO_UI_STYLE_SHADOW_X,
    NEKO_UI_STYLE_SHADOW_Y,

    // Colors
    NEKO_UI_STYLE_COLOR_BACKGROUND,
    NEKO_UI_STYLE_COLOR_BORDER,
    NEKO_UI_STYLE_COLOR_SHADOW,
    NEKO_UI_STYLE_COLOR_CONTENT,
    NEKO_UI_STYLE_COLOR_CONTENT_BACKGROUND,
    NEKO_UI_STYLE_COLOR_CONTENT_BORDER,
    NEKO_UI_STYLE_COLOR_CONTENT_SHADOW,

    // Font
    NEKO_UI_STYLE_FONT,

    NEKO_UI_STYLE_COUNT

} neko_ui_style_element_type;

enum { NEKO_UI_ANIMATION_DIRECTION_FORWARD = 0x00, NEKO_UI_ANIMATION_DIRECTION_BACKWARD };

typedef struct {
    neko_ui_style_element_type type;
    union {
        s32 value;
        neko_color_t color;
        neko_asset_ascii_font_t* font;
    };
} neko_ui_style_element_t;

typedef struct neko_ui_animation_t {
    s16 max;          // max time
    s16 time;         // current time
    s16 delay;        // delay
    s16 curve;        // curve type
    s16 direction;    // current direction
    s16 playing;      // whether or not active
    s16 iterations;   // number of iterations to play the animation
    s16 focus_state;  // cached focus_state from frame (want to delete this somehow)
    s16 hover_state;  // cached hover_state from frame (want to delete this somehow)
    s16 start_state;  // starting state for animation blend
    s16 end_state;    // ending state for animation blend
    s32 frame;        // current frame (to match)
} neko_ui_animation_t;

typedef struct neko_ui_style_t {
    // font
    neko_asset_ascii_font_t* font;

    // dimensions
    float size[2];
    s16 spacing;         // get rid of    (use padding)
    s16 indent;          // get rid of    (use margin)
    s16 title_height;    // get rid of    (use title_bar style)
    s16 scrollbar_size;  // get rid of    (use scroll style)
    s16 thumb_size;      // get rid of    (use various styles)

    // colors
    neko_color_t colors[NEKO_UI_COLOR_MAX];

    // padding/margin
    s32 padding[4];
    s16 margin[4];

    // border
    s16 border_width[4];
    s16 border_radius[4];

    // flex direction / justification / alignment
    s16 direction;
    s16 justify_content;
    s16 align_content;

    // shadow amount each direction
    s16 shadow_x;
    s16 shadow_y;

} neko_ui_style_t;

// Keep animation properties lists within style sheet to look up

typedef struct neko_ui_animation_property_t {
    neko_ui_style_element_type type;
    s16 time;
    s16 delay;
} neko_ui_animation_property_t;

typedef struct neko_ui_animation_property_list_t {
    neko_dyn_array(neko_ui_animation_property_t) properties[3];
} neko_ui_animation_property_list_t;

/*
   element type
   classes
   id

   neko_ui_button(gui, "Text##.cls#id");
   neko_ui_label(gui, "Title###title");

    button .class #id : hover {         // All of these styles get concat into one?
    }
*/

typedef struct {
    neko_dyn_array(neko_ui_style_element_t) styles[3];
} neko_ui_style_list_t;

typedef struct neko_ui_style_sheet_t {
    neko_ui_style_t styles[NEKO_UI_ELEMENT_COUNT][3];  // default | hovered | focused
    neko_hash_table(neko_ui_element_type, neko_ui_animation_property_list_t) animations;

    neko_hash_table(u64, neko_ui_style_list_t) cid_styles;
    neko_hash_table(u64, neko_ui_animation_property_list_t) cid_animations;
} neko_ui_style_sheet_t;

typedef struct neko_ui_style_sheet_element_desc_t {

    struct {

        struct {
            neko_ui_style_element_t* data;
            size_t size;
        } style;

        struct {
            neko_ui_animation_property_t* data;
            size_t size;
        } animation;

    } all;

    struct {

        struct {
            neko_ui_style_element_t* data;
            size_t size;
        } style;

        struct {
            neko_ui_animation_property_t* data;
            size_t size;
        } animation;

    } def;

    struct {
        struct {
            neko_ui_style_element_t* data;
            size_t size;
        } style;

        struct {
            neko_ui_animation_property_t* data;
            size_t size;
        } animation;
    } hover;

    struct {
        struct {
            neko_ui_style_element_t* data;
            size_t size;
        } style;

        struct {
            neko_ui_animation_property_t* data;
            size_t size;
        } animation;
    } focus;

} neko_ui_style_sheet_element_desc_t;

typedef neko_ui_style_sheet_element_desc_t neko_ui_inline_style_desc_t;

typedef struct neko_ui_style_sheet_desc_t {
    neko_ui_style_sheet_element_desc_t container;
    neko_ui_style_sheet_element_desc_t button;
    neko_ui_style_sheet_element_desc_t panel;
    neko_ui_style_sheet_element_desc_t input;
    neko_ui_style_sheet_element_desc_t text;
    neko_ui_style_sheet_element_desc_t label;
    neko_ui_style_sheet_element_desc_t scroll;
    neko_ui_style_sheet_element_desc_t tab;
    neko_ui_style_sheet_element_desc_t menu;
    neko_ui_style_sheet_element_desc_t title;
    neko_ui_style_sheet_element_desc_t image;
} neko_ui_style_sheet_desc_t;

typedef enum neko_ui_request_type {
    NEKO_UI_SPLIT_NEW = 0x01,
    NEKO_UI_CNT_MOVE,
    NEKO_UI_CNT_FOCUS,
    NEKO_UI_SPLIT_MOVE,
    NEKO_UI_SPLIT_RESIZE_SW,
    NEKO_UI_SPLIT_RESIZE_SE,
    NEKO_UI_SPLIT_RESIZE_NW,
    NEKO_UI_SPLIT_RESIZE_NE,
    NEKO_UI_SPLIT_RESIZE_W,
    NEKO_UI_SPLIT_RESIZE_E,
    NEKO_UI_SPLIT_RESIZE_N,
    NEKO_UI_SPLIT_RESIZE_S,
    NEKO_UI_SPLIT_RESIZE_CENTER,
    NEKO_UI_SPLIT_RATIO,
    NEKO_UI_SPLIT_RESIZE_INVALID,
    NEKO_UI_TAB_SWAP_LEFT,
    NEKO_UI_TAB_SWAP_RIGHT
} neko_ui_request_type;

typedef struct neko_ui_request_t {
    neko_ui_request_type type;
    union {
        neko_ui_split_type split_type;
        neko_ui_split_t* split;
        neko_ui_container_t* cnt;
    };
    u32 frame;
} neko_ui_request_t;

typedef struct neko_ui_inline_style_stack_t {
    neko_dyn_array(neko_ui_style_element_t) styles[3];
    neko_dyn_array(neko_ui_animation_property_t) animations[3];
    neko_dyn_array(u32) style_counts;      // amount of styles to pop off at "top of stack" for each state
    neko_dyn_array(u32) animation_counts;  // amount of animations to pop off at "top of stack" for each state
} neko_ui_inline_style_stack_t;

typedef struct neko_ui_context_t {
    // Core state
    neko_ui_style_t* style;              // Active style
    neko_ui_style_sheet_t* style_sheet;  // Active style sheet
    neko_ui_id hover;
    neko_ui_id focus;
    neko_ui_id last_id;
    neko_ui_id state_switch_id;  // Id that had a state switch
    s32 switch_state;
    neko_ui_id lock_focus;
    s32 last_hover_state;
    s32 last_focus_state;
    neko_ui_id prev_hover;
    neko_ui_id prev_focus;
    neko_ui_rect_t last_rect;
    s32 last_zindex;
    s32 updated_focus;
    s32 frame;
    neko_vec2 framebuffer_size;
    neko_ui_rect_t viewport;
    neko_ui_container_t* active_root;
    neko_ui_container_t* hover_root;
    neko_ui_container_t* next_hover_root;
    neko_ui_container_t* scroll_target;
    neko_ui_container_t* focus_root;
    neko_ui_container_t* next_focus_root;
    neko_ui_container_t* dockable_root;
    neko_ui_container_t* prev_dockable_root;
    neko_ui_container_t* docked_root;
    neko_ui_container_t* undock_root;
    neko_ui_split_t* focus_split;
    neko_ui_split_t* next_hover_split;
    neko_ui_split_t* hover_split;
    neko_ui_id next_lock_hover_id;
    neko_ui_id lock_hover_id;
    char number_edit_buf[NEKO_UI_MAX_FMT];
    neko_ui_id number_edit;
    neko_ui_alt_drag_mode_type alt_drag_mode;
    neko_dyn_array(neko_ui_request_t) requests;

    // Stacks
    neko_ui_stack(u8, NEKO_UI_COMMANDLIST_SIZE) command_list;
    neko_ui_stack(neko_ui_container_t*, NEKO_UI_ROOTLIST_SIZE) root_list;
    neko_ui_stack(neko_ui_container_t*, NEKO_UI_CONTAINERSTACK_SIZE) container_stack;
    neko_ui_stack(neko_ui_rect_t, NEKO_UI_CLIPSTACK_SIZE) clip_stack;
    neko_ui_stack(neko_ui_id, NEKO_UI_IDSTACK_SIZE) id_stack;
    neko_ui_stack(neko_ui_layout_t, NEKO_UI_LAYOUTSTACK_SIZE) layout_stack;

    // Style sheet element stacks
    neko_hash_table(neko_ui_element_type, neko_ui_inline_style_stack_t) inline_styles;

    // Retained state pools
    neko_ui_pool_item_t container_pool[NEKO_UI_CONTAINERPOOL_SIZE];
    neko_ui_container_t containers[NEKO_UI_CONTAINERPOOL_SIZE];
    neko_ui_pool_item_t treenode_pool[NEKO_UI_TREENODEPOOL_SIZE];

    neko_slot_array(neko_ui_split_t) splits;
    neko_slot_array(neko_ui_tab_bar_t) tab_bars;

    // Input state
    neko_vec2 mouse_pos;
    neko_vec2 last_mouse_pos;
    neko_vec2 mouse_delta;
    neko_vec2 scroll_delta;
    s32 mouse_down;
    s32 mouse_pressed;
    s32 key_down;
    s32 key_pressed;
    char input_text[32];

    // Backend resources
    u32 window_hndl;
    neko_immediate_draw_t gui_idraw;
    neko_immediate_draw_t overlay_draw_list;

    // Active Transitions
    neko_hash_table(neko_ui_id, neko_ui_animation_t) animations;

    // Font stash
    neko_hash_table(u64, neko_asset_ascii_font_t*) font_stash;

    // Callbacks
    struct {
        neko_ui_on_draw_button_callback button;
    } callbacks;

} neko_ui_context_t;

typedef struct {
    // Core state
    neko_ui_style_t* style;              // Active style
    neko_ui_style_sheet_t* style_sheet;  // Active style sheet
    neko_ui_id hover;
    neko_ui_id focus;
    neko_ui_id last_id;
    neko_ui_id lock_focus;
    neko_ui_rect_t last_rect;
    s32 last_zindex;
    s32 updated_focus;
    s32 frame;
    neko_ui_container_t* hover_root;
    neko_ui_container_t* next_hover_root;
    neko_ui_container_t* scroll_target;
    neko_ui_container_t* focus_root;
    neko_ui_container_t* next_focus_root;
    neko_ui_container_t* dockable_root;
    neko_ui_container_t* prev_dockable_root;
    neko_ui_container_t* docked_root;
    neko_ui_container_t* undock_root;
    neko_ui_split_t* focus_split;
    neko_ui_split_t* next_hover_split;
    neko_ui_split_t* hover_split;
    neko_ui_id next_lock_hover_id;
    neko_ui_id lock_hover_id;
    char number_edit_buf[NEKO_UI_MAX_FMT];
    neko_ui_id number_edit;
    neko_ui_alt_drag_mode_type alt_drag_mode;
    neko_dyn_array(neko_ui_request_t) requests;

    // Stacks
    neko_ui_stack(neko_ui_container_t*, NEKO_UI_CONTAINERSTACK_SIZE) container_stack;

    neko_dyn_array(u8) command_list;
    neko_dyn_array(neko_ui_container_t*) root_list;
    neko_dyn_array(neko_ui_rect_t) clip_stack;
    neko_dyn_array(neko_ui_id) id_stack;
    neko_dyn_array(neko_ui_layout_t) layout_stack;

    // Retained state pools
    neko_ui_pool_item_t container_pool[NEKO_UI_CONTAINERPOOL_SIZE];
    neko_ui_pool_item_t treenode_pool[NEKO_UI_TREENODEPOOL_SIZE];

    neko_slot_array(neko_ui_split_t) splits;
    neko_slot_array(neko_ui_tab_bar_t) tab_bars;

    // Input state
    neko_vec2 mouse_pos;
    neko_vec2 last_mouse_pos;
    neko_vec2 mouse_delta;
    neko_vec2 scroll_delta;
    s16 mouse_down;
    s16 mouse_pressed;
    s16 key_down;
    s16 key_pressed;
    char input_text[32];

    // Backend resources
    u32 window_hndl;
    neko_immediate_draw_t gui_idraw;
    neko_immediate_draw_t overlay_draw_list;

    // Active Transitions
    neko_hash_table(neko_ui_id, neko_ui_animation_t) animations;

    // Callbacks
    struct {
        neko_ui_on_draw_button_callback button;
    } callbacks;

} neko_ui_context_pruned_t;

typedef struct {
    const char* key;
    neko_asset_ascii_font_t* font;
} neko_ui_font_desc_t;

typedef struct {
    neko_ui_font_desc_t* fonts;
    size_t size;
} neko_ui_font_stash_desc_t;

typedef struct {
    const char* id;                                 // Id selector
    const char* classes[NEKO_UI_CLS_SELECTOR_MAX];  // Class selectors
} neko_ui_selector_desc_t;

enum { NEKO_UI_HINT_FLAG_NO_SCALE_BIAS_MOUSE = (1 << 0), NEKO_UI_HINT_FLAG_NO_INVERT_Y = (1 << 1) };

typedef struct neko_ui_hints_s {
    neko_vec2 framebuffer_size;  // Overall framebuffer size
    neko_ui_rect_t viewport;     // Viewport within framebuffer for gui context
    s32 flags;                   // Flags for hints
} neko_ui_hints_t;

NEKO_API_DECL neko_ui_rect_t neko_ui_rect(float x, float y, float w, float h);

//=== Context ===//

NEKO_API_DECL neko_ui_context_t neko_ui_new(u32 window_hndl);
NEKO_API_DECL void neko_ui_init(neko_ui_context_t* ctx, u32 window_hndl);
NEKO_API_DECL void neko_ui_init_font_stash(neko_ui_context_t* ctx, neko_ui_font_stash_desc_t* desc);
NEKO_API_DECL neko_ui_context_t neko_ui_context_new(u32 window_hndl);
NEKO_API_DECL void neko_ui_free(neko_ui_context_t* ctx);
NEKO_API_DECL void neko_ui_begin(neko_ui_context_t* ctx, const neko_ui_hints_t* hints);
NEKO_API_DECL void neko_ui_end(neko_ui_context_t* ctx, b32 update);
NEKO_API_DECL void neko_ui_render(neko_ui_context_t* ctx, neko_command_buffer_t* cb);

//=== Util ===//
NEKO_API_DECL void neko_ui_renderpass_submit(neko_ui_context_t* ctx, neko_command_buffer_t* cb, neko_color_t clear);
NEKO_API_DECL void neko_ui_renderpass_submit_ex(neko_ui_context_t* ctx, neko_command_buffer_t* cb, neko_render_clear_action_t action);
NEKO_API_DECL void neko_ui_parse_id_tag(neko_ui_context_t* ctx, const char* str, char* buffer, size_t sz, u64 opt);
NEKO_API_DECL void neko_ui_parse_label_tag(neko_ui_context_t* ctx, const char* str, char* buffer, size_t sz);

//=== Main API ===//

NEKO_API_DECL void neko_ui_set_focus(neko_ui_context_t* ctx, neko_ui_id id);
NEKO_API_DECL void neko_ui_set_hover(neko_ui_context_t* ctx, neko_ui_id id);
NEKO_API_DECL neko_ui_id neko_ui_get_id(neko_ui_context_t* ctx, const void* data, s32 size);
NEKO_API_DECL void neko_ui_push_id(neko_ui_context_t* ctx, const void* data, s32 size);
NEKO_API_DECL void neko_ui_pop_id(neko_ui_context_t* ctx);
NEKO_API_DECL void neko_ui_push_clip_rect(neko_ui_context_t* ctx, neko_ui_rect_t rect);
NEKO_API_DECL void neko_ui_pop_clip_rect(neko_ui_context_t* ctx);
NEKO_API_DECL neko_ui_rect_t neko_ui_get_clip_rect(neko_ui_context_t* ctx);
NEKO_API_DECL s32 neko_ui_check_clip(neko_ui_context_t* ctx, neko_ui_rect_t r);
NEKO_API_DECL s32 neko_ui_mouse_over(neko_ui_context_t* ctx, neko_ui_rect_t rect);
NEKO_API_DECL void neko_ui_update_control(neko_ui_context_t* ctx, neko_ui_id id, neko_ui_rect_t rect, u64 opt);

//=== Conatiner ===//

NEKO_API_DECL neko_ui_container_t* neko_ui_get_current_container(neko_ui_context_t* ctx);
NEKO_API_DECL neko_ui_container_t* neko_ui_get_container(neko_ui_context_t* ctx, const char* name);
NEKO_API_DECL neko_ui_container_t* neko_ui_get_top_most_container(neko_ui_context_t* ctx, neko_ui_split_t* split);
NEKO_API_DECL neko_ui_container_t* neko_ui_get_container_ex(neko_ui_context_t* ctx, neko_ui_id id, u64 opt);
NEKO_API_DECL void neko_ui_bring_to_front(neko_ui_context_t* ctx, neko_ui_container_t* cnt);
NEKO_API_DECL void neko_ui_bring_split_to_front(neko_ui_context_t* ctx, neko_ui_split_t* split);
NEKO_API_DECL neko_ui_split_t* neko_ui_get_split(neko_ui_context_t* ctx, neko_ui_container_t* cnt);
NEKO_API_DECL neko_ui_tab_bar_t* neko_ui_get_tab_bar(neko_ui_context_t* ctx, neko_ui_container_t* cnt);
NEKO_API_DECL void neko_ui_tab_item_swap(neko_ui_context_t* ctx, neko_ui_container_t* cnt, s32 direction);
NEKO_API_DECL neko_ui_container_t* neko_ui_get_root_container(neko_ui_context_t* ctx, neko_ui_container_t* cnt);
NEKO_API_DECL neko_ui_container_t* neko_ui_get_root_container_from_split(neko_ui_context_t* ctx, neko_ui_split_t* split);
NEKO_API_DECL neko_ui_container_t* neko_ui_get_parent(neko_ui_context_t* ctx, neko_ui_container_t* cnt);
NEKO_API_DECL void neko_ui_current_container_close(neko_ui_context_t* ctx);

//=== Animation ===//

NEKO_API_DECL neko_ui_animation_t* neko_ui_get_animation(neko_ui_context_t* ctx, neko_ui_id id, const neko_ui_selector_desc_t* desc, s32 elementid);

NEKO_API_DECL neko_ui_style_t neko_ui_animation_get_blend_style(neko_ui_context_t* ctx, neko_ui_animation_t* anim, const neko_ui_selector_desc_t* desc, s32 elementid);

//=== Style Sheet ===//

NEKO_API_DECL neko_ui_style_sheet_t neko_ui_style_sheet_create(neko_ui_context_t* ctx, neko_ui_style_sheet_desc_t* desc);
NEKO_API_DECL void neko_ui_style_sheet_destroy(neko_ui_style_sheet_t* ss);
NEKO_API_DECL void neko_ui_set_element_style(neko_ui_context_t* ctx, neko_ui_element_type element, neko_ui_element_state state, neko_ui_style_element_t* style, size_t size);
NEKO_API_DECL void neko_ui_style_sheet_set_element_styles(neko_ui_style_sheet_t* style_sheet, neko_ui_element_type element, neko_ui_element_state state, neko_ui_style_element_t* styles, size_t size);
NEKO_API_DECL void neko_ui_set_style_sheet(neko_ui_context_t* ctx, neko_ui_style_sheet_t* style_sheet);

NEKO_API_DECL void neko_ui_push_inline_style(neko_ui_context_t* ctx, neko_ui_element_type elementid, neko_ui_inline_style_desc_t* desc);
NEKO_API_DECL void neko_ui_pop_inline_style(neko_ui_context_t* ctx, neko_ui_element_type elementid);

NEKO_API_DECL neko_ui_style_t* neko_ui_push_style(neko_ui_context_t* ctx, neko_ui_style_t* style);
NEKO_API_DECL void neko_ui_pop_style(neko_ui_context_t* ctx, neko_ui_style_t* style);

//=== Resource Loading ===//

NEKO_API_DECL neko_ui_style_sheet_t neko_ui_style_sheet_load_from_file(neko_ui_context_t* ctx, const char* file_path);
NEKO_API_DECL neko_ui_style_sheet_t neko_ui_style_sheet_load_from_memory(neko_ui_context_t* ctx, const char* memory, size_t sz, bool* success);

//=== Pools ===//

NEKO_API_DECL s32 neko_ui_pool_init(neko_ui_context_t* ctx, neko_ui_pool_item_t* items, s32 len, neko_ui_id id);
NEKO_API_DECL s32 neko_ui_pool_get(neko_ui_context_t* ctx, neko_ui_pool_item_t* items, s32 len, neko_ui_id id);
NEKO_API_DECL void neko_ui_pool_update(neko_ui_context_t* ctx, neko_ui_pool_item_t* items, s32 idx);

//=== Input ===//

NEKO_API_DECL void neko_ui_input_mousemove(neko_ui_context_t* ctx, s32 x, s32 y);
NEKO_API_DECL void neko_ui_input_mousedown(neko_ui_context_t* ctx, s32 x, s32 y, s32 btn);
NEKO_API_DECL void neko_ui_input_mouseup(neko_ui_context_t* ctx, s32 x, s32 y, s32 btn);
NEKO_API_DECL void neko_ui_input_scroll(neko_ui_context_t* ctx, s32 x, s32 y);
NEKO_API_DECL void neko_ui_input_keydown(neko_ui_context_t* ctx, s32 key);
NEKO_API_DECL void neko_ui_input_keyup(neko_ui_context_t* ctx, s32 key);
NEKO_API_DECL void neko_ui_input_text(neko_ui_context_t* ctx, const char* text);

//=== Commands ===//

NEKO_API_DECL neko_ui_command_t* neko_ui_push_command(neko_ui_context_t* ctx, s32 type, s32 size);
NEKO_API_DECL s32 neko_ui_next_command(neko_ui_context_t* ctx, neko_ui_command_t** cmd);
NEKO_API_DECL void neko_ui_set_clip(neko_ui_context_t* ctx, neko_ui_rect_t rect);
NEKO_API_DECL void neko_ui_set_pipeline(neko_ui_context_t* ctx, neko_handle(neko_render_pipeline_t) pip, void* layout, size_t layout_sz, neko_idraw_layout_type layout_type);
NEKO_API_DECL void neko_ui_bind_uniforms(neko_ui_context_t* ctx, neko_render_bind_uniform_desc_t* uniforms, size_t uniforms_sz);

//=== Drawing ===//

NEKO_API_DECL void neko_ui_draw_rect(neko_ui_context_t* ctx, neko_ui_rect_t rect, neko_color_t color);
NEKO_API_DECL void neko_ui_draw_circle(neko_ui_context_t* ctx, neko_vec2 position, float radius, neko_color_t color);
NEKO_API_DECL void neko_ui_draw_triangle(neko_ui_context_t* ctx, neko_vec2 a, neko_vec2 b, neko_vec2 c, neko_color_t color);
NEKO_API_DECL void neko_ui_draw_box(neko_ui_context_t* ctx, neko_ui_rect_t rect, s16* width, neko_color_t color);
NEKO_API_DECL void neko_ui_draw_line(neko_ui_context_t* ctx, neko_vec2 start, neko_vec2 end, neko_color_t color);
NEKO_API_DECL void neko_ui_draw_text(neko_ui_context_t* ctx, neko_asset_ascii_font_t* font, const char* str, s32 len, neko_vec2 pos, neko_color_t color, s32 shadow_x, s32 shadow_y,
                                     neko_color_t shadow_color);
NEKO_API_DECL void neko_ui_draw_image(neko_ui_context_t* ctx, neko_handle(neko_render_texture_t) hndl, neko_ui_rect_t rect, neko_vec2 uv0, neko_vec2 uv1, neko_color_t color);
NEKO_API_DECL void neko_ui_draw_nine_rect(neko_ui_context_t* ctx, neko_handle(neko_render_texture_t) hndl, neko_ui_rect_t rect, neko_vec2 uv0, neko_vec2 uv1, u32 left, u32 right, u32 top, u32 bottom,
                                          neko_color_t color);
NEKO_API_DECL void neko_ui_draw_control_frame(neko_ui_context_t* ctx, neko_ui_id id, neko_ui_rect_t rect, s32 elementid, u64 opt);
NEKO_API_DECL void neko_ui_draw_control_text(neko_ui_context_t* ctx, const char* str, neko_ui_rect_t rect, const neko_ui_style_t* style, u64 opt);
NEKO_API_DECL void neko_ui_draw_custom(neko_ui_context_t* ctx, neko_ui_rect_t rect, neko_ui_draw_callback_t cb, void* data, size_t sz);

//=== Layout ===//

NEKO_API_DECL neko_ui_layout_t* neko_ui_get_layout(neko_ui_context_t* ctx);
NEKO_API_DECL void neko_ui_layout_row(neko_ui_context_t* ctx, s32 items, const s32* widths, s32 height);
NEKO_API_DECL void neko_ui_layout_row_ex(neko_ui_context_t* ctx, s32 items, const s32* widths, s32 height, s32 justification);
NEKO_API_DECL void neko_ui_layout_width(neko_ui_context_t* ctx, s32 width);
NEKO_API_DECL void neko_ui_layout_height(neko_ui_context_t* ctx, s32 height);
NEKO_API_DECL void neko_ui_layout_column_begin(neko_ui_context_t* ctx);
NEKO_API_DECL void neko_ui_layout_column_end(neko_ui_context_t* ctx);
NEKO_API_DECL void neko_ui_layout_set_next(neko_ui_context_t* ctx, neko_ui_rect_t r, s32 relative);
NEKO_API_DECL neko_ui_rect_t neko_ui_layout_peek_next(neko_ui_context_t* ctx);
NEKO_API_DECL neko_ui_rect_t neko_ui_layout_next(neko_ui_context_t* ctx);
NEKO_API_DECL neko_ui_rect_t neko_ui_layout_anchor(const neko_ui_rect_t* parent, s32 width, s32 height, s32 xoff, s32 yoff, neko_ui_layout_anchor_type type);

//=== Elements ===//

#define neko_ui_button(_CTX, _LABEL) neko_ui_button_ex((_CTX), (_LABEL), NULL, NEKO_UI_OPT_LEFTCLICKONLY)
#define neko_ui_text(_CTX, _TXT) neko_ui_text_ex((_CTX), (_TXT), 1, NULL, 0x00)
// #define neko_ui_text_fc(_CTX, _TXT) neko_ui_text_fc_ex((_CTX), (_TXT), (-1))
#define neko_ui_textbox(_CTX, _BUF, _BUFSZ) neko_ui_textbox_ex((_CTX), (_BUF), (_BUFSZ), NULL, 0x00)
#define neko_ui_slider(_CTX, _VALUE, _LO, _HI) neko_ui_slider_ex((_CTX), (_VALUE), (_LO), (_HI), 0, NEKO_UI_SLIDER_FMT, NULL, 0x00)
#define neko_ui_number(_CTX, _VALUE, _STEP) neko_ui_number_ex((_CTX), (_VALUE), (_STEP), NEKO_UI_SLIDER_FMT, NULL, 0x00)
#define neko_ui_header(_CTX, _LABEL) neko_ui_header_ex((_CTX), (_LABEL), NULL, 0x00)
#define neko_ui_checkbox(_CTX, _LABEL, _STATE) neko_ui_checkbox_ex((_CTX), (_LABEL), (_STATE), NULL, NEKO_UI_OPT_LEFTCLICKONLY)
#define neko_ui_treenode_begin(_CTX, _LABEL) neko_ui_treenode_begin_ex((_CTX), (_LABEL), NULL, 0x00)
#define neko_ui_window_begin(_CTX, _TITLE, _RECT) neko_ui_window_begin_ex((_CTX), (_TITLE), (_RECT), 0, NULL, 0x00)
#define neko_ui_popup_begin(_CTX, _TITLE, _RECT) neko_ui_popup_begin_ex((_CTX), (_TITLE), (_RECT), NULL, 0x00)
#define neko_ui_panel_begin(_CTX, _NAME) neko_ui_panel_begin_ex((_CTX), (_NAME), NULL, 0x00)
#define neko_ui_image(_CTX, _HNDL) neko_ui_image_ex((_CTX), (_HNDL), neko_v2s(0.f), neko_v2s(1.f), NULL, 0x00)
#define neko_ui_combo_begin(_CTX, _ID, _ITEM, _MAX) neko_ui_combo_begin_ex((_CTX), (_ID), (_ITEM), (_MAX), NULL, 0x00)
#define neko_ui_combo_item(_CTX, _NAME) neko_ui_combo_item_ex((_CTX), (_NAME), NULL, 0x00)
#define neko_ui_dock(_CTX, _DST, _SRC, _TYPE) neko_ui_dock_ex((_CTX), (_DST), (_SRC), (_TYPE), 0.5f)
#define neko_ui_undock(_CTX, _NAME) neko_ui_undock_ex((_CTX), (_NAME))
#define neko_ui_label(_CTX, _FMT, ...) (neko_snprintf((_CTX)->number_edit_buf, sizeof((_CTX)->number_edit_buf), _FMT, ##__VA_ARGS__), neko_ui_label_ex((_CTX), (_CTX)->number_edit_buf, NULL, 0x00))
#define neko_ui_labelf(STR, ...)                         \
    do {                                                 \
        neko_snprintfc(BUFFER, 256, STR, ##__VA_ARGS__); \
        neko_ui_label(&CL_GAME_USERDATA()->ui, BUFFER);  \
    } while (0)

//=== Elements (Extended) ===//

NEKO_API_DECL s32 neko_ui_image_ex(neko_ui_context_t* ctx, neko_handle(neko_render_texture_t) hndl, neko_vec2 uv0, neko_vec2 uv1, const neko_ui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL s32 neko_ui_text_ex(neko_ui_context_t* ctx, const char* text, s32 text_wrap, const neko_ui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL s32 neko_ui_label_ex(neko_ui_context_t* ctx, const char* text, const neko_ui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL s32 neko_ui_button_ex(neko_ui_context_t* ctx, const char* label, const neko_ui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL s32 neko_ui_checkbox_ex(neko_ui_context_t* ctx, const char* label, s32* state, const neko_ui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL s32 neko_ui_textbox_raw(neko_ui_context_t* ctx, char* buf, s32 bufsz, neko_ui_id id, neko_ui_rect_t r, const neko_ui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL s32 neko_ui_textbox_ex(neko_ui_context_t* ctx, char* buf, s32 bufsz, const neko_ui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL s32 neko_ui_slider_ex(neko_ui_context_t* ctx, neko_ui_real* value, neko_ui_real low, neko_ui_real high, neko_ui_real step, const char* fmt, const neko_ui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL s32 neko_ui_number_ex(neko_ui_context_t* ctx, neko_ui_real* value, neko_ui_real step, const char* fmt, const neko_ui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL s32 neko_ui_header_ex(neko_ui_context_t* ctx, const char* label, const neko_ui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL s32 neko_ui_treenode_begin_ex(neko_ui_context_t* ctx, const char* label, const neko_ui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL void neko_ui_treenode_end(neko_ui_context_t* ctx);
NEKO_API_DECL s32 neko_ui_window_begin_ex(neko_ui_context_t* ctx, const char* title, neko_ui_rect_t rect, bool* open, const neko_ui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL void neko_ui_window_end(neko_ui_context_t* ctx);
NEKO_API_DECL void neko_ui_popup_open(neko_ui_context_t* ctx, const char* name);
NEKO_API_DECL s32 neko_ui_popup_begin_ex(neko_ui_context_t* ctx, const char* name, neko_ui_rect_t r, const neko_ui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL void neko_ui_popup_end(neko_ui_context_t* ctx);
NEKO_API_DECL void neko_ui_panel_begin_ex(neko_ui_context_t* ctx, const char* name, const neko_ui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL void neko_ui_panel_end(neko_ui_context_t* ctx);
NEKO_API_DECL s32 neko_ui_combo_begin_ex(neko_ui_context_t* ctx, const char* id, const char* current_item, s32 max_items, neko_ui_selector_desc_t* desc, u64 opt);
NEKO_API_DECL void neko_ui_combo_end(neko_ui_context_t* ctx);
NEKO_API_DECL s32 neko_ui_combo_item_ex(neko_ui_context_t* ctx, const char* label, const neko_ui_selector_desc_t* desc, u64 opt);

//=== Demos ===//

NEKO_API_DECL s32 neko_ui_style_editor(neko_ui_context_t* ctx, neko_ui_style_sheet_t* style_sheet, neko_ui_rect_t rect, bool* open);
NEKO_API_DECL s32 neko_ui_demo_window(neko_ui_context_t* ctx, neko_ui_rect_t rect, bool* open);

//=== Docking ===//

NEKO_API_DECL void neko_ui_dock_ex(neko_ui_context_t* ctx, const char* dst, const char* src, s32 split_type, float ratio);
NEKO_API_DECL void neko_ui_undock_ex(neko_ui_context_t* ctx, const char* name);
NEKO_API_DECL void neko_ui_dock_ex_cnt(neko_ui_context_t* ctx, neko_ui_container_t* dst, neko_ui_container_t* src, s32 split_type, float ratio);
NEKO_API_DECL void neko_ui_undock_ex_cnt(neko_ui_context_t* ctx, neko_ui_container_t* cnt);

//=== Gizmo ===//

NEKO_API_DECL s32 neko_ui_gizmo(neko_ui_context_t* ctx, neko_camera_t* camera, neko_vqs* model, neko_ui_rect_t viewport, bool invert_view_y, float snap, s32 op, s32 mode, u64 opt);

#endif

#ifndef NEKO_BATCH
#define NEKO_BATCH

typedef struct neko_spritebatch_t neko_spritebatch_t;
typedef struct neko_spritebatch_config_t neko_spritebatch_config_t;
typedef struct neko_spritebatch_sprite_t neko_spritebatch_sprite_t;

struct neko_spritebatch_sprite_t {

    u64 image_id;

    u64 texture_id;

    int w, h;
    float x, y;
    float sx, sy;
    float c, s;
    float minx, miny;
    float maxx, maxy;

    int sort_bits;

#ifdef SPRITEBATCH_SPRITE_USERDATA
    SPRITEBATCH_SPRITE_USERDATA udata;
#endif
};

NEKO_API_DECL int neko_spritebatch_push(neko_spritebatch_t* sb, neko_spritebatch_sprite_t sprite);
NEKO_API_DECL void neko_spritebatch_prefetch(neko_spritebatch_t* sb, u64 image_id, int w, int h);
NEKO_API_DECL struct neko_spritebatch_sprite_t neko_spritebatch_fetch(neko_spritebatch_t* sb, u64 image_id, int w, int h);
NEKO_API_DECL void neko_spritebatch_tick(neko_spritebatch_t* sb);
NEKO_API_DECL int neko_spritebatch_flush(neko_spritebatch_t* sb);
NEKO_API_DECL int neko_spritebatch_defrag(neko_spritebatch_t* sb);
NEKO_API_DECL int neko_spritebatch_init(neko_spritebatch_t* sb, neko_spritebatch_config_t* config, void* udata);
NEKO_API_DECL void neko_spritebatch_term(neko_spritebatch_t* sb);
NEKO_API_DECL void neko_spritebatch_register_premade_atlas(neko_spritebatch_t* sb, u64 image_id, int w, int h);
NEKO_API_DECL void neko_spritebatch_cleanup_premade_atlas(neko_spritebatch_t* sb, u64 image_id);

typedef void(submit_batch_fn)(neko_spritebatch_sprite_t* sprites, int count, int texture_w, int texture_h, void* udata);
typedef void(get_pixels_fn)(u64 image_id, void* buffer, int bytes_to_fill, void* udata);
typedef u64(generate_texture_handle_fn)(void* pixels, int w, int h, void* udata);
typedef void(destroy_texture_handle_fn)(u64 texture_id, void* udata);
typedef void(sprites_sorter_fn)(neko_spritebatch_sprite_t* sprites, int count);

NEKO_API_DECL void neko_spritebatch_reset_function_ptrs(neko_spritebatch_t* sb, submit_batch_fn* batch_callback, get_pixels_fn* get_pixels_callback,
                                                        generate_texture_handle_fn* generate_texture_callback, destroy_texture_handle_fn* delete_texture_callback,
                                                        sprites_sorter_fn* sprites_sorter_callback);
NEKO_API_DECL void neko_spritebatch_set_default_config(neko_spritebatch_config_t* config);

struct neko_spritebatch_config_t {
    int pixel_stride;
    int atlas_width_in_pixels;
    int atlas_height_in_pixels;
    int atlas_use_border_pixels;
    int ticks_to_decay_texture;
    int lonely_buffer_count_till_flush;

    float ratio_to_decay_atlas;
    float ratio_to_merge_atlases;
    submit_batch_fn* batch_callback;
    get_pixels_fn* get_pixels_callback;
    generate_texture_handle_fn* generate_texture_callback;
    destroy_texture_handle_fn* delete_texture_callback;
    sprites_sorter_fn* sprites_sorter_callback;
};

typedef struct {
    u64 image_id;
    int sort_bits;
    int w;
    int h;
    float x, y;
    float sx, sy;
    float c, s;

    float premade_minx, premade_miny;
    float premade_maxx, premade_maxy;

#ifdef SPRITEBATCH_SPRITE_USERDATA
    SPRITEBATCH_SPRITE_USERDATA udata;
#endif
} neko_spritebatch_internal_sprite_t;

typedef struct {
    int timestamp;
    int w, h;
    float minx, miny;
    float maxx, maxy;
    u64 image_id;
} neko_spritebatch_internal_texture_t;

typedef struct neko_spritebatch_internal_atlas_t {
    u64 texture_id;
    float volume_ratio;
    hashtable_t sprites_to_textures;
    struct neko_spritebatch_internal_atlas_t* next;
    struct neko_spritebatch_internal_atlas_t* prev;
} neko_spritebatch_internal_atlas_t;

typedef struct {
    int timestamp;
    int w, h;
    u64 image_id;
    u64 texture_id;
} neko_spritebatch_internal_lonely_texture_t;

typedef struct {
    int w, h;
    int mark_for_cleanup;
    u64 image_id;
    u64 texture_id;
} neko_spritebatch_internal_premade_atlas;

struct neko_spritebatch_t {
    int input_count;
    int input_capacity;
    neko_spritebatch_internal_sprite_t* input_buffer;

    int sprite_count;
    int sprite_capacity;
    neko_spritebatch_sprite_t* sprites;
    neko_spritebatch_sprite_t* sprites_scratch;

    int key_buffer_count;
    int key_buffer_capacity;
    u64* key_buffer;

    int pixel_buffer_size;
    void* pixel_buffer;

    hashtable_t sprites_to_premade_textures;
    hashtable_t sprites_to_lonely_textures;
    hashtable_t sprites_to_atlases;

    neko_spritebatch_internal_atlas_t* atlases;

    int pixel_stride;
    int atlas_width_in_pixels;
    int atlas_height_in_pixels;
    int atlas_use_border_pixels;
    int ticks_to_decay_texture;
    int lonely_buffer_count_till_flush;
    int lonely_buffer_count_till_decay;
    float ratio_to_decay_atlas;
    float ratio_to_merge_atlases;
    submit_batch_fn* batch_callback;
    get_pixels_fn* get_pixels_callback;
    generate_texture_handle_fn* generate_texture_callback;
    destroy_texture_handle_fn* delete_texture_callback;
    sprites_sorter_fn* sprites_sorter_callback;
    void* udata;
};

#define SPRITEBATCH_ATLAS_FLIP_Y_AXIS_FOR_UV 1

#define SPRITEBATCH_LONELY_FLIP_Y_AXIS_FOR_UV 1
#define SPRITEBATCH_ATLAS_EMPTY_COLOR 0x00000000

#endif

#endif