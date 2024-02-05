

/*=============================
// NEKO_GRAPHICS
=============================*/

#include "engine/neko_engine.h"
#include "engine/neko_platform.h"

#ifndef NEKO_GRAPHICS_IMPL_CUSTOM

#if (defined NEKO_PLATFORM_WIN || defined NEKO_PLATFORM_APPLE || defined NEKO_PLATFORM_LINUX)

#define NEKO_GRAPHICS_IMPL_OPENGL_CORE

#else

#define NEKO_GRAPHICS_IMPL_OPENGL_ES

#endif

#endif

#ifndef NEKO_GRAPHICS_IMPL_H
#define NEKO_GRAPHICS_IMPL_H

#define GLAD_IMPL
#include "libs/glad/glad_impl.h"

#ifndef NEKO_GRAPHICS_IMPL_CUSTOM
#define NEKO_GRAPHICS_IMPL_DEFAULT
#endif

#ifdef NEKO_GRAPHICS_IMPL_DEFAULT

/* Graphics Info Object Query */
neko_graphics_info_t* neko_graphics_info() { return &neko_subsystem(graphics)->info; }

#endif

#if (defined NEKO_GRAPHICS_IMPL_OPENGL_CORE || defined NEKO_GRAPHICS_IMPL_OPENGL_ES)

#ifdef NEKO_GRAPHICS_IMPL_OPENGL_CORE
#define CHECK_GL_CORE(...) __VA_ARGS__
#else
#define CHECK_GL_CORE(...) neko_empty_instruction(void)
#endif

const char* gl_get_error_string(GLenum error_code) {
    switch (error_code) {
        case GL_NO_ERROR:
            return "GL_NO_ERROR";
        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";
        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
        default:
            return "UNKNOWN_ERROR";
    }
}

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

void neko_graphics_print_errors_internal(const char* file, u32 line) {
    GLenum code = glGetError();

    if (code != GL_NO_ERROR) {
        const char* last_slash = file;

        while (*file) {
            char c = *file;
            if (c == '\\' || c == '/') last_slash = file + 1;
            ++file;
        }

        const char* str = gl_get_error_string(code);
        const char* des = gl_get_error_description(code);
        neko_println("OpenGL Error %s (%u): %u, %s, %s:", last_slash, line, code, str, des);
    }
}

struct neko_graphics_custom_batch_context_t {
    u32 max_draw_calls;
    u32 count;
    neko_graphics_custom_batch_draw_call_t* calls;
};

neko_graphics_custom_batch_context_t* neko_graphics_custom_batch_make_ctx(u32 max_draw_calls) {
    neko_graphics_custom_batch_context_t* ctx = (neko_graphics_custom_batch_context_t*)neko_safe_malloc(sizeof(neko_graphics_custom_batch_context_t));
    // ctx->clear_bits = clear_bits;
    // ctx->settings_bits = settings_bits;
    ctx->max_draw_calls = max_draw_calls;
    ctx->count = 0;
    ctx->calls = (neko_graphics_custom_batch_draw_call_t*)neko_safe_malloc(sizeof(neko_graphics_custom_batch_draw_call_t) * max_draw_calls);
    if (!ctx->calls) {
        neko_safe_free(ctx);
        return 0;
    }
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    return ctx;
}

void neko_graphics_custom_batch_free(void* ctx) {
    neko_graphics_custom_batch_context_t* context = (neko_graphics_custom_batch_context_t*)ctx;
    neko_safe_free(context->calls);
    neko_safe_free(context);
}

void neko_graphics_custom_batch_make_frame_buffer(neko_graphics_custom_batch_framebuffer_t* fb, neko_graphics_custom_batch_shader_t* shader, int w, int h, int use_depth_test) {
    // Generate the frame buffer
    GLuint fb_id;
    glGenFramebuffers(1, &fb_id);
    glBindFramebuffer(GL_FRAMEBUFFER, fb_id);

    // Generate a texture to use as the color buffer.
    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach color buffer to frame buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0);

    // Generate depth and stencil attachments for the fb using a RenderBuffer.
    GLuint rb_id = (GLuint)~0;
    if (use_depth_test) {
        glGenRenderbuffers(1, &rb_id);
        glBindRenderbuffer(GL_RENDERBUFFER, rb_id);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rb_id);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) neko_log_warning("cute_gl: failed to generate framebuffer");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Prepare quad
    GLuint quad_id;
    glGenBuffers(1, &quad_id);
    glBindBuffer(GL_ARRAY_BUFFER, quad_id);
    static GLfloat quad[] = {-1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,

                             -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f};
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    fb->fb_id = fb_id;
    fb->tex_id = tex_id;
    fb->rb_id = rb_id;
    fb->quad_id = quad_id;
    fb->shader = shader;
    fb->w = w;
    fb->h = h;
}

void neko_graphics_custom_batch_free_frame_buffer(neko_graphics_custom_batch_framebuffer_t* fb) {
    glDeleteTextures(1, &fb->tex_id);
    glDeleteRenderbuffers(1, &fb->rb_id);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fb->fb_id);
    glDeleteBuffers(1, &fb->quad_id);
    memset(fb, 0, sizeof(neko_graphics_custom_batch_framebuffer_t));
}

u64 gl_FNV1a(const char* str) {
    u64 h = (u64)14695981039346656037ULL;
    char c;

    while ((c = *str++)) {
        h = h ^ (u64)c;
        h = h * (u64)1099511628211ULL;
    }

    return h;
}

void neko_graphics_custom_batch_make_vertex_data(neko_graphics_custom_batch_vertex_data_t* vd, u32 buffer_size, u32 primitive, u32 vertex_stride, u32 usage) {
    vd->buffer_size = buffer_size;
    vd->vertex_stride = vertex_stride;
    vd->primitive = primitive;
    vd->usage = usage;
    vd->attribute_count = 0;
}

static u32 gl_get_gl_type_internal(u32 type) {
    switch (type) {
        case GL_INT:
        case GL_INT_VEC2:
        case GL_INT_VEC3:
        case GL_INT_VEC4:
            return NEKO_GL_CUSTOM_INT;

        case GL_FLOAT:
        case GL_FLOAT_VEC2:
        case GL_FLOAT_VEC3:
        case GL_FLOAT_VEC4:
        case GL_FLOAT_MAT2:
        case GL_FLOAT_MAT3:
        case GL_FLOAT_MAT4:
            return NEKO_GL_CUSTOM_FLOAT;

        case GL_BOOL:
        case GL_BOOL_VEC2:
        case GL_BOOL_VEC3:
        case GL_BOOL_VEC4:
            return NEKO_GL_CUSTOM_BOOL;

// seems undefined for GLES
#if GL_SAMPLER_1D
        case GL_SAMPLER_1D:
#endif
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
            return NEKO_GL_CUSTOM_SAMPLER;

        default:
            return NEKO_GL_CUSTOM_UNKNOWN;
    }
}

void neko_graphics_custom_batch_add_attribute(neko_graphics_custom_batch_vertex_data_t* vd, const char* name, u32 size, u32 type, u32 offset) {
    neko_graphics_custom_batch_vertex_attribute_t va;
    va.name = name;
    va.hash = gl_FNV1a(name);
    va.size = size;
    va.type = type;
    va.offset = offset;

    neko_assert(vd->attribute_count < NEKO_GL_CUSTOM_ATTRIBUTE_MAX_COUNT);
    vd->attributes[vd->attribute_count++] = va;
}

void neko_graphics_custom_batch_make_renderable(neko_graphics_custom_batch_renderable_t* r, neko_graphics_custom_batch_vertex_data_t* vd) {
    r->data = *vd;
    r->index0 = 0;
    r->index1 = 0;
    r->buffer_number = 0;
    r->need_new_sync = 0;
    r->program = 0;
    r->state.u.key = 0;

    if (vd->usage == GL_STATIC_DRAW) {
        r->buffer_count = 1;
        r->need_new_sync = 1;
    } else
        r->buffer_count = 3;
}

// WARNING: Messes with GL global state via glUnmapBuffer(GL_ARRAY_BUFFER) and
// glBindBuffer(GL_ARRAY_BUFFER, ...), so call gl_map_internal, fill in data, then call gl_unmap_internal.
void* gl_map_internal(neko_graphics_custom_batch_renderable_t* r, u32 count) {
    // Cannot map a buffer when the buffer is too small
    // Make your buffer is bigger or draw less data
    neko_assert(count <= r->data.buffer_size);

    u32 newIndex = r->index1 + count;

    if (newIndex > r->data.buffer_size) {
        // should never overflow a static buffer
        neko_assert(r->data.usage != GL_STATIC_DRAW);

        ++r->buffer_number;
        r->buffer_number %= r->buffer_count;
        GLsync fence = r->fences[r->buffer_number];

        // Ensure buffer is not in use by GPU
        // If we stall here we are GPU bound
        GLenum result = glClientWaitSync(fence, 0, (GLuint64)1000000000);
        neko_assert(result != GL_TIMEOUT_EXPIRED);
        neko_assert(result != GL_WAIT_FAILED);
        glDeleteSync(fence);

        r->index0 = 0;
        r->index1 = count;
        r->need_new_sync = 1;
    }

    else {
        r->index0 = r->index1;
        r->index1 = newIndex;
    }

    glBindBuffer(GL_ARRAY_BUFFER, r->buffers[r->buffer_number]);
    u32 stream_size = (r->index1 - r->index0) * r->data.vertex_stride;
    void* memory = glMapBufferRange(GL_ARRAY_BUFFER, r->index0 * r->data.vertex_stride, stream_size, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);

#if NEKO_GL_CUSTOM_DEBUG_CHECKS
    if (!memory) {
        neko_log_warning("\n%u\n", glGetError());
        neko_assert(memory);
    }
#endif

    return memory;
}

void gl_unmap_internal() { glUnmapBuffer(GL_ARRAY_BUFFER); }

void neko_graphics_custom_batch_set_shader(neko_graphics_custom_batch_renderable_t* r, neko_graphics_custom_batch_shader_t* program) {
    // Cannot set the shader of a Renderable more than once
    neko_assert(!r->program);

    r->program = program;
    glGetProgramiv(program->program, GL_ACTIVE_ATTRIBUTES, (GLint*)&r->attribute_count);

#if NEKO_GL_CUSTOM_DEBUG_CHECKS
    if (r->attribute_count != r->data.attribute_count) {
        neko_log_warning("Mismatch between VertexData attribute count (%d), and shader attribute count (%d).", r->attribute_count, r->data.attribute_count);
    }
#endif

    u32 size;
    u32 type;
    char buffer[256];
    u64 hash;

    // Query and set all attribute locations as defined by the shader linking
    for (u32 i = 0; i < r->attribute_count; ++i) {
        glGetActiveAttrib(program->program, i, 256, 0, (GLint*)&size, (GLenum*)&type, buffer);
        hash = gl_FNV1a(buffer);
        type = gl_get_gl_type_internal(type);

#if NEKO_GL_CUSTOM_DEBUG_CHECKS
        neko_graphics_custom_batch_vertex_attribute_t* a = 0;

        // Make sure data.AddAttribute(name, ...) has matching named attribute
        // Also make sure the GL::Type matches
        // This helps to catch common mismatch errors between glsl and C++
        for (u32 j = 0; j < r->data.attribute_count; ++j) {
            neko_graphics_custom_batch_vertex_attribute_t* b = r->data.attributes + j;

            if (b->hash == hash) {
                a = b;
                break;
            }
        }
#endif

        // Make sure the user did not have a mismatch between VertexData
        // attributes and the attributes defined in the vertex shader
        neko_assert(a);
        neko_assert(a->type == type);

        a->location = glGetAttribLocation(program->program, buffer);
    }

    // Generate VBOs and initialize fences
    u32 usage = r->data.usage;

    for (u32 i = 0; i < r->buffer_count; ++i) {
        GLuint* data_buffer = (GLuint*)r->buffers + i;

        glGenBuffers(1, data_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, *data_buffer);
        glBufferData(GL_ARRAY_BUFFER, r->data.buffer_size * r->data.vertex_stride, NULL, usage);
        r->fences[i] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLuint gl_compile_shader_internal(const char* Shader, u32 type) {
    GLuint handle = glCreateShader(type);
    glShaderSource(handle, 1, (const GLchar**)&Shader, NULL);
    glCompileShader(handle);

    u32 compiled;
    glGetShaderiv(handle, GL_COMPILE_STATUS, (GLint*)&compiled);

#if NEKO_GL_CUSTOM_DEBUG_CHECKS
    if (!compiled) {
        neko_log_warning("Shader of type %d failed compilation.", type);
        char out[2000];
        GLsizei outLen;
        glGetShaderInfoLog(handle, 2000, &outLen, out);
        neko_log_warning("%s", out);
        neko_assert(0);
    }
#endif

    return handle;
}

void neko_graphics_custom_batch_load_shader(neko_graphics_custom_batch_shader_t* s, const char* vertex, const char* pixel) {
    // Compile vertex and pixel Shader
    u32 program = glCreateProgram();
    u32 vs = gl_compile_shader_internal(vertex, GL_VERTEX_SHADER);
    u32 ps = gl_compile_shader_internal(pixel, GL_FRAGMENT_SHADER);
    glAttachShader(program, vs);
    glAttachShader(program, ps);

    // Link the Shader to form a program
    glLinkProgram(program);

    u32 linked;
    glGetProgramiv(program, GL_LINK_STATUS, (GLint*)&linked);

#if NEKO_GL_CUSTOM_DEBUG_CHECKS
    if (!linked) {
        neko_log_warning("Shaders failed to link.");
        char out[2000];
        GLsizei outLen;
        glGetProgramInfoLog(program, 2000, &outLen, out);
        neko_log_warning("%s", out);
        neko_assert(0);
    }
#endif

    glDetachShader(program, vs);
    glDetachShader(program, ps);
    glDeleteShader(vs);
    glDeleteShader(ps);

    // Insert Shader into the Shaders array for future lookups
    s->program = program;

    // Query Uniform information and fill out the Shader Uniforms
    GLint uniform_count;
    u32 nameSize = sizeof(char) * NEKO_GL_CUSTOM_UNIFORM_NAME_LENGTH;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniform_count);
    neko_assert(uniform_count < NEKO_GL_CUSTOM_UNIFORM_MAX_COUNT);
    s->uniform_count = uniform_count;

    for (u32 i = 0; i < (u32)uniform_count; ++i) {
        u32 nameLength;
        neko_graphics_custom_batch_uniform_t u;

        glGetActiveUniform(program, (GLint)i, nameSize, (GLsizei*)&nameLength, (GLsizei*)&u.size, (GLenum*)&u.type, u.name);

        // Uniform named in a Shader is too long for the UNIFORM_NAME_LENGTH constant
        neko_assert(nameLength <= NEKO_GL_CUSTOM_UNIFORM_NAME_LENGTH);

        u.location = glGetUniformLocation(program, u.name);
        u.type = gl_get_gl_type_internal(u.type);
        u.hash = gl_FNV1a(u.name);
        u.id = i;

        // @TODO: Perhaps need to handle appended [0] to Uniform names?

        s->uniforms[i] = u;
    }

#if NEKO_GL_CUSTOM_DEBUG_CHECKS
    // prevent hash collisions
    for (u32 i = 0; i < (u32)uniform_count; ++i)
        for (u32 j = i + 1; j < (u32)uniform_count; ++j) neko_assert(s->uniforms[i].hash != s->uniforms[j].hash);
#endif
}

void neko_graphics_custom_batch_free_shader(neko_graphics_custom_batch_shader_t* s) {
    glDeleteProgram(s->program);
    memset(s, 0, sizeof(neko_graphics_custom_batch_shader_t));
}

neko_graphics_custom_batch_uniform_t* gl_find_uniform_internal(neko_graphics_custom_batch_shader_t* s, const char* name) {
    u32 uniform_count = s->uniform_count;
    neko_graphics_custom_batch_uniform_t* uniforms = s->uniforms;
    u64 hash = gl_FNV1a(name);

    for (u32 i = 0; i < uniform_count; ++i) {
        neko_graphics_custom_batch_uniform_t* u = uniforms + i;

        if (u->hash == hash) {
            return u;
        }
    }

    return 0;
}

void neko_graphics_custom_batch_set_active_shader(neko_graphics_custom_batch_shader_t* s) { glUseProgram(s->program); }

void neko_graphics_custom_batch_deactivate_shader() { glUseProgram(0); }

void neko_graphics_custom_batch_send_f32(neko_graphics_custom_batch_shader_t* s, const char* uniform_name, u32 size, float* floats, u32 count) {
    neko_graphics_custom_batch_uniform_t* u = gl_find_uniform_internal(s, uniform_name);

    if (!u) {
        neko_log_warning("Unable to find uniform: %s", uniform_name);
        return;
    }

    neko_assert(size == u->size);
    neko_assert(u->type == NEKO_GL_CUSTOM_FLOAT);

    neko_graphics_custom_batch_set_active_shader(s);
    switch (count) {
        case 1:
            glUniform1f(u->location, floats[0]);
            break;

        case 2:
            glUniform2f(u->location, floats[0], floats[1]);
            break;

        case 3:
            glUniform3f(u->location, floats[0], floats[1], floats[2]);
            break;

        case 4:
            glUniform4f(u->location, floats[0], floats[1], floats[2], floats[3]);
            break;

        default:
            neko_assert(0);
            break;
    }
    neko_graphics_custom_batch_deactivate_shader();
}

void neko_graphics_custom_batch_send_matrix(neko_graphics_custom_batch_shader_t* s, const char* uniform_name, float* floats) {
    neko_graphics_custom_batch_uniform_t* u = gl_find_uniform_internal(s, uniform_name);

    if (!u) {
        neko_log_warning("Unable to find uniform: %s", uniform_name);
        return;
    }

    neko_assert(u->size == 1);
    neko_assert(u->type == NEKO_GL_CUSTOM_FLOAT);

    neko_graphics_custom_batch_set_active_shader(s);
    glUniformMatrix4fv(u->id, 1, 0, floats);
    neko_graphics_custom_batch_deactivate_shader();
}

void neko_graphics_custom_batch_send_texture(neko_graphics_custom_batch_shader_t* s, const char* uniform_name, u32 index) {
    neko_graphics_custom_batch_uniform_t* u = gl_find_uniform_internal(s, uniform_name);

    if (!u) {
        neko_log_warning("Unable to find uniform: %s", uniform_name);
        return;
    }

    neko_assert(u->type == NEKO_GL_CUSTOM_SAMPLER);

    neko_graphics_custom_batch_set_active_shader(s);
    glUniform1i(u->location, index);
    neko_graphics_custom_batch_deactivate_shader();
}

static u32 gl_call_sort_pred_internal(neko_graphics_custom_batch_draw_call_t* a, neko_graphics_custom_batch_draw_call_t* b) { return a->r->state.u.key < b->r->state.u.key; }

static void neko_custom_batch_qsort_internal(neko_graphics_custom_batch_draw_call_t* items, u32 count) {
    if (count <= 1) return;

    neko_graphics_custom_batch_draw_call_t pivot = items[count - 1];
    u32 low = 0;
    for (u32 i = 0; i < count - 1; ++i) {
        if (gl_call_sort_pred_internal(items + i, &pivot)) {
            neko_graphics_custom_batch_draw_call_t tmp = items[i];
            items[i] = items[low];
            items[low] = tmp;
            low++;
        }
    }

    items[count - 1] = items[low];
    items[low] = pivot;
    neko_custom_batch_qsort_internal(items, low);
    neko_custom_batch_qsort_internal(items + low + 1, count - 1 - low);
}

void neko_graphics_custom_batch_push_draw_call(void* ctx, neko_graphics_custom_batch_draw_call_t call) {
    neko_graphics_custom_batch_context_t* context = (neko_graphics_custom_batch_context_t*)ctx;
    neko_assert(context->count < context->max_draw_calls);
    context->calls[context->count++] = call;
}

u32 gl_get_enum(u32 type) {
    switch (type) {
        case NEKO_GL_CUSTOM_FLOAT:
            return GL_FLOAT;
            break;

        case NEKO_GL_CUSTOM_INT:
            return GL_UNSIGNED_BYTE;
            break;

        default:
            neko_assert(0);
            return (u32)~0;
    }
}

void gl_do_map_internal(neko_graphics_custom_batch_draw_call_t* call, neko_graphics_custom_batch_renderable_t* render) {
    u32 count = call->vert_count;
    void* driver_memory = gl_map_internal(render, count);
    memcpy(driver_memory, call->verts, render->data.vertex_stride * count);
    gl_unmap_internal();
}

static void neko_custom_batch_render_internal(neko_graphics_custom_batch_draw_call_t* call) {
    neko_graphics_custom_batch_renderable_t* render = call->r;
    u32 texture_count = call->texture_count;
    u32* textures = call->textures;

    if (render->data.usage == GL_STATIC_DRAW) {
        if (render->need_new_sync) {
            render->need_new_sync = 0;
            gl_do_map_internal(call, render);
        }
    } else
        gl_do_map_internal(call, render);

    neko_graphics_custom_batch_vertex_data_t* data = &render->data;
    neko_graphics_custom_batch_vertex_attribute_t* attributes = data->attributes;
    u32 vertexStride = data->vertex_stride;
    u32 attributeCount = data->attribute_count;

    neko_graphics_custom_batch_set_active_shader(render->program);

    u32 bufferNumber = render->buffer_number;
    u32 buffer = render->buffers[bufferNumber];
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    for (u32 i = 0; i < attributeCount; ++i) {
        neko_graphics_custom_batch_vertex_attribute_t* attribute = attributes + i;

        u32 location = attribute->location;
        u32 size = attribute->size;
        u32 type = gl_get_enum(attribute->type);
        u32 offset = attribute->offset;

        glEnableVertexAttribArray(location);
        glVertexAttribPointer(location, size, type, GL_FALSE, vertexStride, (void*)((size_t)offset));
    }

    for (u32 i = 0; i < texture_count; ++i) {
        u32 gl_id = textures[i];

        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, gl_id);
    }

    u32 streamOffset = render->index0;
    u32 streamSize = render->index1 - streamOffset;
    glDrawArrays(data->primitive, streamOffset, streamSize);

    if (render->need_new_sync) {
        // @TODO: This shouldn't be called for static buffers, only needed for streaming.
        render->fences[bufferNumber] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        render->need_new_sync = 0;
    }

    for (u32 i = 0; i < attributeCount; ++i) {
        neko_graphics_custom_batch_vertex_attribute_t* attribute = attributes + i;

        u32 location = attribute->location;
        glDisableVertexAttribArray(location);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}

void neko_custom_batch_present_internal(void* context, neko_graphics_custom_batch_framebuffer_t* fb, int w, int h) {
    neko_graphics_custom_batch_context_t* ctx = (neko_graphics_custom_batch_context_t*)context;
    neko_custom_batch_qsort_internal(ctx->calls, ctx->count);

    if (fb) {
        glBindFramebuffer(GL_FRAMEBUFFER, fb->fb_id);
        glViewport(0, 0, fb->w, fb->h);
    }
    // if (ctx->clear_bits) glClear(ctx->clear_bits);
    // if (ctx->settings_bits) glEnable(ctx->settings_bits);

    // flush all draw calls to the GPU
    for (u32 i = 0; i < ctx->count; ++i) {
        neko_graphics_custom_batch_draw_call_t* call = ctx->calls + i;
        neko_custom_batch_render_internal(call);
    }

    if (fb) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, w, h);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        neko_graphics_custom_batch_set_active_shader(fb->shader);
        glBindBuffer(GL_ARRAY_BUFFER, fb->quad_id);
        glBindTexture(GL_TEXTURE_2D, fb->tex_id);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, NEKO_GL_CUSTOM_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, NEKO_GL_CUSTOM_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        neko_graphics_custom_batch_deactivate_shader();
    }
}

