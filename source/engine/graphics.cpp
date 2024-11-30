#include "graphics.h"

#include <stdio.h>
#include <stdlib.h>

#include "engine/asset.h"
#include "engine/base.hpp"
#include "base/common/profiler.hpp"
#include "base/cbase.hpp"

// deps
#include "extern/stb_image.h"

#define CUTE_ASEPRITE_ASSERT neko_assert
#define CUTE_ASEPRITE_ALLOC(size, ctx) mem_alloc(size)
#define CUTE_ASEPRITE_FREE(mem, ctx) mem_free(mem)

#define CUTE_ASEPRITE_IMPLEMENTATION
#include "extern/cute_aseprite.h"

extern const mat3* camera_get_inverse_view_matrix_ptr();  // for GLSL binding : in component.cpp

bool neko_init_shader(AssetShader* shader, char* source) {
    neko_assert(shader);

    shader->panic_mode = false;

    const u32 source_len = (u32)strlen(source);

    char* vertex_source = (char*)mem_alloc(source_len);
    char* fragment_source = (char*)mem_alloc(source_len);
    char* geometry_source = (char*)mem_alloc(source_len);
    neko_defer(mem_free(vertex_source));
    neko_defer(mem_free(fragment_source));
    neko_defer(mem_free(geometry_source));

    memset(vertex_source, 0, source_len);
    memset(fragment_source, 0, source_len);
    memset(geometry_source, 0, source_len);

    bool has_geometry = false;

    u32 count = 0;
    i32 adding_to = -1;
    for (char* current = source; *current != '\0'; current++) {
        if (*current == '\n' && *(current + 1) != '\0') {
            i32 minus = 1;

            current++;

            char* line = current - count - minus;
            line[count] = '\0';

            if (strstr(line, "#begin VERTEX")) {
                adding_to = 0;
            } else if (strstr(line, "#begin FRAGMENT")) {
                adding_to = 1;
            } else if (strstr(line, "#begin GEOMETRY")) {
                adding_to = 2;
                has_geometry = true;
            } else if (strstr(line, "#end VERTEX") || strstr(line, "#end FRAGMENT") || strstr(line, "#end GEOMETRY")) {
                adding_to = -1;
            } else if (adding_to == 0) {
                strcat(vertex_source, line);
                strcat(vertex_source, "\n");
            } else if (adding_to == 1) {
                strcat(fragment_source, line);
                strcat(fragment_source, "\n");
            } else if (adding_to == 2) {
                strcat(geometry_source, line);
                strcat(geometry_source, "\n");
            }

            count = 0;
        }

        count++;
    }

    const char* vsp = vertex_source;
    const char* fsp = fragment_source;
    const char* gsp = geometry_source;

    i32 success;
    u32 v, f, g;
    v = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v, 1, &vsp, NULL);
    glCompileShader(v);

    glGetShaderiv(v, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[1024];
        glGetShaderInfoLog(v, 1024, 0x0, info_log);
        console_log("error %s", info_log);
        shader->panic_mode = true;
    }

    f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f, 1, &fsp, NULL);
    glCompileShader(f);

    glGetShaderiv(f, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[1024];
        glGetShaderInfoLog(f, 1024, 0x0, info_log);
        console_log("error %s", info_log);
        shader->panic_mode = true;
    }

    if (has_geometry) {
        g = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(g, 1, &gsp, NULL);
        glCompileShader(g);

        glGetShaderiv(g, GL_COMPILE_STATUS, &success);
        if (!success) {
            char info_log[1024];
            glGetShaderInfoLog(g, 1024, 0x0, info_log);
            console_log("error %s", info_log);
            shader->panic_mode = true;
        }
    }

    u32 id;
    id = glCreateProgram();
    glAttachShader(id, v);
    glAttachShader(id, f);
    if (has_geometry) {
        glAttachShader(id, g);
    }
    glLinkProgram(id);

    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        shader->panic_mode = true;
    }

    glDeleteShader(v);
    glDeleteShader(f);

    if (has_geometry) {
        glDeleteShader(g);
    }

    shader->id = id;
    return !shader->panic_mode;
}

