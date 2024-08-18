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

// Useful macro for forcing enum decl to be u32 type with default = 0x00 for quick init
#define neko_enum_decl(NAME, ...) typedef enum NAME { _neko_##NAME##_default = 0x0, __VA_ARGS__, _neko_##NAME##_count, _neko_##NAME##_force_u32 = 0x7fffffff } NAME;

#define neko_enum_count(NAME) _neko_##NAME##_count

// Shader Stage Type
neko_enum_decl(gfx_shader_stage_type, R_SHADER_STAGE_VERTEX, R_SHADER_STAGE_FRAGMENT, R_SHADER_STAGE_COMPUTE);

// Winding Order Type
neko_enum_decl(gfx_winding_order_type, R_WINDING_ORDER_CW, R_WINDING_ORDER_CCW);

// Face Culling Type
neko_enum_decl(gfx_face_culling_type, R_FACE_CULLING_FRONT, R_FACE_CULLING_BACK, R_FACE_CULLING_FRONT_AND_BACK);

// Blend Equation Type
neko_enum_decl(gfx_blend_equation_type, R_BLEND_EQUATION_ADD, R_BLEND_EQUATION_SUBTRACT, R_BLEND_EQUATION_REVERSE_SUBTRACT, R_BLEND_EQUATION_MIN, R_BLEND_EQUATION_MAX);

// Blend Mode Type
neko_enum_decl(gfx_blend_mode_type, R_BLEND_MODE_ZERO, R_BLEND_MODE_ONE, R_BLEND_MODE_SRC_COLOR, R_BLEND_MODE_ONE_MINUS_SRC_COLOR, R_BLEND_MODE_DST_COLOR, R_BLEND_MODE_ONE_MINUS_DST_COLOR,
               R_BLEND_MODE_SRC_ALPHA, R_BLEND_MODE_ONE_MINUS_SRC_ALPHA, R_BLEND_MODE_DST_ALPHA, R_BLEND_MODE_ONE_MINUS_DST_ALPHA, R_BLEND_MODE_CONSTANT_COLOR, R_BLEND_MODE_ONE_MINUS_CONSTANT_COLOR,
               R_BLEND_MODE_CONSTANT_ALPHA, R_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA);

// Shader Language Type
neko_enum_decl(gfx_shader_language_type, R_SHADER_LANGUAGE_GLSL);

// Push Constant Type
// Really don't want to handle "auto-merging" of data types

// Uniform Type
neko_enum_decl(gfx_uniform_type, R_UNIFORM_FLOAT, R_UNIFORM_INT, R_UNIFORM_VEC2, R_UNIFORM_VEC3, R_UNIFORM_VEC4, R_UNIFORM_MAT4, R_UNIFORM_SAMPLER2D, R_UNIFORM_USAMPLER2D, R_UNIFORM_SAMPLERCUBE,
               R_UNIFORM_IMAGE2D_RGBA32F, R_UNIFORM_BLOCK);

// Uniform Block Usage Type
neko_enum_decl(gfx_uniform_block_usage_type,
               R_UNIFORM_BLOCK_USAGE_STATIC,  // Default of 0x00 is static
               R_UNIFORM_BLOCK_USAGE_PUSH_CONSTANT);

// Sampler Type
neko_enum_decl(gfx_sampler_type, R_SAMPLER_2D);

// Primitive Type
neko_enum_decl(gfx_primitive_type, R_PRIMITIVE_LINES, R_PRIMITIVE_TRIANGLES, R_PRIMITIVE_QUADS);

// Vertex Atribute Type
neko_enum_decl(gfx_vertex_attribute_type, R_VERTEX_ATTRIBUTE_FLOAT4, R_VERTEX_ATTRIBUTE_FLOAT3, R_VERTEX_ATTRIBUTE_FLOAT2, R_VERTEX_ATTRIBUTE_FLOAT, R_VERTEX_ATTRIBUTE_UINT4, R_VERTEX_ATTRIBUTE_UINT3,
               R_VERTEX_ATTRIBUTE_UINT2, R_VERTEX_ATTRIBUTE_UINT, R_VERTEX_ATTRIBUTE_BYTE4, R_VERTEX_ATTRIBUTE_BYTE3, R_VERTEX_ATTRIBUTE_BYTE2, R_VERTEX_ATTRIBUTE_BYTE);

// Buffer Type
neko_enum_decl(gfx_buffer_type, R_BUFFER_VERTEX, R_BUFFER_INDEX, R_BUFFER_FRAME, R_BUFFER_UNIFORM, R_BUFFER_UNIFORM_CONSTANT, R_BUFFER_SHADER_STORAGE, R_BUFFER_SAMPLER);

// Buffer Usage Type
neko_enum_decl(gfx_buffer_usage_type, R_BUFFER_USAGE_STATIC, R_BUFFER_USAGE_STREAM, R_BUFFER_USAGE_DYNAMIC);