void neko_graphics_custom_batch_flush(void* ctx, neko_graphics_custom_batch_framebuffer_t* fb, int w, int h) {
    neko_custom_batch_present_internal(ctx, fb, w, h);
    neko_graphics_custom_batch_context_t* context = (neko_graphics_custom_batch_context_t*)ctx;
    context->count = 0;
}

int neko_graphics_custom_batch_draw_call_count(void* ctx) {
    neko_graphics_custom_batch_context_t* context = (neko_graphics_custom_batch_context_t*)ctx;
    return context->count;
}

#include <math.h>

void neko_graphics_custom_batch_perspective(float* m, float y_fov_radians, float aspect, float n, float f) {
    float a = 1.0f / (float)tanf(y_fov_radians / 2.0f);

    m[0] = a / aspect;
    m[1] = 0;
    m[2] = 0;
    m[3] = 0;

    m[4] = 0;
    m[5] = a;
    m[6] = 0;
    m[7] = 0;

    m[8] = 0;
    m[9] = 0;
    m[10] = -((f + n) / (f - n));
    m[11] = -1.0f;

    m[12] = 0;
    m[13] = 0;
    m[14] = -((2.0f * f * n) / (f - n));
    m[15] = 0;
}

void neko_graphics_custom_batch_ortho_2d(float w, float h, float x, float y, float* m) {
    float left = -w / 2;
    float right = w / 2;
    float top = h / 2;
    float bottom = -h / 2;
    float far_ = 1000.0f;
    float near_ = -1000.0f;

    memset(m, 0, sizeof(float) * 4 * 4);

    // ortho
    m[0] = 2.0f / (right - left);
    m[5] = 2.0f / (top - bottom);
    m[10] = -2.0f / (far_ - near_);
    m[15] = 1.0f;

    // translate
    m[12] = -x;
    m[13] = -y;
}

void neko_graphics_custom_batch_copy(float* dst, float* src) {
    for (int i = 0; i < 16; ++i) dst[i] = src[i];
}

void neko_graphics_custom_batch_mul(float* a, float* b, float* out) {
    float c[16];

    c[0 + 0 * 4] = a[0 + 0 * 4] * b[0 + 0 * 4] + a[0 + 1 * 4] * b[1 + 0 * 4] + a[0 + 2 * 4] * b[2 + 0 * 4] + a[0 + 3 * 4] * b[3 + 0 * 4];
    c[0 + 1 * 4] = a[0 + 0 * 4] * b[0 + 1 * 4] + a[0 + 1 * 4] * b[1 + 1 * 4] + a[0 + 2 * 4] * b[2 + 1 * 4] + a[0 + 3 * 4] * b[3 + 1 * 4];
    c[0 + 2 * 4] = a[0 + 0 * 4] * b[0 + 2 * 4] + a[0 + 1 * 4] * b[1 + 2 * 4] + a[0 + 2 * 4] * b[2 + 2 * 4] + a[0 + 3 * 4] * b[3 + 2 * 4];
    c[0 + 3 * 4] = a[0 + 0 * 4] * b[0 + 3 * 4] + a[0 + 1 * 4] * b[1 + 3 * 4] + a[0 + 2 * 4] * b[2 + 3 * 4] + a[0 + 3 * 4] * b[3 + 3 * 4];
    c[1 + 0 * 4] = a[1 + 0 * 4] * b[0 + 0 * 4] + a[1 + 1 * 4] * b[1 + 0 * 4] + a[1 + 2 * 4] * b[2 + 0 * 4] + a[1 + 3 * 4] * b[3 + 0 * 4];
    c[1 + 1 * 4] = a[1 + 0 * 4] * b[0 + 1 * 4] + a[1 + 1 * 4] * b[1 + 1 * 4] + a[1 + 2 * 4] * b[2 + 1 * 4] + a[1 + 3 * 4] * b[3 + 1 * 4];
    c[1 + 2 * 4] = a[1 + 0 * 4] * b[0 + 2 * 4] + a[1 + 1 * 4] * b[1 + 2 * 4] + a[1 + 2 * 4] * b[2 + 2 * 4] + a[1 + 3 * 4] * b[3 + 2 * 4];
    c[1 + 3 * 4] = a[1 + 0 * 4] * b[0 + 3 * 4] + a[1 + 1 * 4] * b[1 + 3 * 4] + a[1 + 2 * 4] * b[2 + 3 * 4] + a[1 + 3 * 4] * b[3 + 3 * 4];
    c[2 + 0 * 4] = a[2 + 0 * 4] * b[0 + 0 * 4] + a[2 + 1 * 4] * b[1 + 0 * 4] + a[2 + 2 * 4] * b[2 + 0 * 4] + a[2 + 3 * 4] * b[3 + 0 * 4];
    c[2 + 1 * 4] = a[2 + 0 * 4] * b[0 + 1 * 4] + a[2 + 1 * 4] * b[1 + 1 * 4] + a[2 + 2 * 4] * b[2 + 1 * 4] + a[2 + 3 * 4] * b[3 + 1 * 4];
    c[2 + 2 * 4] = a[2 + 0 * 4] * b[0 + 2 * 4] + a[2 + 1 * 4] * b[1 + 2 * 4] + a[2 + 2 * 4] * b[2 + 2 * 4] + a[2 + 3 * 4] * b[3 + 2 * 4];
    c[2 + 3 * 4] = a[2 + 0 * 4] * b[0 + 3 * 4] + a[2 + 1 * 4] * b[1 + 3 * 4] + a[2 + 2 * 4] * b[2 + 3 * 4] + a[2 + 3 * 4] * b[3 + 3 * 4];
    c[3 + 0 * 4] = a[3 + 0 * 4] * b[0 + 0 * 4] + a[3 + 1 * 4] * b[1 + 0 * 4] + a[3 + 2 * 4] * b[2 + 0 * 4] + a[3 + 3 * 4] * b[3 + 0 * 4];
    c[3 + 1 * 4] = a[3 + 0 * 4] * b[0 + 1 * 4] + a[3 + 1 * 4] * b[1 + 1 * 4] + a[3 + 2 * 4] * b[2 + 1 * 4] + a[3 + 3 * 4] * b[3 + 1 * 4];
    c[3 + 2 * 4] = a[3 + 0 * 4] * b[0 + 2 * 4] + a[3 + 1 * 4] * b[1 + 2 * 4] + a[3 + 2 * 4] * b[2 + 2 * 4] + a[3 + 3 * 4] * b[3 + 2 * 4];
    c[3 + 3 * 4] = a[3 + 0 * 4] * b[0 + 3 * 4] + a[3 + 1 * 4] * b[1 + 3 * 4] + a[3 + 2 * 4] * b[2 + 3 * 4] + a[3 + 3 * 4] * b[3 + 3 * 4];

    neko_graphics_custom_batch_copy(out, c);
}

void gl_mulv(float* a, float* b) {
    float result[4];

    result[0] = a[0] * b[0] + a[4] * b[1] + a[8] * b[2] + a[12] * b[3];
    result[1] = a[1] * b[0] + a[5] * b[1] + a[9] * b[2] + a[13] * b[3];
    result[2] = a[2] * b[0] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3];
    result[3] = a[3] * b[0] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3];

    b[0] = result[0];
    b[1] = result[1];
    b[2] = result[2];
    b[3] = result[3];
}

void neko_graphics_custom_batch_identity(float* m) {
    memset(m, 0, sizeof(float) * 16);
    m[0] = 1.0f;
    m[5] = 1.0f;
    m[10] = 1.0f;
    m[15] = 1.0f;
}

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
    neko_graphics_blend_state_desc_t blend;
    neko_graphics_depth_state_desc_t depth;
    neko_graphics_raster_state_desc_t raster;
    neko_graphics_stencil_state_desc_t stencil;
    neko_graphics_compute_state_desc_t compute;
    neko_dyn_array(neko_graphics_vertex_attribute_desc_t) layout;
} neko_gl_pipeline_t;

/* Render Pass */
typedef struct neko_gl_renderpass_t {
    neko_handle(neko_graphics_framebuffer_t) fbo;
    neko_dyn_array(neko_handle(neko_graphics_texture_t)) color;
    neko_handle(neko_graphics_texture_t) depth;
    neko_handle(neko_graphics_texture_t) stencil;
} neko_gl_renderpass_t;

/* Shader */
typedef u32 neko_gl_shader_t;

/* Gfx Buffer */
typedef u32 neko_gl_buffer_t;

/* Texture */
typedef struct neko_gl_texture_t {
    u32 id;
    neko_graphics_texture_desc_t desc;
} neko_gl_texture_t;

typedef struct neko_gl_vertex_buffer_decl_t {
    neko_gl_buffer_t vbo;
    neko_graphics_vertex_data_type data_type;
    size_t offset;
} neko_gl_vertex_buffer_decl_t;

/* Cached data between draws */
typedef struct neko_gl_data_cache_t {
    neko_gl_buffer_t vao;
    neko_gl_buffer_t ibo;
    size_t ibo_elem_sz;
    neko_dyn_array(neko_gl_vertex_buffer_decl_t) vdecls;
    neko_handle(neko_graphics_pipeline_t) pipeline;
} neko_gl_data_cache_t;

/* Internal Opengl Data */
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

void neko_gl_reset_data_cache(neko_gl_data_cache_t* cache) {
    cache->ibo = 0;
    cache->ibo_elem_sz = 0;
    cache->pipeline = neko_handle_invalid(neko_graphics_pipeline_t);
    neko_dyn_array_clear(cache->vdecls);
}

void neko_gl_pipeline_state() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    CHECK_GL_CORE(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);)
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);
    CHECK_GL_CORE(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS););
    glDisable(GL_MULTISAMPLE);

    CHECK_GL_CORE(neko_graphics_info_t* info = neko_graphics_info(); if (info->compute.available) {
        // neko_invoke_once(neko_log_trace("Compute shader available: %s", neko_bool_str(info->compute.available)););
        glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    });
}

/* Utilities */
s32 neko_gl_buffer_usage_to_gl_enum(neko_graphics_buffer_usage_type type) {
    s32 mode = GL_STATIC_DRAW;
    switch (type) {
        default:
        case NEKO_GRAPHICS_BUFFER_USAGE_STATIC:
            mode = GL_STATIC_DRAW;
            break;
        case NEKO_GRAPHICS_BUFFER_USAGE_STREAM:
            mode = GL_STREAM_DRAW;
            break;
        case NEKO_GRAPHICS_BUFFER_USAGE_DYNAMIC:
            mode = GL_DYNAMIC_DRAW;
            break;
    }
    return mode;
}

u32 neko_gl_access_type_to_gl_access_type(neko_graphics_access_type type) {
    CHECK_GL_CORE(u32 access = GL_WRITE_ONLY; switch (type) {
        case NEKO_GRAPHICS_ACCESS_WRITE_ONLY:
            access = GL_WRITE_ONLY;
            break;
        case NEKO_GRAPHICS_ACCESS_READ_ONLY:
            access = GL_READ_ONLY;
            break;
        case NEKO_GRAPHICS_ACCESS_READ_WRITE:
            access = GL_READ_WRITE;
            break;
        default:
            break;
    } return access;)
    return 0;
}

u32 neko_gl_texture_wrap_to_gl_texture_wrap(neko_graphics_texture_wrapping_type type) {
    u32 wrap = GL_REPEAT;
    switch (type) {
        default:
        case NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT:
            wrap = GL_REPEAT;
            break;
        case NEKO_GRAPHICS_TEXTURE_WRAP_MIRRORED_REPEAT:
            wrap = GL_MIRRORED_REPEAT;
            break;
        case NEKO_GRAPHICS_TEXTURE_WRAP_CLAMP_TO_EDGE:
            wrap = GL_CLAMP_TO_EDGE;
            break;
            CHECK_GL_CORE(case NEKO_GRAPHICS_TEXTURE_WRAP_CLAMP_TO_BORDER : wrap = GL_CLAMP_TO_BORDER; break;)
    };

    return wrap;
}

u32 neko_gl_texture_format_to_gl_data_type(neko_graphics_texture_format_type type) {
    u32 format = GL_UNSIGNED_BYTE;
    switch (type) {
        default:
        case NEKO_GRAPHICS_TEXTURE_FORMAT_A8:
            format = GL_UNSIGNED_BYTE;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_R8:
            format = GL_UNSIGNED_BYTE;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_R32:
            format = GL_UNSIGNED_INT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGB8:
            format = GL_UNSIGNED_BYTE;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8:
            format = GL_UNSIGNED_BYTE;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA16F:
            format = GL_FLOAT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA32F:
            format = GL_FLOAT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_R32F:
            format = GL_FLOAT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH8:
            format = GL_FLOAT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH16:
            format = GL_FLOAT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24:
            format = GL_FLOAT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F:
            format = GL_FLOAT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24_STENCIL8:
            format = GL_UNSIGNED_INT_24_8;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
            format = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
            break;
    }
    return format;
}

