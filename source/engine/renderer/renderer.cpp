
#include "renderer.h"

#include "base/common/base.hpp"
#include "base/common/mem.hpp"
#include "engine/bootstrap.h"
#include "engine/graphics.h"
#include "engine/renderer/texture.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void __stdcall gl_debug_callback(u32 source, u32 type, u32 id, u32 severity, i32 length, const char* message, const void* up) {

    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) {
        return;
    }

    const char* s;
    const char* t;

    switch (source) {
        case GL_DEBUG_SOURCE_API:
            s = "API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            s = "window system";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            s = "shader compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            s = "third party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            s = "application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            s = "other";
            break;
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            t = "type error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            t = "deprecated behaviour";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            t = "undefined behaviour";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            t = "portability";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            t = "performance";
            break;
        case GL_DEBUG_TYPE_MARKER:
            t = "marker";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            t = "push group";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            t = "pop group";
            break;
        case GL_DEBUG_TYPE_OTHER:
            t = "other";
            break;
    }

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
        case GL_DEBUG_SEVERITY_MEDIUM:
            LOG_ERROR("OpenGL (source: {}; type: {}): {}", s, t, message);
            break;
        case GL_DEBUG_SEVERITY_LOW:
            LOG_ERROR("OpenGL (source: {}; type: {}): {}", s, t, message);
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            LOG_ERROR("OpenGL (source: {}; type: {}): {}", s, t, message);
            break;
    }
}

static u32 get_gl_thing(u32 thing) {
    switch (thing) {
        case vt_clip:
            return GL_SCISSOR_TEST;
        case vt_depth_test:
            return GL_DEPTH_TEST;
    }

    fprintf(stderr, "warning: Invalid enum passed to get_gl_thing.\n");

    return 0;
}

void draw_enable(u32 thing) {
    if (thing == vt_depth_test) {
        // depth_test_enabled = true;
    }

    glEnable(get_gl_thing(thing));
}

void draw_disable(u32 thing) {
    if (thing == vt_depth_test) {
        // depth_test_enabled = false;
    }

    glDisable(get_gl_thing(thing));
}

void draw_clip(rect_t rect) { glScissor(rect.x, rect.y, rect.w, rect.h); }

#pragma pack(push, 1)
struct bmp_header {
    u16 ftype;
    u32 fsize;
    u16 res1, res2;
    u32 bmp_offset;
    u32 size;
    i32 w, h;
    u16 planes;
    u16 bits_per_pixel;
};
#pragma pack(pop)

void VertexBuffer::init_vb(const i32 flags) {
    this->flags = flags;

    glGenVertexArrays(1, &this->va_id);
    glGenBuffers(1, &this->vb_id);
    glGenBuffers(1, &this->ib_id);
}

void VertexBuffer::fini_vb() {
    glDeleteVertexArrays(1, &this->va_id);
    glDeleteBuffers(1, &this->vb_id);
    glDeleteBuffers(1, &this->ib_id);
}

void VertexBuffer::bind_vb_for_draw(bool bind) const { glBindVertexArray(bind ? this->va_id : 0); }

void VertexBuffer::bind_vb_for_edit(bool bind) const {
    if (!bind) {
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    } else {
        glBindVertexArray(this->va_id);
        glBindBuffer(GL_ARRAY_BUFFER, this->vb_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ib_id);
    }
}

void VertexBuffer::push_vertices(f32* vertices, u32 count) const {
    const u32 mode = this->flags & vb_static ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;

    glBufferData(GL_ARRAY_BUFFER, count * sizeof(f32), vertices, mode);
}

void VertexBuffer::push_indices(u32* indices, u32 count) {
    const u32 mode = this->flags & vb_static ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;

    this->index_count = count;

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(f32), indices, mode);
}

void VertexBuffer::update_vertices(f32* vertices, u32 offset, u32 count) const { glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(f32), count * sizeof(f32), vertices); }

void VertexBuffer::update_indices(u32* indices, u32 offset, u32 count) {
    this->index_count = count;

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset * sizeof(u32), count * sizeof(u32), indices);
}

void VertexBuffer::configure_vb(u32 index, u32 component_count, u32 stride, u32 offset) const {

    glVertexAttribPointer(index, component_count, GL_FLOAT, GL_FALSE, stride * sizeof(f32), (void*)(u64)(offset * sizeof(f32)));
    glEnableVertexAttribArray(index);
}

