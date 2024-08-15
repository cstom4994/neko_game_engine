#ifndef GFX_H
#define GFX_H

#include "engine/base.h"
#include "engine/glew_glfw.h"
#include "engine/math.h"
#include "engine/prelude.h"

struct shader_pair {
    GLuint id;
    const char* name;
};

// compile link program given paths to shader files possibly NULL doesn't glUseProgram(...)
GLuint gfx_create_program(const char* name, const char* vert_path, const char* geom_path, const char* frag_path);
void gfx_free_program(GLuint program);

// get pointer offset of 'field' in struct 'type'
#define poffsetof(type, field) ((void*)(&((type*)0)->field))

// bind vertex attribute data to 'field' in struct 'type' -- gl_type
// is the GL_* type of the parameter (GL_FLOAT etc.), components is the
// number of components, param_name is a string containing the name of
// the parameter as it appears in the program
#define gfx_bind_vertex_attrib(program, gl_type, components, param_name, type, field)                    \
    do {                                                                                                 \
        GLuint a__ = glGetAttribLocation(program, param_name);                                           \
        glVertexAttribPointer(a__, components, gl_type, GL_FALSE, sizeof(type), poffsetof(type, field)); \
        glEnableVertexAttribArray(a__);                                                                  \
    } while (0)

/*=============================
// NEKO_GRAPHICS
=============================*/

const_str __neko_gl_error_string(GLenum const err);
const_str neko_opengl_string(GLenum e);

#define neko_check_gl_error() gfx_print_error(__FILE__, __LINE__)
void gfx_print_error(const char* file, u32 line);

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
neko_enum_decl(gfx_shader_stage_type, R_SHADER_STAGE_VERTEX, R_SHADER_STAGE_FRAGMENT, R_SHADER_STAGE_COMPUTE);

/* Winding Order Type */
neko_enum_decl(gfx_winding_order_type, R_WINDING_ORDER_CW, R_WINDING_ORDER_CCW);

/* Face Culling Type */
neko_enum_decl(gfx_face_culling_type, R_FACE_CULLING_FRONT, R_FACE_CULLING_BACK, R_FACE_CULLING_FRONT_AND_BACK);

/* Blend Equation Type */
neko_enum_decl(gfx_blend_equation_type, R_BLEND_EQUATION_ADD, R_BLEND_EQUATION_SUBTRACT, R_BLEND_EQUATION_REVERSE_SUBTRACT, R_BLEND_EQUATION_MIN, R_BLEND_EQUATION_MAX);

/* Blend Mode Type */
neko_enum_decl(gfx_blend_mode_type, R_BLEND_MODE_ZERO, R_BLEND_MODE_ONE, R_BLEND_MODE_SRC_COLOR, R_BLEND_MODE_ONE_MINUS_SRC_COLOR, R_BLEND_MODE_DST_COLOR, R_BLEND_MODE_ONE_MINUS_DST_COLOR,
               R_BLEND_MODE_SRC_ALPHA, R_BLEND_MODE_ONE_MINUS_SRC_ALPHA, R_BLEND_MODE_DST_ALPHA, R_BLEND_MODE_ONE_MINUS_DST_ALPHA, R_BLEND_MODE_CONSTANT_COLOR, R_BLEND_MODE_ONE_MINUS_CONSTANT_COLOR,
               R_BLEND_MODE_CONSTANT_ALPHA, R_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA);

/* Shader Language Type */
neko_enum_decl(gfx_shader_language_type, R_SHADER_LANGUAGE_GLSL);

/* Push Constant Type */
// Really don't want to handle "auto-merging" of data types

/* Uniform Type */
neko_enum_decl(gfx_uniform_type, R_UNIFORM_FLOAT, R_UNIFORM_INT, R_UNIFORM_VEC2, R_UNIFORM_VEC3, R_UNIFORM_VEC4, R_UNIFORM_MAT4, R_UNIFORM_SAMPLER2D, R_UNIFORM_USAMPLER2D,
               R_UNIFORM_SAMPLERCUBE, R_UNIFORM_IMAGE2D_RGBA32F, R_UNIFORM_BLOCK);

/* Uniform Block Usage Type */
neko_enum_decl(gfx_uniform_block_usage_type,
               R_UNIFORM_BLOCK_USAGE_STATIC,  // Default of 0x00 is static
               R_UNIFORM_BLOCK_USAGE_PUSH_CONSTANT);

/* Sampler Type */
neko_enum_decl(gfx_sampler_type, R_SAMPLER_2D);

/* Primitive Type */
neko_enum_decl(gfx_primitive_type, R_PRIMITIVE_LINES, R_PRIMITIVE_TRIANGLES, R_PRIMITIVE_QUADS);

/* Vertex Atribute Type */
neko_enum_decl(gfx_vertex_attribute_type, R_VERTEX_ATTRIBUTE_FLOAT4, R_VERTEX_ATTRIBUTE_FLOAT3, R_VERTEX_ATTRIBUTE_FLOAT2, R_VERTEX_ATTRIBUTE_FLOAT, R_VERTEX_ATTRIBUTE_UINT4,
               R_VERTEX_ATTRIBUTE_UINT3, R_VERTEX_ATTRIBUTE_UINT2, R_VERTEX_ATTRIBUTE_UINT, R_VERTEX_ATTRIBUTE_BYTE4, R_VERTEX_ATTRIBUTE_BYTE3, R_VERTEX_ATTRIBUTE_BYTE2, R_VERTEX_ATTRIBUTE_BYTE);

/* Buffer Type */
neko_enum_decl(gfx_buffer_type, R_BUFFER_VERTEX, R_BUFFER_INDEX, R_BUFFER_FRAME, R_BUFFER_UNIFORM, R_BUFFER_UNIFORM_CONSTANT, R_BUFFER_SHADER_STORAGE, R_BUFFER_SAMPLER);

/* Buffer Usage Type */
neko_enum_decl(gfx_buffer_usage_type, R_BUFFER_USAGE_STATIC, R_BUFFER_USAGE_STREAM, R_BUFFER_USAGE_DYNAMIC);

/* Buffer Update Type */
neko_enum_decl(gfx_buffer_update_type, R_BUFFER_UPDATE_RECREATE, R_BUFFER_UPDATE_SUBDATA);

neko_enum_decl(gfx_access_type, R_ACCESS_READ_ONLY, R_ACCESS_WRITE_ONLY, R_ACCESS_READ_WRITE);

neko_enum_decl(gfx_buffer_flags, R_BUFFER_FLAG_MAP_PERSISTENT, R_BUFFER_FLAG_MAP_COHERENT);

//=== Texture ===//
typedef enum { R_TEXTURE_2D = 0x00, R_TEXTURE_CUBEMAP } gfx_texture_type;

typedef enum {
    R_TEXTURE_CUBEMAP_POSITIVE_X = 0x00,
    R_TEXTURE_CUBEMAP_NEGATIVE_X,
    R_TEXTURE_CUBEMAP_POSITIVE_Y,
    R_TEXTURE_CUBEMAP_NEGATIVE_Y,
    R_TEXTURE_CUBEMAP_POSITIVE_Z,
    R_TEXTURE_CUBEMAP_NEGATIVE_Z
} gfx_cubemap_face_type;

neko_enum_decl(gfx_texture_format_type, R_TEXTURE_FORMAT_RGBA8, R_TEXTURE_FORMAT_RGB8, R_TEXTURE_FORMAT_RG8, R_TEXTURE_FORMAT_R32, R_TEXTURE_FORMAT_R32F, R_TEXTURE_FORMAT_RGBA16F,
               R_TEXTURE_FORMAT_RGBA32F, R_TEXTURE_FORMAT_A8, R_TEXTURE_FORMAT_R8, R_TEXTURE_FORMAT_DEPTH8, R_TEXTURE_FORMAT_DEPTH16, R_TEXTURE_FORMAT_DEPTH24, R_TEXTURE_FORMAT_DEPTH32F,
               R_TEXTURE_FORMAT_DEPTH24_STENCIL8, R_TEXTURE_FORMAT_DEPTH32F_STENCIL8, R_TEXTURE_FORMAT_STENCIL8);

neko_enum_decl(gfx_texture_wrapping_type, R_TEXTURE_WRAP_REPEAT, R_TEXTURE_WRAP_MIRRORED_REPEAT, R_TEXTURE_WRAP_CLAMP_TO_EDGE, R_TEXTURE_WRAP_CLAMP_TO_BORDER);

neko_enum_decl(gfx_texture_filtering_type, R_TEXTURE_FILTER_NEAREST, R_TEXTURE_FILTER_LINEAR);

//=== Clear ===//
neko_enum_decl(gfx_clear_flag, R_CLEAR_COLOR = 0x01, R_CLEAR_DEPTH = 0x02, R_CLEAR_STENCIL = 0x04, R_CLEAR_NONE = 0x08);

#define R_CLEAR_ALL R_CLEAR_COLOR | R_CLEAR_DEPTH | R_CLEAR_STENCIL

//=== Bind Type ===//
neko_enum_decl(gfx_bind_type, R_BIND_VERTEX_BUFFER, R_BIND_INDEX_BUFFER, R_BIND_UNIFORM_BUFFER, R_BIND_STORAGE_BUFFER, R_BIND_IMAGE_BUFFER, R_BIND_UNIFORM);

/* Depth Function Type */
neko_enum_decl(gfx_depth_func_type,  // Default value of 0x00 means depth is disabled
               R_DEPTH_FUNC_NEVER, R_DEPTH_FUNC_LESS, R_DEPTH_FUNC_EQUAL, R_DEPTH_FUNC_LEQUAL, R_DEPTH_FUNC_GREATER, R_DEPTH_FUNC_NOTEQUAL, R_DEPTH_FUNC_GEQUAL, R_DEPTH_FUNC_ALWAYS);

neko_enum_decl(gfx_depth_mask_type,  // Default value 0x00 means depth writing enabled
               R_DEPTH_MASK_ENABLED, R_DEPTH_MASK_DISABLED);

/* Stencil Function Type */
neko_enum_decl(gfx_stencil_func_type,
               R_STENCIL_FUNC_NEVER,  // Default value of 0x00 means stencil is disabled
               R_STENCIL_FUNC_LESS, R_STENCIL_FUNC_EQUAL, R_STENCIL_FUNC_LEQUAL, R_STENCIL_FUNC_GREATER, R_STENCIL_FUNC_NOTEQUAL, R_STENCIL_FUNC_GEQUAL, R_STENCIL_FUNC_ALWAYS);

/* Stencil Op Type */
neko_enum_decl(gfx_stencil_op_type,  // Default value of 0x00 means keep is used
               R_STENCIL_OP_KEEP, R_STENCIL_OP_ZERO, R_STENCIL_OP_REPLACE, R_STENCIL_OP_INCR, R_STENCIL_OP_INCR_WRAP, R_STENCIL_OP_DECR, R_STENCIL_OP_DECR_WRAP, R_STENCIL_OP_INVERT);

/* Internal Graphics Resource Handles */
neko_handle_decl(gfx_shader_t);
neko_handle_decl(gfx_texture_t);
neko_handle_decl(gfx_vertex_buffer_t);
neko_handle_decl(gfx_index_buffer_t);
neko_handle_decl(gfx_uniform_buffer_t);
neko_handle_decl(gfx_storage_buffer_t);
neko_handle_decl(gfx_framebuffer_t);
neko_handle_decl(gfx_uniform_t);
neko_handle_decl(gfx_renderpass_t);
neko_handle_decl(gfx_pipeline_t);

/* Graphics Shader Source Desc */
typedef struct gfx_shader_source_desc_t {
    gfx_shader_stage_type type;  // Shader stage type (vertex, fragment, tesselation, geometry, compute)
    const char* source;                  // Source for shader
} gfx_shader_source_desc_t;

/* Graphics Shader Desc */
typedef struct gfx_shader_desc_t {
    gfx_shader_source_desc_t* sources;  // Array of shader source descriptions
    size_t size;                                // Size in bytes of shader source desc array
    char name[64];                              // Optional (for logging and debugging mainly)
} gfx_shader_desc_t;

#define R_TEXTURE_DATA_MAX 6

/* Graphics Texture Desc */
typedef struct gfx_texture_desc_t {
    gfx_texture_type type;
    u32 width;                                      // Width of texture in texels
    u32 height;                                     // Height of texture in texels
    u32 depth;                                      // Depth of texture
    void* data[R_TEXTURE_DATA_MAX];                 // Texture data to upload (can be null)
    gfx_texture_format_type format;         // Format of texture data (rgba32, rgba8, rgba32f, r8, depth32f, etc...)
    gfx_texture_wrapping_type wrap_s;       // Wrapping type for s axis of texture
    gfx_texture_wrapping_type wrap_t;       // Wrapping type for t axis of texture
    gfx_texture_wrapping_type wrap_r;       // Wrapping type for r axis of texture
    gfx_texture_filtering_type min_filter;  // Minification filter for texture
    gfx_texture_filtering_type mag_filter;  // Magnification filter for texture
    gfx_texture_filtering_type mip_filter;  // Mip filter for texture
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
} gfx_texture_desc_t;