u32 neko_gl_texture_format_to_gl_texture_format(neko_graphics_texture_format_type type) {
    u32 dt = GL_RGBA;
    switch (type) {
        default:
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8:
            dt = GL_RGBA;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_A8:
            dt = GL_ALPHA;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_R8:
            dt = GL_RED;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_R32:
            dt = GL_RED_INTEGER;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_R32F:
            dt = GL_RED;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGB8:
            dt = GL_RGB;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA16F:
            dt = GL_RGBA;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA32F:
            dt = GL_RGBA;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH8:
            dt = GL_DEPTH_COMPONENT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH16:
            dt = GL_DEPTH_COMPONENT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24:
            dt = GL_DEPTH_COMPONENT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F:
            dt = GL_DEPTH_COMPONENT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24_STENCIL8:
            dt = GL_DEPTH_STENCIL;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
            dt = GL_DEPTH_STENCIL;
            break;
    }
    return dt;
}

u32 neko_gl_texture_format_to_gl_texture_internal_format(neko_graphics_texture_format_type type) {
    u32 format = GL_UNSIGNED_BYTE;
    switch (type) {
        case NEKO_GRAPHICS_TEXTURE_FORMAT_A8:
            format = GL_ALPHA;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_R8:
            format = GL_RED;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_R32:
            format = GL_R32UI;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_R32F:
            format = GL_R32F;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGB8:
            format = GL_RGB8;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8:
            format = GL_RGBA8;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA16F:
            format = GL_RGBA16F;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA32F:
            format = GL_RGBA32F;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH8:
            format = GL_DEPTH_COMPONENT;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH16:
            format = GL_DEPTH_COMPONENT16;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24:
            format = GL_DEPTH_COMPONENT24;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F:
            format = GL_DEPTH_COMPONENT32F;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24_STENCIL8:
            format = GL_DEPTH24_STENCIL8;
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
            format = GL_DEPTH32F_STENCIL8;
            break;
    }
    return format;
}

u32 neko_gl_shader_stage_to_gl_stage(neko_graphics_shader_stage_type type) {
    u32 stage = GL_VERTEX_SHADER;
    switch (type) {
        default:
        case NEKO_GRAPHICS_SHADER_STAGE_VERTEX:
            stage = GL_VERTEX_SHADER;
            break;
        case NEKO_GRAPHICS_SHADER_STAGE_FRAGMENT:
            stage = GL_FRAGMENT_SHADER;
            break;
            CHECK_GL_CORE(case NEKO_GRAPHICS_SHADER_STAGE_COMPUTE : stage = GL_COMPUTE_SHADER; break;)
    }
    return stage;
}

u32 neko_gl_primitive_to_gl_primitive(neko_graphics_primitive_type type) {
    u32 prim = GL_TRIANGLES;
    switch (type) {
        default:
        case NEKO_GRAPHICS_PRIMITIVE_TRIANGLES:
            prim = GL_TRIANGLES;
            break;
        case NEKO_GRAPHICS_PRIMITIVE_LINES:
            prim = GL_LINES;
            break;
            CHECK_GL_CORE(case NEKO_GRAPHICS_PRIMITIVE_QUADS : prim = GL_QUADS; break;)
    }
    return prim;
}

u32 neko_gl_blend_equation_to_gl_blend_eq(neko_graphics_blend_equation_type eq) {
    u32 beq = GL_FUNC_ADD;
    switch (eq) {
        default:
        case NEKO_GRAPHICS_BLEND_EQUATION_ADD:
            beq = GL_FUNC_ADD;
            break;
        case NEKO_GRAPHICS_BLEND_EQUATION_SUBTRACT:
            beq = GL_FUNC_SUBTRACT;
            break;
        case NEKO_GRAPHICS_BLEND_EQUATION_REVERSE_SUBTRACT:
            beq = GL_FUNC_REVERSE_SUBTRACT;
            break;
        case NEKO_GRAPHICS_BLEND_EQUATION_MIN:
            beq = GL_MIN;
            break;
        case NEKO_GRAPHICS_BLEND_EQUATION_MAX:
            beq = GL_MAX;
            break;
    };

    return beq;
}

u32 neko_gl_blend_mode_to_gl_blend_mode(neko_graphics_blend_mode_type type, u32 def) {
    u32 mode = def;
    switch (type) {
        case NEKO_GRAPHICS_BLEND_MODE_ZERO:
            mode = GL_ZERO;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_ONE:
            mode = GL_ONE;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_SRC_COLOR:
            mode = GL_SRC_COLOR;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_COLOR:
            mode = GL_ONE_MINUS_SRC_COLOR;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_DST_COLOR:
            mode = GL_DST_COLOR;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_DST_COLOR:
            mode = GL_ONE_MINUS_DST_COLOR;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_SRC_ALPHA:
            mode = GL_SRC_ALPHA;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA:
            mode = GL_ONE_MINUS_SRC_ALPHA;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_DST_ALPHA:
            mode = GL_DST_ALPHA;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_DST_ALPHA:
            mode = GL_ONE_MINUS_DST_ALPHA;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_CONSTANT_COLOR:
            mode = GL_CONSTANT_COLOR;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_CONSTANT_COLOR:
            mode = GL_ONE_MINUS_CONSTANT_COLOR;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_CONSTANT_ALPHA:
            mode = GL_CONSTANT_ALPHA;
            break;
        case NEKO_GRAPHICS_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA:
            mode = GL_ONE_MINUS_CONSTANT_ALPHA;
            break;
    }
    return mode;
}

u32 neko_gl_cull_face_to_gl_cull_face(neko_graphics_face_culling_type type) {
    u32 fc = GL_BACK;
    switch (type) {
        default:
        case NEKO_GRAPHICS_FACE_CULLING_BACK:
            fc = GL_BACK;
            break;
        case NEKO_GRAPHICS_FACE_CULLING_FRONT:
            fc = GL_FRONT;
            break;
        case NEKO_GRAPHICS_FACE_CULLING_FRONT_AND_BACK:
            fc = GL_FRONT_AND_BACK;
            break;
    }
    return fc;
}

u32 neko_gl_winding_order_to_gl_winding_order(neko_graphics_winding_order_type type) {
    u32 wo = GL_CCW;
    switch (type) {
        case NEKO_GRAPHICS_WINDING_ORDER_CCW:
            wo = GL_CCW;
            break;
        case NEKO_GRAPHICS_WINDING_ORDER_CW:
            wo = GL_CW;
            break;
    }
    return wo;
}

u32 neko_gl_depth_func_to_gl_depth_func(neko_graphics_depth_func_type type) {
    u32 func = GL_LESS;
    switch (type) {
        default:
        case NEKO_GRAPHICS_DEPTH_FUNC_LESS:
            func = GL_LESS;
            break;
        case NEKO_GRAPHICS_DEPTH_FUNC_NEVER:
            func = GL_NEVER;
            break;
        case NEKO_GRAPHICS_DEPTH_FUNC_EQUAL:
            func = GL_EQUAL;
            break;
        case NEKO_GRAPHICS_DEPTH_FUNC_LEQUAL:
            func = GL_LEQUAL;
            break;
        case NEKO_GRAPHICS_DEPTH_FUNC_GREATER:
            func = GL_GREATER;
            break;
        case NEKO_GRAPHICS_DEPTH_FUNC_NOTEQUAL:
            func = GL_NOTEQUAL;
            break;
        case NEKO_GRAPHICS_DEPTH_FUNC_GEQUAL:
            func = GL_GEQUAL;
            break;
        case NEKO_GRAPHICS_DEPTH_FUNC_ALWAYS:
            func = GL_ALWAYS;
            break;
    }
    return func;
}

bool neko_gl_depth_mask_to_gl_mask(neko_graphics_depth_mask_type type) {
    bool ret = true;
    switch (type) {
        default:
        case NEKO_GRAPHICS_DEPTH_MASK_ENABLED:
            ret = true;
            break;
        case NEKO_GRAPHICS_DEPTH_MASK_DISABLED:
            ret = false;
            break;
    }
    return ret;
}

u32 neko_gl_stencil_func_to_gl_stencil_func(neko_graphics_stencil_func_type type) {
    u32 func = GL_ALWAYS;
    switch (type) {
        default:
        case NEKO_GRAPHICS_STENCIL_FUNC_LESS:
            func = GL_LESS;
            break;
        case NEKO_GRAPHICS_STENCIL_FUNC_NEVER:
            func = GL_NEVER;
            break;
        case NEKO_GRAPHICS_STENCIL_FUNC_EQUAL:
            func = GL_EQUAL;
            break;
        case NEKO_GRAPHICS_STENCIL_FUNC_LEQUAL:
            func = GL_LEQUAL;
            break;
        case NEKO_GRAPHICS_STENCIL_FUNC_GREATER:
            func = GL_GREATER;
            break;
        case NEKO_GRAPHICS_STENCIL_FUNC_NOTEQUAL:
            func = GL_NOTEQUAL;
            break;
        case NEKO_GRAPHICS_STENCIL_FUNC_GEQUAL:
            func = GL_GEQUAL;
            break;
        case NEKO_GRAPHICS_STENCIL_FUNC_ALWAYS:
            func = GL_ALWAYS;
            break;
    }
    return func;
}

u32 neko_gl_stencil_op_to_gl_stencil_op(neko_graphics_stencil_op_type type) {
    u32 op = GL_KEEP;
    switch (type) {
        default:
        case NEKO_GRAPHICS_STENCIL_OP_KEEP:
            op = GL_KEEP;
            break;
        case NEKO_GRAPHICS_STENCIL_OP_ZERO:
            op = GL_ZERO;
            break;
        case NEKO_GRAPHICS_STENCIL_OP_REPLACE:
            op = GL_REPLACE;
            break;
        case NEKO_GRAPHICS_STENCIL_OP_INCR:
            op = GL_INCR;
            break;
        case NEKO_GRAPHICS_STENCIL_OP_INCR_WRAP:
            op = GL_INCR_WRAP;
            break;
        case NEKO_GRAPHICS_STENCIL_OP_DECR:
            op = GL_DECR;
            break;
        case NEKO_GRAPHICS_STENCIL_OP_DECR_WRAP:
            op = GL_DECR_WRAP;
            break;
        case NEKO_GRAPHICS_STENCIL_OP_INVERT:
            op = GL_INVERT;
            break;
    }
    return op;
}

neko_gl_uniform_type neko_gl_uniform_type_to_gl_uniform_type(neko_graphics_uniform_type gstype) {
    neko_gl_uniform_type type = NEKO_GL_UNIFORMTYPE_FLOAT;
    switch (gstype) {
        default:
        case NEKO_GRAPHICS_UNIFORM_FLOAT:
            type = NEKO_GL_UNIFORMTYPE_FLOAT;
            break;
        case NEKO_GRAPHICS_UNIFORM_INT:
            type = NEKO_GL_UNIFORMTYPE_INT;
            break;
        case NEKO_GRAPHICS_UNIFORM_VEC2:
            type = NEKO_GL_UNIFORMTYPE_VEC2;
            break;
        case NEKO_GRAPHICS_UNIFORM_VEC3:
            type = NEKO_GL_UNIFORMTYPE_VEC3;
            break;
        case NEKO_GRAPHICS_UNIFORM_VEC4:
            type = NEKO_GL_UNIFORMTYPE_VEC4;
            break;
        case NEKO_GRAPHICS_UNIFORM_MAT4:
            type = NEKO_GL_UNIFORMTYPE_MAT4;
            break;
        case NEKO_GRAPHICS_UNIFORM_USAMPLER2D:
            type = NEKO_GL_UNIFORMTYPE_SAMPLER2D;
            break;
        case NEKO_GRAPHICS_UNIFORM_SAMPLER2D:
            type = NEKO_GL_UNIFORMTYPE_SAMPLER2D;
            break;
        case NEKO_GRAPHICS_UNIFORM_SAMPLERCUBE:
            type = NEKO_GL_UNIFORMTYPE_SAMPLERCUBE;
            break;
    }
    return type;
}

u32 neko_gl_index_buffer_size_to_gl_index_type(size_t sz) {
    u32 type = GL_UNSIGNED_INT;
    switch (sz) {
        default:
        case 4:
            type = GL_UNSIGNED_INT;
            break;
        case 2:
            type = GL_UNSIGNED_SHORT;
            break;
        case 1:
            type = GL_UNSIGNED_BYTE;
            break;
    }
    return type;
}

size_t neko_gl_get_byte_size_of_vertex_attribute(neko_graphics_vertex_attribute_type type) {
    size_t byte_size = 0;
    switch (type) {
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT4: {
            byte_size = sizeof(f32) * 4;
        } break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3: {
            byte_size = sizeof(f32) * 3;
        } break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2: {
            byte_size = sizeof(f32) * 2;
        } break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT: {
            byte_size = sizeof(f32) * 1;
        } break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT4: {
            byte_size = sizeof(u32) * 4;
        } break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT3: {
            byte_size = sizeof(u32) * 3;
        } break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT2: {
            byte_size = sizeof(u32) * 2;
        } break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT: {
            byte_size = sizeof(u32) * 1;
        } break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4: {
            byte_size = sizeof(uint8_t) * 4;
        } break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE3: {
            byte_size = sizeof(uint8_t) * 3;
        } break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE2: {
            byte_size = sizeof(uint8_t) * 2;
        } break;
        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE: {
            byte_size = sizeof(uint8_t) * 1;
        } break;
    }

    return byte_size;
}

size_t neko_gl_calculate_vertex_size_in_bytes(neko_graphics_vertex_attribute_desc_t* layout, u32 count) {
    // Iterate through all formats in delcarations and calculate total size
    size_t sz = 0;
    for (u32 i = 0; i < count; ++i) {
        neko_graphics_vertex_attribute_type type = layout[i].format;
        sz += neko_gl_get_byte_size_of_vertex_attribute(type);
    }

    return sz;
}

size_t neko_gl_get_vertex_attr_byte_offest(neko_dyn_array(neko_graphics_vertex_attribute_desc_t) layout, u32 idx) {
    // Recursively calculate offset
    size_t total_offset = 0;

    // Base case
    if (idx == 0) {
        return total_offset;
    }

    // Calculate total offset up to this point
    for (u32 i = 0; i < idx; ++i) {
        total_offset += neko_gl_get_byte_size_of_vertex_attribute(layout[i].format);
    }

    return total_offset;
}

size_t neko_gl_uniform_data_size_in_bytes(neko_graphics_uniform_type type) {
    size_t sz = 0;
    switch (type) {
        case NEKO_GRAPHICS_UNIFORM_FLOAT:
            sz = sizeof(float);
            break;
        case NEKO_GRAPHICS_UNIFORM_INT:
            sz = sizeof(s32);
            break;
        case NEKO_GRAPHICS_UNIFORM_VEC2:
            sz = 2 * sizeof(float);
            break;
        case NEKO_GRAPHICS_UNIFORM_VEC3:
            sz = 3 * sizeof(float);
            break;
        case NEKO_GRAPHICS_UNIFORM_VEC4:
            sz = 4 * sizeof(float);
            break;
        case NEKO_GRAPHICS_UNIFORM_MAT4:
            sz = 16 * sizeof(float);
            break;
        case NEKO_GRAPHICS_UNIFORM_SAMPLER2D:
            sz = sizeof(neko_handle(neko_graphics_texture_t));
            break;  // handle size
        case NEKO_GRAPHICS_UNIFORM_USAMPLER2D:
            sz = sizeof(neko_handle(neko_graphics_texture_t));
            break;  // handle size
        case NEKO_GRAPHICS_UNIFORM_SAMPLERCUBE:
            sz = sizeof(neko_handle(neko_graphics_texture_t));
            break;  // handle size
        default: {
            sz = 0;
        } break;
    }
    return sz;
}

thread_mutex_t g_graphics_mutex;

/* Graphics Interface Creation / Initialization / Shutdown / Destruction */
NEKO_API_DECL neko_graphics_t* neko_graphics_create() {
    // Construct new graphics interface
    neko_graphics_t* gfx = neko_malloc_init(neko_graphics_t);

    // Construct internal data for opengl
    gfx->user_data = neko_malloc_init(neko_gl_data_t);

    thread_mutex_init(&g_graphics_mutex);

    return gfx;
}