void neko_unload_shader(AssetShader* shader) {
    neko_assert(shader);
    if (shader->panic_mode) return;
    glDeleteProgram(shader->id);
}

bool neko_load_shader(AssetShader* shader, String path) {
    neko_assert(shader);

    String source = {};
    bool ok = vfs_read_entire_file(&source, path);
    neko_defer(mem_free(source.data));

    if (ok) {
        ok = neko_init_shader(shader, source.data);
    }

    return ok;
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

gfx_t* gfx_create() {
    // 构建新的图形界面
    gfx_t* gfx = (gfx_t*)mem_alloc(sizeof(gfx_t));
    memset(gfx, 0, sizeof(gfx_t));
    // 为OpenGL构建内部数据
    gfx->ud = (neko_gl_data_t*)mem_alloc(sizeof(neko_gl_data_t));
    memset(gfx->ud, 0, sizeof(neko_gl_data_t));
    return gfx;
}

void gfx_fini(gfx_t* render) {
    // 释放所有资源 (假设它们已从GPU中释放)
    if (render == NULL) return;

    neko_gl_data_t* ogl = (neko_gl_data_t*)render->ud;

    if (ogl->textures.len)
        for (auto& tex : ogl->textures) {
        }

    ogl->textures.trash();

    mem_free(ogl);

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
        console_log("AssetTexture handle invalid: %zu", hndl.id);
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

neko_gl_data_t* gfx_ogl() {
    neko_gl_data_t* ogl = (neko_gl_data_t*)RENDER()->ud;
    return ogl;
}

void gfx_init(gfx_t* render) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)render->ud;

    gfx_info_t* info = &RENDER()->info;

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

    info->compute.available = info->major_version >= 4 && info->minor_version >= 3;
    if (info->compute.available) {
        CHECK_GL_CORE({
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, (i32*)&info->compute.max_work_group_count[0]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, (i32*)&info->compute.max_work_group_count[1]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, (i32*)&info->compute.max_work_group_count[2]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, (i32*)&info->compute.max_work_group_size[0]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, (i32*)&info->compute.max_work_group_size[1]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, (i32*)&info->compute.max_work_group_size[2]);
            glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, (i32*)&info->compute.max_work_group_invocations);
        });
    } else {
        console_log("OpenGL compute shaders not available");
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

    neko_shader_set_v3f(shader, name, vec3{rgb.r, rgb.g, rgb.b});
}

void neko_shader_set_rgb_color(u32 shader, const char* name, neko_rgb_color_t color) { neko_shader_set_v3f(shader, name, vec3{color.r, color.g, color.b}); }

void neko_shader_set_v2f(u32 shader, const char* name, vec2 v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniform2f(location, v.x, v.y);
}

void neko_shader_set_v3f(u32 shader, const char* name, vec3 v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniform3f(location, v.x, v.y, v.z);
}

void neko_shader_set_v4f(u32 shader, const char* name, vec4 v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniform4f(location, v.x, v.y, v.z, v.w);
}

void neko_shader_set_m4f(u32 shader, const char* name, mat4 v) {

    u32 location = glGetUniformLocation(shader, name);
    glUniformMatrix4fv(location, 1, GL_FALSE, (float*)v.elements);
}

AssetTexture* neko_new_texture(neko_resource_t* resource, neko_texture_flags_t flags) {
    assert(resource);

    AssetTexture* texture = (AssetTexture*)mem_alloc(sizeof(AssetTexture));
    neko_init_texture(texture, resource, flags);
    return texture;
}

AssetTexture* neko_new_texture_from_memory(void* data, u32 size, neko_texture_flags_t flags) {
    AssetTexture* texture = (AssetTexture*)mem_alloc(sizeof(AssetTexture));
    neko_init_texture_from_memory(texture, data, size, flags);
    return texture;
}

AssetTexture* neko_new_texture_from_memory_uncompressed(unsigned char* pixels, u32 size, i32 width, i32 height, i32 component_count, neko_texture_flags_t flags) {
    AssetTexture* texture = (AssetTexture*)mem_alloc(sizeof(AssetTexture));
    neko_init_texture_from_memory_uncompressed(texture, pixels, width, height, component_count, flags);
    return texture;
}

