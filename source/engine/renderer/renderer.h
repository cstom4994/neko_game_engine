#pragma once

#include "base/common/base.hpp"
#include "engine/graphics.h"
#include "base/common/color.hpp"
#include "base/common/math.hpp"
#include "base/common/singleton.hpp"
#include "engine/renderer/shader.h"

#define max_lights 100

enum { vt_clip = 0, vt_depth_test };

void draw_enable(u32 thing);
void draw_disable(u32 thing);
void draw_clip(rect_t rect);

enum { vb_static = 1 << 0, vb_dynamic = 1 << 1, vb_lines = 1 << 2, vb_line_strip = 1 << 3, vb_tris = 1 << 4 };

struct VertexBuffer {
    u32 va_id;
    u32 vb_id;
    u32 ib_id;
    u32 index_count;

    i32 flags;

    void init_vb(const i32 flags);
    void fini_vb();
    void bind_vb_for_draw(bool bind) const;
    void bind_vb_for_edit(bool bind) const;
    void push_vertices(f32* vertices, u32 count) const;
    void push_indices(u32* indices, u32 count);
    void update_vertices(f32* vertices, u32 offset, u32 count) const;
    void update_indices(u32* indices, u32 offset, u32 count);
    void configure_vb(u32 index, u32 component_count, u32 stride, u32 offset) const;
    void draw_vb() const;
    void draw_vb_n(u32 count) const;
};

struct RenderTarget {
    u32 id;
    u32 width, height;

    u32 output;
};

void init_render_target(RenderTarget* target, u32 width, u32 height);
void deinit_render_target(RenderTarget* target);
void resize_render_target(RenderTarget* target, u32 width, u32 height);
void bind_render_target(RenderTarget* target);
void bind_render_target_output(RenderTarget* target, u32 unit);

Color256 make_color(u32 rgb, u8 alpha);

struct AssetTexture;

struct TexturedQuad {
    AssetTexture* texture;
    vec2 position;
    vec2 dimentions;
    rect_t rect;
    Color256 color;

    bool inverted;
    bool unlit;

    vec2 origin;
    f32 rotation;
};

struct light {
    vec2 position;
    f32 range;
    f32 intensity;
};

struct QuadRenderer {
    AssetShader shader;
    VertexBuffer vb;

    u32 quad_count;

    AssetTexture* textures[32];
    u32 texture_count;

    bool clip_enable;
    bool camera_enable;

    rect_t clip;
    vec2 dimentions;

    vec2 camera_pos;
    mat4 camera;

    vec2 light_pos;

    f32 ambient_light;

    struct light lights[max_lights];
    u32 light_count;

    f32 verts[100 * 11 * 4];
    u32 indices[100 * 6];
};

QuadRenderer* new_renderer(AssetShader shader, vec2 dimentions);
void free_renderer(QuadRenderer* renderer);
void renderer_flush(QuadRenderer* renderer);
void renderer_end_frame(QuadRenderer* renderer);
void renderer_push(QuadRenderer* renderer, TexturedQuad* quad);
void renderer_push_light(QuadRenderer* renderer, struct light light);
void renderer_clip(QuadRenderer* renderer, rect_t clip);
void renderer_resize(QuadRenderer* renderer, vec2 size);
void renderer_fit_to_main_window(QuadRenderer* renderer);

struct PostProcessor {
    RenderTarget target;

    AssetShader shader;
    VertexBuffer vb;

    vec2 dimentions;
};

PostProcessor* new_post_processor(AssetShader shader);
void free_post_processor(PostProcessor* p);
void use_post_processor(PostProcessor* p);
void resize_post_processor(PostProcessor* p, vec2 dimentions);
void post_processor_fit_to_main_window(PostProcessor* p);
void flush_post_processor(PostProcessor* p, bool default_rt);

class Renderer : public Neko::SingletonClass<Renderer> {
public:
    void InitOpenGL();
};
