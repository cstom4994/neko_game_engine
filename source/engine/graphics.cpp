#include "graphics.h"

#include <stdio.h>
#include <stdlib.h>

#include "engine/asset.h"
#include "engine/base.hpp"
#include "base/common/profiler.hpp"
#include "base/cbase.hpp"
#include "base/common/logger.hpp"
#include "engine/renderer/texture.h"

extern const mat3* camera_get_inverse_view_matrix_ptr();  // for GLSL binding : in component.cpp

/*=============================
// R_IMPL
=============================*/

gfx_t* g_render;

#define RENDER() (g_render)

#if (defined NEKO_IS_WIN32 || defined NEKO_IS_APPLE || defined NEKO_IS_LINUX)
#define R_IMPL_OpenGL_CORE
#else
#define R_IMPL_OpenGL_ES
#endif

// macOS 不支持 OpenGL 4.1+
#if defined(NEKO_IS_APPLE)
#define CHECK_GL_CORE(...)
#else
#define CHECK_GL_CORE(...) __VA_ARGS__
#endif

// 图形信息对象查询
gfx_t::glinfo_t& gfx_info() { return RENDER()->glinfo; }

const char* gl_get_error_description(GLenum error_code) {
    switch (error_code) {
        case GL_NO_ERROR:
            return "No error detected.";
        case GL_INVALID_ENUM:
            return "Enum argument out of range.";
        case GL_INVALID_VALUE:
            return "Numeric argument out of range.";
        case GL_INVALID_OPERATION:
            return "Operation illegal in current state.";
        case GL_OUT_OF_MEMORY:
            return "Not enough memory left to execute command.";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "Framebuffer object is not complete.";
        default:
            return "No description available for UNKNOWN_ERROR.";
    }
}

void neko_gl_pipeline_state() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);
    CHECK_GL_CORE(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS););
    glDisable(GL_MULTISAMPLE);

    CHECK_GL_CORE(auto& info = gfx_info(); if (info.compute.available) {
        NEKO_INVOKE_ONCE(LOG_INFO("compute shader available: {}", NEKO_BOOL_STR(info.compute.available)););
        glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    });
}

gfx_t* gfx_create() {
    // 构建新的图形界面
    gfx_t* gfx = (gfx_t*)mem_alloc(sizeof(gfx_t));
    memset(gfx, 0, sizeof(gfx_t));
    return gfx;
}

void gfx_fini(gfx_t* render) {
    // 释放所有资源 (假设它们已从GPU中释放)
    if (render == NULL) return;

    if (render->textures.len)
        for (auto& tex : render->textures) {
        }

    render->textures.trash();

    mem_free(render);
    render = NULL;
}

#if 0
void gfx_texture_desc_query(AssetTexture hndl, gfx_texture_desc_t* out) {
    if (!out) return;

    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    neko_gl_texture_t* tex = &ogl->textures[hndl.id];

    // Read back pixels
    if (out->data && out->read.width && out->read.height) {
        u32 type = (tex->desc.format);
        u32 format = (tex->desc.format);
        CHECK_GL_CORE(glActiveTexture(GL_TEXTURE0); glGetTextureSubImage(tex->id, 0, out->read.x, out->read.y, 0, out->read.width, out->read.height, 1, format, type, out->read.size, out->data););
    }

    *out = tex->desc;
}

void gfx_texture_read_impl(AssetTexture hndl, gfx_texture_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    if (!desc) return;
    if (!ogl->textures.valid(hndl.id)) {
        LOG_INFO("AssetTexture handle invalid: {}", hndl.id);
    }

    neko_gl_texture_t* tex = &ogl->textures[hndl.id];
    // Bind texture
    GLenum target = GL_TEXTURE_2D;
    u32 gl_format = (tex->desc.format);
    u32 gl_type = (tex->desc.format);
    glBindTexture(target, tex->id);
    glReadPixels(desc->read.x, desc->read.y, desc->read.width, desc->read.height, gl_format, gl_type, desc->data);
    glBindTexture(target, 0x00);
}
#endif