void neko_init_texture(AssetTexture* texture, neko_resource_t* resource, neko_texture_flags_t flags) {
    if (resource->payload == NULL || resource->type != NEKO_RESOURCE_BINARY) {
        console_log("Resource insufficient for texture creation.");
        return;
    }

    neko_init_texture_from_memory(texture, resource->payload, resource->payload_size, flags);
}

void neko_init_texture_from_memory(AssetTexture* texture, void* data, u32 size, neko_texture_flags_t flags) {
    if (data == NULL || size == 0) {
        console_log("Data insufficient for texture creation.");
        return;
    }

    i32 width, height, component_count;

    if (!(flags & NEKO_TEXTURE_NO_FLIP_VERTICAL)) stbi_set_flip_vertically_on_load(true);

    u8* pixels = stbi_load_from_memory((u8*)data, size, &width, &height, &component_count, 0);

    if (pixels == NULL) {
        console_log("Failed to load texture: %s", stbi_failure_reason());
        return;
    }

    neko_init_texture_from_memory_uncompressed(texture, pixels, width, height, component_count, flags);

    mem_free(pixels);
}

void neko_init_texture_from_memory_uncompressed(AssetTexture* texture, unsigned char* pixels, i32 width, i32 height, i32 component_count, neko_texture_flags_t flags) {
    if (pixels == NULL) {
        console_log("Data insufficient for texture creation.");
        return;
    }

    texture->width = width;
    texture->height = height;
    texture->components = component_count;

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
    if (texture->components == 4) {
        internal_format = GL_RGBA;
        format = GL_RGBA;
    } else if (texture->components == 1) {
        internal_format = GL_RED;
        format = GL_RED;
    }

    if (flags & NEKO_TEXTURE_SUBTEX) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, internal_format, GL_UNSIGNED_BYTE, pixels);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, internal_format, GL_UNSIGNED_BYTE, pixels);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

void neko_deinit_texture(AssetTexture* texture) {
    assert(texture);
    glDeleteTextures(1, &texture->id);
}

void neko_free_texture(AssetTexture* texture) {
    assert(texture);

    neko_deinit_texture(texture);

    mem_free(texture);
}

void neko_bind_texture(AssetTexture* texture, u32 slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture ? texture->id : 0);
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

static void _flip_image_vertical(unsigned char* data, unsigned int width, unsigned int height) {
    unsigned int size, stride, i, j;
    unsigned char* new_data;

    size = width * height * 4;
    stride = sizeof(char) * width * 4;

    new_data = (unsigned char*)mem_alloc(sizeof(char) * size);
    for (i = 0; i < height; i++) {
        j = height - i - 1;
        memcpy(new_data + j * stride, data + i * stride, stride);
    }

    memcpy(data, new_data, size);
    mem_free(new_data);
}

NEKO_FORCE_INLINE void neko_tex_flip_vertically(int width, int height, u8* data) {
    u8 rgb[4];
    for (int y = 0; y < height / 2; y++) {
        for (int x = 0; x < width; x++) {
            int top = 4 * (x + y * width);
            int bottom = 4 * (x + (height - y - 1) * width);
            memcpy(rgb, data + top, sizeof(rgb));
            memcpy(data + top, data + bottom, sizeof(rgb));
            memcpy(data + bottom, rgb, sizeof(rgb));
        }
    }
}