void VertexBuffer::draw_vb() const {
    u32 draw_type = GL_TRIANGLES;
    if (this->flags & vb_lines) {
        draw_type = GL_LINES;
    } else if (this->flags & vb_line_strip) {
        draw_type = GL_LINE_STRIP;
    }

    glDrawElements(draw_type, this->index_count, GL_UNSIGNED_INT, 0);
}

void VertexBuffer::draw_vb_n(u32 count) const {
    u32 draw_type = GL_TRIANGLES;
    if (this->flags & vb_lines) {
        draw_type = GL_LINES;
    } else if (this->flags & vb_line_strip) {
        draw_type = GL_LINE_STRIP;
    }

    glDrawElements(draw_type, count, GL_UNSIGNED_INT, 0);
}

void init_render_target(RenderTarget* target, u32 width, u32 height) {
    glGenFramebuffers(1, &target->id);

    glBindFramebuffer(GL_FRAMEBUFFER, target->id);

    /* Attach a texture */
    glGenTextures(1, &target->output);
    glBindTexture(GL_TEXTURE_2D, target->output);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    /* Allow multiple attachments? I don't think it's necessary for the time being;
     * A 2-D game won't need *that* much post-processing. */
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target->output, 0);

    target->width = width;
    target->height = height;

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Failed to create render target.\n");
    }

    bind_render_target(NULL);
}

void deinit_render_target(RenderTarget* target) {
    glDeleteFramebuffers(1, &target->id);
    glDeleteTextures(1, &target->output);
}

void resize_render_target(RenderTarget* target, u32 width, u32 height) {
    if (target->width == width && target->height == height) {
        return;
    }

    target->width = width;
    target->height = height;

    glBindTexture(GL_TEXTURE_2D, target->output);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void bind_render_target(RenderTarget* target) {
    if (!target) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, target->id);
}

void bind_render_target_output(RenderTarget* target, u32 unit) {
    if (!target) {
        glBindTexture(GL_TEXTURE_2D, 0);
        return;
    }

    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, target->output);
}

#define batch_size 100
#define els_per_vert 11
#define verts_per_quad 4
#define indices_per_quad 6

Color256 make_color(u32 rgb, u8 alpha) { return Color256{(u8)((rgb >> 16) & 0xFF), (u8)((rgb >> 8) & 0xFF), (u8)(rgb & 0xff), alpha}; }

void QuadRenderer::new_renderer(AssetShader shader, vec2 dimentions) {

    this->quad_count = 0;
    this->texture_count = 0;

    this->ambient_light = 1.0f;

    this->vb.init_vb(vb_dynamic | vb_tris);
    this->vb.bind_vb_for_edit(true);
    this->vb.push_vertices(NULL, els_per_vert * verts_per_quad * batch_size);
    this->vb.push_indices(NULL, indices_per_quad * batch_size);
    this->vb.configure_vb(0, 2, els_per_vert, 0);  /* vec2 position */
    this->vb.configure_vb(1, 2, els_per_vert, 2);  /* vec2 uv */
    this->vb.configure_vb(2, 4, els_per_vert, 4);  /* vec4 color */
    this->vb.configure_vb(3, 1, els_per_vert, 8);  /* f32 texture_id */
    this->vb.configure_vb(4, 1, els_per_vert, 9);  /* f32 inverted */
    this->vb.configure_vb(5, 1, els_per_vert, 10); /* f32 unlit */
    this->vb.bind_vb_for_edit(NULL);

    this->clip_enable = false;
    this->camera_enable = false;

    this->shader = shader;
    neko_bind_shader(this->shader.id);

    this->camera = mat4_ortho(0.0f, (f32)dimentions.x, (f32)dimentions.y, 0.0f, -1.0f, 1.0f);
    neko_shader_set_m4f(this->shader.id, "camera", this->camera);
    this->dimentions = dimentions;

    neko_bind_shader(NULL);
}

void QuadRenderer::free_renderer() { this->vb.fini_vb(); }