NEKO_API_DECL void neko_graphics_destroy(neko_graphics_t* graphics) {
    // Free all resources (assuming they've been freed from the GPU already)
    if (graphics == NULL) return;

    neko_gl_data_t* ogl = (neko_gl_data_t*)graphics->user_data;

#define OGL_FREE_DATA(SA, T, FUNC)                                                                                    \
    do {                                                                                                              \
        for (neko_slot_array_iter it = 1; neko_slot_array_iter_valid(SA, it); neko_slot_array_iter_advance(SA, it)) { \
            neko_handle(T) hndl = neko_default_val();                                                                 \
            hndl.id = it;                                                                                             \
            FUNC(hndl);                                                                                               \
        }                                                                                                             \
    } while (0)

    // Free all gl data
    if (ogl->pipelines) OGL_FREE_DATA(ogl->pipelines, neko_graphics_pipeline_t, neko_graphics_pipeline_destroy);
    if (ogl->shaders) OGL_FREE_DATA(ogl->shaders, neko_graphics_shader_t, neko_graphics_shader_destroy);
    if (ogl->vertex_buffers) OGL_FREE_DATA(ogl->vertex_buffers, neko_graphics_vertex_buffer_t, neko_graphics_vertex_buffer_destroy);
    if (ogl->index_buffers) OGL_FREE_DATA(ogl->index_buffers, neko_graphics_index_buffer_t, neko_graphics_index_buffer_destroy);
    if (ogl->renderpasses) OGL_FREE_DATA(ogl->renderpasses, neko_graphics_renderpass_t, neko_graphics_renderpass_destroy);
    if (ogl->frame_buffers) OGL_FREE_DATA(ogl->frame_buffers, neko_graphics_framebuffer_t, neko_graphics_framebuffer_destroy);
    if (ogl->textures) OGL_FREE_DATA(ogl->textures, neko_graphics_texture_t, neko_graphics_texture_destroy);
    if (ogl->uniforms) OGL_FREE_DATA(ogl->uniforms, neko_graphics_uniform_t, neko_graphics_uniform_destroy);
    if (ogl->uniform_buffers) OGL_FREE_DATA(ogl->uniform_buffers, neko_graphics_uniform_buffer_t, neko_graphics_uniform_buffer_destroy);
    // if (ogl->storage_buffers)   OGL_FREE_DATA(ogl->storage_buffers, neko_graphics_storage_buffer_t, neko_graphics_storage_buffer_destroy);

    neko_slot_array_free(ogl->shaders);
    neko_slot_array_free(ogl->vertex_buffers);
    neko_slot_array_free(ogl->index_buffers);
    neko_slot_array_free(ogl->frame_buffers);
    neko_slot_array_free(ogl->uniforms);
    neko_slot_array_free(ogl->textures);
    neko_slot_array_free(ogl->pipelines);
    neko_slot_array_free(ogl->renderpasses);
    neko_slot_array_free(ogl->uniform_buffers);
    neko_slot_array_free(ogl->storage_buffers);

    // Free uniform data array
    neko_dyn_array_free(ogl->uniform_data.mat4);
    neko_dyn_array_free(ogl->uniform_data.vec4);
    neko_dyn_array_free(ogl->uniform_data.vec3);
    neko_dyn_array_free(ogl->uniform_data.vec2);
    neko_dyn_array_free(ogl->uniform_data.flt);
    neko_dyn_array_free(ogl->uniform_data.i32);
    neko_dyn_array_free(ogl->uniform_data.ui32);

    // Free data cache
    neko_dyn_array_free(ogl->cache.vdecls);

    neko_free(graphics);
    graphics = NULL;

    thread_mutex_term(&g_graphics_mutex);
}

NEKO_API_DECL void neko_graphics_shutdown(neko_graphics_t* graphics) {}

neko_gl_texture_t gl_texture_update_internal(const neko_graphics_texture_desc_t* desc, u32 hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;

    neko_gl_texture_t tex = neko_default_val();
    if (hndl) tex = neko_slot_array_get(ogl->textures, hndl);
    u32 width = desc->width;
    u32 height = desc->height;
    void* data = desc->data[0];

    if (!hndl) {
        glGenTextures(1, &tex.id);
    }

    GLenum target = 0x00;
    switch (desc->type) {
        default:
        case NEKO_GRAPHICS_TEXTURE_2D: {
            target = GL_TEXTURE_2D;
        } break;
        case NEKO_GRAPHICS_TEXTURE_CUBEMAP: {
            target = GL_TEXTURE_CUBE_MAP;
        } break;
    }

    glBindTexture(target, tex.id);

    u32 cnt = NEKO_GRAPHICS_TEXTURE_DATA_MAX;
    switch (desc->type) {
        case NEKO_GRAPHICS_TEXTURE_2D:
            cnt = 1;
            break;
    }

    for (u32 i = 0; i < cnt; ++i) {
        GLenum itarget = 0x00;
        data = desc->data[i];
        switch (desc->type) {
            default:
            case NEKO_GRAPHICS_TEXTURE_2D: {
                itarget = GL_TEXTURE_2D;
            } break;
            case NEKO_GRAPHICS_TEXTURE_CUBEMAP: {
                itarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
            } break;
        }

        if (tex.desc.width * tex.desc.height < width * height) {
            // Construct texture based on appropriate format
            switch (desc->format) {
                case NEKO_GRAPHICS_TEXTURE_FORMAT_A8:
                    glTexImage2D(itarget, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_R8:
                    glTexImage2D(itarget, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_R32:
                    glTexImage2D(itarget, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_RG8:
                    glTexImage2D(itarget, 0, GL_RG8, width, height, 0, GL_RG, GL_UNSIGNED_BYTE, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_RGB8:
                    glTexImage2D(itarget, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8:
                    glTexImage2D(itarget, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_R32F:
                    glTexImage2D(itarget, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA16F:
                    glTexImage2D(itarget, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA32F:
                    glTexImage2D(itarget, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH8:
                    glTexImage2D(itarget, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH16:
                    glTexImage2D(itarget, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24:
                    glTexImage2D(itarget, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F:
                    glTexImage2D(itarget, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24_STENCIL8:
                    glTexImage2D(itarget, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
                    glTexImage2D(itarget, 0, GL_DEPTH32F_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, data);
                    break;

                // NOTE: 因为 Apple 是一家垃圾公司 所以必须将其分开并仅提供对 4.1 功能的支持。
                // case NEKO_GRAPHICS_TEXTURE_FORMAT_STENCIL8:            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
                default:
                    break;
            }
        } else {
            // Subimage
            switch (desc->format) {
                case NEKO_GRAPHICS_TEXTURE_FORMAT_A8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_ALPHA, GL_UNSIGNED_BYTE, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_R8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RED, GL_UNSIGNED_BYTE, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_R32:
                    glTexImage2D(itarget, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_RG8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RG, GL_UNSIGNED_BYTE, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_RGB8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_R32F:
                    glTexImage2D(itarget, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA16F:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RGBA, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA32F:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RGBA, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH16:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24_STENCIL8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, data);
                    break;
                case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, data);
                    break;

                // NOTE: Because Apple is a shit company, I have to section this off and provide support for 4.1 only features.
                // case NEKO_GRAPHICS_TEXTURE_FORMAT_STENCIL8:            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
                default:
                    break;
            }
        }
    }

    s32 mag_filter = desc->mag_filter == NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST ? GL_NEAREST : GL_LINEAR;
    s32 min_filter = desc->min_filter == NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST ? GL_NEAREST : GL_LINEAR;

    if (desc->num_mips) {
        if (desc->min_filter == NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST) {
            min_filter = desc->mip_filter == NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_LINEAR;
        } else {
            min_filter = desc->mip_filter == NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_LINEAR;
        }
    }

    const u32 texture_wrap_s = neko_gl_texture_wrap_to_gl_texture_wrap(desc->wrap_s);
    const u32 texture_wrap_t = neko_gl_texture_wrap_to_gl_texture_wrap(desc->wrap_t);
    const u32 texture_wrap_r = neko_gl_texture_wrap_to_gl_texture_wrap(desc->wrap_r);

    if (desc->num_mips) {
        glGenerateMipmap(target);
    }

    // Need to make sure this is available before being able to use

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

/* Resource Creation */
NEKO_API_DECL neko_handle(neko_graphics_texture_t) neko_graphics_texture_create_impl(const neko_graphics_texture_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_gl_texture_t tex = gl_texture_update_internal(desc, 0);
    // Add texture to internal resource pool and return handle
    return (neko_handle_create(neko_graphics_texture_t, neko_slot_array_insert(ogl->textures, tex)));
}

NEKO_API_DECL neko_handle(neko_graphics_uniform_t) neko_graphics_uniform_create_impl(const neko_graphics_uniform_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;

    // Assert if data isn't named
    if (desc->name == NULL) {
        neko_log_warning("Uniform must be named for OpenGL.");
        return neko_handle_invalid(neko_graphics_uniform_t);
    }

    u32 ct = !desc->layout ? 0 : !desc->layout_size ? 1 : (u32)desc->layout_size / (u32)sizeof(neko_graphics_uniform_layout_desc_t);
    if (ct < 1) {
        neko_log_warning("Uniform layout description must not be empty for: %s.", desc->name);
        return neko_handle_invalid(neko_graphics_uniform_t);
    }

    // Construct list for uniform handles
    neko_gl_uniform_list_t ul = neko_default_val();
    memcpy(ul.name, desc->name, 64);

    // Iterate layout, construct individual handles
    for (u32 i = 0; i < ct; ++i) {
        // Uniform to fill out
        neko_gl_uniform_t u = neko_default_val();
        // Cache layout
        neko_graphics_uniform_layout_desc_t* layout = &desc->layout[i];

        memcpy(u.name, layout->fname, 64);
        u.type = neko_gl_uniform_type_to_gl_uniform_type(layout->type);
        u.size = neko_gl_uniform_data_size_in_bytes(layout->type);
        u.count = layout->count ? layout->count : 1;
        u.location = UINT32_MAX;

        // Add to size of ul
        ul.size += u.size * u.count;

        // Push uniform into list
        neko_dyn_array_push(ul.uniforms, u);
    }

    return neko_handle_create(neko_graphics_uniform_t, neko_slot_array_insert(ogl->uniforms, ul));
}

NEKO_API_DECL neko_handle(neko_graphics_vertex_buffer_t) neko_graphics_vertex_buffer_create_impl(const neko_graphics_vertex_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_handle(neko_graphics_vertex_buffer_t) hndl = neko_default_val();
    neko_gl_buffer_t buffer = 0;

    // Assert if data isn't filled for vertex data when static draw enabled
    if (desc->usage == NEKO_GRAPHICS_BUFFER_USAGE_STATIC && !desc->data) {
        neko_println("Error: Vertex buffer desc must contain data when NEKO_GRAPHICS_BUFFER_USAGE_STATIC set.");
        neko_assert(false);
    }

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, desc->size, desc->data, neko_gl_buffer_usage_to_gl_enum(desc->usage));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    hndl = neko_handle_create(neko_graphics_vertex_buffer_t, neko_slot_array_insert(ogl->vertex_buffers, buffer));

    return hndl;
}

NEKO_API_DECL neko_handle(neko_graphics_index_buffer_t) neko_graphics_index_buffer_create_impl(const neko_graphics_index_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_handle(neko_graphics_index_buffer_t) hndl = neko_default_val();
    neko_gl_buffer_t buffer = 0;

    // Assert if data isn't filled for vertex data when static draw enabled
    if (desc->usage == NEKO_GRAPHICS_BUFFER_USAGE_STATIC && !desc->data) {
        neko_println("Error: Index buffer desc must contain data when NEKO_GRAPHICS_BUFFER_USAGE_STATIC set.");
        neko_assert(false);
    }

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, desc->size, desc->data, neko_gl_buffer_usage_to_gl_enum(desc->usage));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    hndl = neko_handle_create(neko_graphics_index_buffer_t, neko_slot_array_insert(ogl->index_buffers, buffer));

    return hndl;
}

NEKO_API_DECL neko_handle(neko_graphics_uniform_buffer_t) neko_graphics_uniform_buffer_create_impl(const neko_graphics_uniform_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_handle(neko_graphics_uniform_buffer_t) hndl = neko_default_val();

    // Assert if data isn't named
    if (desc->name == NULL) {
        neko_log_warning("Uniform buffer must be named for Opengl.");
    }

    neko_gl_uniform_buffer_t u = neko_default_val();
    memcpy(u.name, desc->name, 64);
    u.size = desc->size;
    u.location = UINT32_MAX;

    // Generate buffer (if needed)
    glGenBuffers(1, &u.ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, u.ubo);
    glBufferData(GL_UNIFORM_BUFFER, u.size, 0, neko_gl_buffer_usage_to_gl_enum(desc->usage));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    hndl = neko_handle_create(neko_graphics_uniform_buffer_t, neko_slot_array_insert(ogl->uniform_buffers, u));

    return hndl;
}

NEKO_API_DECL neko_handle(neko_graphics_storage_buffer_t) neko_graphics_storage_buffer_create_impl(const neko_graphics_storage_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_handle(neko_graphics_storage_buffer_t) hndl = neko_default_val();
    neko_gl_storage_buffer_t sbo = neko_default_val();

    if (desc->name == NULL) {
        neko_log_warning("Storage buffer must be named for Opengl.");
    }

    if (desc->usage == NEKO_GRAPHICS_BUFFER_USAGE_STATIC && !desc->data) {
        neko_println("Error: Storage buffer desc must contain data when NEKO_GRAPHICS_BUFFER_USAGE_STATIC set.");
        neko_assert(false);
    }

    glGenBuffers(1, &sbo.buffer);

    CHECK_GL_CORE(glBindBuffer(GL_SHADER_STORAGE_BUFFER, sbo.buffer); glBufferData(GL_SHADER_STORAGE_BUFFER, desc->size, desc->data, neko_gl_buffer_usage_to_gl_enum(desc->usage));
                  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0););

    memcpy(sbo.name, desc->name, 64);
    sbo.access = desc->access;
    sbo.size = desc->size;
    sbo.block_idx = UINT32_MAX;

    hndl = neko_handle_create(neko_graphics_storage_buffer_t, neko_slot_array_insert(ogl->storage_buffers, sbo));

    return hndl;
}

NEKO_API_DECL neko_handle(neko_graphics_framebuffer_t) neko_graphics_framebuffer_create_impl(const neko_graphics_framebuffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_handle(neko_graphics_framebuffer_t) hndl = neko_default_val();
    neko_gl_buffer_t buffer = 0;
    glGenFramebuffers(1, &buffer);
    hndl = neko_handle_create(neko_graphics_framebuffer_t, neko_slot_array_insert(ogl->frame_buffers, buffer));
    return hndl;
}

#define NEKO_GL_GRAPHICS_SHADER_PIPELINE_GFX 0x01
#define NEKO_GL_GRAPHICS_SHADER_PIPELINE_COMPUTE 0x02
#define NEKO_GL_GRAPHICS_MAX_SID 128

NEKO_API_DECL neko_handle(neko_graphics_shader_t) neko_graphics_shader_create_impl(const neko_graphics_shader_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_gl_shader_t shader = 0;
    u32 pip = 0x00;

    u32 sid_ct = 0;
    u32 sids[NEKO_GL_GRAPHICS_MAX_SID] = neko_default_val();

    // Create shader program
    shader = glCreateProgram();

    u32 ct = (u32)desc->size / (u32)sizeof(neko_graphics_shader_source_desc_t);
    for (u32 i = 0; i < ct; ++i) {
        if (desc->sources[i].type == NEKO_GRAPHICS_SHADER_STAGE_VERTEX) pip |= NEKO_GL_GRAPHICS_SHADER_PIPELINE_GFX;
        if (desc->sources[i].type == NEKO_GRAPHICS_SHADER_STAGE_COMPUTE) pip |= NEKO_GL_GRAPHICS_SHADER_PIPELINE_COMPUTE;

        // Validity Check: Desc must have vertex source if compute not selected. All other source is optional.
        if ((pip & NEKO_GL_GRAPHICS_SHADER_PIPELINE_COMPUTE) && ((pip & NEKO_GL_GRAPHICS_SHADER_PIPELINE_GFX))) {
            neko_println("Error: Cannot have compute and graphics stages for shader program.");
            neko_assert(false);
        }

        u32 stage = neko_gl_shader_stage_to_gl_stage(desc->sources[i].type);
        u32 sid = glCreateShader(stage);

        if (!sid) {
            neko_println("Error: Failed to allocate memory for shader: '%s': stage: {put stage id here}", desc->name);
            neko_assert(sid);
        }

        // Set source
        glShaderSource(sid, 1, &desc->sources[i].source, NULL);

        // Compile shader
        glCompileShader(sid);

        // Check for errors
        GLint success = 0;
        glGetShaderiv(sid, GL_COMPILE_STATUS, &success);

        if (success == GL_FALSE) {
            GLint max_len = 0;
            glGetShaderiv(sid, GL_INFO_LOG_LENGTH, &max_len);

            char* log = (char*)neko_safe_malloc(max_len);
            memset(log, 0, max_len);

            // The max_len includes the NULL character
            glGetShaderInfoLog(sid, max_len, &max_len, log);

            // Delete shader.
            glDeleteShader(shader);

            // Provide the infolog
            neko_println("Opengl::opengl_compile_shader::shader: '%s'\nFAILED_TO_COMPILE: %s\n %s", desc->name, log, desc->sources[i].source);

            neko_safe_free(log);
            log = NULL;

            // neko_assert(false);
            return neko_handle_invalid(neko_graphics_shader_t);
        }

        // Attach shader to program
        glAttachShader(shader, sid);

        // Add to shader array
        sids[sid_ct++] = sid;
    }

    // Link shaders into final program
    glLinkProgram(shader);

    // Create info log for errors
    s32 is_good = 0;
    glGetProgramiv(shader, GL_LINK_STATUS, (s32*)&is_good);
    if (is_good == GL_FALSE) {
        GLint max_len = 0;
        glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &max_len);

        char* log = (char*)neko_safe_malloc(max_len);
        memset(log, 0, max_len);
        glGetProgramInfoLog(shader, max_len, &max_len, log);

        // Print error
        neko_log_error("Fail To Link::opengl_link_shaders::shader: '%s', \n%s", desc->name, log);

        // //We don't need the program anymore.
        glDeleteProgram(shader);

        neko_safe_free(log);
        log = NULL;

        // Just assert for now
        neko_assert(false);
    }

    glValidateProgram(shader);
    glGetProgramiv(shader, GL_VALIDATE_STATUS, &is_good);
    if (!is_good) {
        neko_log_error("Failed to validate shader: '%s'", desc->name);
    }

    // Free shaders after use
    for (u32 i = 0; i < sid_ct; ++i) {
        glDeleteShader(sids[i]);
    }

    // Iterate over uniforms
    /*
    {
        char tmp_name[256] = neko_default_val();
        s32 count = 0;
        glGetProgramiv(shader, GL_ACTIVE_UNIFORMS, &count);
        neko_println("Active Uniforms: %d\n", count);

        for (u32 i = 0; i < count; i++) {
            s32 sz = 0;
            u32 type;
            glGetActiveUniform(shader, (GLuint)i, 256, NULL, &sz, &type, tmp_name);
            neko_println("Uniform #%d Type: %u the_name: %s\n", i, type, tmp_name);
        }
    }
    */

    // Add to pool and return handle
    return (neko_handle_create(neko_graphics_shader_t, neko_slot_array_insert(ogl->shaders, shader)));
}

NEKO_API_DECL neko_handle(neko_graphics_renderpass_t) neko_graphics_renderpass_create_impl(const neko_graphics_renderpass_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;

    neko_gl_renderpass_t pass = neko_default_val();

    // Set fbo
    pass.fbo = desc->fbo;

    // Set color attachments
    u32 ct = (u32)desc->color_size / (u32)sizeof(neko_handle(neko_graphics_texture_t));
    for (u32 i = 0; i < ct; ++i) {
        neko_dyn_array_push(pass.color, desc->color[i]);
    }
    // Set depth attachment
    pass.depth = desc->depth;

    // Create handle and return
    return (neko_handle_create(neko_graphics_renderpass_t, neko_slot_array_insert(ogl->renderpasses, pass)));
}

NEKO_API_DECL neko_handle(neko_graphics_pipeline_t) neko_graphics_pipeline_create_impl(const neko_graphics_pipeline_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;

    neko_gl_pipeline_t pipe = neko_default_val();

    // Add states
    pipe.blend = desc->blend;
    pipe.depth = desc->depth;
    pipe.raster = desc->raster;
    pipe.stencil = desc->stencil;
    pipe.compute = desc->compute;

    // Add layout
    u32 ct = (u32)desc->layout.size / (u32)sizeof(neko_graphics_vertex_attribute_desc_t);
    neko_dyn_array_reserve(pipe.layout, ct);
    for (u32 i = 0; i < ct; ++i) {
        neko_dyn_array_push(pipe.layout, desc->layout.attrs[i]);
    }

    // Create handle and return
    return (neko_handle_create(neko_graphics_pipeline_t, neko_slot_array_insert(ogl->pipelines, pipe)));
}

// Resource Destruction
NEKO_API_DECL void neko_graphics_texture_destroy_impl(neko_handle(neko_graphics_texture_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!neko_slot_array_handle_valid(ogl->textures, hndl.id)) return;
    neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, hndl.id);
    glDeleteTextures(1, &tex->id);
    neko_slot_array_erase(ogl->textures, hndl.id);
}

NEKO_API_DECL void neko_graphics_uniform_destroy_impl(neko_handle(neko_graphics_uniform_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!neko_slot_array_handle_valid(ogl->uniforms, hndl.id)) return;
    neko_gl_uniform_list_t* ul = neko_slot_array_getp(ogl->uniforms, hndl.id);
    neko_dyn_array_free(ul->uniforms);
    neko_slot_array_erase(ogl->uniforms, hndl.id);
}

NEKO_API_DECL void neko_graphics_shader_destroy_impl(neko_handle(neko_graphics_shader_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!neko_slot_array_handle_valid(ogl->shaders, hndl.id)) return;
    glDeleteProgram(neko_slot_array_get(ogl->shaders, hndl.id));
    neko_slot_array_erase(ogl->shaders, hndl.id);
}

NEKO_API_DECL void neko_graphics_vertex_buffer_destroy_impl(neko_handle(neko_graphics_vertex_buffer_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!neko_slot_array_handle_valid(ogl->vertex_buffers, hndl.id)) return;
    neko_gl_buffer_t buffer = neko_slot_array_get(ogl->vertex_buffers, hndl.id);
    glDeleteBuffers(1, &buffer);
    neko_slot_array_erase(ogl->vertex_buffers, hndl.id);
}

NEKO_API_DECL void neko_graphics_index_buffer_destroy_impl(neko_handle(neko_graphics_index_buffer_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!neko_slot_array_handle_valid(ogl->index_buffers, hndl.id)) return;
    neko_gl_buffer_t buffer = neko_slot_array_get(ogl->index_buffers, hndl.id);
    glDeleteBuffers(1, &buffer);
    neko_slot_array_erase(ogl->index_buffers, hndl.id);
}

NEKO_API_DECL void neko_graphics_uniform_buffer_destroy_impl(neko_handle(neko_graphics_uniform_buffer_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!neko_slot_array_handle_valid(ogl->uniform_buffers, hndl.id)) return;
    neko_gl_uniform_buffer_t* u = neko_slot_array_getp(ogl->uniform_buffers, hndl.id);

    // Delete buffer (if needed)
    glDeleteBuffers(1, &u->ubo);

    // Delete from slot array
    neko_slot_array_erase(ogl->uniform_buffers, hndl.id);
}

NEKO_API_DECL void neko_graphics_storage_buffer_destroy_impl(neko_handle(neko_graphics_storage_buffer_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!neko_slot_array_handle_valid(ogl->storage_buffers, hndl.id)) return;
    neko_gl_storage_buffer_t* sb = neko_slot_array_getp(ogl->storage_buffers, hndl.id);

    // Delete buffer (if needed)
    glDeleteBuffers(1, &sb->buffer);

    // Delete from slot array
    neko_slot_array_erase(ogl->storage_buffers, hndl.id);
}

NEKO_API_DECL void neko_graphics_framebuffer_destroy_impl(neko_handle(neko_graphics_framebuffer_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!neko_slot_array_handle_valid(ogl->frame_buffers, hndl.id)) return;
    neko_gl_buffer_t buffer = neko_slot_array_get(ogl->frame_buffers, hndl.id);
    glDeleteFramebuffers(1, &buffer);
    neko_slot_array_erase(ogl->frame_buffers, hndl.id);
}

NEKO_API_DECL void neko_graphics_renderpass_destroy_impl(neko_handle(neko_graphics_renderpass_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!neko_slot_array_handle_valid(ogl->renderpasses, hndl.id)) return;
    // TODO: erase all color attachments from renderpasss
    neko_slot_array_erase(ogl->renderpasses, hndl.id);
}

NEKO_API_DECL void neko_graphics_pipeline_destroy_impl(neko_handle(neko_graphics_pipeline_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!neko_slot_array_handle_valid(ogl->pipelines, hndl.id)) return;
    neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, hndl.id);

    // Free layout
    neko_dyn_array_free(pip->layout);

    // Erase handles from slot arrays
    neko_slot_array_erase(ogl->pipelines, hndl.id);
}

// Resource Query
NEKO_API_DECL void neko_graphics_pipeline_desc_query(neko_handle(neko_graphics_pipeline_t) hndl, neko_graphics_pipeline_desc_t* out) {
    if (!out) return;

    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, hndl.id);

    // Add states
    out->blend = pip->blend;
    out->depth = pip->depth;
    out->raster = pip->raster;
    out->stencil = pip->stencil;
    out->compute = pip->compute;

    // Add layout
    u32 ct = neko_dyn_array_size(pip->layout);
    for (u32 i = 0; i < ct; ++i) {
        neko_dyn_array_push(out->layout.attrs, pip->layout[i]);
    }
}

NEKO_API_DECL void neko_graphics_texture_desc_query(neko_handle(neko_graphics_texture_t) hndl, neko_graphics_texture_desc_t* out) {
    if (!out) return;

    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, hndl.id);

    // Read back pixels
    if (out->data && out->read.width && out->read.height) {
        u32 type = neko_gl_texture_format_to_gl_data_type(tex->desc.format);
        u32 format = neko_gl_texture_format_to_gl_texture_format(tex->desc.format);
        CHECK_GL_CORE(glActiveTexture(GL_TEXTURE0); glGetTextureSubImage(tex->id, 0, out->read.x, out->read.y, 0, out->read.width, out->read.height, 1, format, type, out->read.size, out->data););
    }

    *out = tex->desc;
}

NEKO_API_DECL size_t neko_graphics_uniform_size_query(neko_handle(neko_graphics_uniform_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_gl_uniform_list_t* u = neko_slot_array_getp(ogl->uniforms, hndl.id);
    return u->uniforms[0].size;
}

// Resource Updates (main thread only)
NEKO_API_DECL void neko_graphics_texture_update_impl(neko_handle(neko_graphics_texture_t) hndl, neko_graphics_texture_desc_t* desc) {
    if (!desc) return;

    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!neko_slot_array_handle_valid(ogl->textures, hndl.id)) {
        neko_log_warning("Texture handle invalid: %zu", hndl.id);
        return;
    }
    gl_texture_update_internal(desc, hndl.id);
}

NEKO_API_DECL void neko_graphics_vertex_buffer_update_impl(neko_handle(neko_graphics_vertex_buffer_t) hndl, neko_graphics_vertex_buffer_desc_t* desc) {
    /*
    void __neko_graphics_update_buffer_internal(neko_command_buffer_t* cb,
        u32 id,
        neko_graphics_buffer_type type,
        neko_graphics_buffer_usage_type usage,
        size_t sz,
        size_t offset,
        neko_graphics_buffer_update_type update_type,
        void* data)
    {
        // Write command
        neko_byte_buffer_write(&cb->commands, u32, (u32)NEKO_OPENGL_OP_REQUEST_BUFFER_UPDATE);
        cb->num_commands++;

        // Write handle id
        neko_byte_buffer_write(&cb->commands, u32, id);
        // Write type
        neko_byte_buffer_write(&cb->commands, neko_graphics_buffer_type, type);
        // Write usage
        neko_byte_buffer_write(&cb->commands, neko_graphics_buffer_usage_type, usage);
        // Write data size
        neko_byte_buffer_write(&cb->commands, size_t, sz);
        // Write data offset
        neko_byte_buffer_write(&cb->commands, size_t, offset);
        // Write data update type
        neko_byte_buffer_write(&cb->commands, neko_graphics_buffer_update_type, update_type);
        // Write data
        neko_byte_buffer_write_bulk(&cb->commands, data, sz);
    }
    __neko_graphics_update_buffer_internal(cb, hndl.id, NEKO_GRAPHICS_BUFFER_VERTEX, desc->usage, desc->size, desc->update.offset, desc->update.type, desc->data);
    */

    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_gl_buffer_t buffer = neko_slot_array_get(ogl->vertex_buffers, hndl.id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    s32 glusage = neko_gl_buffer_usage_to_gl_enum(desc->usage);
    switch (desc->update.type) {
        case NEKO_GRAPHICS_BUFFER_UPDATE_SUBDATA:
            glBufferSubData(GL_ARRAY_BUFFER, desc->update.offset, desc->size, desc->data);
            break;
        default:
            glBufferData(GL_ARRAY_BUFFER, desc->size, desc->data, glusage);
            break;
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

NEKO_API_DECL void neko_graphics_index_buffer_update_impl(neko_handle(neko_graphics_index_buffer_t) hndl, neko_graphics_index_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_gl_buffer_t buffer = neko_slot_array_get(ogl->index_buffers, hndl.id);
    s32 glusage = neko_gl_buffer_usage_to_gl_enum(desc->usage);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    switch (desc->update.type) {
        case NEKO_GRAPHICS_BUFFER_UPDATE_SUBDATA:
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, desc->update.offset, desc->size, desc->data);
            break;
        default:
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, desc->size, desc->data, glusage);
            break;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

NEKO_API_DECL void neko_graphics_storage_buffer_update_impl(neko_handle(neko_graphics_storage_buffer_t) hndl, neko_graphics_storage_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    neko_gl_storage_buffer_t* sbo = neko_slot_array_getp(ogl->storage_buffers, hndl.id);
    if (!sbo) {
        neko_log_warning("Storage buffer %zu not found.", hndl.id);
        return;
    }

    CHECK_GL_CORE(glBindBuffer(GL_SHADER_STORAGE_BUFFER, sbo->buffer); switch (desc->update.type) {
        case NEKO_GRAPHICS_BUFFER_UPDATE_SUBDATA:
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, desc->update.offset, desc->size, desc->data);
            break;
        default:
            glBufferData(GL_SHADER_STORAGE_BUFFER, desc->size, desc->data, neko_gl_buffer_usage_to_gl_enum(desc->usage));
    } glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0););
}

NEKO_API_DECL void neko_graphics_texture_read_impl(neko_handle(neko_graphics_texture_t) hndl, neko_graphics_texture_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
    if (!desc) return;
    if (!neko_slot_array_handle_valid(ogl->textures, hndl.id)) {
        neko_log_warning("Texture handle invalid: %zu", hndl.id);
    }

    neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, hndl.id);
    // Bind texture
    GLenum target = 0x00;
    switch (tex->desc.type) {
        default:
        case NEKO_GRAPHICS_TEXTURE_2D: {
            target = GL_TEXTURE_2D;
        } break;
        case NEKO_GRAPHICS_TEXTURE_CUBEMAP: {
            target = GL_TEXTURE_CUBE_MAP;
        } break;
    }
    u32 gl_format = neko_gl_texture_format_to_gl_texture_format(tex->desc.format);
    u32 gl_type = neko_gl_texture_format_to_gl_data_type(tex->desc.format);
    glBindTexture(target, tex->id);
    glReadPixels(desc->read.x, desc->read.y, desc->read.width, desc->read.height, gl_format, gl_type, *desc->data);
    glBindTexture(target, 0x00);
}

#define __ogl_push_command(CB, OP_CODE, ...)                                         \
    do {                                                                             \
        neko_gl_data_t* DATA = (neko_gl_data_t*)neko_subsystem(graphics)->user_data; \
        neko_byte_buffer_write(&CB->commands, u32, (u32)OP_CODE);                    \
        __VA_ARGS__                                                                  \
        CB->num_commands++;                                                          \
    } while (0)

/* Command Buffer Ops: Pipeline / Pass / Bind / Draw */
NEKO_API_DECL void neko_graphics_renderpass_begin(neko_command_buffer_t* cb, neko_handle(neko_graphics_renderpass_t) hndl) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_BEGIN_RENDER_PASS, { neko_byte_buffer_write(&cb->commands, u32, hndl.id); });
}

NEKO_API_DECL void neko_graphics_renderpass_end(neko_command_buffer_t* cb) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_END_RENDER_PASS,
                       {
                               // Nothing...
                       });
}

NEKO_API_DECL void neko_graphics_clear(neko_command_buffer_t* cb, neko_graphics_clear_desc_t* desc) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_CLEAR, {
        u32 count = !desc->actions ? 0 : !desc->size ? 1 : (u32)((size_t)desc->size / (size_t)sizeof(neko_graphics_clear_action_t));
        neko_byte_buffer_write(&cb->commands, u32, count);
        for (u32 i = 0; i < count; ++i) {
            neko_byte_buffer_write(&cb->commands, neko_graphics_clear_action_t, desc->actions[i]);
        }
    });
}

NEKO_API_DECL void neko_graphics_set_viewport(neko_command_buffer_t* cb, u32 x, u32 y, u32 w, u32 h) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_SET_VIEWPORT, {
        neko_byte_buffer_write(&cb->commands, u32, x);
        neko_byte_buffer_write(&cb->commands, u32, y);
        neko_byte_buffer_write(&cb->commands, u32, w);
        neko_byte_buffer_write(&cb->commands, u32, h);
    });
}