// Buffer Update Type
neko_enum_decl(gfx_buffer_update_type, R_BUFFER_UPDATE_RECREATE, R_BUFFER_UPDATE_SUBDATA);

neko_enum_decl(gfx_access_type, R_ACCESS_READ_ONLY, R_ACCESS_WRITE_ONLY, R_ACCESS_READ_WRITE);

neko_enum_decl(gfx_buffer_flags, R_BUFFER_FLAG_MAP_PERSISTENT, R_BUFFER_FLAG_MAP_COHERENT);

// Texture
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

neko_enum_decl(gfx_clear_flag, R_CLEAR_COLOR = 0x01, R_CLEAR_DEPTH = 0x02, R_CLEAR_STENCIL = 0x04, R_CLEAR_NONE = 0x08);

#define R_CLEAR_ALL R_CLEAR_COLOR | R_CLEAR_DEPTH | R_CLEAR_STENCIL

neko_enum_decl(gfx_bind_type, R_BIND_VERTEX_BUFFER, R_BIND_INDEX_BUFFER, R_BIND_UNIFORM_BUFFER, R_BIND_STORAGE_BUFFER, R_BIND_IMAGE_BUFFER, R_BIND_UNIFORM);

neko_enum_decl(gfx_depth_func_type,  // Default value of 0x00 means depth is disabled
               R_DEPTH_FUNC_NEVER, R_DEPTH_FUNC_LESS, R_DEPTH_FUNC_EQUAL, R_DEPTH_FUNC_LEQUAL, R_DEPTH_FUNC_GREATER, R_DEPTH_FUNC_NOTEQUAL, R_DEPTH_FUNC_GEQUAL, R_DEPTH_FUNC_ALWAYS);

neko_enum_decl(gfx_depth_mask_type,  // Default value 0x00 means depth writing enabled
               R_DEPTH_MASK_ENABLED, R_DEPTH_MASK_DISABLED);

neko_enum_decl(gfx_stencil_func_type,
               R_STENCIL_FUNC_NEVER,  // Default value of 0x00 means stencil is disabled
               R_STENCIL_FUNC_LESS, R_STENCIL_FUNC_EQUAL, R_STENCIL_FUNC_LEQUAL, R_STENCIL_FUNC_GREATER, R_STENCIL_FUNC_NOTEQUAL, R_STENCIL_FUNC_GEQUAL, R_STENCIL_FUNC_ALWAYS);

neko_enum_decl(gfx_stencil_op_type,  // Default value of 0x00 means keep is used
               R_STENCIL_OP_KEEP, R_STENCIL_OP_ZERO, R_STENCIL_OP_REPLACE, R_STENCIL_OP_INCR, R_STENCIL_OP_INCR_WRAP, R_STENCIL_OP_DECR, R_STENCIL_OP_DECR_WRAP, R_STENCIL_OP_INVERT);

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

typedef struct gfx_shader_source_desc_t {
    gfx_shader_stage_type type;  // Shader stage type (vertex, fragment, tesselation, geometry, compute)
    const char* source;          // Source for shader
} gfx_shader_source_desc_t;

typedef struct gfx_shader_desc_t {
    gfx_shader_source_desc_t* sources;  // Array of shader source descriptions
    size_t size;                        // Size in bytes of shader source desc array
    char name[64];                      // Optional (for logging and debugging mainly)
} gfx_shader_desc_t;

#define R_TEXTURE_DATA_MAX 6

// Graphics Texture Desc
typedef struct gfx_texture_desc_t {
    gfx_texture_type type;
    u32 width;                              // Width of texture in texels
    u32 height;                             // Height of texture in texels
    u32 depth;                              // Depth of texture
    void* data[R_TEXTURE_DATA_MAX];         // Texture data to upload (can be null)
    gfx_texture_format_type format;         // Format of texture data (rgba32, rgba8, rgba32f, r8, depth32f, etc...)
    gfx_texture_wrapping_type wrap_s;       // Wrapping type for s axis of texture
    gfx_texture_wrapping_type wrap_t;       // Wrapping type for t axis of texture
    gfx_texture_wrapping_type wrap_r;       // Wrapping type for r axis of texture
    gfx_texture_filtering_type min_filter;  // Minification filter for texture
    gfx_texture_filtering_type mag_filter;  // Magnification filter for texture
    gfx_texture_filtering_type mip_filter;  // Mip filter for texture
    vec2 offset;                            // Offset for updates
    u32 num_mips;                           // Number of mips to generate (default 0 is disable mip generation)
    struct {
        u32 x;        // X offset in texels to start read
        u32 y;        // Y offset in texels to start read
        u32 width;    // Width in texels for texture
        u32 height;   // Height in texels for texture
        size_t size;  // Size in bytes for data to be read
    } read;
    u16 flip_y;  // Whether or not y is flipped
} gfx_texture_desc_t;