void QuadRenderer::renderer_flush() {
    if (this->quad_count == 0) {
        return;
    }

    if (this->clip_enable) {
        draw_enable(vt_clip);
        draw_clip(rect_t{this->clip.x, (i32)this->dimentions.y - (this->clip.y + this->clip.h), this->clip.w, this->clip.h});
    } else {
        draw_disable(vt_clip);
    }

    neko_bind_shader(this->shader.id);

    for (u32 i = 0; i < this->texture_count; i++) {
        neko_bind_texture(this->textures[i], i);

        char name[32];
        sprintf(name, "textures[%u]", i);

        neko_shader_set_int(this->shader.id, name, i);
    }

    for (u32 i = 0; i < this->light_count; i++) {
        char name[64];

        sprintf(name, "lights[%u].position", i);
        neko_shader_set_v2f(this->shader.id, name, this->lights[i].position);

        sprintf(name, "lights[%u].intensity", i);
        neko_shader_set_float(this->shader.id, name, this->lights[i].intensity);

        sprintf(name, "lights[%u].range", i);
        neko_shader_set_float(this->shader.id, name, this->lights[i].range);
    }

    neko_shader_set_int(this->shader.id, "light_count", this->light_count);

    neko_shader_set_m4f(this->shader.id, "camera", this->camera);
    neko_shader_set_float(this->shader.id, "ambient_light", this->ambient_light);

    if (this->camera_enable) {
        mat4 view = mat4_translate(mat4_identity(), neko_v3(-((f32)this->camera_pos.x) + ((f32)this->dimentions.x / 2), -((f32)this->camera_pos.y) + ((f32)this->dimentions.y / 2), 0.0f));
        neko_shader_set_m4f(this->shader.id, "view", view);
    } else {
        neko_shader_set_m4f(this->shader.id, "view", mat4_identity());
    }

    this->vb.bind_vb_for_edit(true);
    this->vb.update_vertices(this->verts, 0, this->quad_count * els_per_vert * verts_per_quad);
    this->vb.update_indices(this->indices, 0, this->quad_count * indices_per_quad);
    this->vb.bind_vb_for_edit(false);

    this->vb.bind_vb_for_draw(true);
    this->vb.draw_vb_n(this->quad_count * indices_per_quad);
    this->vb.bind_vb_for_draw(false);
    neko_bind_shader(NULL);

    this->quad_count = 0;
    this->texture_count = 0;

    draw_disable(vt_clip);
}

void QuadRenderer::renderer_end_frame() {
    renderer_flush();
    this->light_count = 0;
}

void QuadRenderer::renderer_push_light(struct light light) {
    if (this->light_count > max_lights) {
        fprintf(stderr, "Too many lights! Max: %d\n", max_lights);
        return;
    }

    this->lights[this->light_count++] = light;
}

void QuadRenderer::renderer_push(TexturedQuad* quad) {
    f32 tx = 0, ty = 0, tw = 0, th = 0;

    i32 tidx = -1;
    if (quad->texture) {
        for (u32 i = 0; i < this->texture_count; i++) {
            if (this->textures[i] == quad->texture) {
                tidx = (i32)i;
                break;
            }
        }

        if (tidx == -1) {
            this->textures[this->texture_count] = quad->texture;
            tidx = this->texture_count;

            this->texture_count++;

            if (this->texture_count >= 32) {
                renderer_flush();
                tidx = 0;
                this->textures[0] = quad->texture;
            }
        }

        tx = (f32)quad->rect.x / (f32)quad->texture->width;
        ty = (f32)quad->rect.y / (f32)quad->texture->height;
        tw = (f32)quad->rect.w / (f32)quad->texture->width;
        th = (f32)quad->rect.h / (f32)quad->texture->height;
    }

    const f32 r = (f32)quad->color.r / 255.0f;
    const f32 g = (f32)quad->color.g / 255.0f;
    const f32 b = (f32)quad->color.b / 255.0f;
    const f32 a = (f32)quad->color.a / 255.0f;

    const f32 w = (f32)quad->dimentions.x;
    const f32 h = (f32)quad->dimentions.y;
    const f32 x = (f32)quad->position.x;
    const f32 y = (f32)quad->position.y;

    const bool use_origin = quad->origin.x != 0.0f && quad->origin.y != 0.0f;

    mat4 transform = mat4_translate(mat4_identity(), vec3{(f32)quad->position.x, (f32)quad->position.y, 0.0f});

    transform = use_origin ? mat4_translate(transform, vec3{quad->origin.x, quad->origin.y, 0.0f}) : transform;

    if (quad->rotation != 0.0f) {
        transform = mat4_rotate(transform, DEG2RAD(quad->rotation), vec3{0.0f, 0.0f, 1.0f});
    }

    transform = mat4_scale(transform, vec3{(f32)quad->dimentions.x, (f32)quad->dimentions.y, 0.0f});
    transform = use_origin ? mat4_translate(transform, vec3{-quad->origin.x, -quad->origin.y, 0.0f}) : transform;

    const vec4 p0 = mat4_transform(transform, neko_v4(0.0f, 0.0f, 0.0f, 1.0f));
    const vec4 p1 = mat4_transform(transform, neko_v4(1.0f, 0.0f, 0.0f, 1.0f));
    const vec4 p2 = mat4_transform(transform, neko_v4(1.0f, 1.0f, 0.0f, 1.0f));
    const vec4 p3 = mat4_transform(transform, neko_v4(0.0f, 1.0f, 0.0f, 1.0f));

    f32 verts[] = {
            p0.x, p0.y, tx,      ty,      r, g, b, a, (f32)tidx, (f32)quad->inverted, (f32)quad->unlit, p1.x, p1.y, tx + tw, ty,      r, g, b, a, (f32)tidx, (f32)quad->inverted, (f32)quad->unlit,
            p2.x, p2.y, tx + tw, ty + th, r, g, b, a, (f32)tidx, (f32)quad->inverted, (f32)quad->unlit, p3.x, p3.y, tx,      ty + th, r, g, b, a, (f32)tidx, (f32)quad->inverted, (f32)quad->unlit};

    const u32 idx_off = this->quad_count * verts_per_quad;

    u32 indices[] = {idx_off + 3, idx_off + 2, idx_off + 1, idx_off + 3, idx_off + 1, idx_off + 0};

    memcpy(this->verts + (this->quad_count * els_per_vert * verts_per_quad), verts, els_per_vert * verts_per_quad * sizeof(f32));
    memcpy(this->indices + (this->quad_count * indices_per_quad), indices, indices_per_quad * sizeof(u32));

    this->quad_count++;

    if (this->quad_count >= batch_size) {
        renderer_flush();
    }
}