NEKO_API_DECL void neko_graphics_set_view_scissor(neko_command_buffer_t* cb, u32 x, u32 y, u32 w, u32 h) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_SET_VIEW_SCISSOR, {
        neko_byte_buffer_write(&cb->commands, u32, x);
        neko_byte_buffer_write(&cb->commands, u32, y);
        neko_byte_buffer_write(&cb->commands, u32, w);
        neko_byte_buffer_write(&cb->commands, u32, h);
    });
}

NEKO_API_DECL void neko_graphics_texture_request_update(neko_command_buffer_t* cb, neko_handle(neko_graphics_texture_t) hndl, neko_graphics_texture_desc_t* desc) {
    // Write command
    neko_byte_buffer_write(&cb->commands, u32, (u32)NEKO_OPENGL_OP_REQUEST_TEXTURE_UPDATE);
    cb->num_commands++;

    u32 num_comps = 0;
    size_t data_type_size = 0;
    size_t total_size = 0;
    switch (desc->format) {
        default:
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8:
            num_comps = 4;
            data_type_size = sizeof(uint8_t);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGB8:
            num_comps = 3;
            data_type_size = sizeof(uint8_t);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_A8:
            num_comps = 1;
            data_type_size = sizeof(uint8_t);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_R8:
            num_comps = 1;
            data_type_size = sizeof(uint8_t);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_R32:
            num_comps = 1;
            data_type_size = sizeof(u32);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA16F:
            num_comps = 4;
            data_type_size = sizeof(float);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA32F:
            num_comps = 4;
            data_type_size = sizeof(float);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH8:
            num_comps = 1;
            data_type_size = sizeof(float);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH16:
            num_comps = 1;
            data_type_size = sizeof(float);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24:
            num_comps = 1;
            data_type_size = sizeof(float);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F:
            num_comps = 1;
            data_type_size = sizeof(float);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH24_STENCIL8:
            num_comps = 1;
            data_type_size = sizeof(u32);
            break;
        case NEKO_GRAPHICS_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
            num_comps = 1;
            data_type_size = sizeof(float) + sizeof(uint8_t);
            break;

            // NOTE: Because Apple is a shit company, I have to section this off and provide support for 4.1 only features.
            // case NEKO_GRAPHICS_TEXTURE_FORMAT_STENCIL8:            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
    }
    total_size = desc->width * desc->height * num_comps * data_type_size;
    neko_byte_buffer_write(&cb->commands, u32, hndl.id);
    neko_byte_buffer_write(&cb->commands, neko_graphics_texture_desc_t, *desc);
    neko_byte_buffer_write(&cb->commands, size_t, total_size);
    neko_byte_buffer_write_bulk(&cb->commands, *desc->data, total_size);
}

void __neko_graphics_update_buffer_internal(neko_command_buffer_t* cb, u32 id, neko_graphics_buffer_type type, neko_graphics_buffer_usage_type usage, size_t sz, size_t offset,
                                            neko_graphics_buffer_update_type update_type, void* data) {
    // Write command
    neko_byte_buffer_write(&cb->commands, u32, (u32)NEKO_OPENGL_OP_REQUEST_BUFFER_UPDATE);
    cb->num_commands++;

    // Write handle id
    neko_byte_buffer_write(&cb->commands, u32, id);
    // Write type
    neko_byte_buffer_write(&cb->commands, neko_graphics_buffer_type, type);
    // Write usage
    neko_byte_buffer_write(&cb->commands, neko_graphics_buffer_usage_type, usage);
    // Write data size
    neko_byte_buffer_write(&cb->commands, size_t, sz);
    // Write data offset
    neko_byte_buffer_write(&cb->commands, size_t, offset);
    // Write data update type
    neko_byte_buffer_write(&cb->commands, neko_graphics_buffer_update_type, update_type);
    // Write data
    neko_byte_buffer_write_bulk(&cb->commands, data, sz);
}