// Graphics Uniform Layout Desc
typedef struct gfx_uniform_layout_desc_t {
    gfx_uniform_type type;  // Type of field
    char fname[64];         // The name of field (required for implicit APIs, like OpenGL/ES)
    u32 count;              // Count variable (used for arrays such as glUniformXXXv)
} gfx_uniform_layout_desc_t;

// Graphics Uniform Desc
typedef struct gfx_uniform_desc_t {
    gfx_shader_stage_type stage;
    char name[64];                      // The name of uniform (required for OpenGL/ES, WebGL)
    gfx_uniform_layout_desc_t* layout;  // Layout array for uniform data
    size_t layout_size;                 // Size of uniform data in bytes
} gfx_uniform_desc_t;

typedef struct gfx_buffer_update_desc_t {
    gfx_buffer_update_type type;
    size_t offset;
} gfx_buffer_update_desc_t;

// Graphics Buffer Desc General
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

// Graphics Clear Action
typedef struct gfx_clear_action_t {
    gfx_clear_flag flag;  // Flag to be set (clear color, clear depth, clear stencil, clear all)
    union {
        float color[4];   // Clear color value
        float depth;      // Clear depth value
        int32_t stencil;  // Clear stencil value
    };
} gfx_clear_action_t;

// Graphics Clear Desc
typedef struct gfx_clear_desc_t {
    gfx_clear_action_t* actions;  // Clear action
    size_t size;                  // Size
} gfx_clear_desc_t;