AssetTexture neko_aseprite_simple(String filename) {
    String contents = {};
    bool ok = vfs_read_entire_file(&contents, filename);

    neko_defer(mem_free(contents.data));

    ase_t* ase;

    {
        PROFILE_BLOCK("ase_image load");
        ase = cute_aseprite_load_from_memory(contents.data, contents.len, nullptr);

        neko_assert(ase->frame_count == 1);  // image_load_ase 用于加载简单的单帧 aseprite
        // neko_aseprite_default_blend_bind(ase);
    }

    u8* data = reinterpret_cast<u8*>(ase->frames->pixels);

    // gfx_texture_desc_t t_desc = {};

    // t_desc.format = GL_RGBA;
    // t_desc.mag_filter = GL_NEAREST;
    // t_desc.min_filter = GL_NEAREST;
    // t_desc.num_mips = 0;
    // t_desc.width = ase->w;
    // t_desc.height = ase->h;
    //// t_desc.num_comps = 4;
    // t_desc.data = data;

    neko_tex_flip_vertically(ase->w, ase->h, (u8*)(data));
    // gfx_texture_t tex = gfx_texture_create(t_desc);

    AssetTexture tex{};

    neko_init_texture_from_memory_uncompressed(&tex, data, ase->w, ase->h, 4, NEKO_TEXTURE_ALIASED);

    cute_aseprite_free(ase);
    return tex;
}

static void ase_default_blend_bind(ase_t* ase) {

    neko_assert(ase);
    neko_assert(ase->frame_count);

    // 为了方便起见，将所有单元像素混合到各自的帧中
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t* frame = ase->frames + i;
        if (frame->pixels != NULL) mem_free(frame->pixels);
        frame->pixels = (ase_color_t*)mem_alloc((size_t)(sizeof(ase_color_t)) * ase->w * ase->h);
        memset(frame->pixels, 0, sizeof(ase_color_t) * (size_t)ase->w * (size_t)ase->h);
        ase_color_t* dst = frame->pixels;

        console_log("neko_aseprite_default_blend_bind: frame: %d cel_count: %d", i, frame->cel_count);

        for (int j = 0; j < frame->cel_count; ++j) {  //

            ase_cel_t* cel = frame->cels + j;

            if (!(cel->layer->flags & ASE_LAYER_FLAGS_VISIBLE) || (cel->layer->parent && !(cel->layer->parent->flags & ASE_LAYER_FLAGS_VISIBLE))) {
                continue;
            }

            while (cel->is_linked) {
                ase_frame_t* frame = ase->frames + cel->linked_frame_index;
                int found = 0;
                for (int k = 0; k < frame->cel_count; ++k) {
                    if (frame->cels[k].layer == cel->layer) {
                        cel = frame->cels + k;
                        found = 1;
                        break;
                    }
                }
                neko_assert(found);
            }
            void* src = cel->pixels;
            u8 opacity = (u8)(cel->opacity * cel->layer->opacity * 255.0f);
            int cx = cel->x;
            int cy = cel->y;
            int cw = cel->w;
            int ch = cel->h;
            int cl = -NEKO_MIN(cx, 0);
            int ct = -NEKO_MIN(cy, 0);
            int dl = NEKO_MAX(cx, 0);
            int dt = NEKO_MAX(cy, 0);
            int dr = NEKO_MIN(ase->w, cw + cx);
            int db = NEKO_MIN(ase->h, ch + cy);
            int aw = ase->w;
            for (int dx = dl, sx = cl; dx < dr; dx++, sx++) {
                for (int dy = dt, sy = ct; dy < db; dy++, sy++) {
                    int dst_index = aw * dy + dx;
                    ase_color_t src_color = s_color(ase, src, cw * sy + sx);
                    ase_color_t dst_color = dst[dst_index];
                    ase_color_t result = s_blend(src_color, dst_color, opacity);
                    dst[dst_index] = result;
                }
            }
        }
    }
}

