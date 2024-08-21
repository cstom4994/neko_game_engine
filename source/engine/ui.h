#pragma once

#include "engine/gfx.h"
#include "engine/prelude.h"

extern "C" {
#include "vendor/ui.h"
}

typedef struct mu_Context mu_Context;

// typedef struct neko_font_t neko_font_t;
typedef struct neko_glyph_set_t neko_glyph_set_t;

// neko_glyph_set_t* neko_get_glyph_set(neko_font_t* font, i32 code_point);

// void neko_set_font_tab_size(neko_font_t* font, i32 n);
// i32 neko_get_font_tab_size(neko_font_t* font);

typedef struct neko_opengl_state_backup_t {
    bool blend;
    bool cull_face;
    bool depth_test;
    bool scissor_test;
} neko_opengl_state_backup_t;

typedef struct engine_ui_renderer_t {
    neko_opengl_state_backup_t backup;

    u32 quad_count;
    u32 draw_call_count;

    neko_vertex_buffer_t* vb;
    u32 shader;

    // neko_font_t* font;

    neko_m4f_t camera;
    u32 width, height;

    neko_rect_t icon_rects[256];
    neko_texture_t* icon_texture;
} engine_ui_renderer_t;

engine_ui_renderer_t* neko_new_ui_renderer(u32 shader);
void neko_free_ui_renderer(engine_ui_renderer_t* renderer);
void engine_ui_renderer_push_quad(engine_ui_renderer_t* renderer, neko_rect_t dst, neko_rect_t src, neko_color_t color, float transparency, u32 mode);
void neko_begin_ui_renderer(engine_ui_renderer_t* renderer, u32 width, u32 height);
void neko_end_ui_renderer(engine_ui_renderer_t* renderer);
void neko_flush_ui_renderer(engine_ui_renderer_t* renderer);
void neko_set_ui_renderer_clip(engine_ui_renderer_t* renderer, neko_rect_t rect);
float engine_ui_renderer_draw_text(engine_ui_renderer_t* renderer, const char* text, neko_v2f_t position, neko_color_t color, float transparency);
void engine_ui_renderer_draw_rect(engine_ui_renderer_t* renderer, neko_rect_t rect, neko_color_t color, float transparency);
void engine_ui_renderer_draw_icon(engine_ui_renderer_t* renderer, u32 id, neko_rect_t rect, neko_color_t color, float transparency);
float engine_ui_text_width(engine_ui_renderer_t* rendere, const char* text);
float engine_ui_tect_height(engine_ui_renderer_t* renderer);

void neko_init_ui_renderer(u32 shader);
void neko_deinit_ui_renderer();
i32 neko_ui_text_width(mu_Font font, const char* test, i32 len);
i32 neko_ui_text_height(mu_Font font);
void neko_update_ui(mu_Context* context);
void neko_render_ui(mu_Context* context, u32 width, u32 height);
engine_ui_renderer_t* neko_get_ui_renderer();