/* Graphics Uniform Layout Desc */
typedef struct gfx_uniform_layout_desc_t {
    gfx_uniform_type type;  // Type of field
    char fname[64];                 // The name of field (required for implicit APIs, like OpenGL/ES)
    u32 count;                      // Count variable (used for arrays such as glUniformXXXv)
} gfx_uniform_layout_desc_t;

/* Graphics Uniform Desc */
typedef struct gfx_uniform_desc_t {
    gfx_shader_stage_type stage;
    char name[64];                              // The name of uniform (required for OpenGL/ES, WebGL)
    gfx_uniform_layout_desc_t* layout;  // Layout array for uniform data
    size_t layout_size;                         // Size of uniform data in bytes
} gfx_uniform_desc_t;

typedef struct gfx_buffer_update_desc_t {
    gfx_buffer_update_type type;
    size_t offset;
} gfx_buffer_update_desc_t;

/* Graphics Buffer Desc General */
typedef struct gfx_buffer_base_desc_t {
    void* data;
    size_t size;
    gfx_buffer_usage_type usage;
} gfx_buffer_base_desc_t;

typedef struct gfx_vertex_buffer_desc_t {
    void* data;
    size_t size;
    gfx_buffer_usage_type usage;
    gfx_buffer_update_desc_t update;
} gfx_vertex_buffer_desc_t;

typedef gfx_vertex_buffer_desc_t gfx_index_buffer_desc_t;

typedef struct gfx_uniform_buffer_desc_t {
    void* data;
    size_t size;
    gfx_buffer_usage_type usage;
    const char* name;
    gfx_shader_stage_type stage;
    gfx_buffer_update_desc_t update;
} gfx_uniform_buffer_desc_t;

typedef struct gfx_storage_buffer_desc_t {
    void* data;
    void* map;
    size_t size;
    char name[64];
    gfx_buffer_usage_type usage;
    gfx_access_type access;
    gfx_buffer_flags flags;
    gfx_buffer_update_desc_t update;
} gfx_storage_buffer_desc_t;

typedef struct gfx_framebuffer_desc_t {
    void* data;
} gfx_framebuffer_desc_t;

/* Graphics Clear Action */
typedef struct gfx_clear_action_t {
    gfx_clear_flag flag;  // Flag to be set (clear color, clear depth, clear stencil, clear all)
    union {
        float color[4];   // Clear color value
        float depth;      // Clear depth value
        int32_t stencil;  // Clear stencil value
    };
} gfx_clear_action_t;

/* Graphics Clear Desc */
typedef struct gfx_clear_desc_t {
    gfx_clear_action_t* actions;  // Clear action
    size_t size;                          // Size
} gfx_clear_desc_t;

/* Graphics Render Pass Desc */
typedef struct gfx_renderpass_desc_t {
    neko_handle(gfx_framebuffer_t) fbo;  // Default is set to invalid for backbuffer
    neko_handle(gfx_texture_t) * color;  // Array of color attachments to be bound (useful for MRT, if supported)
    size_t color_size;                           // Size of color attachment array
    neko_handle(gfx_texture_t) depth;    // Depth attachment to be bound
    neko_handle(gfx_texture_t) stencil;  // Depth attachment to be bound
} gfx_renderpass_desc_t;

/*
    // If you want to write to a color attachment, you have to have a frame buffer attached that isn't the backbuffer
*/

typedef enum gfx_vertex_data_type { R_VERTEX_DATA_INTERLEAVED = 0x00, R_VERTEX_DATA_NONINTERLEAVED } gfx_vertex_data_type;

typedef struct gfx_bind_vertex_buffer_desc_t {
    neko_handle(gfx_vertex_buffer_t) buffer;
    size_t offset;
    gfx_vertex_data_type data_type;
} gfx_bind_vertex_buffer_desc_t;

typedef struct gfx_bind_index_buffer_desc_t {
    neko_handle(gfx_index_buffer_t) buffer;
} gfx_bind_index_buffer_desc_t;

typedef struct gfx_bind_image_buffer_desc_t {
    neko_handle(gfx_texture_t) tex;
    u32 binding;
    gfx_access_type access;
} gfx_bind_image_buffer_desc_t;

typedef struct gfx_bind_uniform_buffer_desc_t {
    neko_handle(gfx_uniform_buffer_t) buffer;
    u32 binding;
    struct {
        size_t offset;  // Specify an offset for ranged binds.
        size_t size;    // Specify size for ranged binds.
    } range;
} gfx_bind_uniform_buffer_desc_t;

typedef struct gfx_bind_storage_buffer_desc_t {
    neko_handle(gfx_storage_buffer_t) buffer;
    u32 binding;
} gfx_bind_storage_buffer_desc_t;

typedef struct gfx_bind_uniform_desc_t {
    neko_handle(gfx_uniform_t) uniform;
    void* data;
    u32 binding;  // Base binding for samplers?
} gfx_bind_uniform_desc_t;

/* Graphics Binding Desc */
typedef struct gfx_bind_desc_t {
    struct {
        gfx_bind_vertex_buffer_desc_t* desc;  // Array of vertex buffer declarations (NULL by default)
        size_t size;                                  // Size of array in bytes (optional if only one)
    } vertex_buffers;

    struct {
        gfx_bind_index_buffer_desc_t* desc;  // Array of index buffer declarations (NULL by default)
        size_t size;                                 // Size of array in bytes (optional if only one)
    } index_buffers;

    struct {
        gfx_bind_uniform_buffer_desc_t* desc;  // Array of uniform buffer declarations (NULL by default)
        size_t size;                                   // Size of array in bytes (optional if only one)
    } uniform_buffers;

    struct {
        gfx_bind_uniform_desc_t* desc;  // Array of uniform declarations (NULL by default)
        size_t size;                            // Size of array in bytes (optional if one)
    } uniforms;

    struct {
        gfx_bind_image_buffer_desc_t* desc;
        size_t size;
    } image_buffers;

    struct {
        gfx_bind_storage_buffer_desc_t* desc;
        size_t size;
    } storage_buffers;

} gfx_bind_desc_t;

/* Graphics Blend State Desc */
typedef struct gfx_blend_state_desc_t {
    gfx_blend_equation_type func;  // Equation function to use for blend ops
    gfx_blend_mode_type src;       // Source blend mode
    gfx_blend_mode_type dst;       // Destination blend mode
} gfx_blend_state_desc_t;

/* Graphics Depth State Desc */
typedef struct gfx_depth_state_desc_t {
    gfx_depth_func_type func;  // Function to set for depth test
    gfx_depth_mask_type mask;  // Whether or not writing is enabled/disabled
} gfx_depth_state_desc_t;

/* Graphics Stencil State Desc */
typedef struct gfx_stencil_state_desc_t {
    gfx_stencil_func_type func;  // Function to set for stencil test
    u32 ref;                             // Specifies reference val for stencil test
    u32 comp_mask;                       // Specifies mask that is ANDed with both ref val and stored stencil val
    u32 write_mask;                      // Specifies mask that is ANDed with both ref val and stored stencil val
    gfx_stencil_op_type sfail;   // Action to take when stencil test fails
    gfx_stencil_op_type dpfail;  // Action to take when stencil test passes but depth test fails
    gfx_stencil_op_type dppass;  // Action to take when both stencil test passes and either depth passes or is not enabled
} gfx_stencil_state_desc_t;

/* Graphics Raster State Desc */
typedef struct gfx_raster_state_desc_t {
    gfx_face_culling_type face_culling;    // Face culling mode to be used (front, back, front and back)
    gfx_winding_order_type winding_order;  // Winding order mode to be used (ccw, cw)
    gfx_primitive_type primitive;          // Primitive type for drawing (lines, quads, triangles, triangle strip)
    neko_handle(gfx_shader_t) shader;      // Shader to bind and use (might be in bindables later on, not sure)
    size_t index_buffer_element_size;              // Element size of index buffer (used for parsing internal data)
} gfx_raster_state_desc_t;

/* Graphics Compute State Desc */
typedef struct gfx_compute_state_desc_t {
    neko_handle(gfx_shader_t) shader;  // Compute shader to bind
} gfx_compute_state_desc_t;

/* Graphics Vertex Attribute Desc */
typedef struct gfx_vertex_attribute_desc_t {
    char name[64];                             // Attribute name (required for lower versions of OpenGL and ES)
    gfx_vertex_attribute_type format;  // Format for vertex attribute
    size_t stride;                             // Total stride of vertex layout (optional, calculated by default)
    size_t offset;                             // Offset of this vertex from base pointer of data (optional, calaculated by default)
    size_t divisor;                            // Used for instancing. (optional, default = 0x00 for no instancing)
    u32 buffer_idx;                            // Vertex buffer to use (optional, default = 0x00)
} gfx_vertex_attribute_desc_t;

/* Graphics Vertex Layout Desc */
typedef struct gfx_vertex_layout_desc_t {
    gfx_vertex_attribute_desc_t* attrs;  // Vertex attribute array
    size_t size;                                 // Size in bytes of vertex attribute array
} gfx_vertex_layout_desc_t;

/* Graphics Pipeline Desc */
typedef struct gfx_pipeline_desc_t {
    gfx_blend_state_desc_t blend;      // Blend state desc for pipeline
    gfx_depth_state_desc_t depth;      // Depth state desc for pipeline
    gfx_raster_state_desc_t raster;    // Raster state desc for pipeline
    gfx_stencil_state_desc_t stencil;  // Stencil state desc for pipeline
    gfx_compute_state_desc_t compute;  // Compute state desc for pipeline
    gfx_vertex_layout_desc_t layout;   // Vertex layout desc for pipeline
} gfx_pipeline_desc_t;

/* Graphics Draw Desc */
typedef struct gfx_draw_desc_t {
    u32 start;
    u32 count;
    u32 instances;
    u32 base_vertex;
    struct {
        u32 start;
        u32 end;
    } range;
} gfx_draw_desc_t;

NEKO_FORCE_INLINE neko_handle(gfx_renderpass_t) __neko_renderpass() {
    neko_handle(gfx_renderpass_t) hndl = NEKO_DEFAULT_VAL();
    return hndl;
}

// Convenience define for default render pass to back buffer
#define R_RENDER_PASS_DEFAULT __neko_renderpass()

typedef struct gfx_info_t {
    const_str version;
    const_str vendor;
    u32 major_version;
    u32 minor_version;
    u32 max_texture_units;
    struct {
        bool available;
        u32 max_work_group_count[3];
        u32 max_work_group_size[3];
        u32 max_work_group_invocations;
    } compute;
} gfx_info_t;

/*==========================
// Graphics Interface
==========================*/

typedef struct gfx_t {
    void* user_data;          // For internal use
    gfx_info_t info;  // Used for querying by user for features
    struct {

        // Create
        neko_handle(gfx_texture_t) (*texture_create)(const gfx_texture_desc_t desc);
        neko_handle(gfx_uniform_t) (*uniform_create)(const gfx_uniform_desc_t desc);
        neko_handle(gfx_shader_t) (*shader_create)(const gfx_shader_desc_t desc);
        neko_handle(gfx_vertex_buffer_t) (*vertex_buffer_create)(const gfx_vertex_buffer_desc_t desc);
        neko_handle(gfx_index_buffer_t) (*index_buffer_create)(const gfx_index_buffer_desc_t desc);
        neko_handle(gfx_uniform_buffer_t) (*uniform_buffer_create)(const gfx_uniform_buffer_desc_t desc);
        neko_handle(gfx_storage_buffer_t) (*storage_buffer_create)(const gfx_storage_buffer_desc_t desc);
        neko_handle(gfx_framebuffer_t) (*framebuffer_create)(const gfx_framebuffer_desc_t desc);
        neko_handle(gfx_renderpass_t) (*renderpass_create)(const gfx_renderpass_desc_t desc);
        neko_handle(gfx_pipeline_t) (*pipeline_create)(const gfx_pipeline_desc_t desc);

        // Destroy
        void (*texture_fini)(neko_handle(gfx_texture_t) hndl);
        void (*uniform_fini)(neko_handle(gfx_uniform_t) hndl);
        void (*shader_fini)(neko_handle(gfx_shader_t) hndl);
        void (*vertex_buffer_fini)(neko_handle(gfx_vertex_buffer_t) hndl);
        void (*index_buffer_fini)(neko_handle(gfx_index_buffer_t) hndl);
        void (*uniform_buffer_fini)(neko_handle(gfx_uniform_buffer_t) hndl);
        void (*storage_buffer_fini)(neko_handle(gfx_storage_buffer_t) hndl);
        void (*framebuffer_fini)(neko_handle(gfx_framebuffer_t) hndl);
        void (*renderpass_fini)(neko_handle(gfx_renderpass_t) hndl);
        void (*pipeline_fini)(neko_handle(gfx_pipeline_t) hndl);

        // Resource Updates (main thread only)
        void (*vertex_buffer_update)(neko_handle(gfx_vertex_buffer_t) hndl, gfx_vertex_buffer_desc_t* desc);
        void (*index_buffer_update)(neko_handle(gfx_index_buffer_t) hndl, gfx_index_buffer_desc_t* desc);
        void (*storage_buffer_update)(neko_handle(gfx_storage_buffer_t) hndl, gfx_storage_buffer_desc_t* desc);
        void (*texture_update)(neko_handle(gfx_texture_t) hndl, gfx_texture_desc_t* desc);
        void (*texture_read)(neko_handle(gfx_texture_t) hndl, gfx_texture_desc_t* desc);

        // Util
        void* (*storage_buffer_map_get)(neko_handle(gfx_storage_buffer_t) hndl);
        void* (*storage_buffer_lock)(neko_handle(gfx_storage_buffer_t) hndl);
        void (*storage_buffer_unlock)(neko_handle(gfx_storage_buffer_t) hndl);

        // Submission (Main Thread)
        void (*command_buffer_submit)(neko_command_buffer_t* cb);

    } api;
} gfx_t;