NEKO_API_DECL void neko_graphics_vertex_buffer_request_update(neko_command_buffer_t* cb, neko_handle(neko_graphics_vertex_buffer_t) hndl, neko_graphics_vertex_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;

    // Return if handle not valid
    if (!hndl.id) return;

    __neko_graphics_update_buffer_internal(cb, hndl.id, NEKO_GRAPHICS_BUFFER_VERTEX, desc->usage, desc->size, desc->update.offset, desc->update.type, desc->data);
}

NEKO_API_DECL void neko_graphics_index_buffer_request_update(neko_command_buffer_t* cb, neko_handle(neko_graphics_index_buffer_t) hndl, neko_graphics_index_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;

    // Return if handle not valid
    if (!hndl.id) return;

    __neko_graphics_update_buffer_internal(cb, hndl.id, NEKO_GRAPHICS_BUFFER_INDEX, desc->usage, desc->size, desc->update.offset, desc->update.type, desc->data);
}

NEKO_API_DECL void neko_graphics_uniform_buffer_request_update(neko_command_buffer_t* cb, neko_handle(neko_graphics_uniform_buffer_t) hndl, neko_graphics_uniform_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;

    // Return if handle not valid
    if (!hndl.id) return;

    __neko_graphics_update_buffer_internal(cb, hndl.id, NEKO_GRAPHICS_BUFFER_UNIFORM, desc->usage, desc->size, desc->update.offset, desc->update.type, desc->data);
}

NEKO_API_DECL void neko_graphics_storage_buffer_request_update(neko_command_buffer_t* cb, neko_handle(neko_graphics_storage_buffer_t) hndl, neko_graphics_storage_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;

    // Return if handle not valid
    if (!hndl.id) return;

    __neko_graphics_update_buffer_internal(cb, hndl.id, NEKO_GRAPHICS_BUFFER_SHADER_STORAGE, desc->usage, desc->size, desc->update.offset, desc->update.type, desc->data);
}

void neko_graphics_apply_bindings(neko_command_buffer_t* cb, neko_graphics_bind_desc_t* binds) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;

    // Increment commands
    neko_byte_buffer_write(&cb->commands, u32, (u32)NEKO_OPENGL_OP_APPLY_BINDINGS);
    cb->num_commands++;

    // __ogl_push_command(cb, NEKO_OPENGL_OP_APPLY_BINDINGS,
    {
        // Get counts from buffers
        u32 vct = binds->vertex_buffers.desc ? binds->vertex_buffers.size ? binds->vertex_buffers.size / sizeof(neko_graphics_bind_vertex_buffer_desc_t) : 1 : 0;
        u32 ict = binds->index_buffers.desc ? binds->index_buffers.size ? binds->index_buffers.size / sizeof(neko_graphics_bind_index_buffer_desc_t) : 1 : 0;
        u32 uct = binds->uniform_buffers.desc ? binds->uniform_buffers.size ? binds->uniform_buffers.size / sizeof(neko_graphics_bind_uniform_buffer_desc_t) : 1 : 0;
        u32 pct = binds->uniforms.desc ? binds->uniforms.size ? binds->uniforms.size / sizeof(neko_graphics_bind_uniform_desc_t) : 1 : 0;
        u32 ibc = binds->image_buffers.desc ? binds->image_buffers.size ? binds->image_buffers.size / sizeof(neko_graphics_bind_image_buffer_desc_t) : 1 : 0;
        u32 sbc = binds->storage_buffers.desc ? binds->storage_buffers.size ? binds->storage_buffers.size / sizeof(neko_graphics_bind_storage_buffer_desc_t) : 1 : 0;

        // Determine total count to write into command buffer
        u32 ct = vct + ict + uct + pct + ibc + sbc;
        neko_byte_buffer_write(&cb->commands, u32, ct);

        // Determine if need to clear any previous vertex buffers (if vct != 0)
        neko_byte_buffer_write(&cb->commands, bool, (vct != 0));

        // Vertex buffers
        for (u32 i = 0; i < vct; ++i) {
            neko_graphics_bind_vertex_buffer_desc_t* decl = &binds->vertex_buffers.desc[i];
            neko_byte_buffer_write(&cb->commands, neko_graphics_bind_type, NEKO_GRAPHICS_BIND_VERTEX_BUFFER);
            neko_byte_buffer_write(&cb->commands, u32, decl->buffer.id);
            neko_byte_buffer_write(&cb->commands, size_t, decl->offset);
            neko_byte_buffer_write(&cb->commands, neko_graphics_vertex_data_type, decl->data_type);
        }

        // Index buffers
        for (u32 i = 0; i < ict; ++i) {
            neko_graphics_bind_index_buffer_desc_t* decl = &binds->index_buffers.desc[i];
            neko_byte_buffer_write(&cb->commands, neko_graphics_bind_type, NEKO_GRAPHICS_BIND_INDEX_BUFFER);
            neko_byte_buffer_write(&cb->commands, u32, decl->buffer.id);
        }

        // Uniform buffers
        for (u32 i = 0; i < uct; ++i) {
            neko_graphics_bind_uniform_buffer_desc_t* decl = &binds->uniform_buffers.desc[i];

            u32 id = decl->buffer.id;
            size_t sz = (size_t)(neko_slot_array_getp(ogl->uniform_buffers, id))->size;
            neko_byte_buffer_write(&cb->commands, neko_graphics_bind_type, NEKO_GRAPHICS_BIND_UNIFORM_BUFFER);
            neko_byte_buffer_write(&cb->commands, u32, decl->buffer.id);
            neko_byte_buffer_write(&cb->commands, u32, decl->binding);
            neko_byte_buffer_write(&cb->commands, size_t, decl->range.offset);
            neko_byte_buffer_write(&cb->commands, size_t, decl->range.size);
        }

        // Image buffers
        for (u32 i = 0; i < ibc; ++i) {
            neko_graphics_bind_image_buffer_desc_t* decl = &binds->image_buffers.desc[i];
            neko_byte_buffer_write(&cb->commands, neko_graphics_bind_type, NEKO_GRAPHICS_BIND_IMAGE_BUFFER);
            neko_byte_buffer_write(&cb->commands, u32, decl->tex.id);
            neko_byte_buffer_write(&cb->commands, u32, decl->binding);
            neko_byte_buffer_write(&cb->commands, neko_graphics_access_type, decl->access);
        }

        // Uniforms
        for (u32 i = 0; i < pct; ++i) {
            neko_graphics_bind_uniform_desc_t* decl = &binds->uniforms.desc[i];

            // Get size from uniform list
            size_t sz = neko_slot_array_getp(ogl->uniforms, decl->uniform.id)->size;
            neko_byte_buffer_write(&cb->commands, neko_graphics_bind_type, NEKO_GRAPHICS_BIND_UNIFORM);
            neko_byte_buffer_write(&cb->commands, u32, decl->uniform.id);
            neko_byte_buffer_write(&cb->commands, size_t, sz);
            neko_byte_buffer_write(&cb->commands, u32, decl->binding);
            neko_byte_buffer_write_bulk(&cb->commands, decl->data, sz);
        }

        // Storage buffers
        CHECK_GL_CORE(for (u32 i = 0; i < sbc; ++i) {
            neko_graphics_bind_storage_buffer_desc_t* decl = &binds->storage_buffers.desc[i];
            neko_byte_buffer_write(&cb->commands, neko_graphics_bind_type, NEKO_GRAPHICS_BIND_STORAGE_BUFFER);
            neko_byte_buffer_write(&cb->commands, u32, decl->buffer.id);
            neko_byte_buffer_write(&cb->commands, u32, decl->binding);
        });
    };
}

void neko_graphics_pipeline_bind(neko_command_buffer_t* cb, neko_handle(neko_graphics_pipeline_t) hndl) {
    // TODO: 不确定这将来是否安全 因为管道的数据位于主线程上 并且可能会在单独的线程上被篡改
    __ogl_push_command(cb, NEKO_OPENGL_OP_BIND_PIPELINE, { neko_byte_buffer_write(&cb->commands, u32, hndl.id); });
}

void neko_graphics_draw(neko_command_buffer_t* cb, neko_graphics_draw_desc_t* desc) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_DRAW, {
        neko_byte_buffer_write(&cb->commands, u32, desc->start);
        neko_byte_buffer_write(&cb->commands, u32, desc->count);
        neko_byte_buffer_write(&cb->commands, u32, desc->instances);
        neko_byte_buffer_write(&cb->commands, u32, desc->base_vertex);
        neko_byte_buffer_write(&cb->commands, u32, desc->range.start);
        neko_byte_buffer_write(&cb->commands, u32, desc->range.end);
    });
}

void neko_graphics_draw_batch(neko_command_buffer_t* cb, neko_graphics_custom_batch_context_t* ctx, neko_graphics_custom_batch_framebuffer_t* fb, s32 w, s32 h) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_DRAW_BATCH, {
        neko_byte_buffer_write(&cb->commands, neko_graphics_custom_batch_context_ptr, ctx);
        neko_byte_buffer_write(&cb->commands, neko_graphics_custom_batch_framebuffer_ptr, fb);
        neko_byte_buffer_write(&cb->commands, s32, w);
        neko_byte_buffer_write(&cb->commands, s32, h);
    });
}

void neko_graphics_dispatch_compute(neko_command_buffer_t* cb, u32 num_x_groups, u32 num_y_groups, u32 num_z_groups) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_DISPATCH_COMPUTE, {
        neko_byte_buffer_write(&cb->commands, u32, num_x_groups);
        neko_byte_buffer_write(&cb->commands, u32, num_y_groups);
        neko_byte_buffer_write(&cb->commands, u32, num_z_groups);
    });
}

