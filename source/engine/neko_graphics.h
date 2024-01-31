

#ifndef NEKO_GRAPHICS_H
#define NEKO_GRAPHICS_H

#include "engine/neko.h"

// OpenGL
#include "libs/glad/glad.h"

/*=============================
// NEKO_GRAPHICS
=============================*/

neko_inline const char* __neko_gl_error_string(GLenum const err) {
    switch (err) {
        // opengl 2 errors (8)
        case GL_NO_ERROR:
            return "GL_NO_ERROR";

        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";

        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";

        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";

        case GL_STACK_OVERFLOW:
            return "GL_STACK_OVERFLOW";

        case GL_STACK_UNDERFLOW:
            return "GL_STACK_UNDERFLOW";

        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";

        // case GL_TABLE_TOO_LARGE:
        //     return "GL_TABLE_TOO_LARGE";

        // opengl 3 errors (1)
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";

        // gles 2, 3 and gl 4 error are handled by the switch above
        default:
            return "unknown error";
    }
}

#define neko_check_gl_error() neko_graphics_print_errors_internal(__FILE__, __LINE__)
NEKO_API_DECL void neko_graphics_print_errors_internal(const char* file, u32 line);

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
    u32 width;                                        // Width of texture in texels
    u32 height;                                       // Height of texture in texels
    u32 depth;                                        // Depth of texture
    void* data[NEKO_GRAPHICS_TEXTURE_DATA_MAX];       // Texture data to upload (can be null)
    neko_graphics_texture_format_type format;         // Format of texture data (rgba32, rgba8, rgba32f, r8, depth32f, etc...)
    neko_graphics_texture_wrapping_type wrap_s;       // Wrapping type for s axis of texture
    neko_graphics_texture_wrapping_type wrap_t;       // Wrapping type for t axis of texture
    neko_graphics_texture_wrapping_type wrap_r;       // Wrapping type for r axis of texture
    neko_graphics_texture_filtering_type min_filter;  // Minification filter for texture
    neko_graphics_texture_filtering_type mag_filter;  // Magnification filter for texture
    neko_graphics_texture_filtering_type mip_filter;  // Mip filter for texture
    neko_vec2 offset;                                 // Offset for updates
    u32 num_mips;                                     // Number of mips to generate (default 0 is disable mip generation)
    struct {
        u32 x;        // X offset in texels to start read
        u32 y;        // Y offset in texels to start read
        u32 width;    // Width in texels for texture
        u32 height;   // Height in texels for texture
        size_t size;  // Size in bytes for data to be read
    } read;
    u16 flip_y;  // Whether or not y is flipped
} neko_graphics_texture_desc_t;

/* Graphics Uniform Layout Desc */
typedef struct neko_graphics_uniform_layout_desc_t {
    neko_graphics_uniform_type type;  // Type of field
    char fname[64];                   // the_name of field (required for implicit APIs, like OpenGL/ES)
    u32 count;                        // Count variable (used for arrays such as glUniformXXXv)
} neko_graphics_uniform_layout_desc_t;

/* Graphics Uniform Desc */
typedef struct neko_graphics_uniform_desc_t {
    neko_graphics_shader_stage_type stage;
    char name[64];                                // the_name of uniform (required for OpenGL/ES, WebGL)
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
    u32 binding;
    neko_graphics_access_type access;
} neko_graphics_bind_image_buffer_desc_t;

typedef struct neko_graphics_bind_uniform_buffer_desc_t {
    neko_handle(neko_graphics_uniform_buffer_t) buffer;
    u32 binding;
    struct {
        size_t offset;  // Specify an offset for ranged binds.
        size_t size;    // Specify size for ranged binds.
    } range;
} neko_graphics_bind_uniform_buffer_desc_t;

typedef struct neko_graphics_bind_storage_buffer_desc_t {
    neko_handle(neko_graphics_storage_buffer_t) buffer;
    u32 binding;
} neko_graphics_bind_storage_buffer_desc_t;

typedef struct neko_graphics_bind_uniform_desc_t {
    neko_handle(neko_graphics_uniform_t) uniform;
    void* data;
    u32 binding;  // Base binding for samplers?
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
    u32 ref;                               // Specifies reference val for stencil test
    u32 comp_mask;                         // Specifies mask that is ANDed with both ref val and stored stencil val
    u32 write_mask;                        // Specifies mask that is ANDed with both ref val and stored stencil val
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
    u32 buffer_idx;                              // Vertex buffer to use (optional, default = 0x00)
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
    u32 start;
    u32 count;
    u32 instances;
    u32 base_vertex;
    struct {
        u32 start;
        u32 end;
    } range;
} neko_graphics_draw_desc_t;

neko_inline neko_handle(neko_graphics_renderpass_t) __neko_renderpass_default_impl() {
    neko_handle(neko_graphics_renderpass_t) hndl = neko_default_val();
    return hndl;
}