extern gfx_t* g_render;

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
    i32 access;
    size_t size;
    u32 block_idx;
    u32 location;
    void* map;
    GLsync sync;
} neko_gl_storage_buffer_t;

/* Pipeline */
typedef struct neko_gl_pipeline_t {
    gfx_blend_state_desc_t blend;
    gfx_depth_state_desc_t depth;
    gfx_raster_state_desc_t raster;
    gfx_stencil_state_desc_t stencil;
    gfx_compute_state_desc_t compute;
    neko_dyn_array(gfx_vertex_attribute_desc_t) layout;
} neko_gl_pipeline_t;

/* Render Pass */
typedef struct neko_gl_renderpass_t {
    neko_handle(gfx_framebuffer_t) fbo;
    neko_dyn_array(neko_handle(gfx_texture_t)) color;
    neko_handle(gfx_texture_t) depth;
    neko_handle(gfx_texture_t) stencil;
} neko_gl_renderpass_t;

/* Shader */
typedef struct neko_gl_shader_t {
    u32 id;
} neko_gl_shader_t;

/* Gfx Buffer */
typedef struct neko_gl_buffer_t {
    u32 id;
} neko_gl_buffer_t;

/* Texture */
typedef struct neko_gl_texture_t {
    u32 id;
    gfx_texture_desc_t desc;
} neko_gl_texture_t;

typedef struct neko_gl_vertex_buffer_decl_t {
    neko_gl_buffer_t vbo;
    gfx_vertex_data_type data_type;
    size_t offset;
} neko_gl_vertex_buffer_decl_t;

// 绘制之间的缓存数据
typedef struct neko_gl_data_cache_t {
    neko_gl_buffer_t vao;
    neko_gl_buffer_t ibo;
    size_t ibo_elem_sz;
    neko_dyn_array(neko_gl_vertex_buffer_decl_t) vdecls;
    neko_handle(gfx_pipeline_t) pipeline;
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
        neko_dyn_array(i32) i32;
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
    NEKO_OPENGL_OP_DRAW_FUNC,
} neko_opengl_op_code_type;

/*==========================
// Graphics API
==========================*/

#define neko_render() RENDER()

// Graphics Interface Creation / Initialization / Shutdown / Destruction
gfx_t* gfx_create();
void gfx_fini(gfx_t* render);
void gfx_init(gfx_t* render);

// Graphics Info Object Query
gfx_info_t* gfx_info();

typedef void (*R_DRAW_FUNC)(void);

// recommended to leave this on as long as possible (perhaps until release)
#define R_BATCH_DEBUG_CHECKS 1

enum {
    R_BATCH_FLOAT,
    R_BATCH_INT,
    R_BATCH_BOOL,
    R_BATCH_SAMPLER,
    R_BATCH_UNKNOWN,
};

typedef struct gfx_batch_vertex_attribute_t {
    const char* name;
    u64 hash;
    u32 size;
    u32 type;
    u32 offset;
    u32 location;
} gfx_batch_vertex_attribute_t;

#define R_BATCH_ATTRIBUTE_MAX_COUNT 16
typedef struct gfx_batch_vertex_data_t {
    u32 buffer_size;
    u32 vertex_stride;
    u32 primitive;
    u32 usage;

    u32 attribute_count;
    gfx_batch_vertex_attribute_t attributes[R_BATCH_ATTRIBUTE_MAX_COUNT];
} gfx_batch_vertex_data_t;

// 根据需要调整此项以创建绘制调用顺序
// see: http://realtimecollisiondetection.net/blog/?p=86
typedef struct gfx_batch_render_internal_state_t {
    union {
        struct {
            int fullscreen : 2;
            int hud : 5;
            int depth : 25;
            int translucency : 32;
        } bits;

        u64 key;
    } u;
} gfx_batch_render_internal_state_t;

struct gfx_batch_shader_t;
typedef struct gfx_batch_shader_t gfx_batch_shader_t;

typedef struct gfx_batch_renderable_t {
    gfx_batch_vertex_data_t data;
    gfx_batch_shader_t* program;
    gfx_batch_render_internal_state_t state;
    u32 attribute_count;

    u32 index0;
    u32 index1;
    u32 buffer_number;
    u32 need_new_sync;
    u32 buffer_count;
    u32 buffers[3];
    GLsync fences[3];
} gfx_batch_renderable_t;

#define R_BATCH_UNIFORM_NAME_LENGTH 64
#define R_BATCH_UNIFORM_MAX_COUNT 16

typedef struct gfx_batch_uniform_t {
    char name[R_BATCH_UNIFORM_NAME_LENGTH];
    u32 id;
    u64 hash;
    u32 size;
    u32 type;
    u32 location;
} gfx_batch_uniform_t;

struct gfx_batch_shader_t {
    u32 program;
    u32 uniform_count;
    gfx_batch_uniform_t uniforms[R_BATCH_UNIFORM_MAX_COUNT];
};

typedef struct gfx_batch_framebuffer_t {
    u32 fb_id;
    u32 tex_id;
    u32 rb_id;
    u32 quad_id;
    gfx_batch_shader_t* shader;
    int w, h;
} gfx_batch_framebuffer_t;

typedef struct {
    u32 vert_count;
    void* verts;
    gfx_batch_renderable_t* r;
    u32 texture_count;
    u32 textures[8];
} gfx_batch_draw_call_t;

struct gfx_batch_context_t;
typedef struct gfx_batch_context_t gfx_batch_context_t;
typedef struct gfx_batch_context_t* gfx_batch_context_ptr;
typedef struct gfx_batch_framebuffer_t* gfx_batch_framebuffer_ptr;

gfx_batch_context_t* gfx_batch_make_ctx(u32 max_draw_calls);
void gfx_batch_free(void* ctx);

void gfx_batch_make_frame_buffer(gfx_batch_framebuffer_t* fb, gfx_batch_shader_t* shader, int w, int h, int use_depth_test);
void gfx_batch_free_frame_buffer(gfx_batch_framebuffer_t* fb);

void gfx_batch_make_vertex_data(gfx_batch_vertex_data_t* vd, u32 buffer_size, u32 primitive, u32 vertex_stride, u32 usage);
void gfx_batch_add_attribute(gfx_batch_vertex_data_t* vd, const char* name, u32 size, u32 type, u32 offset);
void gfx_batch_make_renderable(gfx_batch_renderable_t* r, gfx_batch_vertex_data_t* vd);

// Must be called after gl_make_renderable
void gfx_batch_set_shader(gfx_batch_renderable_t* r, gfx_batch_shader_t* s);
void gfx_batch_load_shader(gfx_batch_shader_t* s, const char* vertex, const char* pixel);
void gfx_batch_free_shader(gfx_batch_shader_t* s);

void gfx_batch_set_active_shader(gfx_batch_shader_t* s);
void gfx_batch_deactivate_shader();
void gfx_batch_send_f32(gfx_batch_shader_t* s, const char* uniform_name, u32 size, float* floats, u32 count);
void gfx_batch_send_matrix(gfx_batch_shader_t* s, const char* uniform_name, float* floats);
void gfx_batch_send_texture(gfx_batch_shader_t* s, const char* uniform_name, u32 index);

void gfx_batch_push_draw_call(void* ctx, gfx_batch_draw_call_t call);

void gfx_batch_flush(void* ctx, gfx_batch_framebuffer_t* fb, int w, int h);
int gfx_batch_draw_call_count(void* ctx);

// 4x4 matrix helper functions
void gfx_batch_ortho_2d(float w, float h, float x, float y, float* m);
void gfx_batch_perspective(float* m, float y_fov_radians, float aspect, float n, float f);
void gfx_batch_mul(float* a, float* b, float* out);  // perform a * b, stores result in out
void gfx_batch_identity(float* m);
void gfx_batch_copy(float* dst, float* src);

neko_gl_data_t* gfx_userdata();

// Resource Creation
// Create
neko_handle(gfx_texture_t) gfx_texture_create(const gfx_texture_desc_t desc);
neko_handle(gfx_uniform_t) gfx_uniform_create(const gfx_uniform_desc_t desc);
neko_handle(gfx_shader_t) gfx_shader_create(const gfx_shader_desc_t desc);
neko_handle(gfx_vertex_buffer_t) gfx_vertex_buffer_create(const gfx_vertex_buffer_desc_t desc);
neko_handle(gfx_index_buffer_t) gfx_index_buffer_create(const gfx_index_buffer_desc_t desc);
neko_handle(gfx_uniform_buffer_t) gfx_uniform_buffer_create(const gfx_uniform_buffer_desc_t desc);
neko_handle(gfx_storage_buffer_t) gfx_storage_buffer_create(const gfx_storage_buffer_desc_t desc);
neko_handle(gfx_framebuffer_t) gfx_framebuffer_create(const gfx_framebuffer_desc_t desc);
neko_handle(gfx_renderpass_t) gfx_renderpass_create(const gfx_renderpass_desc_t desc);
neko_handle(gfx_pipeline_t) gfx_pipeline_create(const gfx_pipeline_desc_t desc);

// Destroy
void gfx_texture_fini(neko_handle(gfx_texture_t) hndl);
void gfx_uniform_fini(neko_handle(gfx_uniform_t) hndl);
void gfx_shader_fini(neko_handle(gfx_shader_t) hndl);
void gfx_vertex_buffer_fini(neko_handle(gfx_vertex_buffer_t) hndl);
void gfx_index_buffer_fini(neko_handle(gfx_index_buffer_t) hndl);
void gfx_uniform_buffer_fini(neko_handle(gfx_uniform_buffer_t) hndl);
void gfx_storage_buffer_fini(neko_handle(gfx_storage_buffer_t) hndl);
void gfx_framebuffer_fini(neko_handle(gfx_framebuffer_t) hndl);
void gfx_renderpass_fini(neko_handle(gfx_renderpass_t) hndl);
void gfx_pipeline_fini(neko_handle(gfx_pipeline_t) hndl);

// Resource Updates (main thread only)
void gfx_vertex_buffer_update(neko_handle(gfx_vertex_buffer_t) hndl, gfx_vertex_buffer_desc_t* desc);
void gfx_index_buffer_update(neko_handle(gfx_index_buffer_t) hndl, gfx_index_buffer_desc_t* desc);
void gfx_storage_buffer_update(neko_handle(gfx_storage_buffer_t) hndl, gfx_storage_buffer_desc_t* desc);
void gfx_texture_update(neko_handle(gfx_texture_t) hndl, gfx_texture_desc_t* desc);
void gfx_texture_read(neko_handle(gfx_texture_t) hndl, gfx_texture_desc_t* desc);

void* gfx_storage_buffer_map_get(neko_handle(gfx_storage_buffer_t) hndl);
void* gfx_storage_buffer_lock(neko_handle(gfx_storage_buffer_t) hndl);
void gfx_storage_buffer_unlock(neko_handle(gfx_storage_buffer_t) hndl);

// Resource Queries
void gfx_pipeline_desc_query(neko_handle(gfx_pipeline_t) hndl, gfx_pipeline_desc_t* out);
void gfx_texture_desc_query(neko_handle(gfx_texture_t) hndl, gfx_texture_desc_t* out);
size_t gfx_uniform_size_query(neko_handle(gfx_uniform_t) hndl);