void gfx_init(gfx_t* render) {

    auto& info = RENDER()->glinfo;

    glGetIntegerv(GL_MAJOR_VERSION, (GLint*)&info.major_version);
    glGetIntegerv(GL_MINOR_VERSION, (GLint*)&info.minor_version);
    info.version = (const_str)glGetString(GL_VERSION);
    info.vendor = (const_str)glGetString(GL_RENDERER);

    // LOG_INFO("OpenGL vendor:   {}", glGetString(GL_VENDOR));
    // LOG_INFO("OpenGL renderer: {}", glGetString(GL_RENDERER));
    // LOG_INFO("OpenGL version:  {}", glGetString(GL_VERSION));

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&info.max_texture_size);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, (GLint*)&info.max_texture_units);

    if (info.max_texture_size < 2048) LOG_INFO("OpenGL maximum texture too small");

    info.compute.available = info.major_version >= 4 && info.minor_version >= 3;
    if (info.compute.available) {
        CHECK_GL_CORE({
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, (i32*)&info.compute.max_work_group_count[0]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, (i32*)&info.compute.max_work_group_count[1]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, (i32*)&info.compute.max_work_group_count[2]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, (i32*)&info.compute.max_work_group_size[0]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, (i32*)&info.compute.max_work_group_size[1]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, (i32*)&info.compute.max_work_group_size[2]);
            glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, (i32*)&info.compute.max_work_group_invocations);
        });
    } else {
        LOG_INFO("OpenGL compute shaders not available");
    }

#if defined(_DEBUG) && 0
    int numExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    neko_printf("Supported extensions:");
    for (int i = 0; i < numExtensions; i++) {
        const GLubyte* extension = glGetStringi(GL_EXTENSIONS, i);
        neko_printf("%s;", extension);
    }
    neko_printf("\n");
#endif
}


void streambuffer_init(StreamBuffer* buffer, size_t size) {
    glGenBuffers(1, &buffer->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);

    GLbitfield storageflags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    GLbitfield mapflags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    glBufferStorage(GL_ARRAY_BUFFER, size * BUFFER_FRAMES, NULL, storageflags);

    buffer->data = (char*)glMapBufferRange(GL_ARRAY_BUFFER, 0, size * BUFFER_FRAMES, mapflags);
    buffer->size = size;
    buffer->offset = 0;
    buffer->index = 0;

    for (int i = 0; i < BUFFER_FRAMES; i++) buffer->syncs[i] = 0;
}

void* streambuffer_map(StreamBuffer* buffer) {
    GLsync sync = buffer->syncs[buffer->index];
    GLbitfield flags = 0;
    GLuint64 duration = 0;
    while (sync) {
        GLenum status = glClientWaitSync(sync, flags, duration);
        if (status == GL_ALREADY_SIGNALED || status == GL_CONDITION_SATISFIED || status == GL_WAIT_FAILED) break;

        flags = GL_SYNC_FLUSH_COMMANDS_BIT;
        duration = 1000000000;  // 1 second in nanoseconds
    }
    glDeleteSync(sync);
    buffer->syncs[buffer->index] = 0;

    return buffer->data + (buffer->index * buffer->size) + buffer->offset;
}

size_t streambuffer_unmap(StreamBuffer* buffer, size_t used) {
    size_t offset = (buffer->index * buffer->size) + buffer->offset;
    buffer->offset += used;
    return offset;
}

void streambuffer_nextframe(StreamBuffer* buffer) {
    GLsync sync = buffer->syncs[buffer->index];
    if (sync) glDeleteSync(sync);
    buffer->syncs[buffer->index] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    buffer->index = (buffer->index + 1) % BUFFER_FRAMES;
    buffer->offset = 0;
}

// -------------------------------------------------------------------------