/* Submission (Main Thread) */
void neko_graphics_command_buffer_submit_impl(neko_command_buffer_t* cb) {
    /*
        // Structure of command:
            - Op code
            - Data packet
    */

    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;

    // Set read position of buffer to beginning
    neko_byte_buffer_seek_to_beg(&cb->commands);

    // For each command in buffer
    neko_for_range(cb->num_commands) {
        // Read in op code of command
        neko_byte_buffer_readc(&cb->commands, neko_opengl_op_code_type, op_code);

        switch (op_code) {
            case NEKO_OPENGL_OP_BEGIN_RENDER_PASS: {
                // Bind render pass stuff
                neko_byte_buffer_readc(&cb->commands, u32, rpid);

                // If render pass exists, then we'll bind frame buffer and attachments
                if (rpid && neko_slot_array_exists(ogl->renderpasses, rpid)) {
                    neko_gl_renderpass_t* rp = neko_slot_array_getp(ogl->renderpasses, rpid);

                    // Bind frame buffer since it actually exists
                    if (rp->fbo.id && neko_slot_array_exists(ogl->frame_buffers, rp->fbo.id)) {
                        // Bind frame buffer
                        glBindFramebuffer(GL_FRAMEBUFFER, neko_slot_array_get(ogl->frame_buffers, rp->fbo.id));

                        // Bind color attachments
                        for (u32 r = 0; r < neko_dyn_array_size(rp->color); ++r) {
                            u32 cid = rp->color[r].id;
                            if (cid && neko_slot_array_exists(ogl->textures, cid)) {
                                neko_gl_texture_t* rt = neko_slot_array_getp(ogl->textures, cid);

                                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + r, GL_TEXTURE_2D, rt->id, 0);
                            }
                        }

                        // Bind depth attachment
                        {
                            u32 depth_id = rp->depth.id;
                            if (depth_id && neko_slot_array_exists(ogl->textures, depth_id)) {
                                neko_gl_texture_t* rt = neko_slot_array_getp(ogl->textures, depth_id);
                                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, rt->id, 0);
                            }
                        }
                    }
                }
            } break;

            case NEKO_OPENGL_OP_END_RENDER_PASS: {
                neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;
                neko_gl_reset_data_cache(&ogl->cache);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                CHECK_GL_CORE(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0););
                glDisable(GL_SCISSOR_TEST);
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_STENCIL_TEST);
                glDisable(GL_BLEND);
            } break;

            case NEKO_OPENGL_OP_CLEAR: {
                // Actions
                neko_byte_buffer_readc(&cb->commands, u32, action_count);
                for (u32 j = 0; j < action_count; ++j) {
                    neko_byte_buffer_readc(&cb->commands, neko_graphics_clear_action_t, action);

                    // No clear
                    if (action.flag & NEKO_GRAPHICS_CLEAR_NONE) {
                        continue;
                    }

                    u32 bit = 0x00;

                    if (action.flag & NEKO_GRAPHICS_CLEAR_COLOR || action.flag == 0x00) {
                        glClearColor(action.color[0], action.color[1], action.color[2], action.color[3]);
                        bit |= GL_COLOR_BUFFER_BIT;
                    }
                    if (action.flag & NEKO_GRAPHICS_CLEAR_DEPTH || action.flag == 0x00) {
                        bit |= GL_DEPTH_BUFFER_BIT;
                    }
                    if (action.flag & NEKO_GRAPHICS_CLEAR_STENCIL || action.flag == 0x00) {
                        bit |= GL_STENCIL_BUFFER_BIT;
                        glStencilMask(~0);
                    }

                    glClear(bit);
                }
            } break;

            case NEKO_OPENGL_OP_SET_VIEWPORT: {
                neko_byte_buffer_readc(&cb->commands, u32, x);
                neko_byte_buffer_readc(&cb->commands, u32, y);
                neko_byte_buffer_readc(&cb->commands, u32, w);
                neko_byte_buffer_readc(&cb->commands, u32, h);

                glViewport(x, y, w, h);
            } break;

            case NEKO_OPENGL_OP_SET_VIEW_SCISSOR: {
                neko_byte_buffer_readc(&cb->commands, u32, x);
                neko_byte_buffer_readc(&cb->commands, u32, y);
                neko_byte_buffer_readc(&cb->commands, u32, w);
                neko_byte_buffer_readc(&cb->commands, u32, h);

                glEnable(GL_SCISSOR_TEST);
                glScissor(x, y, w, h);
            } break;

            case NEKO_OPENGL_OP_APPLY_BINDINGS: {
                neko_byte_buffer_readc(&cb->commands, u32, ct);

                // Determine if need to clear any previous vertex buffers here
                neko_byte_buffer_readc(&cb->commands, bool, clear_vertex_buffers);

                // Clear previous vertex decls if necessary
                if (clear_vertex_buffers) {
                    neko_dyn_array_clear(ogl->cache.vdecls);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                }

                for (u32 i = 0; i < ct; ++i) {
                    neko_byte_buffer_readc(&cb->commands, neko_graphics_bind_type, type);
                    switch (type) {
                        case NEKO_GRAPHICS_BIND_VERTEX_BUFFER: {
                            neko_byte_buffer_readc(&cb->commands, u32, id);
                            neko_byte_buffer_readc(&cb->commands, size_t, offset);
                            neko_byte_buffer_readc(&cb->commands, neko_graphics_vertex_data_type, data_type);

                            if (!id || !neko_slot_array_exists(ogl->vertex_buffers, id)) {
                                neko_timed_action(1000, {
                                    neko_log_warning("Opengl:BindBindings:VertexBuffer %d does not exist.", id);
                                    continue;
                                });
                            }

                            // Grab vbo to bind
                            neko_gl_buffer_t vbo = neko_slot_array_get(ogl->vertex_buffers, id);

                            // If the data type is non-interleaved, then push size into vertex buffer decl
                            neko_gl_vertex_buffer_decl_t vbo_decl = neko_default_val();
                            vbo_decl.vbo = vbo;
                            vbo_decl.data_type = data_type;
                            vbo_decl.offset = offset;

                            // Cache vertex buffer for later use
                            neko_dyn_array_push(ogl->cache.vdecls, vbo_decl);

                        } break;

                        case NEKO_GRAPHICS_BIND_INDEX_BUFFER: {
                            neko_byte_buffer_readc(&cb->commands, u32, id);

                            if (!neko_slot_array_exists(ogl->index_buffers, id)) {
                                /*
                                neko_timed_action(1000, {
                                    neko_println("Warning:Opengl:BindBindings:IndexBuffer %d does not exist.", id);
                                });
                                */
                            } else {
                                neko_gl_buffer_t ibo = neko_slot_array_get(ogl->index_buffers, id);

                                // Store in cache
                                ogl->cache.ibo = id;
                            }
                        } break;

                        case NEKO_GRAPHICS_BIND_UNIFORM: {
                            // Get size from uniform list
                            neko_byte_buffer_readc(&cb->commands, u32, id);
                            // Read data size for uniform list
                            neko_byte_buffer_readc(&cb->commands, size_t, sz);
                            // Read binding from uniform list (could make this a binding list? not sure how to handle this)
                            neko_byte_buffer_readc(&cb->commands, u32, binding);

                            // Check buffer id. If invalid, then we can't operate, and instead just need to pass over the data.
                            if (!id || !neko_slot_array_exists(ogl->uniforms, id)) {
                                neko_timed_action(1000, { neko_log_warning("Bind Uniform:Uniform %d does not exist.", id); });
                                neko_byte_buffer_advance_position(&cb->commands, sz);
                                continue;
                            }

                            // Grab currently bound pipeline (TODO: assert if this isn't valid)
                            if (!ogl->cache.pipeline.id || !neko_slot_array_exists(ogl->pipelines, ogl->cache.pipeline.id)) {
                                neko_timed_action(1000, { neko_log_warning("Bind Uniform Buffer:Pipeline %d does not exist.", ogl->cache.pipeline.id); });
                                neko_byte_buffer_advance_position(&cb->commands, sz);
                                continue;
                            }

                            neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                            // Get uniform
                            neko_gl_uniform_list_t* ul = neko_slot_array_getp(ogl->uniforms, id);

                            // Get bound shader from pipeline (either compute or raster)
                            u32 sid = pip->compute.shader.id ? pip->compute.shader.id : pip->raster.shader.id;

                            // Check uniform location. If UINT32_T max, then must construct and place location
                            for (u32 ui = 0; ui < neko_dyn_array_size(ul->uniforms); ++ui) {
                                neko_gl_uniform_t* u = &ul->uniforms[ui];

                                // Searching for location if not bound or sid doesn't match previous use
                                if ((u->location == UINT32_MAX && u->location != UINT32_MAX - 1) || u->sid != pip->raster.shader.id) {
                                    if (!sid || !neko_slot_array_exists(ogl->shaders, sid)) {

                                        neko_timed_action(1000, { neko_log_warning("Bind Uniform:Shader %d does not exist.", sid); });

                                        // Advance by size of uniform
                                        neko_byte_buffer_advance_position(&cb->commands, sz);
                                        continue;
                                    }

                                    neko_gl_shader_t shader = neko_slot_array_get(ogl->shaders, sid);

                                    // Construct temp name, concat with base name + uniform field name
                                    char name[256] = neko_default_val();
                                    memcpy(name, ul->name, 64);
                                    if (u->name) {
                                        neko_snprintfc(UTMP, 256, "%s%s", ul->name, u->name);
                                        memcpy(name, UTMP, 256);
                                    }

                                    // Grab location of uniform based on name
                                    u->location = glGetUniformLocation(shader, name ? name : "__EMPTY_UNIFORM_NAME");

                                    if (u->location >= UINT32_MAX) {
                                        neko_log_warning("Bind Uniform: Uniform not found: \"%s\"", name);
                                        u->location = UINT32_MAX - 1;
                                    }

                                    u->sid = pip->raster.shader.id;
                                }

                                // Switch on uniform type to upload data
                                switch (u->type) {
                                    case NEKO_GL_UNIFORMTYPE_FLOAT: {
                                        // Need to read bulk data for array.
                                        neko_assert(u->size == sizeof(float));
                                        neko_dyn_array_clear(ogl->uniform_data.flt);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        neko_for_range(ct) {
                                            neko_byte_buffer_readc(&cb->commands, float, v);
                                            neko_dyn_array_push(ogl->uniform_data.flt, v);
                                        }
                                        glUniform1fv(u->location, ct, ogl->uniform_data.flt);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_INT: {
                                        neko_assert(u->size == sizeof(s32));
                                        neko_dyn_array_clear(ogl->uniform_data.i32);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        neko_for_range(ct) {
                                            neko_byte_buffer_readc(&cb->commands, s32, v);
                                            neko_dyn_array_push(ogl->uniform_data.i32, v);
                                        }
                                        glUniform1iv(u->location, ct, ogl->uniform_data.i32);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_VEC2: {
                                        neko_assert(u->size == sizeof(neko_vec2));
                                        neko_dyn_array_clear(ogl->uniform_data.vec2);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        neko_for_range(ct) {
                                            neko_byte_buffer_readc(&cb->commands, neko_vec2, v);
                                            neko_dyn_array_push(ogl->uniform_data.vec2, v);
                                        }
                                        glUniform2fv(u->location, ct, (float*)ogl->uniform_data.vec2);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_VEC3: {
                                        neko_assert(u->size == sizeof(neko_vec3));
                                        neko_dyn_array_clear(ogl->uniform_data.vec3);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        neko_for_range(ct) {
                                            neko_byte_buffer_readc(&cb->commands, neko_vec3, v);
                                            neko_dyn_array_push(ogl->uniform_data.vec3, v);
                                        }
                                        glUniform3fv(u->location, ct, (float*)ogl->uniform_data.vec3);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_VEC4: {
                                        neko_assert(u->size == sizeof(neko_vec4));
                                        neko_dyn_array_clear(ogl->uniform_data.vec4);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        neko_for_range(ct) {
                                            neko_byte_buffer_readc(&cb->commands, neko_vec4, v);
                                            neko_dyn_array_push(ogl->uniform_data.vec4, v);
                                        }
                                        glUniform4fv(u->location, ct, (float*)ogl->uniform_data.vec4);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_MAT4: {
                                        neko_assert(u->size == sizeof(neko_mat4));
                                        neko_dyn_array_clear(ogl->uniform_data.mat4);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        neko_for_range(ct) {
                                            neko_byte_buffer_readc(&cb->commands, neko_mat4, v);
                                            neko_dyn_array_push(ogl->uniform_data.mat4, v);
                                        }
                                        glUniformMatrix4fv(u->location, ct, false, (float*)ogl->uniform_data.mat4);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_SAMPLERCUBE:
                                    case NEKO_GL_UNIFORMTYPE_SAMPLER2D: {
                                        neko_assert(u->size == sizeof(neko_handle(neko_graphics_texture_t)));
                                        u32 ct = u->count ? u->count : 1;
                                        s32 binds[128] = neko_default_val();
                                        for (u32 i = 0; (i < ct && i < 128); ++i)  // Max of 128 texture binds. Get real.
                                        {
                                            neko_byte_buffer_read_bulkc(&cb->commands, neko_handle(neko_graphics_texture_t), v, u->size);

                                            // Get texture, also need binding, but will worry about that in a bit
                                            neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, v.id);

                                            // Activate texture slot
                                            glActiveTexture(GL_TEXTURE0 + binding);

                                            // Bind texture
                                            GLenum target = 0x00;
                                            switch (tex->desc.type) {
                                                default:
                                                case NEKO_GRAPHICS_TEXTURE_2D: {
                                                    target = GL_TEXTURE_2D;
                                                } break;
                                                case NEKO_GRAPHICS_TEXTURE_CUBEMAP: {
                                                    target = GL_TEXTURE_CUBE_MAP;
                                                } break;
                                            }

                                            glBindTexture(target, tex->id);

                                            binds[i] = (s32)binding++;
                                        }

                                        // Bind uniforms
                                        glUniform1iv(u->location, ct, (s32*)binds);

                                    } break;

                                    default: {
                                        // Shouldn't hit here
                                        neko_println("Assert: Bind Uniform: Invalid uniform type specified.");
                                        neko_assert(false);
                                    } break;
                                }
                            }

                        } break;

                        case NEKO_GRAPHICS_BIND_UNIFORM_BUFFER: {
                            // Read slot id of uniform buffer
                            neko_byte_buffer_readc(&cb->commands, u32, id);
                            // Read binding
                            neko_byte_buffer_readc(&cb->commands, u32, binding);
                            // Read range offset
                            neko_byte_buffer_readc(&cb->commands, size_t, range_offset);
                            // Read range size
                            neko_byte_buffer_readc(&cb->commands, size_t, range_size);

                            // Check buffer id. If invalid, then we can't operate, and instead just need to pass over the data.
                            if (!id || !neko_slot_array_exists(ogl->uniform_buffers, id)) {
                                neko_timed_action(1000, { neko_log_warning("Bind Uniform Buffer:Uniform %d does not exist.", id); });
                                continue;
                            }

                            // Grab currently bound pipeline (TODO: assert if this isn't valid)
                            if (!ogl->cache.pipeline.id || !neko_slot_array_exists(ogl->pipelines, ogl->cache.pipeline.id)) {
                                neko_timed_action(1000, { neko_println("Warning:Bind Uniform Buffer:Pipeline %d does not exist.", ogl->cache.pipeline.id); });
                                continue;
                            }

                            neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                            // Get uniform
                            neko_gl_uniform_buffer_t* u = neko_slot_array_getp(ogl->uniform_buffers, id);

                            // Get bound shader from pipeline (either compute or raster)
                            u32 sid = pip->compute.shader.id ? pip->compute.shader.id : pip->raster.shader.id;

                            // Check uniform location.
                            // If UINT32_T max, then must construct and place location, or if shader id doesn't match previously used shader with this uniform
                            // TODO: To avoid constant lookups in this case, allow for shaders to hold uniform handles references instead.
                            if ((u->location == UINT32_MAX && u->location != UINT32_MAX - 1) || u->sid != pip->raster.shader.id) {
                                if (!sid || !neko_slot_array_exists(ogl->shaders, sid)) {
                                    neko_timed_action(1000, { neko_log_warning("Bind Uniform Buffer:Shader %d does not exist.", sid); });
                                    continue;
                                }

                                neko_gl_shader_t shader = neko_slot_array_get(ogl->shaders, sid);

                                // Get uniform location based on name and bound shader
                                u->location = glGetUniformBlockIndex(shader, u->name ? u->name : "__EMPTY_UNIFORM_NAME");

                                // Set binding for uniform block
                                glUniformBlockBinding(shader, u->location, binding);

                                if (u->location >= UINT32_MAX) {
                                    neko_log_warning("Bind Uniform Buffer: Uniform not found: \"%s\"", u->name);
                                    u->location = UINT32_MAX - 1;
                                }

                                u->sid = pip->raster.shader.id;
                            }

                            glBindBufferRange(GL_UNIFORM_BUFFER, binding, u->ubo, range_offset, range_size ? range_size : u->size);

                        } break;

                        case NEKO_GRAPHICS_BIND_STORAGE_BUFFER: {
                            neko_byte_buffer_readc(&cb->commands, u32, sb_slot_id);
                            neko_byte_buffer_readc(&cb->commands, u32, binding);

                            // Grab storage buffer from id
                            if (!sb_slot_id || !neko_slot_array_exists(ogl->storage_buffers, sb_slot_id)) {
                                /*
                                neko_timed_action(1000, {
                                    neko_println("Warning:Bind Storage Buffer:Storage Buffer %d does not exist.", sb_slot_id);
                                });
                                */
                                continue;
                            }

                            // Grab currently bound pipeline (TODO: assert if this isn't valid)
                            if (!ogl->cache.pipeline.id || !neko_slot_array_exists(ogl->pipelines, ogl->cache.pipeline.id)) {
                                /*
                                neko_timed_action(1000, {
                                    neko_println("Warning:Bind Storage Buffer:Pipeline %d does not exist or is not bound.", ogl->cache.pipeline.id);
                                });
                                */
                                continue;
                            }

                            neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                            neko_gl_storage_buffer_t* sbo = neko_slot_array_getp(ogl->storage_buffers, sb_slot_id);

                            // Get bound shader from pipeline (either compute or raster)
                            u32 sid = pip->compute.shader.id ? pip->compute.shader.id : pip->raster.shader.id;

                            if (!sid || !neko_slot_array_exists(ogl->shaders, sid)) {
                                /*
                                neko_timed_action(1000, {
                                    neko_println("Warning:Bind Uniform Buffer:Shader %d does not exist.", sid);
                                });
                                */
                                continue;
                            }

                            neko_gl_shader_t shader = neko_slot_array_get(ogl->shaders, sid);

                            if ((sbo->block_idx == UINT32_MAX && sbo->block_idx != UINT32_MAX - 1)) {
                                // Get uniform location based on name and bound shader
                                CHECK_GL_CORE(sbo->block_idx = glGetProgramResourceIndex(shader, GL_SHADER_STORAGE_BLOCK, sbo->name ? sbo->name : "__EMPTY_BUFFER_NAME"); s32 params[1];
                                              GLenum props[1] = {GL_BUFFER_BINDING}; glGetProgramResourceiv(shader, GL_SHADER_STORAGE_BLOCK, sbo->block_idx, 1, props, 1, NULL, params);
                                              sbo->location = (u32)params[0]; neko_log_warning("Bind Storage Buffer: Binding \"%s\" to location %zu, block index: %zu, binding: %zu", sbo->name,
                                                                                               sbo->location, sbo->block_idx, binding););

                                if (sbo->block_idx >= UINT32_MAX) {
                                    neko_log_warning("Bind Storage Buffer: Buffer not found: \"%s\"", sbo->name);
                                    sbo->block_idx = UINT32_MAX - 1;
                                }
                            }

                            if (sbo->block_idx < UINT32_MAX - 1) {
                                // Not sure what this actually does atm...
                                CHECK_GL_CORE(glShaderStorageBlockBinding(shader, sbo->block_idx, sbo->location););
                            }

                            // This is required
                            CHECK_GL_CORE(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, sbo->buffer););

                        } break;

                        case NEKO_GRAPHICS_BIND_IMAGE_BUFFER: {
                            neko_byte_buffer_readc(&cb->commands, u32, tex_slot_id);
                            neko_byte_buffer_readc(&cb->commands, u32, binding);
                            neko_byte_buffer_readc(&cb->commands, neko_graphics_access_type, access);

                            // Grab texture from sampler id
                            if (!tex_slot_id || !neko_slot_array_exists(ogl->textures, tex_slot_id)) {
                                neko_timed_action(1000, { neko_log_warning("Bind Image Buffer: Texture %d does not exist.", tex_slot_id); });
                                continue;
                            }

                            neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, tex_slot_id);
                            u32 gl_access = neko_gl_access_type_to_gl_access_type(access);
                            u32 gl_format = neko_gl_texture_format_to_gl_texture_internal_format(tex->desc.format);

                            neko_timed_action(
                                    60, neko_graphics_info_t* info = neko_graphics_info(); if (info->major_version < 4) {
                                        neko_log_error("%s", "OpenGL4 not available, failed to call glBindImageTexture.");
                                        continue;
                                    });

                            // Bind image texture
                            CHECK_GL_CORE(glBindImageTexture(binding, tex->id, 0, GL_FALSE, 0, gl_access, gl_format);)
                        } break;

                        default:
                            neko_assert(false);
                            break;
                    }
                }

            } break;

            case NEKO_OPENGL_OP_BIND_PIPELINE: {
                // Bind pipeline stuff
                neko_byte_buffer_readc(&cb->commands, u32, pipid);

                // Make sure pipeline exists
                if (!pipid || !neko_slot_array_exists(ogl->pipelines, pipid)) {

                    neko_timed_action(1000, { neko_log_warning("Pipeline %d does not exist.", pipid); });

                    continue;
                }

                // Reset cache
                neko_gl_reset_data_cache(&ogl->cache);

                // Reset state as well
                neko_gl_pipeline_state();

                /* Cache pipeline id */
                ogl->cache.pipeline = neko_handle_create(neko_graphics_pipeline_t, pipid);

                neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, pipid);

                /* Compute */
                // Early out if compute, since we're not doing a rasterization stage
                if (pip->compute.shader.id) {
                    /* Shader */
                    if (pip->compute.shader.id && neko_slot_array_exists(ogl->shaders, pip->compute.shader.id)) {
                        glUseProgram(neko_slot_array_get(ogl->shaders, pip->compute.shader.id));
                    } else {
                        neko_timed_action(1000, { neko_log_warning("Opengl:BindPipeline:Compute:Shader %d does not exist.", pip->compute.shader.id); });
                    }

                    continue;
                }

                /* Depth */
                if (!pip->depth.func) {
                    // If no depth function (default), then disable
                    glDisable(GL_DEPTH_TEST);
                } else {
                    glEnable(GL_DEPTH_TEST);
                    glDepthFunc(neko_gl_depth_func_to_gl_depth_func(pip->depth.func));
                }
                glDepthMask(neko_gl_depth_mask_to_gl_mask(pip->depth.mask));

                /* Stencil */
                if (!pip->stencil.func) {
                    // If no stencil function (default), then disable
                    glDisable(GL_STENCIL_TEST);
                } else {
                    glEnable(GL_STENCIL_TEST);
                    u32 func = neko_gl_stencil_func_to_gl_stencil_func(pip->stencil.func);
                    u32 sfail = neko_gl_stencil_op_to_gl_stencil_op(pip->stencil.sfail);
                    u32 dpfail = neko_gl_stencil_op_to_gl_stencil_op(pip->stencil.dpfail);
                    u32 dppass = neko_gl_stencil_op_to_gl_stencil_op(pip->stencil.dppass);
                    glStencilFunc(func, pip->stencil.ref, pip->stencil.comp_mask);
                    glStencilMask(pip->stencil.write_mask);
                    glStencilOp(sfail, dpfail, dppass);
                }

                /* Blend */
                if (!pip->blend.func) {
                    glDisable(GL_BLEND);
                } else {
                    glEnable(GL_BLEND);
                    glBlendEquation(neko_gl_blend_equation_to_gl_blend_eq(pip->blend.func));
                    glBlendFunc(neko_gl_blend_mode_to_gl_blend_mode(pip->blend.src, GL_ONE), neko_gl_blend_mode_to_gl_blend_mode(pip->blend.dst, GL_ZERO));
                }

                /* Raster */
                // Face culling
                if (!pip->raster.face_culling) {
                    glDisable(GL_CULL_FACE);
                } else {
                    glEnable(GL_CULL_FACE);
                    glCullFace(neko_gl_cull_face_to_gl_cull_face(pip->raster.face_culling));
                }

                // Winding order
                glFrontFace(neko_gl_winding_order_to_gl_winding_order(pip->raster.winding_order));

                /* Shader */
                if (pip->raster.shader.id && neko_slot_array_exists(ogl->shaders, pip->raster.shader.id)) {
                    glUseProgram(neko_slot_array_get(ogl->shaders, pip->raster.shader.id));
                } else {
                    neko_timed_action(1000, { neko_log_warning("Opengl:BindPipeline:Shader %d does not exist.", pip->raster.shader.id); });
                }
            } break;

            case NEKO_OPENGL_OP_DISPATCH_COMPUTE: {
                neko_byte_buffer_readc(&cb->commands, u32, num_x_groups);
                neko_byte_buffer_readc(&cb->commands, u32, num_y_groups);
                neko_byte_buffer_readc(&cb->commands, u32, num_z_groups);

                // Grab currently bound pipeline (TODO: assert if this isn't valid)
                if (ogl->cache.pipeline.id == 0 || !neko_slot_array_exists(ogl->pipelines, ogl->cache.pipeline.id)) {
                    /*
                    neko_timed_action(1000, {
                        neko_println("Warning:Opengl:DispatchCompute:Compute Pipeline not bound.");
                    });
                    */
                    continue;
                }

                neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                // If pipeline does not have a compute state bound, then leave
                if (!pip->compute.shader.id) {
                    /*
                    neko_timed_action(1000, {
                        neko_println("Warning:Opengl:DispatchCompute:Compute Pipeline not bound.");
                    });
                    */
                    continue;
                }

                // Dispatch shader
                CHECK_GL_CORE(
                        // Memory barrier (TODO: make this specifically set in the pipeline state)
                        glDispatchCompute(num_x_groups, num_y_groups, num_z_groups); glMemoryBarrier(GL_ALL_BARRIER_BITS);)
            } break;

            case NEKO_OPENGL_OP_DRAW: {
                // Grab currently bound pipeline (TODO: assert if this isn't valid)
                neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                // Must have a vertex buffer bound to draw
                if (neko_dyn_array_empty(ogl->cache.vdecls)) {
                    neko_timed_action(1000, { neko_println("Error:Opengl:Draw: No vertex buffer bound."); });
                    // neko_assert(false);
                }

                // Keep track whether or not the data is to be instanced
                bool is_instanced = false;

                for (u32 i = 0; i < neko_dyn_array_size(pip->layout); ++i) {
                    // Vertex buffer to bind
                    u32 vbo_idx = i;  // pip->layout[i].buffer_idx;
                    neko_gl_vertex_buffer_decl_t vdecl = vbo_idx < neko_dyn_array_size(ogl->cache.vdecls) ? ogl->cache.vdecls[vbo_idx] : ogl->cache.vdecls[0];
                    neko_gl_buffer_t vbo = vdecl.vbo;

                    // Manual override. If you manually set divisor/stride/offset, then will not automatically calculate any of those.
                    bool is_manual = pip->layout[i].stride | pip->layout[i].divisor | pip->layout[i].offset | vdecl.data_type == NEKO_GRAPHICS_VERTEX_DATA_NONINTERLEAVED;

                    // Bind buffer
                    glBindBuffer(GL_ARRAY_BUFFER, vbo);

                    // Stride of vertex attribute
                    size_t stride = is_manual ? pip->layout[i].stride : neko_gl_calculate_vertex_size_in_bytes(pip->layout, neko_dyn_array_size(pip->layout));

                    // Byte offset of vertex attribute (if non-interleaved data, then grab offset from decl instead)
                    size_t offset = vdecl.data_type == NEKO_GRAPHICS_VERTEX_DATA_NONINTERLEAVED ? vdecl.offset
                                    : is_manual                                                 ? pip->layout[i].offset
                                                                                                : neko_gl_get_vertex_attr_byte_offest(pip->layout, i);

                    // If there is a vertex divisor for this layout, then we'll draw instanced
                    is_instanced |= (pip->layout[i].divisor != 0);

                    // Enable the vertex attribute pointer
                    glEnableVertexAttribArray(i);

                    switch (pip->layout[i].format) {
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT4:
                            glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, stride, neko_int2voidp(offset));
                            break;
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3:
                            glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, stride, neko_int2voidp(offset));
                            break;
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2:
                            glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, stride, neko_int2voidp(offset));
                            break;
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT:
                            glVertexAttribPointer(i, 1, GL_FLOAT, GL_FALSE, stride, neko_int2voidp(offset));
                            break;
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT4:
                            glVertexAttribIPointer(i, 4, GL_UNSIGNED_INT, stride, neko_int2voidp(offset));
                            break;
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT3:
                            glVertexAttribIPointer(i, 3, GL_UNSIGNED_INT, stride, neko_int2voidp(offset));
                            break;
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT2:
                            glVertexAttribIPointer(i, 2, GL_UNSIGNED_INT, stride, neko_int2voidp(offset));
                            break;
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_UINT:
                            glVertexAttribIPointer(i, 1, GL_UNSIGNED_INT, stride, neko_int2voidp(offset));
                            break;
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE:
                            glVertexAttribPointer(i, 1, GL_UNSIGNED_BYTE, GL_TRUE, stride, neko_int2voidp(offset));
                            break;
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE2:
                            glVertexAttribPointer(i, 2, GL_UNSIGNED_BYTE, GL_TRUE, stride, neko_int2voidp(offset));
                            break;
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE3:
                            glVertexAttribPointer(i, 3, GL_UNSIGNED_BYTE, GL_TRUE, stride, neko_int2voidp(offset));
                            break;
                        case NEKO_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4:
                            glVertexAttribPointer(i, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, neko_int2voidp(offset));
                            break;

                        // Shouldn't get here
                        default: {
                            neko_assert(false);
                        } break;
                    }
                    // Set up divisor (for instancing)
                    glVertexAttribDivisor(i, pip->layout[i].divisor);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                }

                // Bind all vertex buffers after setting up data and pointers
                for (u32 i = 0; i < neko_dyn_array_size(ogl->cache.vdecls); ++i) {
                    glBindBuffer(GL_ARRAY_BUFFER, ogl->cache.vdecls[i].vbo);
                }

                // Draw based on bound primitive type in raster
                neko_byte_buffer_readc(&cb->commands, u32, start);
                neko_byte_buffer_readc(&cb->commands, u32, count);
                neko_byte_buffer_readc(&cb->commands, u32, instance_count);
                neko_byte_buffer_readc(&cb->commands, u32, base_vertex);
                neko_byte_buffer_readc(&cb->commands, u32, range_start);
                neko_byte_buffer_readc(&cb->commands, u32, range_end);

                range_end = (range_end && range_end < range_start) ? range_end : count;

                // Bind element buffer ranged
                if (ogl->cache.ibo) {
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, neko_slot_array_get(ogl->index_buffers, ogl->cache.ibo));
                }

                // If instance count > 1, do instanced drawing
                is_instanced |= (instance_count > 1);

                u32 prim = neko_gl_primitive_to_gl_primitive(pip->raster.primitive);
                u32 itype = neko_gl_index_buffer_size_to_gl_index_type(pip->raster.index_buffer_element_size);

                // Draw
                if (ogl->cache.ibo) {
#ifdef NEKO_GRAPHICS_IMPL_OPENGL_CORE
                    if (is_instanced)
                        glDrawElementsInstancedBaseVertex(prim, count, itype, neko_int2voidp(start), instance_count, base_vertex);
                    else
                        glDrawRangeElementsBaseVertex(prim, range_start, range_end, count, itype, neko_int2voidp(start), base_vertex);
#else
                    if (is_instanced)
                        glDrawElementsInstanced(prim, count, itype, neko_int2voidp(start), instance_count);
                    else
                        glDrawElements(prim, count, itype, neko_int2voidp(start));
#endif
                } else {
                    if (is_instanced)
                        glDrawArraysInstanced(prim, start, count, instance_count);
                    else
                        glDrawArrays(prim, start, count);
                }

            } break;

            case NEKO_OPENGL_OP_DRAW_BATCH: {
                neko_byte_buffer_readc(&cb->commands, neko_graphics_custom_batch_context_ptr, batch_ctx);
                neko_byte_buffer_readc(&cb->commands, neko_graphics_custom_batch_framebuffer_ptr, fb);
                neko_byte_buffer_readc(&cb->commands, s32, w);
                neko_byte_buffer_readc(&cb->commands, s32, h);

                // neko_gl_pipeline_state();

                glEnable(GL_BLEND);
                glBlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                neko_graphics_custom_batch_flush(batch_ctx, fb, w, h);
            } break;

            case NEKO_OPENGL_OP_REQUEST_TEXTURE_UPDATE: {
                neko_byte_buffer_readc(&cb->commands, u32, tex_slot_id);
                neko_byte_buffer_readc(&cb->commands, neko_graphics_texture_desc_t, desc);
                neko_byte_buffer_readc(&cb->commands, size_t, data_size);

                // Update texture with data, depending on update type (for now, just stream new data)

                // Grab texture from sampler id
                if (!tex_slot_id || !neko_slot_array_exists(ogl->textures, tex_slot_id)) {
                    neko_timed_action(60, { neko_log_warning("Bind Image Buffer: Texture %d does not exist.", tex_slot_id); });
                    neko_byte_buffer_advance_position(&cb->commands, data_size);
                }

                neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, tex_slot_id);
                u32 int_format = neko_gl_texture_format_to_gl_texture_internal_format(desc.format);
                u32 format = neko_gl_texture_format_to_gl_texture_format(desc.format);
                u32 dt = neko_gl_texture_format_to_gl_data_type(desc.format);
                *desc.data = (cb->commands.data + cb->commands.position);
                *tex = gl_texture_update_internal(&desc, tex_slot_id);

                // Bind texture
                // glBindTexture(GL_TEXTURE_2D, tex->id);
                // // Update texture data
                // // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, desc.width, desc.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (cb->commands.data, cb->commands.position));
                // // glTexImage2D(GL_TEXTURE_2D, 0, int_format, desc.width, desc.height, 0, format, dt, (cb->commands.data, cb->commands.position));
                // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, desc.width, desc.height, format, dt, (cb->commands.data + cb->commands.position));
                // glBindTexture(GL_TEXTURE_2D, 0);
                neko_byte_buffer_advance_position(&cb->commands, data_size);
            } break;

            case NEKO_OPENGL_OP_REQUEST_BUFFER_UPDATE: {
                neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(graphics)->user_data;

                // Read handle id
                neko_byte_buffer_readc(&cb->commands, u32, id);
                // Read type
                neko_byte_buffer_readc(&cb->commands, neko_graphics_buffer_type, type);
                // Read usage
                neko_byte_buffer_readc(&cb->commands, neko_graphics_buffer_usage_type, usage);
                // Read data size
                neko_byte_buffer_readc(&cb->commands, size_t, sz);
                // Read data offset
                neko_byte_buffer_readc(&cb->commands, size_t, offset);
                // Read update type
                neko_byte_buffer_readc(&cb->commands, neko_graphics_buffer_update_type, update_type);

                s32 glusage = neko_gl_buffer_usage_to_gl_enum(usage);

                switch (type) {
                    // Vertex Buffer
                    default:
                    case NEKO_GRAPHICS_BUFFER_VERTEX: {
                        neko_gl_buffer_t buffer = neko_slot_array_get(ogl->vertex_buffers, id);
                        glBindBuffer(GL_ARRAY_BUFFER, buffer);
                        switch (update_type) {
                            case NEKO_GRAPHICS_BUFFER_UPDATE_SUBDATA:
                                glBufferSubData(GL_ARRAY_BUFFER, offset, sz, (cb->commands.data + cb->commands.position));
                                break;
                            default:
                                glBufferData(GL_ARRAY_BUFFER, sz, (cb->commands.data + cb->commands.position), glusage);
                                break;
                        }
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                    } break;

                    case NEKO_GRAPHICS_BUFFER_INDEX: {
                        neko_gl_buffer_t buffer = neko_slot_array_get(ogl->index_buffers, id);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
                        switch (update_type) {
                            case NEKO_GRAPHICS_BUFFER_UPDATE_SUBDATA:
                                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, sz, (cb->commands.data + cb->commands.position));
                                break;
                            default:
                                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, (cb->commands.data + cb->commands.position), glusage);
                                break;
                        }
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                    } break;

                    case NEKO_GRAPHICS_BUFFER_UNIFORM: {
                        // Have to
                        neko_gl_uniform_buffer_t* u = neko_slot_array_getp(ogl->uniform_buffers, id);

                        glBindBuffer(GL_UNIFORM_BUFFER, u->ubo);

                        switch (update_type) {
                            case NEKO_GRAPHICS_BUFFER_UPDATE_SUBDATA: {
                                glBufferSubData(GL_UNIFORM_BUFFER, offset, sz, (cb->commands.data + cb->commands.position));
                            } break;
                            default: {
                                // Reset uniform size
                                u->size = sz;
                                // Recreate buffer
                                glBufferData(GL_UNIFORM_BUFFER, sz, (cb->commands.data + cb->commands.position), glusage);
                            } break;
                        }

                        glBindBuffer(GL_UNIFORM_BUFFER, 0);
                    } break;

                    case NEKO_GRAPHICS_BUFFER_SHADER_STORAGE: {
                        neko_graphics_storage_buffer_desc_t desc = {
                                .data = cb->commands.data + cb->commands.position,
                                .size = sz,
                                .usage = usage,
                                .update =
                                        {
                                                .type = update_type,
                                                .offset = offset,
                                        },
                        };
                        neko_handle(neko_graphics_storage_buffer_t) hndl;
                        hndl.id = id;
                        neko_graphics_storage_buffer_update(hndl, &desc);

                    } break;
                }

                // Advance past data
                neko_byte_buffer_advance_position(&cb->commands, sz);

            } break;

            default: {
                // Op code not supported yet!
                neko_println("Op code not supported yet: %zu", (u32)op_code);
                neko_assert(false);
            }
        }
    }

    // Clear byte buffer of commands
    neko_byte_buffer_clear(&cb->commands);

    // Set num commands to 0
    cb->num_commands = 0;
}