// Resource In-Flight Update
void gfx_texture_request_update(neko_command_buffer_t* cb, neko_handle(gfx_texture_t) hndl, gfx_texture_desc_t desc);
void gfx_vertex_buffer_request_update(neko_command_buffer_t* cb, neko_handle(gfx_vertex_buffer_t) hndl, gfx_vertex_buffer_desc_t desc);
void gfx_index_buffer_request_update(neko_command_buffer_t* cb, neko_handle(gfx_index_buffer_t) hndl, gfx_index_buffer_desc_t desc);
void gfx_uniform_buffer_request_update(neko_command_buffer_t* cb, neko_handle(gfx_uniform_buffer_t) hndl, gfx_uniform_buffer_desc_t desc);
void gfx_storage_buffer_request_update(neko_command_buffer_t* cb, neko_handle(gfx_storage_buffer_t) hndl, gfx_storage_buffer_desc_t desc);

// Pipeline / Pass / Bind / Draw
void gfx_renderpass_begin(neko_command_buffer_t* cb, neko_handle(gfx_renderpass_t) hndl);
void gfx_renderpass_end(neko_command_buffer_t* cb);
void gfx_set_viewport(neko_command_buffer_t* cb, u32 x, u32 y, u32 w, u32 h);
void gfx_set_view_scissor(neko_command_buffer_t* cb, u32 x, u32 y, u32 w, u32 h);
void gfx_clear(neko_command_buffer_t* cb, gfx_clear_action_t action);
void gfx_clear_ex(neko_command_buffer_t* cb, gfx_clear_desc_t* desc);
void gfx_pipeline_bind(neko_command_buffer_t* cb, neko_handle(gfx_pipeline_t) hndl);
void gfx_apply_bindings(neko_command_buffer_t* cb, gfx_bind_desc_t* binds);
void gfx_draw(neko_command_buffer_t* cb, gfx_draw_desc_t desc);
void gfx_draw_batch(neko_command_buffer_t* cb, gfx_batch_context_t* ctx, gfx_batch_framebuffer_t* fb, i32 w, i32 h);
void gfx_draw_func(neko_command_buffer_t* cb, R_DRAW_FUNC draw_func);
void gfx_dispatch_compute(neko_command_buffer_t* cb, u32 num_x_groups, u32 num_y_groups, u32 num_z_groups);

// Macros
#define gfx_cmd_submit(CB) g_render->api.command_buffer_submit((CB))

typedef neko_handle(gfx_shader_t) neko_shader_t;
typedef neko_handle(gfx_texture_t) neko_texture_t;
typedef neko_handle(gfx_renderpass_t) neko_renderpass_t;
typedef neko_handle(gfx_framebuffer_t) neko_framebuffer_t;
typedef neko_handle(gfx_pipeline_t) neko_pipeline_t;
typedef neko_handle(gfx_vertex_buffer_t) neko_vbo_t;
typedef neko_handle(gfx_index_buffer_t) neko_ibo_t;
typedef neko_handle(gfx_uniform_buffer_t) neko_ubo_t;
typedef neko_handle(gfx_uniform_t) neko_uniform_t;
typedef neko_handle(gfx_storage_buffer_t) neko_storage_buffer_t;

/*=============================
//
=============================*/

NEKO_FORCE_INLINE u64 generate_texture_handle(void* pixels, int w, int h, void* udata) {
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

NEKO_FORCE_INLINE void destroy_texture_handle(u64 texture_id, void* udata) {
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
    neko_handle(gfx_vertex_buffer_t) vbo;
    neko_handle(gfx_index_buffer_t) ibo;
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
    neko_handle(gfx_texture_t) hndl;
    gfx_texture_desc_t desc;
} neko_asset_texture_t;

typedef struct neko_baked_char_t {
    u32 codepoint;
    u16 x0, y0, x1, y1;
    float xoff, yoff, advance;
    u32 width, height;
} neko_baked_char_t;

typedef struct neko_asset_font_t {
    void* font_info;
    // u32 glyphs_num;
    neko_baked_char_t glyphs[96];
    neko_asset_texture_t texture;
    float ascent;
    float descent;
    float line_gap;
} neko_asset_font_t;

/*==========================
// NEKO_ASSET_TYPES
==========================*/

// Texture

#if 0
bool neko_asset_texture_load_from_file(const_str path, void* out, gfx_texture_desc_t* desc, bool flip_on_load, bool keep_data);
bool neko_asset_texture_load_from_memory(const void* memory, size_t sz, void* out, gfx_texture_desc_t* desc, bool flip_on_load, bool keep_data);

// Font
bool neko_asset_font_load_from_file(const_str path, void* out, u32 point_size);
bool neko_asset_font_load_from_memory(const void* memory, size_t sz, void* out, u32 point_size);
neko_vec2 neko_asset_font_text_dimensions(const neko_asset_font_t* font, const_str text, i32 len);
neko_vec2 neko_asset_font_text_dimensions_ex(const neko_asset_font_t* fp, const_str text, i32 len, bool include_past_baseline);
float neko_asset_font_max_height(const neko_asset_font_t* font);

bool neko_asset_mesh_load_from_file(const_str path, void* out, neko_asset_mesh_decl_t* decl, void* data_out, size_t data_size);

//  bool neko_util_load_gltf_data_from_file(const_str path, neko_asset_mesh_decl_t* decl, neko_asset_mesh_raw_data_t** out, u32* mesh_count);
//  bool neko_util_load_gltf_data_from_memory(const void* memory, size_t sz, neko_asset_mesh_decl_t* decl, neko_asset_mesh_raw_data_t** out, u32* mesh_count);

#endif

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

neko_camera_t neko_camera_default();
neko_camera_t neko_camera_perspective();
neko_mat4 neko_camera_get_view(const neko_camera_t* cam);
neko_mat4 neko_camera_get_proj(const neko_camera_t* cam, i32 view_width, i32 view_height);
neko_mat4 neko_camera_get_view_projection(const neko_camera_t* cam, i32 view_width, i32 view_height);
neko_vec3 neko_camera_forward(const neko_camera_t* cam);
neko_vec3 neko_camera_backward(const neko_camera_t* cam);
neko_vec3 neko_camera_up(const neko_camera_t* cam);
neko_vec3 neko_camera_down(const neko_camera_t* cam);
neko_vec3 neko_camera_right(const neko_camera_t* cam);
neko_vec3 neko_camera_left(const neko_camera_t* cam);
neko_vec3 neko_camera_screen_to_world(const neko_camera_t* cam, neko_vec3 coords, i32 view_x, i32 view_y, i32 view_width, i32 view_height);
neko_vec3 neko_camera_world_to_screen(const neko_camera_t* cam, neko_vec3 coords, i32 view_width, i32 view_height);
void neko_camera_offset_orientation(neko_camera_t* cam, f32 yaw, f32 picth);

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
    Color256 color;
} neko_immediate_vert_t;

typedef struct neko_immediate_cache_t {
    neko_dyn_array(neko_handle(gfx_pipeline_t)) pipelines;
    neko_dyn_array(neko_mat4) modelview;
    neko_dyn_array(neko_mat4) projection;
    neko_dyn_array(neko_idraw_matrix_type) modes;
    neko_vec2 uv;
    Color256 color;
    neko_handle(gfx_texture_t) texture;
    neko_idraw_pipeline_state_attr_t pipeline;
} neko_immediate_cache_t;

