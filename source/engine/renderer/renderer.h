#pragma once

#include "base/common/base.hpp"
#include "engine/graphics.h"
#include "base/common/color.hpp"
#include "base/common/math.hpp"

#define max_lights 100

enum { vt_clip = 0, vt_depth_test };

NEKO_API() void draw_enable(u32 thing);
NEKO_API() void draw_disable(u32 thing);
NEKO_API() void draw_clip(rect_t rect);

enum { vb_static = 1 << 0, vb_dynamic = 1 << 1, vb_lines = 1 << 2, vb_line_strip = 1 << 3, vb_tris = 1 << 4 };

struct VertexBuffer {
    u32 va_id;
    u32 vb_id;
    u32 ib_id;
    u32 index_count;

    i32 flags;
};

NEKO_API() void init_vb(VertexBuffer* vb, const i32 flags);
NEKO_API() void deinit_vb(VertexBuffer* vb);
NEKO_API() void bind_vb_for_draw(const VertexBuffer* vb);
NEKO_API() void bind_vb_for_edit(const VertexBuffer* vb);
NEKO_API() void push_vertices(const VertexBuffer* vb, f32* vertices, u32 count);
NEKO_API() void push_indices(VertexBuffer* vb, u32* indices, u32 count);
NEKO_API() void update_vertices(const VertexBuffer* vb, f32* vertices, u32 offset, u32 count);
NEKO_API() void update_indices(VertexBuffer* vb, u32* indices, u32 offset, u32 count);
NEKO_API() void configure_vb(const VertexBuffer* vb, u32 index, u32 component_count, u32 stride, u32 offset);
NEKO_API() void draw_vb(const VertexBuffer* vb);
NEKO_API() void draw_vb_n(const VertexBuffer* vb, u32 count);

enum {
    texture_filter_nearest = 1 << 0,
    texture_filter_linear = 1 << 2,
    texture_repeat = 1 << 3,
    texture_clamp = 1 << 4,
    texture_mono = 1 << 5,
    texture_rgb = 1 << 6,
    texture_rgba = 1 << 7,
    texture_flip = 1 << 8
};

#define sprite_texture (texture_filter_nearest | texture_clamp)

struct texture {
    u32 id;
    u32 width, height;
};

NEKO_API() void init_texture(struct texture* texture, u8* src, u64 size, u32 flags);
NEKO_API() void init_texture_no_bmp(struct texture* texture, u8* src, u32 w, u32 h, u32 flags);
NEKO_API() void update_texture(struct texture* texture, u8* data, u64 size, u32 flags);
NEKO_API() void update_texture_no_bmp(struct texture* texture, u8* src, u32 w, u32 h, u32 flags);
NEKO_API() void deinit_texture(struct texture* texture);
NEKO_API() void bind_texture(const struct texture* texture, u32 unit);

struct RenderTarget {
    u32 id;
    u32 width, height;

    u32 output;
};

NEKO_API() void init_render_target(RenderTarget* target, u32 width, u32 height);
NEKO_API() void deinit_render_target(RenderTarget* target);
NEKO_API() void resize_render_target(RenderTarget* target, u32 width, u32 height);
NEKO_API() void bind_render_target(RenderTarget* target);
NEKO_API() void bind_render_target_output(RenderTarget* target, u32 unit);

NEKO_API() Color256 make_color(u32 rgb, u8 alpha);

struct TexturedQuad {
    struct texture* texture;
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

struct Renderer {
    AssetShader shader;
    VertexBuffer vb;

    u32 quad_count;

    struct texture* textures[32];
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

NEKO_API() Renderer* new_renderer(AssetShader shader, vec2 dimentions);
NEKO_API() void free_renderer(Renderer* renderer);
NEKO_API() void renderer_flush(Renderer* renderer);
NEKO_API() void renderer_end_frame(Renderer* renderer);
NEKO_API() void renderer_push(Renderer* renderer, TexturedQuad* quad);
NEKO_API() void renderer_push_light(Renderer* renderer, struct light light);
NEKO_API() void renderer_clip(Renderer* renderer, rect_t clip);
NEKO_API() void renderer_resize(Renderer* renderer, vec2 size);
NEKO_API() void renderer_fit_to_main_window(Renderer* renderer);

struct PostProcessor {
    RenderTarget target;

    AssetShader shader;
    VertexBuffer vb;

    vec2 dimentions;
};

NEKO_API() PostProcessor* new_post_processor(AssetShader shader);
NEKO_API() void free_post_processor(PostProcessor* p);
NEKO_API() void use_post_processor(PostProcessor* p);
NEKO_API() void resize_post_processor(PostProcessor* p, vec2 dimentions);
NEKO_API() void post_processor_fit_to_main_window(PostProcessor* p);
NEKO_API() void flush_post_processor(PostProcessor* p, bool default_rt);

struct font;

NEKO_API() i32 render_text(Renderer* renderer, struct font* font, const char* text, f32 x, f32 y, Color256 color);

NEKO_API() i32 render_text_n(Renderer* renderer, struct font* font, const char* text, u32 n, f32 x, f32 y, Color256 color);

NEKO_API() i32 render_text_fancy(Renderer* renderer, struct font* font, const char* text, u32 n, f32 x, f32 y, Color256 color, TexturedQuad* coin);

NEKO_API() struct font* load_font_from_memory(void* data, u64 filesize, f32 size);
NEKO_API() void free_font(struct font* font);

NEKO_API() void set_font_tab_size(struct font* font, i32 n);
NEKO_API() i32 get_font_tab_size(struct font* font);

NEKO_API() f32 get_font_size(struct font* font);

NEKO_API() i32 font_height(struct font* font);

NEKO_API() i32 text_width(struct font* font, const char* text);
NEKO_API() i32 text_height(struct font* font, const char* text);
NEKO_API() i32 char_width(struct font* font, char c);
NEKO_API() i32 text_width_n(struct font* font, const char* text, u32 n);
NEKO_API() i32 text_height_n(struct font* font, const char* text, u32 n);

NEKO_API() char* word_wrap(struct font* font, char* buffer, const char* string, i32 width);
