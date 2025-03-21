
#ifndef NEKO_GL_H
#define NEKO_GL_H

#include <stdio.h>

// 确保在 GLFW 标头之前包含 GLEW 标头
#include "extern/glad/glad.h"

// GLFW
#include <GLFW/glfw3.h>
#undef APIENTRY

static const char* opengl_string(GLenum e) {

#define XX(x)      \
    case x:        \
        return #x; \
        break;

    switch (e) {
        // shader:
        XX(GL_VERTEX_SHADER);
        XX(GL_GEOMETRY_SHADER);
        XX(GL_FRAGMENT_SHADER);

        // buffer usage:
        XX(GL_STREAM_DRAW);
        XX(GL_STREAM_READ);
        XX(GL_STREAM_COPY);
        XX(GL_STATIC_DRAW);
        XX(GL_STATIC_READ);
        XX(GL_STATIC_COPY);
        XX(GL_DYNAMIC_DRAW);
        XX(GL_DYNAMIC_READ);
        XX(GL_DYNAMIC_COPY);

        // errors:
        XX(GL_NO_ERROR);
        XX(GL_INVALID_ENUM);
        XX(GL_INVALID_VALUE);
        XX(GL_INVALID_OPERATION);
        XX(GL_INVALID_FRAMEBUFFER_OPERATION);
        XX(GL_OUT_OF_MEMORY);

#if !defined(NEKO_IS_APPLE)
        XX(GL_STACK_UNDERFLOW);
        XX(GL_STACK_OVERFLOW);
#endif

        // types:
        XX(GL_BYTE);
        XX(GL_UNSIGNED_BYTE);
        XX(GL_SHORT);
        XX(GL_UNSIGNED_SHORT);
        XX(GL_FLOAT);
        XX(GL_FLOAT_VEC2);
        XX(GL_FLOAT_VEC3);
        XX(GL_FLOAT_VEC4);
        XX(GL_DOUBLE);
        XX(GL_DOUBLE_VEC2);
        XX(GL_DOUBLE_VEC3);
        XX(GL_DOUBLE_VEC4);
        XX(GL_INT);
        XX(GL_INT_VEC2);
        XX(GL_INT_VEC3);
        XX(GL_INT_VEC4);
        XX(GL_UNSIGNED_INT);
        XX(GL_UNSIGNED_INT_VEC2);
        XX(GL_UNSIGNED_INT_VEC3);
        XX(GL_UNSIGNED_INT_VEC4);
        XX(GL_BOOL);
        XX(GL_BOOL_VEC2);
        XX(GL_BOOL_VEC3);
        XX(GL_BOOL_VEC4);
        XX(GL_FLOAT_MAT2);
        XX(GL_FLOAT_MAT3);
        XX(GL_FLOAT_MAT4);
        XX(GL_FLOAT_MAT2x3);
        XX(GL_FLOAT_MAT2x4);
        XX(GL_FLOAT_MAT3x2);
        XX(GL_FLOAT_MAT3x4);
        XX(GL_FLOAT_MAT4x2);
        XX(GL_FLOAT_MAT4x3);
        XX(GL_DOUBLE_MAT2);
        XX(GL_DOUBLE_MAT3);
        XX(GL_DOUBLE_MAT4);
        XX(GL_DOUBLE_MAT2x3);
        XX(GL_DOUBLE_MAT2x4);
        XX(GL_DOUBLE_MAT3x2);
        XX(GL_DOUBLE_MAT3x4);
        XX(GL_DOUBLE_MAT4x2);
        XX(GL_DOUBLE_MAT4x3);
        XX(GL_SAMPLER_1D);
        XX(GL_SAMPLER_2D);
        XX(GL_SAMPLER_3D);
        XX(GL_SAMPLER_CUBE);
        XX(GL_SAMPLER_1D_SHADOW);
        XX(GL_SAMPLER_2D_SHADOW);
        XX(GL_SAMPLER_1D_ARRAY);
        XX(GL_SAMPLER_2D_ARRAY);
        XX(GL_SAMPLER_1D_ARRAY_SHADOW);
        XX(GL_SAMPLER_2D_ARRAY_SHADOW);
        XX(GL_SAMPLER_2D_MULTISAMPLE);
        XX(GL_SAMPLER_2D_MULTISAMPLE_ARRAY);
        XX(GL_SAMPLER_CUBE_SHADOW);
        XX(GL_SAMPLER_BUFFER);
        XX(GL_SAMPLER_2D_RECT);
        XX(GL_SAMPLER_2D_RECT_SHADOW);
        XX(GL_INT_SAMPLER_1D);
        XX(GL_INT_SAMPLER_2D);
        XX(GL_INT_SAMPLER_3D);
        XX(GL_INT_SAMPLER_CUBE);
        XX(GL_INT_SAMPLER_1D_ARRAY);
        XX(GL_INT_SAMPLER_2D_ARRAY);
        XX(GL_INT_SAMPLER_2D_MULTISAMPLE);
        XX(GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
        XX(GL_INT_SAMPLER_BUFFER);
        XX(GL_INT_SAMPLER_2D_RECT);
        XX(GL_UNSIGNED_INT_SAMPLER_1D);
        XX(GL_UNSIGNED_INT_SAMPLER_2D);
        XX(GL_UNSIGNED_INT_SAMPLER_3D);
        XX(GL_UNSIGNED_INT_SAMPLER_CUBE);
        XX(GL_UNSIGNED_INT_SAMPLER_1D_ARRAY);
        XX(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY);
        XX(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE);
        XX(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
        XX(GL_UNSIGNED_INT_SAMPLER_BUFFER);
        XX(GL_UNSIGNED_INT_SAMPLER_2D_RECT);

#if !defined(NEKO_IS_APPLE)
        XX(GL_IMAGE_1D);
        XX(GL_IMAGE_2D);
        XX(GL_IMAGE_3D);
        XX(GL_IMAGE_2D_RECT);
        XX(GL_IMAGE_CUBE);
        XX(GL_IMAGE_BUFFER);
        XX(GL_IMAGE_1D_ARRAY);
        XX(GL_IMAGE_2D_ARRAY);
        XX(GL_IMAGE_2D_MULTISAMPLE);
        XX(GL_IMAGE_2D_MULTISAMPLE_ARRAY);
        XX(GL_INT_IMAGE_1D);
        XX(GL_INT_IMAGE_2D);
        XX(GL_INT_IMAGE_3D);
        XX(GL_INT_IMAGE_2D_RECT);
        XX(GL_INT_IMAGE_CUBE);
        XX(GL_INT_IMAGE_BUFFER);
        XX(GL_INT_IMAGE_1D_ARRAY);
        XX(GL_INT_IMAGE_2D_ARRAY);
        XX(GL_INT_IMAGE_2D_MULTISAMPLE);
        XX(GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY);
        XX(GL_UNSIGNED_INT_IMAGE_1D);
        XX(GL_UNSIGNED_INT_IMAGE_2D);
        XX(GL_UNSIGNED_INT_IMAGE_3D);
        XX(GL_UNSIGNED_INT_IMAGE_2D_RECT);
        XX(GL_UNSIGNED_INT_IMAGE_CUBE);
        XX(GL_UNSIGNED_INT_IMAGE_BUFFER);
        XX(GL_UNSIGNED_INT_IMAGE_1D_ARRAY);
        XX(GL_UNSIGNED_INT_IMAGE_2D_ARRAY);
        XX(GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE);
        XX(GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY);
        XX(GL_UNSIGNED_INT_ATOMIC_COUNTER);
#endif
    }

#undef XX

    static char buffer[32];
    sprintf(buffer, "Unknown GLenum: (0x%04x)", e);
    return buffer;
}

#endif

#ifndef NEKO_GRAPHICS_h
#define NEKO_GRAPHICS_h

#include "engine/base.hpp"

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

#define gfx_bind_vertex_attrib_auto(program, gl_type, components, param_name, size, offset) \
    do {                                                                                    \
        GLuint a__ = glGetAttribLocation(program, param_name);                              \
        glVertexAttribPointer(a__, components, gl_type, GL_FALSE, size, offset);            \
        glEnableVertexAttribArray(a__);                                                     \
    } while (0)

/*=============================
// NEKO_GRAPHICS
=============================*/

#ifdef _DEBUG
#define neko_check_gl_error() gfx_print_error(__FILE__, __LINE__)
#else
#define neko_check_gl_error()
#endif

NEKO_API() void gfx_print_error(const char* file, u32 line);

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

#define neko_enum_decl(NAME, ...) typedef enum NAME { _neko_##NAME##_default = 0x0, __VA_ARGS__, _neko_##NAME##_count, _neko_##NAME##_force_u32 = 0x7fffffff } NAME;

typedef struct gfx_info_t {
    const_str version;
    const_str vendor;
    u32 major_version;
    u32 minor_version;
    u32 max_texture_units;
    u32 max_texture_size;
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

// 内部 OpenGL 数据
typedef struct neko_gl_data_t {
    Array<AssetTexture> textures;
} neko_gl_data_t;

typedef struct gfx_t {
    neko_gl_data_t* ud;
    gfx_info_t info;  // Used for querying by user for features
} gfx_t;

NEKO_API() gfx_t* g_render;

#define neko_render() RENDER()

NEKO_API() gfx_t* gfx_create();
NEKO_API() void gfx_fini(gfx_t* render);
NEKO_API() void gfx_init(gfx_t* render);

NEKO_API() gfx_info_t* gfx_info();
NEKO_API() neko_gl_data_t* gfx_ogl();

typedef struct neko_rgb_color_t {
    float r, g, b;
} neko_rgb_color_t;

typedef u32 neko_color_t;

struct neko_resource_t;

NEKO_API() neko_color_t neko_color_from_rgb_color(neko_rgb_color_t rgb);
NEKO_API() neko_rgb_color_t neko_rgb_color_from_color(neko_color_t color);

typedef enum neko_vertex_buffer_flags_t {
    NEKO_VERTEXBUFFER_STATIC_DRAW = 1 << 0,
    NEKO_VERTEXBUFFER_DYNAMIC_DRAW = 1 << 1,
    NEKO_VERTEXBUFFER_DRAW_LINES = 1 << 2,
    NEKO_VERTEXBUFFER_DRAW_LINE_STRIP = 1 << 3,
    NEKO_VERTEXBUFFER_DRAW_TRIANGLES = 1 << 4,
} neko_vertex_buffer_flags_t;

typedef struct neko_vertex_buffer_t {
    u32 va_id;
    u32 vb_id;
    u32 ib_id;
    u32 index_count;
    neko_vertex_buffer_flags_t flags;
} neko_vertex_buffer_t;

NEKO_API() neko_vertex_buffer_t* neko_new_vertex_buffer(neko_vertex_buffer_flags_t flags);
NEKO_API() void neko_free_vertex_buffer(neko_vertex_buffer_t* buffer);
NEKO_API() void neko_bind_vertex_buffer_for_draw(neko_vertex_buffer_t* buffer);
NEKO_API() void neko_bind_vertex_buffer_for_edit(neko_vertex_buffer_t* buffer);
NEKO_API() void neko_push_vertices(neko_vertex_buffer_t* buffer, float* vertices, u32 count);
NEKO_API() void neko_push_indices(neko_vertex_buffer_t* buffer, u32* indices, u32 count);
NEKO_API() void neko_update_vertices(neko_vertex_buffer_t* buffer, float* vertices, u32 offset, u32 count);
NEKO_API() void neko_update_indices(neko_vertex_buffer_t* buffer, u32* indices, u32 offset, u32 count);
NEKO_API() void neko_configure_vertex_buffer(neko_vertex_buffer_t* buffer, u32 index, u32 component_count, u32 stride, u32 offset);
NEKO_API() void neko_draw_vertex_buffer(neko_vertex_buffer_t* buffer);
NEKO_API() void neko_draw_vertex_buffer_custom_count(neko_vertex_buffer_t* buffer, u32 count);

NEKO_API() AssetTexture* neko_new_texture(neko_resource_t* resource, neko_texture_flags_t flags);
NEKO_API() AssetTexture* neko_new_texture_from_memory(void* data, u32 size, neko_texture_flags_t flags);
NEKO_API() AssetTexture* neko_new_texture_from_memory_uncompressed(unsigned char* pixels, u32 size, i32 width, i32 height, i32 component_count, neko_texture_flags_t flags);
NEKO_API() void neko_init_texture(AssetTexture* texture, neko_resource_t* resource, neko_texture_flags_t flags);
NEKO_API() void neko_init_texture_from_memory(AssetTexture* texture, void* data, u32 size, neko_texture_flags_t flags);
NEKO_API() void neko_init_texture_from_memory_uncompressed(AssetTexture* texture, unsigned char* pixels, i32 width, i32 height, i32 component_count, neko_texture_flags_t flags);
NEKO_API() void neko_deinit_texture(AssetTexture* texture);

NEKO_API() void neko_free_texture(AssetTexture* texture);
NEKO_API() void neko_bind_texture(AssetTexture* texture, u32 slot);

#define BUFFER_FRAMES 3

typedef struct {
    char* stream[2];
} VertexData;

typedef struct {
    GLuint vbo;
    GLsync syncs[BUFFER_FRAMES];
    char* data;
    size_t size;
    size_t offset;
    int index;
} StreamBuffer;

// typedef struct {
//     int quad_count;
//     u64 texture;
//     u64 shader;
//     StreamBuffer buffer[2];
//     VertexData map;
//     GLuint vao;
//     GLuint vbo;
//     GLuint ebo;
//     uint16_t indexes[2];
// } StreamState;

bool texture_load(AssetTexture* tex, String filename, bool flip_image_vertical = true);
void texture_bind(const char* filename);
vec2 texture_get_size(const char* filename);  // (width, height)
AssetTexture texture_get_ptr(const char* filename);
bool texture_update(AssetTexture* tex, String filename);
bool texture_update_data(AssetTexture* tex, u8* data);

u64 generate_texture_handle(void* pixels, int w, int h, void* udata);
void destroy_texture_handle(u64 texture_id, void* udata);

AssetTexture neko_aseprite_simple(String filename);

// AssetTexture

bool load_texture_data_from_memory(const void* memory, size_t sz, i32* width, i32* height, u32* num_comps, void** data, bool flip_vertically_on_load);
bool load_texture_data_from_file(const char* file_path, i32* width, i32* height, u32* num_comps, void** data, bool flip_vertically_on_load);

#endif