typedef struct neko_immediate_draw_static_data_t {
    neko_handle(gfx_texture_t) tex_default;
    neko_asset_font_t font_default;  // Idraw font
    neko_hash_table(neko_idraw_pipeline_state_attr_t, neko_handle(gfx_pipeline_t)) pipeline_table;
    neko_handle(gfx_uniform_t) uniform;
    neko_handle(gfx_uniform_t) sampler;
    neko_handle(gfx_vertex_buffer_t) vbo;
    neko_handle(gfx_index_buffer_t) ibo;
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
neko_immediate_draw_t neko_immediate_draw_new();
void neko_immediate_draw_free(neko_immediate_draw_t* neko_idraw);
void neko_immediate_draw_static_data_set(neko_immediate_draw_static_data_t* data);
neko_immediate_draw_static_data_t* neko_immediate_draw_static_data_get();  // 用于热更新

// Get pipeline from state
neko_handle(gfx_pipeline_t) neko_idraw_get_pipeline(neko_immediate_draw_t* neko_idraw, neko_idraw_pipeline_state_attr_t state);

// Get default font asset pointer
neko_asset_font_t* neko_idraw_default_font();

// Core Vertex Functions
void neko_idraw_begin(neko_immediate_draw_t* neko_idraw, gfx_primitive_type type);
void neko_idraw_end(neko_immediate_draw_t* neko_idraw);
void neko_idraw_tc2f(neko_immediate_draw_t* neko_idraw, f32 u, f32 v);
void neko_idraw_tc2fv(neko_immediate_draw_t* neko_idraw, neko_vec2 uv);
void neko_idraw_c4ub(neko_immediate_draw_t* neko_idraw, u8 r, u8 g, u8 b, u8 a);
void neko_idraw_c4ubv(neko_immediate_draw_t* neko_idraw, Color256 c);
void neko_idraw_v2f(neko_immediate_draw_t* neko_idraw, f32 x, f32 y);
void neko_idraw_v2fv(neko_immediate_draw_t* neko_idraw, neko_vec2 v);
void neko_idraw_v3f(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z);
void neko_idraw_v3fv(neko_immediate_draw_t* neko_idraw, neko_vec3 v);
void neko_idraw_flush(neko_immediate_draw_t* neko_idraw);
void neko_idraw_texture(neko_immediate_draw_t* neko_idraw, neko_handle(gfx_texture_t) texture);

// neko_hsv_t neko_rgb_to_hsv(Color256 c);
f32 neko_hue_dist(f32 h1, f32 h2);

void neko_idraw_rect_textured(neko_immediate_draw_t* neko_idraw, neko_vec2 a, neko_vec2 b, u32 tex_id, Color256 color);
void neko_idraw_rect_textured_ext(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, f32 u0, f32 v0, f32 u1, f32 v1, u32 tex_id, Color256 color);

// Core pipeline functions
void neko_idraw_blend_enabled(neko_immediate_draw_t* neko_idraw, bool enabled);
void neko_idraw_depth_enabled(neko_immediate_draw_t* neko_idraw, bool enabled);
void neko_idraw_stencil_enabled(neko_immediate_draw_t* neko_idraw, bool enabled);
void neko_idraw_face_cull_enabled(neko_immediate_draw_t* neko_idraw, bool enabled);
void neko_idraw_defaults(neko_immediate_draw_t* neko_idraw);
void neko_idraw_pipeline_set(neko_immediate_draw_t* neko_idraw,
                             neko_handle(gfx_pipeline_t) pipeline);                               // Binds custom user pipeline, sets flag NEKO_IDRAW_FLAG_NO_BIND_CACHED_PIPELINES
void neko_idraw_vattr_list(neko_immediate_draw_t* neko_idraw, neko_idraw_vattr_type* layout, size_t sz);  // Sets user vertex attribute list for custom bound pipeline
void neko_idraw_vattr_list_mesh(neko_immediate_draw_t* neko_idraw, neko_asset_mesh_layout_t* layout,
                                size_t sz);  // Same as above but uses mesh layout to determine which vertex attributes to bind and in what order

// View/Scissor commands
void neko_idraw_set_view_scissor(neko_immediate_draw_t* neko_idraw, u32 x, u32 y, u32 w, u32 h);

// Final Submit / Merge
void neko_idraw_draw(neko_immediate_draw_t* neko_idraw, neko_command_buffer_t* cb);
void neko_idraw_renderpass_submit(neko_immediate_draw_t* neko_idraw, neko_command_buffer_t* cb, neko_vec4 viewport, Color256 clear_color);
void neko_idraw_renderpass_submit_ex(neko_immediate_draw_t* neko_idraw, neko_command_buffer_t* cb, neko_vec4 viewport, gfx_clear_action_t action);

// Core Matrix Functions
void neko_idraw_push_matrix(neko_immediate_draw_t* neko_idraw, neko_idraw_matrix_type type);
void neko_idraw_push_matrix_ex(neko_immediate_draw_t* neko_idraw, neko_idraw_matrix_type type, bool flush);
void neko_idraw_pop_matrix(neko_immediate_draw_t* neko_idraw);
void neko_idraw_pop_matrix_ex(neko_immediate_draw_t* neko_idraw, bool flush);
//  void neko_idraw_matrix_mode(neko_immediate_draw_t* neko_idraw, neko_idraw_matrix_type type);
void neko_idraw_load_matrix(neko_immediate_draw_t* neko_idraw, neko_mat4 m);
void neko_idraw_mul_matrix(neko_immediate_draw_t* neko_idraw, neko_mat4 m);
void neko_idraw_perspective(neko_immediate_draw_t* neko_idraw, f32 fov, f32 aspect, f32 near, f32 far);
void neko_idraw_ortho(neko_immediate_draw_t* neko_idraw, f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
void neko_idraw_rotatef(neko_immediate_draw_t* neko_idraw, f32 angle, f32 x, f32 y, f32 z);
void neko_idraw_rotatev(neko_immediate_draw_t* neko_idraw, f32 angle, neko_vec3 v);
void neko_idraw_translatef(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z);
void neko_idraw_translatev(neko_immediate_draw_t* neko_idraw, neko_vec3 v);
void neko_idraw_scalef(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z);

// Camera Utils
void neko_idraw_camera(neko_immediate_draw_t* neko_idraw, neko_camera_t* cam, u32 width, u32 height);
void neko_idraw_camera2d(neko_immediate_draw_t* neko_idraw, u32 width, u32 height);
void neko_idraw_camera2d_ex(neko_immediate_draw_t* neko_idraw, f32 l, f32 r, f32 t, f32 b);
void neko_idraw_camera3d(neko_immediate_draw_t* neko_idraw, u32 width, u32 height);

// Primitive Drawing Util
void neko_idraw_triangle(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, f32 x2, f32 y2, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type);
void neko_idraw_trianglev(neko_immediate_draw_t* neko_idraw, neko_vec2 a, neko_vec2 b, neko_vec2 c, Color256 color, gfx_primitive_type type);
void neko_idraw_trianglex(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2, f32 u0, f32 v0, f32 u1, f32 v1, f32 u2, f32 v2, u8 r, u8 g, u8 b,
                          u8 a, gfx_primitive_type type);
void neko_idraw_trianglevx(neko_immediate_draw_t* neko_idraw, neko_vec3 v0, neko_vec3 v1, neko_vec3 v2, neko_vec2 uv0, neko_vec2 uv1, neko_vec2 uv2, Color256 color, gfx_primitive_type type);
void neko_idraw_trianglevxmc(neko_immediate_draw_t* neko_idraw, neko_vec3 v0, neko_vec3 v1, neko_vec3 v2, neko_vec2 uv0, neko_vec2 uv1, neko_vec2 uv2, Color256 c0, Color256 c1, Color256 c2,
                             gfx_primitive_type type);
void neko_idraw_line(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, u8 r, u8 g, u8 b, u8 a);
void neko_idraw_linev(neko_immediate_draw_t* neko_idraw, neko_vec2 v0, neko_vec2 v1, Color256 c);
void neko_idraw_line3D(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1, u8 r, u8 g, u8 b, u8 a);
void neko_idraw_line3Dv(neko_immediate_draw_t* neko_idraw, neko_vec3 s, neko_vec3 e, Color256 color);
void neko_idraw_line3Dmc(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 z0, f32 x1, f32 y1, f32 z1, u8 r0, u8 g0, u8 b0, u8 a0, u8 r1, u8 g1, u8 b1, u8 a1);

// Shape Drawing Util
void neko_idraw_rect(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type);
void neko_idraw_rectv(neko_immediate_draw_t* neko_idraw, neko_vec2 bl, neko_vec2 tr, Color256 color, gfx_primitive_type type);
void neko_idraw_rectx(neko_immediate_draw_t* neko_idraw, f32 l, f32 b, f32 r, f32 t, f32 u0, f32 v0, f32 u1, f32 v1, u8 _r, u8 _g, u8 _b, u8 _a, gfx_primitive_type type);
void neko_idraw_rectvx(neko_immediate_draw_t* neko_idraw, neko_vec2 bl, neko_vec2 tr, neko_vec2 uv0, neko_vec2 uv1, Color256 color, gfx_primitive_type type);
void neko_idraw_rectvd(neko_immediate_draw_t* neko_idraw, neko_vec2 xy, neko_vec2 wh, neko_vec2 uv0, neko_vec2 uv1, Color256 color, gfx_primitive_type type);
void neko_idraw_rect3Dv(neko_immediate_draw_t* neko_idraw, neko_vec3 min, neko_vec3 max, neko_vec2 uv0, neko_vec2 uv1, Color256 color, gfx_primitive_type type);
void neko_idraw_rect3Dvd(neko_immediate_draw_t* neko_idraw, neko_vec3 xyz, neko_vec3 whd, neko_vec2 uv0, neko_vec2 uv1, Color256 c, gfx_primitive_type type);
void neko_idraw_circle(neko_immediate_draw_t* neko_idraw, f32 cx, f32 cy, f32 radius, int32_t segments, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type);
void neko_idraw_circlevx(neko_immediate_draw_t* neko_idraw, neko_vec3 c, f32 radius, int32_t segments, Color256 color, gfx_primitive_type type);
void neko_idraw_circle_sector(neko_immediate_draw_t* neko_idraw, f32 cx, f32 cy, f32 radius, int32_t start_angle, int32_t end_angle, int32_t segments, u8 r, u8 g, u8 b, u8 a,
                              gfx_primitive_type type);
void neko_idraw_circle_sectorvx(neko_immediate_draw_t* neko_idraw, neko_vec3 c, f32 radius, int32_t start_angle, int32_t end_angle, int32_t segments, Color256 color, gfx_primitive_type type);
void neko_idraw_arc(neko_immediate_draw_t* neko_idraw, f32 cx, f32 cy, f32 radius_inner, f32 radius_outer, f32 start_angle, f32 end_angle, int32_t segments, u8 r, u8 g, u8 b, u8 a,
                    gfx_primitive_type type);
void neko_idraw_box(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 z0, f32 hx, f32 hy, f32 hz, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type);
void neko_idraw_sphere(neko_immediate_draw_t* neko_idraw, f32 cx, f32 cy, f32 cz, f32 radius, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type);
void neko_idraw_bezier(neko_immediate_draw_t* neko_idraw, f32 x0, f32 y0, f32 x1, f32 y1, u8 r, u8 g, u8 b, u8 a);
void neko_idraw_cylinder(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z, f32 r_top, f32 r_bottom, f32 height, int32_t sides, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type);
void neko_idraw_cone(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z, f32 radius, f32 height, int32_t sides, u8 r, u8 g, u8 b, u8 a, gfx_primitive_type type);

// Draw planes/poly groups

// Text Drawing Util
void neko_idraw_text(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, const char* text, const neko_asset_font_t* fp, bool flip_vertical, Color256 col);

// Paths
/*
 void neko_idraw_path_begin(neko_immediate_draw_t* neko_idraw);
 void neko_idraw_path_end(neko_immediate_draw_t* neko_idraw);
 void neko_idraw_path_moveto(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z);
 void neko_idraw_path_lineto(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z);
 void neko_idraw_path_arcto(neko_immediate_draw_t* neko_idraw, f32 x, f32 y, f32 z);
*/

// Private Internal Utilities (Not user facing)
const char* __neko_internal_GetDefaultCompressedFontDataTTFBase85();
void __neko_internal_Decode85(const unsigned char* src, unsigned char* dst);
unsigned int __neko_internal_Decode85Byte(char c);

unsigned int neko_decompress_length(const unsigned char* input);
unsigned int neko_decompress(unsigned char* output, unsigned char* input, unsigned int length);

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
    gfx_uniform_type type;        // Type of uniform: R_UNIFORM_VEC2, R_UNIFORM_VEC3, etc.
    uint32_t binding;                     // Binding for this uniform in shader
    gfx_shader_stage_type stage;  // Shader stage for this uniform
    gfx_access_type access_type;  // Access type for this uniform (compute only)
} neko_draw_uniform_desc_t;

typedef struct neko_draw_uniform_t {
    neko_handle(gfx_uniform_t) hndl;  // Graphics handle resource for actual uniform
    uint32_t offset;                          // Individual offset for this uniform in material byte buffer data
    uint32_t binding;                         // Binding for this uniform
    size_t size;                              // Size of this uniform data in bytes
    gfx_uniform_type type;            // Type of this uniform
    gfx_access_type access_type;      // Access type of uniform (compute only)
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
typedef neko_handle(gfx_texture_t) neko_draw_texture_t;

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

void neko_draw_mesh_import_options_free(neko_draw_mesh_import_options_t* opt);

typedef struct neko_draw_mesh_desc_t {
    neko_draw_mesh_raw_data_t* meshes;  // Mesh data array
    size_t size;                        // Size of mesh data array in bytes
    bool keep_data;                     // Whether or not to free data after use
} neko_draw_mesh_desc_t;

typedef struct neko_draw_vertex_stream_t {
    neko_handle(gfx_vertex_buffer_t) positions;
    neko_handle(gfx_vertex_buffer_t) normals;
    neko_handle(gfx_vertex_buffer_t) tangents;
    neko_handle(gfx_vertex_buffer_t) colors[NEKO_GFXT_COLOR_MAX];
    neko_handle(gfx_vertex_buffer_t) tex_coords[NEKO_GFXT_TEX_COORD_MAX];
    neko_handle(gfx_vertex_buffer_t) joints[NEKO_GFXT_JOINT_MAX];
    neko_handle(gfx_vertex_buffer_t) weights[NEKO_GFXT_WEIGHT_MAX];
    neko_handle(gfx_vertex_buffer_t) custom_uint[NEKO_GFXT_CUSTOM_UINT_MAX];
} neko_draw_vertex_stream_t;

typedef struct neko_draw_mesh_primitive_t {
    neko_draw_vertex_stream_t stream;                 // All vertex data streams
    neko_handle(gfx_index_buffer_t) indices;  // Index buffer
    uint32_t count;                                   // Total number of vertices
} neko_draw_mesh_primitive_t;

typedef struct neko_draw_mesh_t {
    neko_dyn_array(neko_draw_mesh_primitive_t) primitives;
    neko_draw_mesh_desc_t desc;
} neko_draw_mesh_t;

//=== Pipeline ===//
typedef struct neko_draw_pipeline_desc_t {
    gfx_pipeline_desc_t pip_desc;        // Description for constructing pipeline object
    neko_draw_uniform_block_desc_t ublock_desc;  // Description for constructing uniform block object
} neko_draw_pipeline_desc_t;

typedef struct neko_draw_pipeline_t {
    neko_handle(gfx_pipeline_t) hndl;  // Graphics handle resource for actual pipeline
    neko_draw_uniform_block_t ublock;          // Uniform block for holding all uniform data
    neko_dyn_array(neko_draw_mesh_layout_t) mesh_layout;
    gfx_pipeline_desc_t desc;
} neko_draw_pipeline_t;

//=== Material ===//
typedef struct neko_draw_material_desc_t {
    neko_draw_raw_data_func_desc_t pip_func;  // Description for retrieving raw pipeline pointer data from handle.
} neko_draw_material_desc_t;

typedef struct neko_draw_material_t {
    neko_draw_material_desc_t desc;        // Material description object
    neko_byte_buffer_t uniform_data;       // Byte buffer of actual uniform data to send to GPU
    neko_byte_buffer_t image_buffer_data;  // Image buffer data
} neko_draw_material_t;

//=== Renderable ===//
typedef struct neko_draw_renderable_desc_t {
    neko_draw_raw_data_func_desc_t mesh;      // Description for retrieving raw mesh pointer data from handle.
    neko_draw_raw_data_func_desc_t material;  // Description for retrieving raw material pointer data from handle.
} neko_draw_renderable_desc_t;

typedef struct neko_draw_renderable_t {
    neko_draw_renderable_desc_t desc;  // Renderable description object
    neko_mat4 model_matrix;            // Model matrix for renderable
} neko_draw_renderable_t;

//=== Graphics scene ===//
typedef struct neko_draw_scene_t {
    neko_slot_array(neko_draw_renderable_t) renderables;
} neko_draw_scene_t;

//==== API =====//

//=== Creation ===//
neko_draw_pipeline_t neko_draw_pipeline_create(const neko_draw_pipeline_desc_t* desc);
neko_draw_material_t neko_draw_material_create(neko_draw_material_desc_t* desc);
neko_draw_mesh_t neko_draw_mesh_create(const neko_draw_mesh_desc_t* desc);
void neko_draw_mesh_update_or_create(neko_draw_mesh_t* mesh, const neko_draw_mesh_desc_t* desc);
neko_draw_renderable_t neko_draw_renderable_create(const neko_draw_renderable_desc_t* desc);
neko_draw_uniform_block_t neko_draw_uniform_block_create(const neko_draw_uniform_block_desc_t* desc);
neko_draw_texture_t neko_draw_texture_create(gfx_texture_desc_t desc);

//=== Destruction ===//
void neko_draw_texture_fini(neko_draw_texture_t* texture);
void neko_draw_material_fini(neko_draw_material_t* material);
void neko_draw_mesh_fini(neko_draw_mesh_t* mesh);
void neko_draw_uniform_block_fini(neko_draw_uniform_block_t* ub);
void neko_draw_pipeline_fini(neko_draw_pipeline_t* pipeline);

//=== Resource Loading ===//
//  neko_draw_pipeline_t neko_draw_pipeline_load_from_file(const char* path);
//  neko_draw_pipeline_t neko_draw_pipeline_load_from_memory(const char* data, size_t sz);
//  neko_draw_pipeline_t neko_draw_pipeline_load_from_memory_ext(const char* data, size_t sz, const char* file_dir);
neko_draw_texture_t neko_draw_texture_load_from_file(const char* path, gfx_texture_desc_t* desc, bool flip, bool keep_data);
neko_draw_texture_t neko_draw_texture_load_from_memory(const char* data, size_t sz, gfx_texture_desc_t* desc, bool flip, bool keep_data);

//=== Copy ===//
neko_draw_material_t neko_draw_material_deep_copy(neko_draw_material_t* src);

//=== Pipeline API ===//
neko_draw_uniform_t* neko_draw_pipeline_get_uniform(neko_draw_pipeline_t* pip, const char* name);

//=== Material API ===//
void neko_draw_material_set_uniform(neko_draw_material_t* mat, const char* name, const void* data);
void neko_draw_material_bind(neko_command_buffer_t* cb, neko_draw_material_t* mat);
void neko_draw_material_bind_pipeline(neko_command_buffer_t* cb, neko_draw_material_t* mat);
void neko_draw_material_bind_uniforms(neko_command_buffer_t* cb, neko_draw_material_t* mat);
neko_draw_pipeline_t* neko_draw_material_get_pipeline(neko_draw_material_t* mat);

//=== Mesh API ===//
void neko_draw_mesh_draw_pipeline(neko_command_buffer_t* cb, neko_draw_mesh_t* mesh, neko_draw_pipeline_t* pip);
void neko_draw_mesh_draw_material(neko_command_buffer_t* cb, neko_draw_mesh_t* mesh, neko_draw_material_t* mat);
void neko_draw_mesh_draw_materials(neko_command_buffer_t* cb, neko_draw_mesh_t* mesh, neko_draw_material_t** mats, size_t mats_size);
void neko_draw_mesh_draw_layout(neko_command_buffer_t* cb, neko_draw_mesh_t* mesh, neko_draw_mesh_layout_t* layout, size_t layout_size);
neko_draw_mesh_t neko_draw_mesh_load_from_file(const char* file, neko_draw_mesh_import_options_t* options);
//  bool neko_draw_load_gltf_data_from_file(const char* path, neko_draw_mesh_import_options_t* options, neko_draw_mesh_raw_data_t** out, uint32_t* mesh_count);

// Util API
void* neko_draw_raw_data(NEKO_GFXT_HNDL hndl, void* user_data);

// Mesh Generation API
neko_draw_mesh_t neko_draw_mesh_unit_quad_generate(neko_draw_mesh_import_options_t* options);
neko_handle(gfx_texture_t) neko_draw_texture_generate_default();

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
#define ui_widths(...)                       \
    [&]() -> const i32* {                         \
        static i32 temp_widths[] = {__VA_ARGS__}; \
        return temp_widths;                       \
    }()
#else
#define ui_widths(...) \
    (int[]) { __VA_ARGS__ }
#endif

#define ui_stack(T, n) \
    struct {                \
        i32 idx;            \
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

typedef enum ui_alt_drag_mode_type {
    NEKO_UI_ALT_DRAG_QUAD = 0x00,  // Quadrants
    NEKO_UI_ALT_DRAG_NINE,         // Nine splice the window
    NEKO_UI_ALT_DRAG_SINGLE        // Single window drag (controls the width/height, leaving x/y position of window in tact)
} ui_alt_drag_mode_type;

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

typedef struct ui_context_t ui_context_t;
typedef u32 ui_id;
typedef NEKO_UI_REAL ui_real;

// Shapes
typedef struct {
    float x, y, w, h;
} ui_rect_t;
typedef struct {
    float radius;
    neko_vec2 center;
} ui_circle_t;
typedef struct {
    neko_vec2 points[3];
} ui_triangle_t;
typedef struct {
    neko_vec2 start;
    neko_vec2 end;
} ui_line_t;

typedef struct {
    ui_id id;
    i32 last_update;
} ui_pool_item_t;

typedef struct {
    i32 type, size;
} ui_basecommand_t;

typedef struct {
    ui_basecommand_t base;
    void* dst;
} ui_jumpcommand_t;

typedef struct {
    ui_basecommand_t base;
    ui_rect_t rect;
} ui_clipcommand_t;

typedef struct {
    ui_basecommand_t base;
    neko_asset_font_t* font;
    neko_vec2 pos;
    Color256 color;
    char str[1];
} ui_textcommand_t;

typedef struct {
    ui_basecommand_t base;
    neko_handle(gfx_pipeline_t) pipeline;
    neko_idraw_layout_type layout_type;
    void* layout;
    size_t layout_sz;
} ui_pipelinecommand_t;

typedef struct {
    ui_basecommand_t base;
    void* data;
    size_t sz;
} ui_binduniformscommand_t;

typedef struct {
    ui_basecommand_t base;
    ui_rect_t rect;
    neko_handle(gfx_texture_t) hndl;
    neko_vec4 uvs;
    Color256 color;
} ui_imagecommand_t;

struct ui_customcommand_t;

// Draw Callback
typedef void (*ui_draw_callback_t)(ui_context_t* ctx, struct ui_customcommand_t* cmd);

typedef struct ui_customcommand_t {
    ui_basecommand_t base;
    ui_rect_t clip;
    ui_rect_t viewport;
    ui_id hash;
    ui_id hover;
    ui_id focus;
    ui_draw_callback_t cb;
    void* data;
    size_t sz;
} ui_customcommand_t;

typedef struct {
    ui_basecommand_t base;
    u32 type;
    union {
        ui_rect_t rect;
        ui_circle_t circle;
        ui_triangle_t triangle;
        ui_line_t line;
    };
    Color256 color;
} ui_shapecommand_t;

// 注意 考虑到如何将异构类型推入字节缓冲区 这是浪费的
typedef union {
    i32 type;
    ui_basecommand_t base;
    ui_jumpcommand_t jump;
    ui_clipcommand_t clip;
    ui_shapecommand_t shape;
    ui_textcommand_t text;
    ui_imagecommand_t image;
    ui_customcommand_t custom;
    ui_pipelinecommand_t pipeline;
    ui_binduniformscommand_t uniforms;
} ui_command_t;

struct ui_context_t;

typedef void (*ui_on_draw_button_callback)(struct ui_context_t* ctx, ui_rect_t rect, ui_id id, bool hovered, bool focused, u64 opt, const char* label, i32 icon);

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
} ui_layout_anchor_type;

typedef enum { NEKO_UI_ALIGN_START = 0x00, NEKO_UI_ALIGN_CENTER, NEKO_UI_ALIGN_END } ui_align_type;

typedef enum { NEKO_UI_JUSTIFY_START = 0x00, NEKO_UI_JUSTIFY_CENTER, NEKO_UI_JUSTIFY_END } ui_justification_type;

typedef enum { NEKO_UI_DIRECTION_COLUMN = 0x00, NEKO_UI_DIRECTION_ROW, NEKO_UI_DIRECTION_COLUMN_REVERSE, NEKO_UI_DIRECTION_ROW_REVERSE } ui_direction;

typedef struct ui_layout_t {
    ui_rect_t body;
    ui_rect_t next;
    neko_vec2 position;
    neko_vec2 size;
    neko_vec2 max;
    i32 padding[4];
    i32 widths[NEKO_UI_MAX_WIDTHS];
    i32 items;
    i32 item_index;
    i32 next_row;
    i32 next_type;
    i32 indent;

    // flex direction / justification / alignment
    i32 direction;
    i32 justify_content;
    i32 align_content;

} ui_layout_t;

// Forward decl.
struct ui_container_t;

typedef enum ui_split_node_type { NEKO_UI_SPLIT_NODE_CONTAINER = 0x00, NEKO_UI_SPLIT_NODE_SPLIT } ui_split_node_type;

enum { NEKO_UI_SPLIT_NODE_CHILD = 0x00, NEKO_UI_SPLIT_NODE_PARENT };

typedef struct ui_split_node_t {
    ui_split_node_type type;
    union {
        u32 split;
        struct ui_container_t* container;
    };
} ui_split_node_t;

typedef enum ui_split_type { NEKO_UI_SPLIT_LEFT = 0x00, NEKO_UI_SPLIT_RIGHT, NEKO_UI_SPLIT_TOP, NEKO_UI_SPLIT_BOTTOM, NEKO_UI_SPLIT_TAB } ui_split_type;

typedef struct ui_split_t {
    ui_split_type type;  // NEKO_UI_SPLIT_LEFT, NEKO_UI_SPLIT_RIGHT, NEKO_UI_SPLIT_TAB, NEKO_UI_SPLIT_BOTTOM, NEKO_UI_SPLIT_TOP
    float ratio;              // Split ratio between children [0.f, 1.f], (left node = ratio), right node = (1.f - ratio)
    ui_rect_t rect;
    ui_rect_t prev_rect;
    ui_split_node_t children[2];
    u32 parent;
    u32 id;
    u32 zindex;
    i32 frame;
} ui_split_t;

typedef enum ui_window_flags {
    NEKO_UI_WINDOW_FLAGS_VISIBLE = (1 << 0),     //
    NEKO_UI_WINDOW_FLAGS_FIRST_INIT = (1 << 1),  //
    NEKO_UI_WINDOW_FLAGS_PUSH_ID = (1 << 2)      //
} ui_window_flags;

// Equidistantly sized tabs, based on rect of window
typedef struct ui_tab_item_t {
    ui_id tab_bar;
    u32 zindex;  // Sorting index in tab bar
    void* data;  // User set data pointer for this item
    u32 idx;     // Internal index
} ui_tab_item_t;

typedef struct ui_tab_bar_t {
    ui_tab_item_t items[NEKO_UI_TAB_ITEM_MAX];
    u32 size;             // Current number of items in tab bar
    ui_rect_t rect;  // Cached sized for tab bar
    u32 focus;            // Focused item in tab bar
} ui_tab_bar_t;

typedef struct ui_container_t {
    ui_command_t *head, *tail;
    ui_rect_t rect;
    ui_rect_t body;
    neko_vec2 content_size;
    neko_vec2 scroll;
    i32 zindex;
    i32 open;
    ui_id id;
    ui_id split;  // If container is docked, then will have owning split to get sizing (0x00 for NULL)
    u32 tab_bar;
    u32 tab_item;
    struct ui_container_t* parent;  // Owning parent (for tabbing)
    u64 opt;
    u32 frame;
    u32 visible;
    i32 flags;
    char name[256];
} ui_container_t;

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
} ui_element_state;

typedef enum ui_element_type {
    NEKO_UI_ELEMENT_CONTAINER = 0x00,
    NEKO_UI_ELEMENT_LABEL,
    NEKO_UI_ELEMENT_TEXT,
    NEKO_UI_ELEMENT_PANEL,
    NEKO_UI_ELEMENT_INPUT,
    NEKO_UI_ELEMENT_BUTTON,
    NEKO_UI_ELEMENT_SCROLL,
    NEKO_UI_ELEMENT_IMAGE,
    NEKO_UI_ELEMENT_COUNT
} ui_element_type;

typedef enum { NEKO_UI_PADDING_LEFT = 0x00, NEKO_UI_PADDING_RIGHT, NEKO_UI_PADDING_TOP, NEKO_UI_PADDING_BOTTOM } ui_padding_type;

typedef enum { NEKO_UI_MARGIN_LEFT = 0x00, NEKO_UI_MARGIN_RIGHT, NEKO_UI_MARGIN_TOP, NEKO_UI_MARGIN_BOTTOM } ui_margin_type;

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

} ui_style_element_type;

