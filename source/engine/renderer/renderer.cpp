#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "base/common/base.hpp"
#include "base/common/mem.hpp"
#include "engine/bootstrap.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/graphics.h"
#include "renderer.h"

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

void init_vb(VertexBuffer* vb, const i32 flags) {
    vb->flags = flags;

    glGenVertexArrays(1, &vb->va_id);
    glGenBuffers(1, &vb->vb_id);
    glGenBuffers(1, &vb->ib_id);
}

void deinit_vb(VertexBuffer* vb) {
    glDeleteVertexArrays(1, &vb->va_id);
    glDeleteBuffers(1, &vb->vb_id);
    glDeleteBuffers(1, &vb->ib_id);
}

void bind_vb_for_draw(const VertexBuffer* vb) { glBindVertexArray(vb ? vb->va_id : 0); }

void bind_vb_for_edit(const VertexBuffer* vb) {
    if (!vb) {
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    } else {
        glBindVertexArray(vb->va_id);
        glBindBuffer(GL_ARRAY_BUFFER, vb->vb_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vb->ib_id);
    }
}

void push_vertices(const VertexBuffer* vb, f32* vertices, u32 count) {
    const u32 mode = vb->flags & vb_static ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;

    glBufferData(GL_ARRAY_BUFFER, count * sizeof(f32), vertices, mode);
}

void push_indices(VertexBuffer* vb, u32* indices, u32 count) {
    const u32 mode = vb->flags & vb_static ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;

    vb->index_count = count;

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(f32), indices, mode);
}

void update_vertices(const VertexBuffer* vb, f32* vertices, u32 offset, u32 count) { glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(f32), count * sizeof(f32), vertices); }

void update_indices(VertexBuffer* vb, u32* indices, u32 offset, u32 count) {
    vb->index_count = count;

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset * sizeof(u32), count * sizeof(u32), indices);
}

void configure_vb(const VertexBuffer* vb, u32 index, u32 component_count, u32 stride, u32 offset) {

    glVertexAttribPointer(index, component_count, GL_FLOAT, GL_FALSE, stride * sizeof(f32), (void*)(u64)(offset * sizeof(f32)));
    glEnableVertexAttribArray(index);
}

void draw_vb(const VertexBuffer* vb) {
    u32 draw_type = GL_TRIANGLES;
    if (vb->flags & vb_lines) {
        draw_type = GL_LINES;
    } else if (vb->flags & vb_line_strip) {
        draw_type = GL_LINE_STRIP;
    }

    glDrawElements(draw_type, vb->index_count, GL_UNSIGNED_INT, 0);
}

void draw_vb_n(const VertexBuffer* vb, u32 count) {
    u32 draw_type = GL_TRIANGLES;
    if (vb->flags & vb_lines) {
        draw_type = GL_LINES;
    } else if (vb->flags & vb_line_strip) {
        draw_type = GL_LINE_STRIP;
    }

    glDrawElements(draw_type, count, GL_UNSIGNED_INT, 0);
}

void init_texture(struct texture* texture, u8* data, u64 size, u32 flags) {
    assert(size > sizeof(struct bmp_header));

    if (*data != 'B' && *(data + 1) != 'M') {
        fprintf(stderr, "Not a valid bitmap!\n");
        return;
    }

    struct bmp_header* header = (struct bmp_header*)data;
    u8* src = data + header->bmp_offset;

    u32 mode = texture_rgba;
    if (header->bits_per_pixel == 24) {
        mode = texture_rgb;
    } else if (header->bits_per_pixel == 8) {
        mode = texture_mono;
    }

    init_texture_no_bmp(texture, src, header->w, header->h, flags | mode | texture_flip);
}

