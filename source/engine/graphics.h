
#ifndef NEKO_GL_H
#define NEKO_GL_H

#include <stdio.h>

// 确保在 GLFW 标头之前包含 GLEW 标头
#include "deps/glad/glad.h"

// GLFW
#include <GLFW/glfw3.h>
#undef APIENTRY

#include "engine/renderer/texture.h"

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

/*==========================
// Graphics Interface
==========================*/

typedef struct gfx_t {
    Array<AssetTexture> textures;

    struct glinfo_t {
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
    } glinfo;
} gfx_t;

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

#endif