enum { NEKO_UI_ANIMATION_DIRECTION_FORWARD = 0x00, NEKO_UI_ANIMATION_DIRECTION_BACKWARD };

typedef struct {
    ui_style_element_type type;
    union {
        i32 value;
        Color256 color;
        neko_asset_font_t* font;
    };
} ui_style_element_t;

typedef struct ui_animation_t {
    i16 max;          // max time
    i16 time;         // current time
    i16 delay;        // delay
    i16 curve;        // curve type
    i16 direction;    // current direction
    i16 playing;      // whether or not active
    i16 iterations;   // number of iterations to play the animation
    i16 focus_state;  // cached focus_state from frame (want to delete this somehow)
    i16 hover_state;  // cached hover_state from frame (want to delete this somehow)
    i16 start_state;  // starting state for animation blend
    i16 end_state;    // ending state for animation blend
    i32 frame;        // current frame (to match)
} ui_animation_t;

typedef struct ui_style_t {
    // font
    neko_asset_font_t* font;

    // dimensions
    float size[2];
    i16 spacing;         // get rid of    (use padding)
    i16 indent;          // get rid of    (use margin)
    i16 title_height;    // get rid of    (use title_bar style)
    i16 scrollbar_size;  // get rid of    (use scroll style)
    i16 thumb_size;      // get rid of    (use various styles)

    // colors
    Color256 colors[NEKO_UI_COLOR_MAX];

    // padding/margin
    i32 padding[4];
    i16 margin[4];

    // border
    i16 border_width[4];
    i16 border_radius[4];

    // flex direction / justification / alignment
    i16 direction;
    i16 justify_content;
    i16 align_content;

    // shadow amount each direction
    i16 shadow_x;
    i16 shadow_y;

} ui_style_t;