void init_texture_no_bmp(struct texture* texture, u8* src, u32 w, u32 h, u32 flags) {
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);

    GLenum wrap_mode = GL_REPEAT;
    if (flags & texture_clamp) {
        wrap_mode = GL_CLAMP_TO_EDGE;
    }

    GLenum filter_mode = GL_LINEAR;
    if (flags & texture_filter_nearest) {
        filter_mode = GL_NEAREST;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_mode);

    GLenum format = GL_RGB;
    u32 wf = 3;
    if (flags & texture_rgba) {
        format = GL_RGBA;
        wf = 4;
    } else if (flags & texture_mono) {
        format = GL_RED;
        wf = 1;
    }

    u8* dst = (u8*)mem_alloc(w * h * wf);
    if (flags & texture_flip) {
        for (u32 y = 0; y < h; y++) {
            for (u32 x = 0; x < w; x++) {
                if (!(flags & texture_mono)) {
                    dst[((h - y - 1) * w + x) * wf + 0] = src[(y * w + x) * wf + 0];
                    dst[((h - y - 1) * w + x) * wf + 1] = src[(y * w + x) * wf + 1];
                    dst[((h - y - 1) * w + x) * wf + 2] = src[(y * w + x) * wf + 2];
                }

                if (flags & texture_rgba) {
                    dst[((h - y - 1) * w + x) * wf + 3] = src[(y * w + x) * wf + 3];
                } else if (flags & texture_mono) {
                    dst[((h - y - 1) * w + x) * wf] = src[(y * w + x) * wf];
                }
            }
        }
    } else {
        memcpy(dst, src, w * h * wf);
    }

    if (!(flags & texture_mono)) {
        for (u32 i = 0; i < w * h * wf; i += wf) {
            u8 r = dst[i + 2];
            u8 g = dst[i + 1];
            u8 b = dst[i + 0];

            dst[i + 0] = r;
            dst[i + 1] = g;
            dst[i + 2] = b;
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, dst);

    texture->width = w;
    texture->height = h;

    mem_free(dst);
}

void update_texture(struct texture* texture, u8* data, u64 size, u32 flags) {
    assert(size > sizeof(struct bmp_header));

    if (*data != 'B' && *(data + 1) != 'M') {
        fprintf(stderr, "Not a valid bitmap!\n");
        return;
    }

    struct bmp_header* header = (struct bmp_header*)data;
    u8* src = data + header->bmp_offset;

    u32 mode = texture_rgba;
    if (header->bits_per_pixel == 24) {
        mode = texture_rgb;
    } else if (header->bits_per_pixel == 8) {
        mode = texture_mono;
    }

    update_texture_no_bmp(texture, src, header->w, header->h, flags | mode | texture_flip);
}

void update_texture_no_bmp(struct texture* texture, u8* src, u32 w, u32 h, u32 flags) {
    glBindTexture(GL_TEXTURE_2D, texture->id);

    GLenum format = GL_RGB;
    u32 wf = 3;
    if (flags & texture_rgba) {
        format = GL_RGBA;
        wf = 4;
    } else if (flags & texture_mono) {
        format = GL_RED;
        wf = 1;
    }

    u8* dst = (u8*)mem_alloc(w * h * wf);
    if (flags & texture_flip) {
        for (u32 y = 0; y < h; y++) {
            for (u32 x = 0; x < w; x++) {
                if (!(flags & texture_mono)) {
                    dst[((h - y - 1) * w + x) * wf + 0] = src[(y * w + x) * wf + 0];
                    dst[((h - y - 1) * w + x) * wf + 1] = src[(y * w + x) * wf + 1];
                    dst[((h - y - 1) * w + x) * wf + 2] = src[(y * w + x) * wf + 2];
                }

                if (flags & texture_rgba) {
                    dst[((h - y - 1) * w + x) * wf + 3] = src[(y * w + x) * wf + 3];
                } else if (flags & texture_mono) {
                    dst[((h - y - 1) * w + x) * wf + 0] = src[(y * w + x) * wf + 0];
                }
            }
        }
    } else {
        memcpy(dst, src, w * h * wf);
    }

    if (!(flags & texture_mono)) {
        for (u32 i = 0; i < w * h * wf; i += wf) {
            dst[i + 0] = src[i + 2];
            dst[i + 1] = src[i + 1];
            dst[i + 2] = src[i + 0];
        }
    }

    texture->width = w;
    texture->height = h;

    if (texture->width == w && texture->height == h) {
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, dst);
    } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, dst);
    }

    mem_free(dst);
}