NEKO_API_DECL void neko_graphics_init(neko_graphics_t* graphics) {
    // Push back 0 handles into slot arrays (for 0 init validation)
    neko_gl_data_t* ogl = (neko_gl_data_t*)graphics->user_data;

    neko_slot_array_insert(ogl->shaders, 0);
    neko_slot_array_insert(ogl->vertex_buffers, 0);
    neko_slot_array_insert(ogl->index_buffers, 0);
    neko_slot_array_insert(ogl->frame_buffers, 0);

    neko_gl_uniform_list_t ul = neko_default_val();
    neko_gl_uniform_buffer_t ub = neko_default_val();
    neko_gl_pipeline_t pip = neko_default_val();
    neko_gl_renderpass_t rp = neko_default_val();
    neko_gl_texture_t tex = neko_default_val();
    neko_gl_storage_buffer_t sb = neko_default_val();

    neko_slot_array_insert(ogl->uniforms, ul);
    neko_slot_array_insert(ogl->pipelines, pip);
    neko_slot_array_insert(ogl->renderpasses, rp);
    neko_slot_array_insert(ogl->uniform_buffers, ub);
    neko_slot_array_insert(ogl->textures, tex);
    neko_slot_array_insert(ogl->storage_buffers, sb);

    // Construct vao then bind
    glGenVertexArrays(1, &ogl->cache.vao);
    glBindVertexArray(ogl->cache.vao);

    // Reset data cache for rendering ops
    neko_gl_reset_data_cache(&ogl->cache);

    // Init info object
    neko_graphics_info_t* info = &neko_subsystem(graphics)->info;

    // OpenGL 主机信息
    glGetIntegerv(GL_MAJOR_VERSION, (GLint*)&info->major_version);
    glGetIntegerv(GL_MINOR_VERSION, (GLint*)&info->minor_version);
    info->version = (const_str)glGetString(GL_VERSION);
    info->vendor = (const_str)glGetString(GL_RENDERER);

    // Max texture units
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, (GLint*)&info->max_texture_units);

    // Compute shader info
    CHECK_GL_CORE(info->compute.available = info->major_version >= 4 && info->minor_version >= 3; if (info->compute.available) {
        // Work group counts
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, (s32*)&info->compute.max_work_group_count[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, (s32*)&info->compute.max_work_group_count[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, (s32*)&info->compute.max_work_group_count[2]);
        // Work group sizes
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, (s32*)&info->compute.max_work_group_size[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, (s32*)&info->compute.max_work_group_size[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, (s32*)&info->compute.max_work_group_size[2]);
        // Work group invocations
        glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, (s32*)&info->compute.max_work_group_invocations);
    });

    const GLubyte* glslv = glGetString(GL_SHADING_LANGUAGE_VERSION);
    neko_log_info("GLSL Version: %s", glslv);

#if defined(NEKO_DEBUG) && 0
    int numExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    neko_printf("Supported extensions:");
    for (int i = 0; i < numExtensions; i++) {
        const GLubyte* extension = glGetStringi(GL_EXTENSIONS, i);
        neko_printf("%s;", extension);
    }
    neko_printf("\n");
#endif

    // Set up all function pointers for graphics context

    // Create
    graphics->api.texture_create = neko_graphics_texture_create_impl;
    graphics->api.uniform_create = neko_graphics_uniform_create_impl;
    graphics->api.shader_create = neko_graphics_shader_create_impl;
    graphics->api.vertex_buffer_create = neko_graphics_vertex_buffer_create_impl;
    graphics->api.index_buffer_create = neko_graphics_index_buffer_create_impl;
    graphics->api.uniform_buffer_create = neko_graphics_uniform_buffer_create_impl;
    graphics->api.storage_buffer_create = neko_graphics_storage_buffer_create_impl;
    graphics->api.framebuffer_create = neko_graphics_framebuffer_create_impl;
    graphics->api.renderpass_create = neko_graphics_renderpass_create_impl;
    graphics->api.pipeline_create = neko_graphics_pipeline_create_impl;

    // Destroy
    graphics->api.texture_destroy = neko_graphics_texture_destroy_impl;
    graphics->api.uniform_destroy = neko_graphics_uniform_destroy_impl;
    graphics->api.shader_destroy = neko_graphics_shader_destroy_impl;
    graphics->api.vertex_buffer_destroy = neko_graphics_vertex_buffer_destroy_impl;
    graphics->api.index_buffer_destroy = neko_graphics_index_buffer_destroy_impl;
    graphics->api.uniform_buffer_destroy = neko_graphics_uniform_buffer_destroy_impl;
    graphics->api.storage_buffer_destroy = neko_graphics_storage_buffer_destroy_impl;
    graphics->api.framebuffer_destroy = neko_graphics_framebuffer_destroy_impl;
    graphics->api.renderpass_destroy = neko_graphics_renderpass_destroy_impl;
    graphics->api.pipeline_destroy = neko_graphics_pipeline_destroy_impl;

    // Resource Updates (main thread only)
    graphics->api.vertex_buffer_update = neko_graphics_vertex_buffer_update_impl;
    graphics->api.index_buffer_update = neko_graphics_index_buffer_update_impl;
    graphics->api.storage_buffer_update = neko_graphics_storage_buffer_update_impl;
    graphics->api.texture_update = neko_graphics_texture_update_impl;
    graphics->api.texture_read = neko_graphics_texture_read_impl;

    // Submission (Main Thread)
    graphics->api.command_buffer_submit = neko_graphics_command_buffer_submit_impl;

    /*============================================================
    // Fontcache
    ============================================================*/
    // graphics->api.fontcache_create = &__neko_fontcache_create;
    // graphics->api.fontcache_destroy = &__neko_fontcache_shutdown;
    // graphics->api.fontcache_draw = &__neko_fontcache_draw;
    // graphics->api.fontcache_load = &__neko_fontcache_load;
    // graphics->api.fontcache_push = &__neko_fontcache_push;
    // graphics->api.fontcache_push_x_y = &__neko_fontcache_push_x_y;
}

#endif  // NEKO_GRAPHICS_IMPL_OPENGL
#endif