// Keep animation properties lists within style sheet to look up

typedef struct ui_animation_property_t {
    ui_style_element_type type;
    i16 time;
    i16 delay;
} ui_animation_property_t;

typedef struct ui_animation_property_list_t {
    neko_dyn_array(ui_animation_property_t) properties[3];
} ui_animation_property_list_t;

/*
   element type
   classes
   id

   ui_button(gui, "Text##.cls#id");
   ui_label(gui, "Title###title");

    button .class #id : hover {         // All of these styles get concat into one?
    }
*/

typedef struct {
    neko_dyn_array(ui_style_element_t) styles[3];
} ui_style_list_t;

typedef struct ui_style_sheet_t {
    ui_style_t styles[NEKO_UI_ELEMENT_COUNT][3];  // default | hovered | focused
    neko_hash_table(ui_element_type, ui_animation_property_list_t) animations;

    neko_hash_table(u64, ui_style_list_t) cid_styles;
    neko_hash_table(u64, ui_animation_property_list_t) cid_animations;
} ui_style_sheet_t;

typedef struct ui_style_sheet_element_desc_t {

    struct {

        struct {
            ui_style_element_t* data;
            size_t size;
        } style;

        struct {
            ui_animation_property_t* data;
            size_t size;
        } animation;

    } all;

    struct {

        struct {
            ui_style_element_t* data;
            size_t size;
        } style;

        struct {
            ui_animation_property_t* data;
            size_t size;
        } animation;

    } def;

    struct {
        struct {
            ui_style_element_t* data;
            size_t size;
        } style;

        struct {
            ui_animation_property_t* data;
            size_t size;
        } animation;
    } hover;

    struct {
        struct {
            ui_style_element_t* data;
            size_t size;
        } style;

        struct {
            ui_animation_property_t* data;
            size_t size;
        } animation;
    } focus;

} ui_style_sheet_element_desc_t;

typedef ui_style_sheet_element_desc_t ui_inline_style_desc_t;

typedef struct ui_style_sheet_desc_t {
    ui_style_sheet_element_desc_t container;
    ui_style_sheet_element_desc_t button;
    ui_style_sheet_element_desc_t panel;
    ui_style_sheet_element_desc_t input;
    ui_style_sheet_element_desc_t text;
    ui_style_sheet_element_desc_t label;
    ui_style_sheet_element_desc_t scroll;
    ui_style_sheet_element_desc_t tab;
    ui_style_sheet_element_desc_t menu;
    ui_style_sheet_element_desc_t title;
    ui_style_sheet_element_desc_t image;
} ui_style_sheet_desc_t;

typedef enum ui_request_type {
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
} ui_request_type;

typedef struct ui_request_t {
    ui_request_type type;
    union {
        ui_split_type split_type;
        ui_split_t* split;
        ui_container_t* cnt;
    };
    u32 frame;
} ui_request_t;

typedef struct ui_inline_style_stack_t {
    neko_dyn_array(ui_style_element_t) styles[3];
    neko_dyn_array(ui_animation_property_t) animations[3];
    neko_dyn_array(u32) style_counts;      // amount of styles to pop off at "top of stack" for each state
    neko_dyn_array(u32) animation_counts;  // amount of animations to pop off at "top of stack" for each state
} ui_inline_style_stack_t;

typedef struct ui_context_t {
    // Core state
    ui_style_t* style;              // Active style
    ui_style_sheet_t* style_sheet;  // Active style sheet
    ui_id hover;
    ui_id focus;
    ui_id last_id;
    ui_id state_switch_id;  // Id that had a state switch
    i32 switch_state;
    ui_id lock_focus;
    i32 last_hover_state;
    i32 last_focus_state;
    ui_id prev_hover;
    ui_id prev_focus;
    ui_rect_t last_rect;
    i32 last_zindex;
    i32 updated_focus;
    i32 frame;
    neko_vec2 framebuffer_size;
    ui_rect_t viewport;
    ui_container_t* active_root;
    ui_container_t* hover_root;
    ui_container_t* next_hover_root;
    ui_container_t* scroll_target;
    ui_container_t* focus_root;
    ui_container_t* next_focus_root;
    ui_container_t* dockable_root;
    ui_container_t* prev_dockable_root;
    ui_container_t* docked_root;
    ui_container_t* undock_root;
    ui_split_t* focus_split;
    ui_split_t* next_hover_split;
    ui_split_t* hover_split;
    ui_id next_lock_hover_id;
    ui_id lock_hover_id;
    char number_edit_buf[NEKO_UI_MAX_FMT];
    ui_id number_edit;
    ui_alt_drag_mode_type alt_drag_mode;
    neko_dyn_array(ui_request_t) requests;

    // Stacks
    ui_stack(u8, NEKO_UI_COMMANDLIST_SIZE) command_list;
    ui_stack(ui_container_t*, NEKO_UI_ROOTLIST_SIZE) root_list;
    ui_stack(ui_container_t*, NEKO_UI_CONTAINERSTACK_SIZE) container_stack;
    ui_stack(ui_rect_t, NEKO_UI_CLIPSTACK_SIZE) clip_stack;
    ui_stack(ui_id, NEKO_UI_IDSTACK_SIZE) id_stack;
    ui_stack(ui_layout_t, NEKO_UI_LAYOUTSTACK_SIZE) layout_stack;

    // Style sheet element stacks
    neko_hash_table(ui_element_type, ui_inline_style_stack_t) inline_styles;

    // Retained state pools
    ui_pool_item_t container_pool[NEKO_UI_CONTAINERPOOL_SIZE];
    ui_container_t containers[NEKO_UI_CONTAINERPOOL_SIZE];
    ui_pool_item_t treenode_pool[NEKO_UI_TREENODEPOOL_SIZE];

    neko_slot_array(ui_split_t) splits;
    neko_slot_array(ui_tab_bar_t) tab_bars;

    // Input state
    neko_vec2 mouse_pos;
    neko_vec2 last_mouse_pos;
    neko_vec2 mouse_delta;
    neko_vec2 scroll_delta;
    i32 mouse_down;
    i32 mouse_pressed;
    i32 key_down;
    i32 key_pressed;
    char input_text[32];

    // Backend resources
    u32 window_hndl;
    neko_immediate_draw_t gui_idraw;
    neko_immediate_draw_t overlay_draw_list;

    // Active Transitions
    neko_hash_table(ui_id, ui_animation_t) animations;

    // Font stash
    neko_hash_table(u64, neko_asset_font_t*) font_stash;

    // Callbacks
    struct {
        ui_on_draw_button_callback button;
    } callbacks;

} ui_context_t;

typedef struct {
    // Core state
    ui_style_t* style;              // Active style
    ui_style_sheet_t* style_sheet;  // Active style sheet
    ui_id hover;
    ui_id focus;
    ui_id last_id;
    ui_id lock_focus;
    ui_rect_t last_rect;
    i32 last_zindex;
    i32 updated_focus;
    i32 frame;
    ui_container_t* hover_root;
    ui_container_t* next_hover_root;
    ui_container_t* scroll_target;
    ui_container_t* focus_root;
    ui_container_t* next_focus_root;
    ui_container_t* dockable_root;
    ui_container_t* prev_dockable_root;
    ui_container_t* docked_root;
    ui_container_t* undock_root;
    ui_split_t* focus_split;
    ui_split_t* next_hover_split;
    ui_split_t* hover_split;
    ui_id next_lock_hover_id;
    ui_id lock_hover_id;
    char number_edit_buf[NEKO_UI_MAX_FMT];
    ui_id number_edit;
    ui_alt_drag_mode_type alt_drag_mode;
    neko_dyn_array(ui_request_t) requests;

    // Stacks
    ui_stack(ui_container_t*, NEKO_UI_CONTAINERSTACK_SIZE) container_stack;

    neko_dyn_array(u8) command_list;
    neko_dyn_array(ui_container_t*) root_list;
    neko_dyn_array(ui_rect_t) clip_stack;
    neko_dyn_array(ui_id) id_stack;
    neko_dyn_array(ui_layout_t) layout_stack;

    // Retained state pools
    ui_pool_item_t container_pool[NEKO_UI_CONTAINERPOOL_SIZE];
    ui_pool_item_t treenode_pool[NEKO_UI_TREENODEPOOL_SIZE];

    neko_slot_array(ui_split_t) splits;
    neko_slot_array(ui_tab_bar_t) tab_bars;

    // Input state
    neko_vec2 mouse_pos;
    neko_vec2 last_mouse_pos;
    neko_vec2 mouse_delta;
    neko_vec2 scroll_delta;
    i16 mouse_down;
    i16 mouse_pressed;
    i16 key_down;
    i16 key_pressed;
    char input_text[32];

    // Backend resources
    u32 window_hndl;
    neko_immediate_draw_t gui_idraw;
    neko_immediate_draw_t overlay_draw_list;

    // Active Transitions
    neko_hash_table(ui_id, ui_animation_t) animations;

    // Callbacks
    struct {
        ui_on_draw_button_callback button;
    } callbacks;

} ui_context_pruned_t;

typedef struct {
    const char* key;
    neko_asset_font_t* font;
} ui_font_desc_t;

typedef struct {
    ui_font_desc_t* fonts;
    size_t size;
} ui_font_stash_desc_t;

typedef struct {
    const char* id;                                 // Id selector
    const char* classes[NEKO_UI_CLS_SELECTOR_MAX];  // Class selectors
} ui_selector_desc_t;

enum { NEKO_UI_HINT_FLAG_NO_SCALE_BIAS_MOUSE = (1 << 0), NEKO_UI_HINT_FLAG_NO_INVERT_Y = (1 << 1) };

typedef struct ui_hints_t {
    neko_vec2 framebuffer_size;  // Overall framebuffer size
    ui_rect_t viewport;     // Viewport within framebuffer for gui context
    i32 flags;                   // Flags for hints
} ui_hints_t;

ui_rect_t ui_rect(float x, float y, float w, float h);

//=== Context ===//

ui_context_t ui_new(u32 window_hndl);
void ui_init(ui_context_t* ctx, u32 window_hndl);
void ui_init_font_stash(ui_context_t* ctx, ui_font_stash_desc_t* desc);
ui_context_t ui_context_new(u32 window_hndl);
void ui_free(ui_context_t* ctx);
void ui_begin(ui_context_t* ctx, const ui_hints_t* hints);
void ui_end(ui_context_t* ctx, bool update);
void ui_render(ui_context_t* ctx, neko_command_buffer_t* cb);

//=== Util ===//
void ui_renderpass_submit(ui_context_t* ctx, neko_command_buffer_t* cb, Color256 clear);
void ui_renderpass_submit_ex(ui_context_t* ctx, neko_command_buffer_t* cb, gfx_clear_action_t action);
void ui_parse_id_tag(ui_context_t* ctx, const char* str, char* buffer, size_t sz, u64 opt);
void ui_parse_label_tag(ui_context_t* ctx, const char* str, char* buffer, size_t sz);

//=== Main API ===//

void ui_set_focus(ui_context_t* ctx, ui_id id);
void ui_set_hover(ui_context_t* ctx, ui_id id);
ui_id ui_get_id(ui_context_t* ctx, const void* data, i32 size);
void ui_push_id(ui_context_t* ctx, const void* data, i32 size);
void ui_pop_id(ui_context_t* ctx);
void ui_push_clip_rect(ui_context_t* ctx, ui_rect_t rect);
void ui_pop_clip_rect(ui_context_t* ctx);
ui_rect_t ui_get_clip_rect(ui_context_t* ctx);
i32 ui_check_clip(ui_context_t* ctx, ui_rect_t r);
i32 ui_mouse_over(ui_context_t* ctx, ui_rect_t rect);
void ui_update_control(ui_context_t* ctx, ui_id id, ui_rect_t rect, u64 opt);

//=== Conatiner ===//

ui_container_t* ui_get_current_container(ui_context_t* ctx);
ui_container_t* ui_get_container(ui_context_t* ctx, const char* name);
ui_container_t* ui_get_top_most_container(ui_context_t* ctx, ui_split_t* split);
ui_container_t* ui_get_container_ex(ui_context_t* ctx, ui_id id, u64 opt);
void ui_bring_to_front(ui_context_t* ctx, ui_container_t* cnt);
void ui_bring_split_to_front(ui_context_t* ctx, ui_split_t* split);
ui_split_t* ui_get_split(ui_context_t* ctx, ui_container_t* cnt);
ui_tab_bar_t* ui_get_tab_bar(ui_context_t* ctx, ui_container_t* cnt);
void ui_tab_item_swap(ui_context_t* ctx, ui_container_t* cnt, i32 direction);
ui_container_t* ui_get_root_container(ui_context_t* ctx, ui_container_t* cnt);
ui_container_t* ui_get_root_container_from_split(ui_context_t* ctx, ui_split_t* split);
ui_container_t* ui_get_parent(ui_context_t* ctx, ui_container_t* cnt);
void ui_current_container_close(ui_context_t* ctx);