void deinit_texture(struct texture* texture) { glDeleteTextures(1, &texture->id); }

void bind_texture(const struct texture* texture, u32 unit) {
    if (!texture) {
        glBindTexture(GL_TEXTURE_2D, 0);
        return;
    }

    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture->id);
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

#define MAX_GLYPHSET 256

Color256 make_color(u32 rgb, u8 alpha) { return Color256{(u8)((rgb >> 16) & 0xFF), (u8)((rgb >> 8) & 0xFF), (u8)(rgb & 0xff), alpha}; }

Renderer* new_renderer(AssetShader shader, vec2 dimentions) {
    Renderer* renderer = (Renderer*)mem_calloc(1, sizeof(Renderer));

    renderer->quad_count = 0;
    renderer->texture_count = 0;

    renderer->ambient_light = 1.0f;

    init_vb(&renderer->vb, vb_dynamic | vb_tris);
    bind_vb_for_edit(&renderer->vb);
    push_vertices(&renderer->vb, NULL, els_per_vert * verts_per_quad * batch_size);
    push_indices(&renderer->vb, NULL, indices_per_quad * batch_size);
    configure_vb(&renderer->vb, 0, 2, els_per_vert, 0);  /* vec2 position */
    configure_vb(&renderer->vb, 1, 2, els_per_vert, 2);  /* vec2 uv */
    configure_vb(&renderer->vb, 2, 4, els_per_vert, 4);  /* vec4 color */
    configure_vb(&renderer->vb, 3, 1, els_per_vert, 8);  /* f32 texture_id */
    configure_vb(&renderer->vb, 4, 1, els_per_vert, 9);  /* f32 inverted */
    configure_vb(&renderer->vb, 5, 1, els_per_vert, 10); /* f32 unlit */
    bind_vb_for_edit(NULL);

    renderer->clip_enable = false;
    renderer->camera_enable = false;

    renderer->shader = shader;
    neko_bind_shader(renderer->shader.id);

    renderer->camera = mat4_ortho(0.0f, (f32)dimentions.x, (f32)dimentions.y, 0.0f, -1.0f, 1.0f);
    neko_shader_set_m4f(renderer->shader.id, "camera", renderer->camera);
    renderer->dimentions = dimentions;

    neko_bind_shader(NULL);

    return renderer;
}

void free_renderer(Renderer* renderer) {
    deinit_vb(&renderer->vb);

    mem_free(renderer);
}

void renderer_flush(Renderer* renderer) {
    if (renderer->quad_count == 0) {
        return;
    }

    if (renderer->clip_enable) {
        draw_enable(vt_clip);
        draw_clip(rect_t{renderer->clip.x, (i32)renderer->dimentions.y - (renderer->clip.y + renderer->clip.h), renderer->clip.w, renderer->clip.h});
    } else {
        draw_disable(vt_clip);
    }

    neko_bind_shader(renderer->shader.id);

    for (u32 i = 0; i < renderer->texture_count; i++) {
        bind_texture(renderer->textures[i], i);

        char name[32];
        sprintf(name, "textures[%u]", i);

        neko_shader_set_int(renderer->shader.id, name, i);
    }

    for (u32 i = 0; i < renderer->light_count; i++) {
        char name[64];

        sprintf(name, "lights[%u].position", i);
        neko_shader_set_v2f(renderer->shader.id, name, renderer->lights[i].position);

        sprintf(name, "lights[%u].intensity", i);
        neko_shader_set_float(renderer->shader.id, name, renderer->lights[i].intensity);

        sprintf(name, "lights[%u].range", i);
        neko_shader_set_float(renderer->shader.id, name, renderer->lights[i].range);
    }

    neko_shader_set_int(renderer->shader.id, "light_count", renderer->light_count);

    neko_shader_set_m4f(renderer->shader.id, "camera", renderer->camera);
    neko_shader_set_float(renderer->shader.id, "ambient_light", renderer->ambient_light);

    if (renderer->camera_enable) {
        mat4 view =
                mat4_translate(mat4_identity(), neko_v3(-((f32)renderer->camera_pos.x) + ((f32)renderer->dimentions.x / 2), -((f32)renderer->camera_pos.y) + ((f32)renderer->dimentions.y / 2), 0.0f));
        neko_shader_set_m4f(renderer->shader.id, "view", view);
    } else {
        neko_shader_set_m4f(renderer->shader.id, "view", mat4_identity());
    }

    bind_vb_for_edit(&renderer->vb);
    update_vertices(&renderer->vb, renderer->verts, 0, renderer->quad_count * els_per_vert * verts_per_quad);
    update_indices(&renderer->vb, renderer->indices, 0, renderer->quad_count * indices_per_quad);
    bind_vb_for_edit(NULL);

    bind_vb_for_draw(&renderer->vb);
    draw_vb_n(&renderer->vb, renderer->quad_count * indices_per_quad);
    bind_vb_for_draw(NULL);
    neko_bind_shader(NULL);

    renderer->quad_count = 0;
    renderer->texture_count = 0;

    draw_disable(vt_clip);
}