// Convenience define for default render pass to back buffer
#define NEKO_GRAPHICS_RENDER_PASS_DEFAULT __neko_renderpass_default_impl()

typedef struct neko_graphics_info_t {
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
} neko_graphics_info_t;

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
        // void (*fontcache_create)(void);
        // void (*fontcache_destroy)(void);
        // void (*fontcache_draw)(void);
        // neko_font_index (*fontcache_load)(const void* data, size_t data_size, f32 font_size);
        // void (*fontcache_push)(const char* text, const neko_font_index font, const neko_vec2 pos);
        // void (*fontcache_push_x_y)(const char* text, const neko_font_index font, const f32 x, const f32 y);

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

// Custom Batch

// recommended to leave this on as long as possible (perhaps until release)
#define NEKO_GL_CUSTOM_DEBUG_CHECKS 1

// feel free to turn this off if unused.
// This is used to render lines on-top of all other rendering (besides post fx); rendered with no depth testing.
#define NEKO_GL_CUSTOM_LINE_RENDERER 1

enum {
    NEKO_GL_CUSTOM_FLOAT,
    NEKO_GL_CUSTOM_INT,
    NEKO_GL_CUSTOM_BOOL,
    NEKO_GL_CUSTOM_SAMPLER,
    NEKO_GL_CUSTOM_UNKNOWN,
};

typedef struct neko_graphics_custom_batch_vertex_attribute_t {
    const char* name;
    u64 hash;
    u32 size;
    u32 type;
    u32 offset;
    u32 location;
} neko_graphics_custom_batch_vertex_attribute_t;

#define NEKO_GL_CUSTOM_ATTRIBUTE_MAX_COUNT 16
typedef struct neko_graphics_custom_batch_vertex_data_t {
    u32 buffer_size;
    u32 vertex_stride;
    u32 primitive;
    u32 usage;

    u32 attribute_count;
    neko_graphics_custom_batch_vertex_attribute_t attributes[NEKO_GL_CUSTOM_ATTRIBUTE_MAX_COUNT];
} neko_graphics_custom_batch_vertex_data_t;

// 根据需要调整此项以创建绘制调用顺序
// see: http://realtimecollisiondetection.net/blog/?p=86
typedef struct neko_graphics_custom_batch_render_internal_state_t {
    union {
        struct {
            int fullscreen : 2;
            int hud : 5;
            int depth : 25;
            int translucency : 32;
        } bits;

        u64 key;
    } u;
} neko_graphics_custom_batch_render_internal_state_t;

struct neko_graphics_custom_batch_shader_t;
typedef struct neko_graphics_custom_batch_shader_t neko_graphics_custom_batch_shader_t;

typedef struct neko_graphics_custom_batch_renderable_t {
    neko_graphics_custom_batch_vertex_data_t data;
    neko_graphics_custom_batch_shader_t* program;
    neko_graphics_custom_batch_render_internal_state_t state;
    u32 attribute_count;

    u32 index0;
    u32 index1;
    u32 buffer_number;
    u32 need_new_sync;
    u32 buffer_count;
    u32 buffers[3];
    GLsync fences[3];
} neko_graphics_custom_batch_renderable_t;

#define NEKO_GL_CUSTOM_UNIFORM_NAME_LENGTH 64
#define NEKO_GL_CUSTOM_UNIFORM_MAX_COUNT 16

typedef struct neko_graphics_custom_batch_uniform_t {
    char name[NEKO_GL_CUSTOM_UNIFORM_NAME_LENGTH];
    u32 id;
    u64 hash;
    u32 size;
    u32 type;
    u32 location;
} neko_graphics_custom_batch_uniform_t;

struct neko_graphics_custom_batch_shader_t {
    u32 program;
    u32 uniform_count;
    neko_graphics_custom_batch_uniform_t uniforms[NEKO_GL_CUSTOM_UNIFORM_MAX_COUNT];
};

typedef struct neko_graphics_custom_batch_framebuffer_t {
    u32 fb_id;
    u32 tex_id;
    u32 rb_id;
    u32 quad_id;
    neko_graphics_custom_batch_shader_t* shader;
    int w, h;
} neko_graphics_custom_batch_framebuffer_t;

typedef struct {
    u32 vert_count;
    void* verts;
    neko_graphics_custom_batch_renderable_t* r;
    u32 texture_count;
    u32 textures[8];
} neko_graphics_custom_batch_draw_call_t;

struct neko_graphics_custom_batch_context_t;
typedef struct neko_graphics_custom_batch_context_t neko_graphics_custom_batch_context_t;
typedef struct neko_graphics_custom_batch_context_t* neko_graphics_custom_batch_context_ptr;
typedef struct neko_graphics_custom_batch_framebuffer_t* neko_graphics_custom_batch_framebuffer_ptr;

NEKO_API_DECL neko_graphics_custom_batch_context_t* neko_graphics_custom_batch_make_ctx(u32 max_draw_calls);
NEKO_API_DECL void neko_graphics_custom_batch_free(void* ctx);