u64 generate_texture_handle(void* pixels, int w, int h, void* udata) {
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

void destroy_texture_handle(u64 texture_id, void* udata) {
    (void)udata;
    GLuint id = (GLuint)texture_id;
    glDeleteTextures(1, &id);
}

bool texture_update_data(AssetTexture* tex, u8* data) {

    LockGuard<Mutex> lock{gBase.gpu_mtx};

    // 如果存在 则释放旧的 GL 纹理
    if (tex->id != 0) glDeleteTextures(1, &tex->id);

    // 生成 GL 纹理
    glGenTextures(1, &tex->id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex->id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (tex->flip_image_vertical) {
        _flip_image_vertical(data, tex->width, tex->height);
    }

    // 将纹理数据复制到 GL

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex->width, tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

static bool _texture_load_vfs(AssetTexture* tex, String filename) {
    neko_assert(tex);

    u8* data = nullptr;

    console_log("texture: loading texture '%s' ...", filename.cstr());

    String contents = {};
    bool ok = vfs_read_entire_file(&contents, filename);
    if (!ok) {
        return false;
    }
    neko_defer(mem_free(contents.data));

    ase_t* ase;

    if (filename.ends_with(".ase")) {

        {
            PROFILE_BLOCK("ase_image load");
            ase = cute_aseprite_load_from_memory(contents.data, contents.len, nullptr);

            neko_assert(ase->frame_count == 1);  // image_load_ase 用于加载简单的单帧 aseprite
            // neko_aseprite_default_blend_bind(ase);

            tex->width = ase->w;
            tex->height = ase->h;
        }

        data = reinterpret_cast<u8*>(ase->frames->pixels);

    } else {

        {
            PROFILE_BLOCK("stb_image load");
            stbi_set_flip_vertically_on_load(false);
            data = stbi_load_from_memory((u8*)contents.data, (i32)contents.len, &tex->width, &tex->height, &tex->components, 0);
        }
    }

    // 读入纹理数据
    if (!data) {
        // tex->last_modified = modtime;
        console_log(" unsuccessful");
        return false;  // 保持旧的GL纹理
    }

    {
        LockGuard<Mutex> lock(gBase.gpu_mtx);

        // 如果存在 则释放旧的 GL 纹理
        if (tex->id != 0) glDeleteTextures(1, &tex->id);

        // 生成 GL 纹理
        glGenTextures(1, &tex->id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex->id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if (tex->flip_image_vertical) {
            _flip_image_vertical(data, tex->width, tex->height);
        }

        // 将纹理数据复制到 GL

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex->width, tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (filename.ends_with(".ase")) {
        cute_aseprite_free(ase);
    } else {
        stbi_image_free(data);
    }

    // tex->last_modified = modtime;
    console_log(" successful");
    return true;
}

bool texture_load(AssetTexture* tex, String filename, bool flip_image_vertical) {
    tex->id = 0;
    tex->flip_image_vertical = flip_image_vertical;
    return _texture_load_vfs(tex, filename);
}

void texture_bind(const char* filename) {
    LockGuard<Mutex> lock{gBase.gpu_mtx};

    Asset a = {};
    bool ok = asset_load_kind(AssetKind_Image, filename, &a);

    if (ok && a.texture.id != 0) glBindTexture(GL_TEXTURE_2D, a.texture.id);
}

vec2 texture_get_size(const char* filename) {
    Asset a = {};
    bool ok = asset_load_kind(AssetKind_Image, filename, &a);
    error_assert(ok);
    return luavec2(a.texture.width, a.texture.height);
}

AssetTexture texture_get_ptr(const char* filename) {
    Asset a = {};
    bool ok = asset_load_kind(AssetKind_Image, filename, &a);
    return a.texture;
}

bool texture_update(AssetTexture* tex, String filename) { return _texture_load_vfs(tex, filename); }

bool load_texture_data_from_memory(const void* memory, size_t sz, i32* width, i32* height, u32* num_comps, void** data, bool flip_vertically_on_load) {
    // Load texture data

    int channels;
    u8* image = (u8*)stbi_load_from_memory((unsigned char*)memory, sz, width, height, &channels, 0);

    // if (flip_vertically_on_load) neko_png_flip_image_horizontal(&img);

    *data = image;

    if (!*data) {
        // neko_image_free(&img);
        console_log("could not load image %p", memory);
        return false;
    }
    return true;
}

bool load_texture_data_from_file(const char* file_path, i32* width, i32* height, u32* num_comps, void** data, bool flip_vertically_on_load) {
    size_t len = 0;
    const_str file_data = neko_capi_vfs_read_file(NEKO_PACKS::GAMEDATA, file_path, &len);
    neko_assert(file_data);
    bool ret = load_texture_data_from_memory(file_data, len, width, height, num_comps, data, flip_vertically_on_load);
    if (!ret) {
        console_log("could not load texture: %s", file_path);
    }
    mem_free(file_data);
    return ret;
}