// Graphics Render Pass Desc
typedef struct gfx_renderpass_desc_t {
    neko_handle(gfx_framebuffer_t) fbo;  // Default is set to invalid for backbuffer
    neko_handle(gfx_texture_t) * color;  // Array of color attachments to be bound (useful for MRT, if supported)
    size_t color_size;                   // Size of color attachment array
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

// Graphics Binding Desc
typedef struct gfx_bind_desc_t {
    struct {
        gfx_bind_vertex_buffer_desc_t* desc;  // Array of vertex buffer declarations (NULL by default)
        size_t size;                          // Size of array in bytes (optional if only one)
    } vertex_buffers;

    struct {
        gfx_bind_index_buffer_desc_t* desc;  // Array of index buffer declarations (NULL by default)
        size_t size;                         // Size of array in bytes (optional if only one)
    } index_buffers;

    struct {
        gfx_bind_uniform_buffer_desc_t* desc;  // Array of uniform buffer declarations (NULL by default)
        size_t size;                           // Size of array in bytes (optional if only one)
    } uniform_buffers;

    struct {
        gfx_bind_uniform_desc_t* desc;  // Array of uniform declarations (NULL by default)
        size_t size;                    // Size of array in bytes (optional if one)
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

// Graphics Blend State Desc
typedef struct gfx_blend_state_desc_t {
    gfx_blend_equation_type func;  // Equation function to use for blend ops
    gfx_blend_mode_type src;       // Source blend mode
    gfx_blend_mode_type dst;       // Destination blend mode
} gfx_blend_state_desc_t;

// Graphics Depth State Desc
typedef struct gfx_depth_state_desc_t {
    gfx_depth_func_type func;  // Function to set for depth test
    gfx_depth_mask_type mask;  // Whether or not writing is enabled/disabled
} gfx_depth_state_desc_t;

// Graphics Stencil State Desc
typedef struct gfx_stencil_state_desc_t {
    gfx_stencil_func_type func;  // Function to set for stencil test
    u32 ref;                     // Specifies reference val for stencil test
    u32 comp_mask;               // Specifies mask that is ANDed with both ref val and stored stencil val
    u32 write_mask;              // Specifies mask that is ANDed with both ref val and stored stencil val
    gfx_stencil_op_type sfail;   // Action to take when stencil test fails
    gfx_stencil_op_type dpfail;  // Action to take when stencil test passes but depth test fails
    gfx_stencil_op_type dppass;  // Action to take when both stencil test passes and either depth passes or is not enabled
} gfx_stencil_state_desc_t;

// Graphics Raster State Desc
typedef struct gfx_raster_state_desc_t {
    gfx_face_culling_type face_culling;    // Face culling mode to be used (front, back, front and back)
    gfx_winding_order_type winding_order;  // Winding order mode to be used (ccw, cw)
    gfx_primitive_type primitive;          // Primitive type for drawing (lines, quads, triangles, triangle strip)
    neko_handle(gfx_shader_t) shader;      // Shader to bind and use (might be in bindables later on, not sure)
    size_t index_buffer_element_size;      // Element size of index buffer (used for parsing internal data)
} gfx_raster_state_desc_t;

// Graphics Compute State Desc
typedef struct gfx_compute_state_desc_t {
    neko_handle(gfx_shader_t) shader;  // Compute shader to bind
} gfx_compute_state_desc_t;

// Graphics Vertex Attribute Desc
typedef struct gfx_vertex_attribute_desc_t {
    char name[64];                     // Attribute name (required for lower versions of OpenGL and ES)
    gfx_vertex_attribute_type format;  // Format for vertex attribute
    size_t stride;                     // Total stride of vertex layout (optional, calculated by default)
    size_t offset;                     // Offset of this vertex from base pointer of data (optional, calaculated by default)
    size_t divisor;                    // Used for instancing. (optional, default = 0x00 for no instancing)
    u32 buffer_idx;                    // Vertex buffer to use (optional, default = 0x00)
} gfx_vertex_attribute_desc_t;

// Graphics Vertex Layout Desc
typedef struct gfx_vertex_layout_desc_t {
    gfx_vertex_attribute_desc_t* attrs;  // Vertex attribute array
    size_t size;                         // Size in bytes of vertex attribute array
} gfx_vertex_layout_desc_t;

// Graphics Pipeline Desc
typedef struct gfx_pipeline_desc_t {
    gfx_blend_state_desc_t blend;      // Blend state desc for pipeline
    gfx_depth_state_desc_t depth;      // Depth state desc for pipeline
    gfx_raster_state_desc_t raster;    // Raster state desc for pipeline
    gfx_stencil_state_desc_t stencil;  // Stencil state desc for pipeline
    gfx_compute_state_desc_t compute;  // Compute state desc for pipeline
    gfx_vertex_layout_desc_t layout;   // Vertex layout desc for pipeline
} gfx_pipeline_desc_t;

// Graphics Draw Desc
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

struct neko_gl_data_t;

typedef struct gfx_t {
    neko_gl_data_t* ud;
    gfx_info_t info;  // Used for querying by user for features
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

// Uniform (stores samplers as well as primitive uniforms)
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

// Pipeline
typedef struct neko_gl_pipeline_t {
    gfx_blend_state_desc_t blend;
    gfx_depth_state_desc_t depth;
    gfx_raster_state_desc_t raster;
    gfx_stencil_state_desc_t stencil;
    gfx_compute_state_desc_t compute;
    neko_dyn_array(gfx_vertex_attribute_desc_t) layout;
} neko_gl_pipeline_t;

// Render Pass
typedef struct neko_gl_renderpass_t {
    neko_handle(gfx_framebuffer_t) fbo;
    neko_dyn_array(neko_handle(gfx_texture_t)) color;
    neko_handle(gfx_texture_t) depth;
    neko_handle(gfx_texture_t) stencil;
} neko_gl_renderpass_t;

// Shader
typedef struct neko_gl_shader_t {
    u32 id;
} neko_gl_shader_t;

// Gfx Buffer
typedef struct neko_gl_buffer_t {
    u32 id;
} neko_gl_buffer_t;

// Texture
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
        neko_dyn_array(vec2) vec2;
        neko_dyn_array(vec3) vec3;
        neko_dyn_array(vec4) vec4;
        neko_dyn_array(mat4) mat4;
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

// 图形信息对象查询
gfx_info_t* gfx_info();

typedef void (*R_DRAW_FUNC)(neko_command_buffer_t* cb);

// 建议尽可能长时间地保留此功能 (perhaps until release)
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

neko_gl_data_t* gfx_ogl();

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

void gfx_cmd_submit(neko_command_buffer_t* cb);

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

typedef struct neko_asset_texture_t {
    neko_handle(gfx_texture_t) hndl;
    gfx_texture_desc_t desc;
} neko_asset_texture_t;

/*==========================
// NEKO_ASSET_TYPES
==========================*/

// Texture

bool neko_util_load_texture_data_from_memory(const void* memory, size_t sz, i32* width, i32* height, u32* num_comps, void** data, bool flip_vertically_on_load);
bool neko_util_load_texture_data_from_file(const char* file_path, i32* width, i32* height, u32* num_comps, void** data, bool flip_vertically_on_load);

#if 1
bool neko_asset_texture_load_from_file(const_str path, void* out, gfx_texture_desc_t* desc, bool flip_on_load, bool keep_data);
bool neko_asset_texture_load_from_memory(const void* memory, size_t sz, void* out, gfx_texture_desc_t* desc, bool flip_on_load, bool keep_data);

//  bool neko_util_load_gltf_data_from_file(const_str path, neko_asset_mesh_decl_t* decl, neko_asset_mesh_raw_data_t** out, u32* mesh_count);
//  bool neko_util_load_gltf_data_from_memory(const void* memory, size_t sz, neko_asset_mesh_decl_t* decl, neko_asset_mesh_raw_data_t** out, u32* mesh_count);

#endif

#endif