void QuadRenderer::renderer_clip(rect_t clip) {
    if (this->clip.x != clip.x || this->clip.y != clip.y || this->clip.w != clip.w || this->clip.h != clip.h) {
        renderer_flush();
        this->clip = clip;
    }
}

void QuadRenderer::renderer_resize(vec2 size) {
    this->dimentions = size;
    this->camera = mat4_ortho(0.0f, (f32)size.x, (f32)size.y, 0.0f, -1.0f, 1.0f);
}

void QuadRenderer::renderer_fit_to_main_window() { renderer_resize(neko_v2(the<CL>().state.width, the<CL>().state.height)); }

PostProcessor* new_post_processor(AssetShader shader) {
    PostProcessor* p = (PostProcessor*)mem_calloc(1, sizeof(PostProcessor));

    i32 win_w = the<CL>().state.width, win_h = the<CL>().state.height;

    init_render_target(&p->target, win_w, win_h);

    p->dimentions = neko_v2(win_w, win_h);

    p->shader = shader;

    f32 verts[] = {-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f};

    u32 indices[] = {3, 2, 1, 3, 1, 0};

    p->vb.init_vb(vb_static | vb_tris);
    p->vb.bind_vb_for_edit(true);
    p->vb.push_vertices(verts, 4 * 4);
    p->vb.push_indices(indices, 6);
    p->vb.configure_vb(0, 2, 4, 0);
    p->vb.configure_vb(1, 2, 4, 2);
    p->vb.bind_vb_for_edit(false);

    return p;
}

void free_post_processor(PostProcessor* p) {
    deinit_render_target(&p->target);
    p->vb.fini_vb();

    mem_free(p);
}

void use_post_processor(PostProcessor* p) {
    if (!p) {
        bind_render_target(NULL);
    }

    bind_render_target(&p->target);
}

void resize_post_processor(PostProcessor* p, vec2 dimentions) {
    p->dimentions = dimentions;
    resize_render_target(&p->target, dimentions.x, dimentions.y);
}

void post_processor_fit_to_main_window(PostProcessor* p) { resize_post_processor(p, neko_v2(the<CL>().state.width, the<CL>().state.height)); }

void flush_post_processor(PostProcessor* p, bool default_rt) {
    if (default_rt) {
        bind_render_target(NULL);
    }

    neko_bind_shader(p->shader.id);
    neko_shader_set_int(p->shader.id, "input", 0);
    neko_shader_set_v2f(p->shader.id, "screen_size", neko_v2(p->dimentions.x, p->dimentions.y));

    bind_render_target_output(&p->target, 0);

    p->vb.bind_vb_for_draw(true);
    p->vb.draw_vb();
    p->vb.bind_vb_for_draw(false);

    neko_bind_shader(NULL);
}

void Renderer::InitOpenGL() {

    // initialize GLEW
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG_INFO("Failed to initialize GLAD");
    }

#if defined(_DEBUG) && !defined(NEKO_IS_APPLE)
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(gl_debug_callback, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif

    // some GL settings
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glClearColor(NEKO_COL255(28.f), NEKO_COL255(28.f), NEKO_COL255(28.f), 1.f);
}