NEKO_API_DECL void neko_graphics_custom_batch_make_frame_buffer(neko_graphics_custom_batch_framebuffer_t* fb, neko_graphics_custom_batch_shader_t* shader, int w, int h, int use_depth_test);
NEKO_API_DECL void neko_graphics_custom_batch_free_frame_buffer(neko_graphics_custom_batch_framebuffer_t* fb);

NEKO_API_DECL void neko_graphics_custom_batch_make_vertex_data(neko_graphics_custom_batch_vertex_data_t* vd, u32 buffer_size, u32 primitive, u32 vertex_stride, u32 usage);
NEKO_API_DECL void neko_graphics_custom_batch_add_attribute(neko_graphics_custom_batch_vertex_data_t* vd, const char* name, u32 size, u32 type, u32 offset);
NEKO_API_DECL void neko_graphics_custom_batch_make_renderable(neko_graphics_custom_batch_renderable_t* r, neko_graphics_custom_batch_vertex_data_t* vd);

// Must be called after gl_make_renderable
NEKO_API_DECL void neko_graphics_custom_batch_set_shader(neko_graphics_custom_batch_renderable_t* r, neko_graphics_custom_batch_shader_t* s);
NEKO_API_DECL void neko_graphics_custom_batch_load_shader(neko_graphics_custom_batch_shader_t* s, const char* vertex, const char* pixel);
NEKO_API_DECL void neko_graphics_custom_batch_free_shader(neko_graphics_custom_batch_shader_t* s);

NEKO_API_DECL void neko_graphics_custom_batch_set_active_shader(neko_graphics_custom_batch_shader_t* s);
NEKO_API_DECL void neko_graphics_custom_batch_deactivate_shader();
NEKO_API_DECL void neko_graphics_custom_batch_send_f32(neko_graphics_custom_batch_shader_t* s, const char* uniform_name, u32 size, float* floats, u32 count);
NEKO_API_DECL void neko_graphics_custom_batch_send_matrix(neko_graphics_custom_batch_shader_t* s, const char* uniform_name, float* floats);
NEKO_API_DECL void neko_graphics_custom_batch_send_texture(neko_graphics_custom_batch_shader_t* s, const char* uniform_name, u32 index);

NEKO_API_DECL void neko_graphics_custom_batch_push_draw_call(void* ctx, neko_graphics_custom_batch_draw_call_t call);

NEKO_API_DECL void neko_graphics_custom_batch_flush(void* ctx, neko_graphics_custom_batch_framebuffer_t* fb, int w, int h);
NEKO_API_DECL int neko_graphics_custom_batch_draw_call_count(void* ctx);

// 4x4 matrix helper functions
NEKO_API_DECL void neko_graphics_custom_batch_ortho_2d(float w, float h, float x, float y, float* m);
NEKO_API_DECL void neko_graphics_custom_batch_perspective(float* m, float y_fov_radians, float aspect, float n, float f);
NEKO_API_DECL void neko_graphics_custom_batch_mul(float* a, float* b, float* out);  // perform a * b, stores result in out
NEKO_API_DECL void neko_graphics_custom_batch_identity(float* m);
NEKO_API_DECL void neko_graphics_custom_batch_copy(float* dst, float* src);

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
NEKO_API_DECL void neko_graphics_set_viewport(neko_command_buffer_t* cb, u32 x, u32 y, u32 w, u32 h);
NEKO_API_DECL void neko_graphics_set_view_scissor(neko_command_buffer_t* cb, u32 x, u32 y, u32 w, u32 h);
NEKO_API_DECL void neko_graphics_clear(neko_command_buffer_t* cb, neko_graphics_clear_desc_t* desc);
NEKO_API_DECL void neko_graphics_pipeline_bind(neko_command_buffer_t* cb, neko_handle(neko_graphics_pipeline_t) hndl);
NEKO_API_DECL void neko_graphics_apply_bindings(neko_command_buffer_t* cb, neko_graphics_bind_desc_t* binds);
NEKO_API_DECL void neko_graphics_draw(neko_command_buffer_t* cb, neko_graphics_draw_desc_t* desc);
NEKO_API_DECL void neko_graphics_draw_batch(neko_command_buffer_t* cb, neko_graphics_custom_batch_context_t* ctx, neko_graphics_custom_batch_framebuffer_t* fb, s32 w, s32 h);
NEKO_API_DECL void neko_graphics_dispatch_compute(neko_command_buffer_t* cb, u32 num_x_groups, u32 num_y_groups, u32 num_z_groups);

// Macros
#define neko_graphics_command_buffer_submit(CB) neko_graphics()->api.command_buffer_submit((CB))
// #define neko_graphics_fc_text(text, font, x, y) ((neko_instance()->ctx.graphics))->api.fontcache_push_x_y(text, font, x, y)

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