//=== Animation ===//

ui_animation_t* ui_get_animation(ui_context_t* ctx, ui_id id, const ui_selector_desc_t* desc, i32 elementid);

ui_style_t ui_animation_get_blend_style(ui_context_t* ctx, ui_animation_t* anim, const ui_selector_desc_t* desc, i32 elementid);

//=== Style Sheet ===//

ui_style_sheet_t ui_style_sheet_create(ui_context_t* ctx, ui_style_sheet_desc_t* desc);
void ui_style_sheet_fini(ui_style_sheet_t* ss);
void ui_set_element_style(ui_context_t* ctx, ui_element_type element, ui_element_state state, ui_style_element_t* style, size_t size);
void ui_style_sheet_set_element_styles(ui_style_sheet_t* style_sheet, ui_element_type element, ui_element_state state, ui_style_element_t* styles, size_t size);
void ui_set_style_sheet(ui_context_t* ctx, ui_style_sheet_t* style_sheet);

void ui_push_inline_style(ui_context_t* ctx, ui_element_type elementid, ui_inline_style_desc_t* desc);
void ui_pop_inline_style(ui_context_t* ctx, ui_element_type elementid);

ui_style_t* ui_push_style(ui_context_t* ctx, ui_style_t* style);
void ui_pop_style(ui_context_t* ctx, ui_style_t* style);

//=== Pools ===//

i32 ui_pool_init(ui_context_t* ctx, ui_pool_item_t* items, i32 len, ui_id id);
i32 ui_pool_get(ui_context_t* ctx, ui_pool_item_t* items, i32 len, ui_id id);
void ui_pool_update(ui_context_t* ctx, ui_pool_item_t* items, i32 idx);

//=== Input ===//

void ui_input_mousemove(ui_context_t* ctx, i32 x, i32 y);
void ui_input_mousedown(ui_context_t* ctx, i32 x, i32 y, i32 btn);
void ui_input_mouseup(ui_context_t* ctx, i32 x, i32 y, i32 btn);
void ui_input_scroll(ui_context_t* ctx, i32 x, i32 y);
void ui_input_keydown(ui_context_t* ctx, i32 key);
void ui_input_keyup(ui_context_t* ctx, i32 key);
void ui_input_text(ui_context_t* ctx, const char* text);

//=== Commands ===//

ui_command_t* ui_push_command(ui_context_t* ctx, i32 type, i32 size);
i32 ui_next_command(ui_context_t* ctx, ui_command_t** cmd);
void ui_set_clip(ui_context_t* ctx, ui_rect_t rect);
void ui_set_pipeline(ui_context_t* ctx, neko_handle(gfx_pipeline_t) pip, void* layout, size_t layout_sz, neko_idraw_layout_type layout_type);
void ui_bind_uniforms(ui_context_t* ctx, gfx_bind_uniform_desc_t* uniforms, size_t uniforms_sz);

//=== Drawing ===//

void ui_draw_rect(ui_context_t* ctx, ui_rect_t rect, Color256 color);
void ui_draw_circle(ui_context_t* ctx, neko_vec2 position, float radius, Color256 color);
void ui_draw_triangle(ui_context_t* ctx, neko_vec2 a, neko_vec2 b, neko_vec2 c, Color256 color);
void ui_draw_box(ui_context_t* ctx, ui_rect_t rect, i16* width, Color256 color);
void ui_draw_line(ui_context_t* ctx, neko_vec2 start, neko_vec2 end, Color256 color);
void ui_draw_text(ui_context_t* ctx, neko_asset_font_t* font, const char* str, i32 len, neko_vec2 pos, Color256 color, i32 shadow_x, i32 shadow_y, Color256 shadow_color);
void ui_draw_image(ui_context_t* ctx, neko_handle(gfx_texture_t) hndl, ui_rect_t rect, neko_vec2 uv0, neko_vec2 uv1, Color256 color);
void ui_draw_nine_rect(ui_context_t* ctx, neko_handle(gfx_texture_t) hndl, ui_rect_t rect, neko_vec2 uv0, neko_vec2 uv1, u32 left, u32 right, u32 top, u32 bottom,
                            Color256 color);
void ui_draw_control_frame(ui_context_t* ctx, ui_id id, ui_rect_t rect, i32 elementid, u64 opt);
void ui_draw_control_text(ui_context_t* ctx, const char* str, ui_rect_t rect, const ui_style_t* style, u64 opt);
void ui_draw_custom(ui_context_t* ctx, ui_rect_t rect, ui_draw_callback_t cb, void* data, size_t sz);

//=== Layout ===//

ui_layout_t* ui_get_layout(ui_context_t* ctx);
void ui_layout_row(ui_context_t* ctx, i32 items, const i32* widths, i32 height);
void ui_layout_row_ex(ui_context_t* ctx, i32 items, const i32* widths, i32 height, i32 justification);
void ui_layout_width(ui_context_t* ctx, i32 width);
void ui_layout_height(ui_context_t* ctx, i32 height);
void ui_layout_column_begin(ui_context_t* ctx);
void ui_layout_column_end(ui_context_t* ctx);
void ui_layout_set_next(ui_context_t* ctx, ui_rect_t r, i32 relative);
ui_rect_t ui_layout_peek_next(ui_context_t* ctx);
ui_rect_t ui_layout_next(ui_context_t* ctx);
ui_rect_t ui_layout_anchor(const ui_rect_t* parent, i32 width, i32 height, i32 xoff, i32 yoff, ui_layout_anchor_type type);

//=== Elements ===//

#define ui_button(_CTX, _LABEL) ui_button_ex((_CTX), (_LABEL), NULL, NEKO_UI_OPT_LEFTCLICKONLY)
#define ui_text(_CTX, _TXT) ui_text_ex((_CTX), (_TXT), 1, NULL, 0x00)
// #define ui_text_fc(_CTX, _TXT) ui_text_fc_ex((_CTX), (_TXT), (-1))
#define ui_textbox(_CTX, _BUF, _BUFSZ) ui_textbox_ex((_CTX), (_BUF), (_BUFSZ), NULL, 0x00)
#define ui_slider(_CTX, _VALUE, _LO, _HI) ui_slider_ex((_CTX), (_VALUE), (_LO), (_HI), 0, NEKO_UI_SLIDER_FMT, NULL, 0x00)
#define ui_number(_CTX, _VALUE, _STEP) ui_number_ex((_CTX), (_VALUE), (_STEP), NEKO_UI_SLIDER_FMT, NULL, 0x00)
#define ui_header(_CTX, _LABEL) ui_header_ex((_CTX), (_LABEL), NULL, 0x00)
#define ui_checkbox(_CTX, _LABEL, _STATE) ui_checkbox_ex((_CTX), (_LABEL), (_STATE), NULL, NEKO_UI_OPT_LEFTCLICKONLY)
#define ui_treenode_begin(_CTX, _LABEL) ui_treenode_begin_ex((_CTX), (_LABEL), NULL, 0x00)
#define ui_window_begin(_CTX, _TITLE, _RECT) ui_window_begin_ex((_CTX), (_TITLE), (_RECT), 0, NULL, 0x00)
#define ui_popup_begin(_CTX, _TITLE, _RECT) ui_popup_begin_ex((_CTX), (_TITLE), (_RECT), NULL, 0x00)
#define ui_panel_begin(_CTX, _NAME) ui_panel_begin_ex((_CTX), (_NAME), NULL, 0x00)
#define ui_image(_CTX, _HNDL) ui_image_ex((_CTX), (_HNDL), neko_v2s(0.f), neko_v2s(1.f), NULL, 0x00)
#define ui_combo_begin(_CTX, _ID, _ITEM, _MAX) ui_combo_begin_ex((_CTX), (_ID), (_ITEM), (_MAX), NULL, 0x00)
#define ui_combo_item(_CTX, _NAME) ui_combo_item_ex((_CTX), (_NAME), NULL, 0x00)
#define ui_dock(_CTX, _DST, _SRC, _TYPE) ui_dock_ex((_CTX), (_DST), (_SRC), (_TYPE), 0.5f)
#define ui_undock(_CTX, _NAME) ui_undock_ex((_CTX), (_NAME))
#define ui_label(_CTX, _FMT, ...) (neko_snprintf((_CTX)->number_edit_buf, sizeof((_CTX)->number_edit_buf), _FMT, ##__VA_ARGS__), ui_label_ex((_CTX), (_CTX)->number_edit_buf, NULL, 0x00))
#define ui_labelf(STR, ...)                         \
    do {                                                 \
        neko_snprintfc(BUFFER, 256, STR, ##__VA_ARGS__); \
        ui_label(&ENGINE_INTERFACE()->ui, BUFFER);  \
    } while (0)

//=== Elements (Extended) ===//

i32 ui_image_ex(ui_context_t* ctx, neko_handle(gfx_texture_t) hndl, neko_vec2 uv0, neko_vec2 uv1, const ui_selector_desc_t* desc, u64 opt);
i32 ui_text_ex(ui_context_t* ctx, const char* text, i32 text_wrap, const ui_selector_desc_t* desc, u64 opt);
i32 ui_label_ex(ui_context_t* ctx, const char* text, const ui_selector_desc_t* desc, u64 opt);
i32 ui_button_ex(ui_context_t* ctx, const char* label, const ui_selector_desc_t* desc, u64 opt);
i32 ui_checkbox_ex(ui_context_t* ctx, const char* label, i32* state, const ui_selector_desc_t* desc, u64 opt);
i32 ui_textbox_raw(ui_context_t* ctx, char* buf, i32 bufsz, ui_id id, ui_rect_t r, const ui_selector_desc_t* desc, u64 opt);
i32 ui_textbox_ex(ui_context_t* ctx, char* buf, i32 bufsz, const ui_selector_desc_t* desc, u64 opt);
i32 ui_slider_ex(ui_context_t* ctx, ui_real* value, ui_real low, ui_real high, ui_real step, const char* fmt, const ui_selector_desc_t* desc, u64 opt);
i32 ui_number_ex(ui_context_t* ctx, ui_real* value, ui_real step, const char* fmt, const ui_selector_desc_t* desc, u64 opt);
i32 ui_header_ex(ui_context_t* ctx, const char* label, const ui_selector_desc_t* desc, u64 opt);
i32 ui_treenode_begin_ex(ui_context_t* ctx, const char* label, const ui_selector_desc_t* desc, u64 opt);
void ui_treenode_end(ui_context_t* ctx);
i32 ui_window_begin_ex(ui_context_t* ctx, const char* title, ui_rect_t rect, bool* open, const ui_selector_desc_t* desc, u64 opt);
void ui_window_end(ui_context_t* ctx);
void ui_popup_open(ui_context_t* ctx, const char* name);
i32 ui_popup_begin_ex(ui_context_t* ctx, const char* name, ui_rect_t r, const ui_selector_desc_t* desc, u64 opt);
void ui_popup_end(ui_context_t* ctx);
void ui_panel_begin_ex(ui_context_t* ctx, const char* name, const ui_selector_desc_t* desc, u64 opt);
void ui_panel_end(ui_context_t* ctx);
i32 ui_combo_begin_ex(ui_context_t* ctx, const char* id, const char* current_item, i32 max_items, ui_selector_desc_t* desc, u64 opt);
void ui_combo_end(ui_context_t* ctx);
i32 ui_combo_item_ex(ui_context_t* ctx, const char* label, const ui_selector_desc_t* desc, u64 opt);

//=== Demos ===//

i32 ui_style_editor(ui_context_t* ctx, ui_style_sheet_t* style_sheet, ui_rect_t rect, bool* open);
i32 ui_demo_window(ui_context_t* ctx, ui_rect_t rect, bool* open);

//=== Docking ===//

void ui_dock_ex(ui_context_t* ctx, const char* dst, const char* src, i32 split_type, float ratio);
void ui_undock_ex(ui_context_t* ctx, const char* name);
void ui_dock_ex_cnt(ui_context_t* ctx, ui_container_t* dst, ui_container_t* src, i32 split_type, float ratio);
void ui_undock_ex_cnt(ui_context_t* ctx, ui_container_t* cnt);

//=== Gizmo ===//

i32 ui_gizmo(ui_context_t* ctx, neko_camera_t* camera, neko_vqs* model, ui_rect_t viewport, bool invert_view_y, float snap, i32 op, i32 mode, u64 opt);

#endif

#endif