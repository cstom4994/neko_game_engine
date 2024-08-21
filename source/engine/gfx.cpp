#include "gfx.h"

#include <stdio.h>
#include <stdlib.h>

#include "engine/asset.h"
#include "engine/base.h"
#include "engine/camera.h"
#include "engine/console.h"
#include "engine/game.h"
#include "engine/vfs.h"

// deps
#include <stb_image.h>

static GLint gfx_compile_shader(GLuint shader, const char* filename) {
    char log[512];
    GLint status;

    String contents = {};

    bool ok = vfs_read_entire_file(&contents, filename);
    neko_defer(mem_free(contents.data));

    neko_assert(ok);

    console_printf("gfx: compiling shader '%s' ...", filename);

    glShaderSource(shader, 1, (const GLchar**)&contents.data, NULL);
    glCompileShader(shader);

    // log
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    console_printf(status ? " successful\n" : " unsuccessful\n");
    glGetShaderInfoLog(shader, 512, NULL, log);
    console_printf("%s", log);

    return status;
}

GLuint gfx_create_program(const_str name, const char* vert_path, const char* geom_path, const char* frag_path) {
    GLuint vert, geom, frag, program;

#define compile(shader, type)                                     \
    if (shader##_path) {                                          \
        shader = glCreateShader(type);                            \
        if (!gfx_compile_shader(shader, shader##_path)) return 0; \
    }

    compile(vert, GL_VERTEX_SHADER);
    compile(geom, GL_GEOMETRY_SHADER);
    compile(frag, GL_FRAGMENT_SHADER);

    program = glCreateProgram();

    if (vert_path) glAttachShader(program, vert);
    if (geom_path) glAttachShader(program, geom);
    if (frag_path) glAttachShader(program, frag);

    glLinkProgram(program);

    // GL will automatically detach and free shaders when program is deleted
    if (vert_path) glDeleteShader(vert);
    if (geom_path) glDeleteShader(geom);
    if (frag_path) glDeleteShader(frag);

    shader_pair pair = {program, name};
    neko_dyn_array_push(g_app->shader_array, pair);

    return program;
}

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
gfx_info_t* gfx_info() { return &RENDER()->info; }

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

void gfx_print_error(const char* file, u32 line) {
    GLenum code = glGetError();

    if (code != GL_NO_ERROR) {
        const_str last_slash = file;

        while (*file) {
            char c = *file;
            if (c == '\\' || c == '/') last_slash = file + 1;
            ++file;
        }

        const_str str = opengl_string(code);
        const_str des = gl_get_error_description(code);
        neko_println("OpenGL Error %s (%u): %u, %s, %s", last_slash, line, code, str, des);
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

    CHECK_GL_CORE(gfx_info_t* info = gfx_info(); if (info->compute.available) {
        NEKO_INVOKE_ONCE(console_log("compute shader available: %s", NEKO_BOOL_STR(info->compute.available)););
        glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    });
}

// Utilities

u32 neko_texture_format(gfx_texture_format_type type) {
    u32 format = GL_UNSIGNED_BYTE;
    switch (type) {
        default:
        case R_TEXTURE_FORMAT_A8:
            format = GL_UNSIGNED_BYTE;
            break;
        case R_TEXTURE_FORMAT_R8:
            format = GL_UNSIGNED_BYTE;
            break;
        case R_TEXTURE_FORMAT_R32:
            format = GL_UNSIGNED_INT;
            break;
        case R_TEXTURE_FORMAT_RGB8:
            format = GL_UNSIGNED_BYTE;
            break;
        case R_TEXTURE_FORMAT_RGBA8:
            format = GL_UNSIGNED_BYTE;
            break;
        case R_TEXTURE_FORMAT_RGBA16F:
            format = GL_FLOAT;
            break;
        case R_TEXTURE_FORMAT_RGBA32F:
            format = GL_FLOAT;
            break;
        case R_TEXTURE_FORMAT_R32F:
            format = GL_FLOAT;
            break;
        case R_TEXTURE_FORMAT_DEPTH8:
            format = GL_FLOAT;
            break;
        case R_TEXTURE_FORMAT_DEPTH16:
            format = GL_FLOAT;
            break;
        case R_TEXTURE_FORMAT_DEPTH24:
            format = GL_FLOAT;
            break;
        case R_TEXTURE_FORMAT_DEPTH32F:
            format = GL_FLOAT;
            break;
        case R_TEXTURE_FORMAT_DEPTH24_STENCIL8:
            format = GL_UNSIGNED_INT_24_8;
            break;
        case R_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
            format = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
            break;
    }
    return format;
}

u32 neko_gl_texture_format_to_gl_texture_format(gfx_texture_format_type type) {
    u32 dt = GL_RGBA;
    switch (type) {
        default:
        case R_TEXTURE_FORMAT_RGBA8:
            dt = GL_RGBA;
            break;
        case R_TEXTURE_FORMAT_A8:
            dt = GL_ALPHA;
            break;
        case R_TEXTURE_FORMAT_R8:
            dt = GL_RED;
            break;
        case R_TEXTURE_FORMAT_R32:
            dt = GL_RED_INTEGER;
            break;
        case R_TEXTURE_FORMAT_R32F:
            dt = GL_RED;
            break;
        case R_TEXTURE_FORMAT_RGB8:
            dt = GL_RGB;
            break;
        case R_TEXTURE_FORMAT_RGBA16F:
            dt = GL_RGBA;
            break;
        case R_TEXTURE_FORMAT_RGBA32F:
            dt = GL_RGBA;
            break;
        case R_TEXTURE_FORMAT_DEPTH8:
            dt = GL_DEPTH_COMPONENT;
            break;
        case R_TEXTURE_FORMAT_DEPTH16:
            dt = GL_DEPTH_COMPONENT;
            break;
        case R_TEXTURE_FORMAT_DEPTH24:
            dt = GL_DEPTH_COMPONENT;
            break;
        case R_TEXTURE_FORMAT_DEPTH32F:
            dt = GL_DEPTH_COMPONENT;
            break;
        case R_TEXTURE_FORMAT_DEPTH24_STENCIL8:
            dt = GL_DEPTH_STENCIL;
            break;
        case R_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
            dt = GL_DEPTH_STENCIL;
            break;
    }
    return dt;
}

u32 neko_gl_texture_format_to_gl_texture_internal_format(gfx_texture_format_type type) {
    u32 format = GL_UNSIGNED_BYTE;
    switch (type) {
        case R_TEXTURE_FORMAT_A8:
            format = GL_ALPHA;
            break;
        case R_TEXTURE_FORMAT_R8:
            format = GL_RED;
            break;
        case R_TEXTURE_FORMAT_R32:
            format = GL_R32UI;
            break;
        case R_TEXTURE_FORMAT_R32F:
            format = GL_R32F;
            break;
        case R_TEXTURE_FORMAT_RGB8:
            format = GL_RGB8;
            break;
        case R_TEXTURE_FORMAT_RGBA8:
            format = GL_RGBA8;
            break;
        case R_TEXTURE_FORMAT_RGBA16F:
            format = GL_RGBA16F;
            break;
        case R_TEXTURE_FORMAT_RGBA32F:
            format = GL_RGBA32F;
            break;
        case R_TEXTURE_FORMAT_DEPTH8:
            format = GL_DEPTH_COMPONENT;
            break;
        case R_TEXTURE_FORMAT_DEPTH16:
            format = GL_DEPTH_COMPONENT16;
            break;
        case R_TEXTURE_FORMAT_DEPTH24:
            format = GL_DEPTH_COMPONENT24;
            break;
        case R_TEXTURE_FORMAT_DEPTH32F:
            format = GL_DEPTH_COMPONENT32F;
            break;
        case R_TEXTURE_FORMAT_DEPTH24_STENCIL8:
            format = GL_DEPTH24_STENCIL8;
            break;
        case R_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
            format = GL_DEPTH32F_STENCIL8;
            break;
    }
    return format;
}

gfx_t* gfx_create() {
    // 构建新的图形界面
    gfx_t* gfx = neko_malloc_init(gfx_t);
    // 为OpenGL构建内部数据
    gfx->ud = neko_malloc_init(neko_gl_data_t);
    return gfx;
}

void gfx_fini(gfx_t* render) {
    // 释放所有资源 (假设它们已从GPU中释放)
    if (render == NULL) return;

    neko_gl_data_t* ogl = (neko_gl_data_t*)render->ud;

#define OGL_FREE_DATA(SA, T, FUNC)                                                                                    \
    do {                                                                                                              \
        for (neko_slot_array_iter it = 1; neko_slot_array_iter_valid(SA, it); neko_slot_array_iter_advance(SA, it)) { \
            neko_handle(T) hndl = NEKO_DEFAULT_VAL();                                                                 \
            hndl.id = it;                                                                                             \
            FUNC(hndl);                                                                                               \
        }                                                                                                             \
    } while (0)

    if (ogl->textures) OGL_FREE_DATA(ogl->textures, gfx_texture_t, gfx_texture_fini);

    neko_slot_array_free(ogl->textures);

    mem_free(ogl);

    mem_free(render);
    render = NULL;
}

neko_gl_texture_t gl_texture_update_internal(const gfx_texture_desc_t* desc, u32 hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;

    neko_gl_texture_t tex = NEKO_DEFAULT_VAL();
    if (hndl) tex = neko_slot_array_get(ogl->textures, hndl);
    u32 width = desc->width;
    u32 height = desc->height;
    void* data = desc->data;

    if (!hndl) {
        glGenTextures(1, &tex.id);
    }

    GLenum target = GL_TEXTURE_2D;

    // glActiveTexture(GL_TEXTURE0);
    glBindTexture(target, tex.id);

    if (tex.desc.width * tex.desc.height < width * height) {
        // Construct texture based on appropriate format
        switch (desc->format) {
            case R_TEXTURE_FORMAT_A8:
                glTexImage2D(target, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
                break;
            case R_TEXTURE_FORMAT_R8:
                glTexImage2D(target, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
                break;
            case R_TEXTURE_FORMAT_R32:
                glTexImage2D(target, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
                break;
            case R_TEXTURE_FORMAT_RG8:
                glTexImage2D(target, 0, GL_RG8, width, height, 0, GL_RG, GL_UNSIGNED_BYTE, data);
                break;
            case R_TEXTURE_FORMAT_RGB8:
                glTexImage2D(target, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                break;
            case R_TEXTURE_FORMAT_RGBA8:
                glTexImage2D(target, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                break;
            case R_TEXTURE_FORMAT_R32F:
                glTexImage2D(target, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_RGBA16F:
                glTexImage2D(target, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_RGBA32F:
                glTexImage2D(target, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH8:
                glTexImage2D(target, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH16:
                glTexImage2D(target, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH24:
                glTexImage2D(target, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH32F:
                glTexImage2D(target, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH24_STENCIL8:
                glTexImage2D(target, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
                glTexImage2D(target, 0, GL_DEPTH32F_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, data);
                break;

            // NOTE: 因为 Apple 是一家垃圾公司 所以必须将其分开并仅提供对 4.1 功能的支持。
            // case R_TEXTURE_FORMAT_STENCIL8:            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
            default:
                break;
        }
    } else {
        // Subimage
        switch (desc->format) {
            case R_TEXTURE_FORMAT_A8:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_ALPHA, GL_UNSIGNED_BYTE, data);
                break;
            case R_TEXTURE_FORMAT_R8:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RED, GL_UNSIGNED_BYTE, data);
                break;
            case R_TEXTURE_FORMAT_R32:
                glTexImage2D(target, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
                break;
            case R_TEXTURE_FORMAT_RG8:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RG, GL_UNSIGNED_BYTE, data);
                break;
            case R_TEXTURE_FORMAT_RGB8:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
                break;
            case R_TEXTURE_FORMAT_RGBA8:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
                break;
            case R_TEXTURE_FORMAT_R32F:
                glTexImage2D(target, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_RGBA16F:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RGBA, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_RGBA32F:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RGBA, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH8:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH16:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH24:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH32F:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH24_STENCIL8:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, data);
                break;
            case R_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
                glTexSubImage2D(target, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, data);
                break;

            // NOTE: Because Apple is a shit company, I have to section this off and provide support for 4.1 only features.
            // case R_TEXTURE_FORMAT_STENCIL8:            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
            default:
                break;
        }
    }

    i32 mag_filter = desc->mag_filter == R_TEXTURE_FILTER_NEAREST ? GL_NEAREST : GL_LINEAR;
    i32 min_filter = desc->min_filter == R_TEXTURE_FILTER_NEAREST ? GL_NEAREST : GL_LINEAR;

    if (desc->num_mips) {
        if (desc->min_filter == R_TEXTURE_FILTER_NEAREST) {
            min_filter = desc->mip_filter == R_TEXTURE_FILTER_NEAREST ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_LINEAR;
        } else {
            min_filter = desc->mip_filter == R_TEXTURE_FILTER_NEAREST ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_LINEAR;
        }
    }

    const u32 texture_wrap_s = desc->wrap_s == 0 ? GL_REPEAT : desc->wrap_s;
    const u32 texture_wrap_t = desc->wrap_t == 0 ? GL_REPEAT : desc->wrap_t;
    const u32 texture_wrap_r = desc->wrap_r == 0 ? GL_REPEAT : desc->wrap_r;

    if (desc->num_mips) {
        glGenerateMipmap(target);
    }

    // 在使用之前需要确保它可用
    CHECK_GL_CORE(float aniso = 0.0f; glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &aniso); glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, aniso););

    glTexParameteri(target, GL_TEXTURE_WRAP_S, texture_wrap_s);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, texture_wrap_t);
    glTexParameteri(target, GL_TEXTURE_WRAP_R, texture_wrap_r);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min_filter);

    // Unbind buffers
    glBindTexture(target, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Set description
    tex.desc = *desc;

    // Add texture to internal resource pool and return handle
    return tex;
}

// Resource Creation
neko_handle(gfx_texture_t) gfx_texture_create_impl(const gfx_texture_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    neko_gl_texture_t tex = gl_texture_update_internal(&desc, 0);
    // Add texture to internal resource pool and return handle
    return (neko_handle_create(gfx_texture_t, neko_slot_array_insert(ogl->textures, tex)));
}

// Resource Destruction
void gfx_texture_fini_impl(neko_handle(gfx_texture_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    if (!neko_slot_array_handle_valid(ogl->textures, hndl.id)) return;
    neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, hndl.id);
    glDeleteTextures(1, &tex->id);
    neko_slot_array_erase(ogl->textures, hndl.id);
}

void gfx_texture_desc_query(neko_handle(gfx_texture_t) hndl, gfx_texture_desc_t* out) {
    if (!out) return;

    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, hndl.id);

    // Read back pixels
    if (out->data && out->read.width && out->read.height) {
        u32 type = neko_texture_format(tex->desc.format);
        u32 format = neko_gl_texture_format_to_gl_texture_format(tex->desc.format);
        CHECK_GL_CORE(glActiveTexture(GL_TEXTURE0); glGetTextureSubImage(tex->id, 0, out->read.x, out->read.y, 0, out->read.width, out->read.height, 1, format, type, out->read.size, out->data););
    }

    *out = tex->desc;
}

// Resource Updates (main thread only)
void gfx_texture_update_impl(neko_handle(gfx_texture_t) hndl, gfx_texture_desc_t* desc) {
    if (!desc) return;

    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    if (!neko_slot_array_handle_valid(ogl->textures, hndl.id)) {
        console_log("AssetTexture handle invalid: %zu", hndl.id);
        return;
    }
    gl_texture_update_internal(desc, hndl.id);
}

void gfx_texture_read_impl(neko_handle(gfx_texture_t) hndl, gfx_texture_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    if (!desc) return;
    if (!neko_slot_array_handle_valid(ogl->textures, hndl.id)) {
        console_log("AssetTexture handle invalid: %zu", hndl.id);
    }

    neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, hndl.id);
    // Bind texture
    GLenum target = GL_TEXTURE_2D;
    u32 gl_format = neko_gl_texture_format_to_gl_texture_format(tex->desc.format);
    u32 gl_type = neko_texture_format(tex->desc.format);
    glBindTexture(target, tex->id);
    glReadPixels(desc->read.x, desc->read.y, desc->read.width, desc->read.height, gl_format, gl_type, desc->data);
    glBindTexture(target, 0x00);
}

neko_gl_data_t* gfx_ogl() {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    return ogl;
}

void gfx_init(gfx_t* render) {
    // Push back 0 handles into slot arrays (for 0 init validation)
    neko_gl_data_t* ogl = (neko_gl_data_t*)render->ud;

    neko_gl_texture_t tex = NEKO_DEFAULT_VAL();

    neko_slot_array_insert(ogl->textures, tex);

    // Init info object
    gfx_info_t* info = &RENDER()->info;

    // OpenGL 主机信息
    glGetIntegerv(GL_MAJOR_VERSION, (GLint*)&info->major_version);
    glGetIntegerv(GL_MINOR_VERSION, (GLint*)&info->minor_version);
    info->version = (const_str)glGetString(GL_VERSION);
    info->vendor = (const_str)glGetString(GL_RENDERER);

    // console_log("OpenGL vendor:   %s", glGetString(GL_VENDOR));
    // console_log("OpenGL renderer: %s", glGetString(GL_RENDERER));
    // console_log("OpenGL version:  %s", glGetString(GL_VERSION));

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&info->max_texture_size);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, (GLint*)&info->max_texture_units);

    if (info->max_texture_size < 2048) console_log("OpenGL maximum texture too small");

    // Compute shader info
    info->compute.available = info->major_version >= 4 && info->minor_version >= 3;
    if (info->compute.available) {
        CHECK_GL_CORE({
            // Work group counts
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, (i32*)&info->compute.max_work_group_count[0]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, (i32*)&info->compute.max_work_group_count[1]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, (i32*)&info->compute.max_work_group_count[2]);
            // Work group sizes
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, (i32*)&info->compute.max_work_group_size[0]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, (i32*)&info->compute.max_work_group_size[1]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, (i32*)&info->compute.max_work_group_size[2]);
            // Work group invocations
            glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, (i32*)&info->compute.max_work_group_invocations);
        });
    } else {
        // console_log("OpenGL compute shaders not available");
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

neko_handle(gfx_texture_t) gfx_texture_create(const gfx_texture_desc_t desc) { return gfx_texture_create_impl(desc); }

void gfx_texture_fini(neko_handle(gfx_texture_t) hndl) { gfx_texture_fini_impl(hndl); }

// 资源更新 (仅限主线程)
void gfx_texture_update(neko_handle(gfx_texture_t) hndl, gfx_texture_desc_t* desc) { gfx_texture_update_impl(hndl, desc); }

void gfx_texture_read(neko_handle(gfx_texture_t) hndl, gfx_texture_desc_t* desc) { gfx_texture_read_impl(hndl, desc); }

neko_color_t neko_color_from_rgb_color(neko_rgb_color_t rgb) {
    i8 r = (i8)(rgb.r * 255.0);
    i8 g = (i8)(rgb.g * 255.0);
    i8 b = (i8)(rgb.b * 255.0);

    return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

neko_rgb_color_t neko_rgb_color_from_color(neko_color_t color) {
    int r = (color >> 16) & 0xFF;
    int g = (color >> 8) & 0xFF;
    int b = color & 0xFF;

    float rf = (float)r / 255.0f;
    float gf = (float)g / 255.0f;
    float bf = (float)b / 255.0f;

    return neko_rgb_color_t{rf, gf, bf};
}

u32 total_draw_calls;

u32 neko_get_total_draw_calls() { return total_draw_calls; }

neko_vertex_buffer_t* neko_new_vertex_buffer(neko_vertex_buffer_flags_t flags) {
    neko_vertex_buffer_t* buffer = (neko_vertex_buffer_t*)mem_alloc(sizeof(neko_vertex_buffer_t));

    buffer->flags = flags;

    glGenVertexArrays(1, &buffer->va_id);
    glGenBuffers(1, &buffer->vb_id);
    glGenBuffers(1, &buffer->ib_id);

    return buffer;
}

void neko_free_vertex_buffer(neko_vertex_buffer_t* buffer) {
    assert(buffer);

    glDeleteVertexArrays(1, &buffer->va_id);
    glDeleteBuffers(1, &buffer->vb_id);
    glDeleteBuffers(1, &buffer->ib_id);

    mem_free(buffer);
}

void neko_bind_vertex_buffer_for_draw(neko_vertex_buffer_t* buffer) { glBindVertexArray(buffer ? buffer->va_id : 0); }

void neko_bind_vertex_buffer_for_edit(neko_vertex_buffer_t* buffer) {
    if (!buffer) {
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    } else {
        glBindVertexArray(buffer->va_id);
        glBindBuffer(GL_ARRAY_BUFFER, buffer->vb_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->ib_id);
    }
}

void neko_push_vertices(neko_vertex_buffer_t* buffer, float* vertices, u32 count) {
    assert(buffer);

    u32 mode = GL_STATIC_DRAW;
    if (buffer->flags & NEKO_VERTEXBUFFER_DYNAMIC_DRAW) {
        mode = GL_DYNAMIC_DRAW;
    }

    glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), vertices, mode);
}

void neko_push_indices(neko_vertex_buffer_t* buffer, u32* indices, u32 count) {
    assert(buffer);

    u32 mode = GL_STATIC_DRAW;
    if (buffer->flags & NEKO_VERTEXBUFFER_DYNAMIC_DRAW) {
        mode = GL_DYNAMIC_DRAW;
    }

    buffer->index_count = count;

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(u32), indices, mode);
}

void neko_update_vertices(neko_vertex_buffer_t* buffer, float* vertices, u32 offset, u32 count) {
    assert(buffer);

    glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(float), count * sizeof(float), vertices);
}

void neko_update_indices(neko_vertex_buffer_t* buffer, u32* indices, u32 offset, u32 count) {
    assert(buffer);

    buffer->index_count = count;

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset * sizeof(u32), count * sizeof(u32), indices);
}

void neko_configure_vertex_buffer(neko_vertex_buffer_t* buffer, u32 index, u32 component_count, u32 stride, u32 offset) {
    assert(buffer);

    glVertexAttribPointer(index, component_count, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(u64)(offset * sizeof(float)));
    glEnableVertexAttribArray(index);
}

void neko_draw_vertex_buffer(neko_vertex_buffer_t* buffer) {
    assert(buffer);

    u32 draw_type = GL_TRIANGLES;
    if (buffer->flags & NEKO_VERTEXBUFFER_DRAW_LINES) {
        draw_type = GL_LINES;
    } else if (buffer->flags & NEKO_VERTEXBUFFER_DRAW_LINE_STRIP) {
        draw_type = GL_LINE_STRIP;
    }

    glDrawElements(draw_type, buffer->index_count, GL_UNSIGNED_INT, 0);

    total_draw_calls++;
}

void neko_draw_vertex_buffer_custom_count(neko_vertex_buffer_t* buffer, u32 count) {
    assert(buffer);

    u32 draw_type = GL_TRIANGLES;
    if (buffer->flags & NEKO_VERTEXBUFFER_DRAW_LINES) {
        draw_type = GL_LINES;
    } else if (buffer->flags & NEKO_VERTEXBUFFER_DRAW_LINE_STRIP) {
        draw_type = GL_LINE_STRIP;
    }

    glDrawElements(draw_type, count, GL_UNSIGNED_INT, 0);

    total_draw_calls++;
}

void neko_bind_shader(u32 shader) { glUseProgram(shader); }

void neko_shader_set_int(u32 shader, const char* name, i32 v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniform1i(location, v);
}

void neko_shader_set_uint(u32 shader, const char* name, u32 v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniform1ui(location, v);
}

void neko_shader_set_float(u32 shader, const char* name, float v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniform1f(location, v);
}

void neko_shader_set_color(u32 shader, const char* name, neko_color_t color) {

    neko_rgb_color_t rgb = neko_rgb_color_from_color(color);

    neko_shader_set_v3f(shader, name, neko_v3f_t{rgb.r, rgb.g, rgb.b});
}

void neko_shader_set_rgb_color(u32 shader, const char* name, neko_rgb_color_t color) { neko_shader_set_v3f(shader, name, neko_v3f_t{color.r, color.g, color.b}); }

void neko_shader_set_v2i(u32 shader, const char* name, neko_v2i_t v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniform2i(location, v.x, v.y);
}

void neko_shader_set_v2u(u32 shader, const char* name, neko_v2u_t v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniform2ui(location, v.x, v.y);
}

void neko_shader_set_v2f(u32 shader, const char* name, neko_v2f_t v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniform2f(location, v.x, v.y);
}

void neko_shader_set_v3i(u32 shader, const char* name, neko_v3i_t v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniform3i(location, v.x, v.y, v.z);
}

void neko_shader_set_v3u(u32 shader, const char* name, neko_v3u_t v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniform3ui(location, v.x, v.y, v.z);
}

void neko_shader_set_v3f(u32 shader, const char* name, neko_v3f_t v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniform3f(location, v.x, v.y, v.z);
}

void neko_shader_set_v4i(u32 shader, const char* name, neko_v4i v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniform4i(location, v.x, v.y, v.z, v.w);
}

void neko_shader_set_v4u(u32 shader, const char* name, neko_v4u v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniform4ui(location, v.x, v.y, v.z, v.w);
}

void neko_shader_set_v4f(u32 shader, const char* name, neko_v4f_t v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniform4f(location, v.x, v.y, v.z, v.w);
}

void neko_shader_set_m4f(u32 shader, const char* name, neko_m4f_t v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniformMatrix4fv(location, 1, GL_FALSE, (float*)v.elements);
}

neko_texture_t* neko_new_texture(neko_resource_t* resource, neko_texture_flags_t flags) {
    assert(resource);

    neko_texture_t* texture = (neko_texture_t*)mem_alloc(sizeof(neko_texture_t));
    neko_init_texture(texture, resource, flags);
    return texture;
}

neko_texture_t* neko_new_texture_from_memory(void* data, u32 size, neko_texture_flags_t flags) {
    neko_texture_t* texture = (neko_texture_t*)mem_alloc(sizeof(neko_texture_t));
    neko_init_texture_from_memory(texture, data, size, flags);
    return texture;
}

neko_texture_t* neko_new_texture_from_memory_uncompressed(unsigned char* pixels, u32 size, i32 width, i32 height, i32 component_count, neko_texture_flags_t flags) {
    neko_texture_t* texture = (neko_texture_t*)mem_alloc(sizeof(neko_texture_t));
    neko_init_texture_from_memory_uncompressed(texture, pixels, size, width, height, component_count, flags);
    return texture;
}

void neko_init_texture(neko_texture_t* texture, neko_resource_t* resource, neko_texture_flags_t flags) {
    if (resource->payload == NULL || resource->type != NEKO_RESOURCE_BINARY) {
        console_log("Resource insufficient for texture creation.");
        return;
    }

    neko_init_texture_from_memory(texture, resource->payload, resource->payload_size, flags);
}

void neko_init_texture_from_memory(neko_texture_t* texture, void* data, u32 size, neko_texture_flags_t flags) {
    if (data == NULL || size == 0) {
        console_log("Data insufficient for texture creation.");
        return;
    }

    i32 width, height, component_count;

    stbi_set_flip_vertically_on_load(true);

    u8* pixels = stbi_load_from_memory((u8*)data, size, &width, &height, &component_count, 0);

    if (pixels == NULL) {
        console_log("Failed to load texture: %s", stbi_failure_reason());
        return;
    }

    neko_init_texture_from_memory_uncompressed(texture, pixels, size, width, height, component_count, flags);

    mem_free(pixels);
}

void neko_init_texture_from_memory_uncompressed(neko_texture_t* texture, unsigned char* pixels, u32 size, i32 width, i32 height, i32 component_count, neko_texture_flags_t flags) {
    if (pixels == NULL || size == 0) {
        console_log("Data insufficient for texture creation.");
        return;
    }

    texture->width = width;
    texture->height = height;
    texture->component_count = component_count;

    texture->flags = flags;

    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);

    u32 alias_mode = GL_LINEAR;
    if (flags & NEKO_TEXTURE_ALIASED) {
        alias_mode = GL_NEAREST;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, alias_mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, alias_mode);

    u32 format = GL_RGB;
    u32 internal_format = GL_RGB;
    if (texture->component_count == 4) {
        internal_format = GL_RGBA;
        format = GL_RGBA;
    } else if (texture->component_count == 1) {
        internal_format = GL_RED;
        format = GL_RED;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, internal_format, GL_UNSIGNED_BYTE, pixels);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void neko_deinit_texture(neko_texture_t* texture) {
    assert(texture);
    glDeleteTextures(1, &texture->id);
}

void neko_free_texture(neko_texture_t* texture) {
    assert(texture);

    neko_deinit_texture(texture);

    mem_free(texture);
}

void neko_bind_texture(neko_texture_t* texture, u32 slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture ? texture->id : 0);
}