void renderer_end_frame(Renderer* renderer) {
    renderer_flush(renderer);
    renderer->light_count = 0;
}

void renderer_push_light(Renderer* renderer, struct light light) {
    if (renderer->light_count > max_lights) {
        fprintf(stderr, "Too many lights! Max: %d\n", max_lights);
        return;
    }

    renderer->lights[renderer->light_count++] = light;
}

void renderer_push(Renderer* renderer, TexturedQuad* quad) {
    f32 tx = 0, ty = 0, tw = 0, th = 0;

    i32 tidx = -1;
    if (quad->texture) {
        for (u32 i = 0; i < renderer->texture_count; i++) {
            if (renderer->textures[i] == quad->texture) {
                tidx = (i32)i;
                break;
            }
        }

        if (tidx == -1) {
            renderer->textures[renderer->texture_count] = quad->texture;
            tidx = renderer->texture_count;

            renderer->texture_count++;

            if (renderer->texture_count >= 32) {
                renderer_flush(renderer);
                tidx = 0;
                renderer->textures[0] = quad->texture;
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

    const u32 idx_off = renderer->quad_count * verts_per_quad;

    u32 indices[] = {idx_off + 3, idx_off + 2, idx_off + 1, idx_off + 3, idx_off + 1, idx_off + 0};

    memcpy(renderer->verts + (renderer->quad_count * els_per_vert * verts_per_quad), verts, els_per_vert * verts_per_quad * sizeof(f32));
    memcpy(renderer->indices + (renderer->quad_count * indices_per_quad), indices, indices_per_quad * sizeof(u32));

    renderer->quad_count++;

    if (renderer->quad_count >= batch_size) {
        renderer_flush(renderer);
    }
}

void renderer_clip(Renderer* renderer, rect_t clip) {
    if (renderer->clip.x != clip.x || renderer->clip.y != clip.y || renderer->clip.w != clip.w || renderer->clip.h != clip.h) {
        renderer_flush(renderer);
        renderer->clip = clip;
    }
}

void renderer_resize(Renderer* renderer, vec2 size) {
    renderer->dimentions = size;
    renderer->camera = mat4_ortho(0.0f, (f32)size.x, (f32)size.y, 0.0f, -1.0f, 1.0f);
}

void renderer_fit_to_main_window(Renderer* renderer) {
    i32 win_w, win_h;
    query_window(0, &win_w, &win_h);

    renderer_resize(renderer, neko_v2(win_w, win_h));
}

PostProcessor* new_post_processor(AssetShader shader) {
    PostProcessor* p = (PostProcessor*)mem_calloc(1, sizeof(PostProcessor));

    i32 win_w, win_h;
    query_window(0, &win_w, &win_h);

    init_render_target(&p->target, win_w, win_h);

    p->dimentions = neko_v2(win_w, win_h);

    p->shader = shader;

    f32 verts[] = {-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f};

    u32 indices[] = {3, 2, 1, 3, 1, 0};

    init_vb(&p->vb, vb_static | vb_tris);
    bind_vb_for_edit(&p->vb);
    push_vertices(&p->vb, verts, 4 * 4);
    push_indices(&p->vb, indices, 6);
    configure_vb(&p->vb, 0, 2, 4, 0);
    configure_vb(&p->vb, 1, 2, 4, 2);
    bind_vb_for_edit(NULL);

    return p;
}

void free_post_processor(PostProcessor* p) {
    deinit_render_target(&p->target);
    deinit_vb(&p->vb);

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

void post_processor_fit_to_main_window(PostProcessor* p) {
    i32 win_w, win_h;
    query_window(0, &win_w, &win_h);

    resize_post_processor(p, neko_v2(win_w, win_h));
}

void flush_post_processor(PostProcessor* p, bool default_rt) {
    if (default_rt) {
        bind_render_target(NULL);
    }

    neko_bind_shader(p->shader.id);
    neko_shader_set_int(p->shader.id, "input", 0);
    neko_shader_set_v2f(p->shader.id, "screen_size", neko_v2(p->dimentions.x, p->dimentions.y));

    bind_render_target_output(&p->target, 0);

    bind_vb_for_draw(&p->vb);
    draw_vb(&p->vb);
    bind_vb_for_draw(NULL);

    neko_bind_shader(NULL);
}

struct glyph_set {
    struct texture atlas;
    stbtt_bakedchar glyphs[256];
};

struct font {
    void* data;
    stbtt_fontinfo info;
    struct glyph_set* sets[MAX_GLYPHSET];
    f32 size;
    i32 height;
};

static const char* utf8_to_codepoint(const char* p, u32* dst) {
    u32 res, n;
    switch (*p & 0xf0) {
        case 0xf0:
            res = *p & 0x07;
            n = 3;
            break;
        case 0xe0:
            res = *p & 0x0f;
            n = 2;
            break;
        case 0xd0:
        case 0xc0:
            res = *p & 0x1f;
            n = 1;
            break;
        default:
            res = *p;
            n = 0;
            break;
    }
    while (n--) {
        res = (res << 6) | (*(++p) & 0x3f);
    }
    *dst = res;
    return p + 1;
}

static struct glyph_set* load_glyph_set(struct font* font, i32 idx) {
    i32 width, height, r, ascent, descent, linegap, scaled_ascent, i;
    unsigned char n;
    f32 scale, s;
    struct glyph_set* set;

    set = (struct glyph_set*)mem_calloc(1, sizeof(struct glyph_set));

    width = 128;
    height = 128;

    Color256* pixels;

retry:
    pixels = (Color256*)mem_alloc(width * height * 4);

    s = stbtt_ScaleForMappingEmToPixels(&font->info, 1) / stbtt_ScaleForPixelHeight(&font->info, 1);
    r = stbtt_BakeFontBitmap((u8*)font->data, 0, font->size * s, (u8*)pixels, width, height, idx * 256, 256, set->glyphs);

    if (r <= 0) {
        width *= 2;
        height *= 2;
        mem_free(pixels);
        goto retry;
    }

    stbtt_GetFontVMetrics(&font->info, &ascent, &descent, &linegap);
    scale = stbtt_ScaleForMappingEmToPixels(&font->info, font->size);
    scaled_ascent = (i32)(ascent * scale + 0.5);
    for (i = 0; i < 256; i++) {
        set->glyphs[i].yoff += scaled_ascent;
        set->glyphs[i].xadvance = (f32)floor(set->glyphs[i].xadvance);
    }

    for (i = width * height - 1; i >= 0; i--) {
        n = *((u8*)pixels + i);
        pixels[i] = Color256{255, 255, 255, n};
    }

    init_texture_no_bmp(&set->atlas, (u8*)pixels, width, height, sprite_texture | texture_rgba);

    mem_free(pixels);

    return set;
}

static struct glyph_set* get_glyph_set(struct font* font, i32 code_poi32) {
    i32 idx;

    idx = (code_poi32 >> 8) % MAX_GLYPHSET;
    if (!font->sets[idx]) {
        font->sets[idx] = load_glyph_set(font, idx);
    }
    return font->sets[idx];
}

struct font* load_font_from_memory(void* data, u64 filesize, f32 size) {
    struct font* font;
    i32 r, ascent, descent, linegap;
    f32 scale;

    font = (struct font*)mem_calloc(1, sizeof(struct font));
    font->data = data;

    font->size = size;

    r = stbtt_InitFont(&font->info, (u8*)font->data, 0);
    if (!r) {
        goto fail;
    }

    stbtt_GetFontVMetrics(&font->info, &ascent, &descent, &linegap);
    scale = stbtt_ScaleForMappingEmToPixels(&font->info, size);
    font->height = (i32)((ascent - descent + linegap) * scale + 0.5);

    stbtt_bakedchar* g = get_glyph_set(font, '\n')->glyphs;
    g['\t'].x1 = g['\t'].x0;
    g['\n'].x1 = g['\n'].x0;

    set_font_tab_size(font, 8);

    return font;

fail:
    if (font) {
        if (font->data) {
            mem_free(font->data);
        }
        mem_free(font);
    }
    return NULL;
}

void free_font(struct font* font) {
    i32 i;
    struct glyph_set* set;

    for (i = 0; i < MAX_GLYPHSET; i++) {
        set = font->sets[i];
        if (set) {
            deinit_texture(&set->atlas);
            mem_free(set);
        }
    }

    mem_free(font->data);
    mem_free(font);
}

void set_font_tab_size(struct font* font, i32 n) {
    struct glyph_set* set;

    set = get_glyph_set(font, '\t');
    set->glyphs['\t'].xadvance = n * set->glyphs[' '].xadvance;
}

i32 get_font_tab_size(struct font* font) {
    struct glyph_set* set;

    set = get_glyph_set(font, '\t');
    return (i32)(set->glyphs['\t'].xadvance / set->glyphs[' '].xadvance);
}

f32 get_font_size(struct font* font) { return font->size; }

i32 font_height(struct font* font) { return font->height; }

i32 text_width(struct font* font, const char* text) {
    i32 x;
    u32 codepoint;
    const char* p;
    struct glyph_set* set;
    stbtt_bakedchar* g;

    x = 0;
    p = text;
    while (*p) {
        p = utf8_to_codepoint(p, &codepoint);

        if (*p == '\n') {
            x = 0;
        }

        set = get_glyph_set(font, codepoint);
        g = &set->glyphs[codepoint & 0xff];
        x += (i32)g->xadvance;
    }
    return x;
}

i32 char_width(struct font* font, char c) {
    char p[2];
    p[0] = c;

    u32 codepoint;
    utf8_to_codepoint(p, &codepoint);

    struct glyph_set* set = get_glyph_set(font, codepoint);
    stbtt_bakedchar* g = &set->glyphs[codepoint & 0xff];
    return g->xadvance;
}

i32 text_height(struct font* font, const char* text) {
    i32 height = font->height;

    for (const char* c = text; *c; c++) {
        if (*c == '\n') {
            height += font->height;
        }
    }

    return height;
}

i32 text_width_n(struct font* font, const char* text, u32 n) {
    i32 x;
    u32 codepoint;
    const char* p;
    struct glyph_set* set;
    stbtt_bakedchar* g;

    x = 0;
    p = text;
    u32 i = 0;
    while (*p && i < n) {
        p = utf8_to_codepoint(p, &codepoint);

        if (*p == '\n') {
            x = 0;
        }

        set = get_glyph_set(font, codepoint);
        g = &set->glyphs[codepoint & 0xff];
        x += (i32)g->xadvance;
        i++;
    }
    return x;
}

i32 text_height_n(struct font* font, const char* text, u32 n) {
    i32 height = font->height;

    for (u32 i = 0; i < n; i++) {
        if (text[i] == '\n') {
            height += font->height;
        }
    }

    return height;
}

i32 render_text(Renderer* renderer, struct font* font, const char* text, f32 x, f32 y, Color256 color) {
    const char* p;
    u32 codepoint;
    struct glyph_set* set;
    stbtt_bakedchar* g;
    i32 ori_x = x;

    p = text;
    while (*p) {
        if (*p == '\n') {
            x = ori_x;
            y += font->height;

            p++;
            continue;
        }

        p = utf8_to_codepoint(p, &codepoint);

        set = get_glyph_set(font, codepoint);
        g = &set->glyphs[codepoint & 0xff];

        f32 w = g->x1 - g->x0;
        f32 h = g->y1 - g->y0;

        TexturedQuad quad = {.texture = &set->atlas, .position = {x + g->xoff, y + g->yoff}, .dimentions = {w, h}, .rect = {(f32)g->x0, (f32)g->y0, w, h}, .color = color, .unlit = true};

        renderer_push(renderer, &quad);
        x += (i32)g->xadvance;
    }

    return x;
}

i32 render_text_n(Renderer* renderer, struct font* font, const char* text, u32 n, f32 x, f32 y, Color256 color) {
    const char* p;
    u32 codepoint;
    struct glyph_set* set;
    stbtt_bakedchar* g;
    i32 ori_x = x;

    p = text;
    for (u32 i = 0; i < n && *p; i++) {
        if (*p == '\n') {
            x = ori_x;
            y += font->height;

            p++;
            continue;
        }

        p = utf8_to_codepoint(p, &codepoint);

        set = get_glyph_set(font, codepoint);
        g = &set->glyphs[codepoint & 0xff];

        f32 w = g->x1 - g->x0;
        f32 h = g->y1 - g->y0;

        TexturedQuad quad = {.texture = &set->atlas, .position = {x + g->xoff, y + g->yoff}, .dimentions = {w, h}, .rect = {(f32)g->x0, (f32)g->y0, w, h}, .color = color, .unlit = true};

        renderer_push(renderer, &quad);
        x += (i32)g->xadvance;
    }

    return x;
}

i32 render_text_fancy(Renderer* renderer, struct font* font, const char* text, u32 n, f32 x, f32 y, Color256 color, TexturedQuad* coin) {
    const char* p;
    u32 codepoint;
    struct glyph_set* set;
    stbtt_bakedchar* g;

    p = text;
    for (u32 i = 0; i < n && *p; i++) {
        if (*p == '%' && *(p + 1) == 'c') {
            p += 2;

            coin->position.x = x;
            coin->position.y = y;
            coin->color = color;
            renderer_push(renderer, coin);

            x += coin->dimentions.x;
        } else {
            p = utf8_to_codepoint(p, &codepoint);

            set = get_glyph_set(font, codepoint);
            g = &set->glyphs[codepoint & 0xff];

            f32 w = g->x1 - g->x0;
            f32 h = g->y1 - g->y0;

            TexturedQuad quad = {.texture = &set->atlas, .position = {x + g->xoff, y + g->yoff}, .dimentions = {w, h}, .rect = {(f32)g->x0, (f32)g->y0, w, h}, .color = color};

            renderer_push(renderer, &quad);
            x += (i32)g->xadvance;
        }
    }

    return x;
}

char* word_wrap(struct font* font, char* buffer, const char* string, i32 width) {
    i32 i = 0;

    u32 string_len = (u32)strlen(string);
    i32 line_start = 0;

    while (i < string_len) {
        for (i32 c = 1; c < width - 8; c += char_width(font, string[i])) {
            if (i >= string_len) {
                buffer[i] = '\0';
                return buffer;
            }

            buffer[i] = string[i];

            if (string[i] == '\n') {
                line_start = i;
                c = 1;
            }

            i++;
        }

        if (isspace(string[i])) {
            buffer[i++] = '\n';
            line_start = i;
        } else {
            for (i32 c = i; c > 0; c--) {
                if (isspace(string[c])) {
                    buffer[c] = '\n';
                    i = c + 1;
                    line_start = i;
                    break;
                }

                if (c <= line_start) {
                    buffer[i++] = '\n';
                    break;
                }
            }
        }
    }

    buffer[i] = '\0';

    return buffer;
}
