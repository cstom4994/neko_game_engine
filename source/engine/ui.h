#pragma once

#include "engine/font.h"
#include "engine/gfx.h"
#include "engine/prelude.h"
#include "vendor/ui.h"

#ifdef __cplusplus
#define ui_widths(...)                            \
    [&]() -> const i32* {                         \
        static i32 temp_widths[] = {__VA_ARGS__}; \
        return temp_widths;                       \
    }()
#else
#define ui_widths(...) \
    (int[]) { __VA_ARGS__ }
#endif

#define ui_labelf(STR, ...)                              \
    do {                                                 \
        neko_snprintfc(BUFFER, 256, STR, ##__VA_ARGS__); \
        ui_label(g_app->ui, BUFFER);                     \
    } while (0)

inline void ui_push_id(ui_context_t* ctx, String str) { ui_push_id(ctx, str.data, str.len); }

inline void ui_text_colored(ui_context_t* ctx, const char* text, Color256 col) {
    Color256 save_col = ctx->style->colors[UI_COLOR_TEXT];
    ctx->style->colors[UI_COLOR_TEXT] = col;
    ui_text(ctx, text);
    ctx->style->colors[UI_COLOR_TEXT] = save_col;
}

typedef struct ui_context_t ui_context_t;

// typedef struct neko_font_t neko_font_t;
typedef struct neko_glyph_set_t neko_glyph_set_t;

// neko_glyph_set_t* neko_get_glyph_set(neko_font_t* font, i32 code_point);

// void neko_set_font_tab_size(neko_font_t* font, i32 n);
// i32 neko_get_font_tab_size(neko_font_t* font);

typedef ui_Container ui_container_t;
typedef ui_Style ui_style_t;
typedef ui_context_t ui_context_t;

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

    FontFamily* font;

    mat4 camera;
    u32 width, height;

    rect_t icon_rects[256];
    neko_texture_t* icon_texture;
} engine_ui_renderer_t;

engine_ui_renderer_t* neko_new_ui_renderer(u32 shader);
void neko_free_ui_renderer(engine_ui_renderer_t* renderer);
void engine_ui_renderer_push_quad(engine_ui_renderer_t* renderer, rect_t dst, rect_t src, neko_color_t color, float transparency, u32 mode);
void neko_begin_ui_renderer(engine_ui_renderer_t* renderer, u32 width, u32 height);
void neko_end_ui_renderer(engine_ui_renderer_t* renderer);
void neko_flush_ui_renderer(engine_ui_renderer_t* renderer);
void neko_set_ui_renderer_clip(engine_ui_renderer_t* renderer, rect_t rect);
float engine_ui_renderer_draw_text(engine_ui_renderer_t* renderer, const char* text, vec2 position, neko_color_t color, float transparency);
void engine_ui_renderer_draw_rect(engine_ui_renderer_t* renderer, rect_t rect, neko_color_t color, float transparency);
void engine_ui_renderer_draw_icon(engine_ui_renderer_t* renderer, u32 id, rect_t rect, neko_color_t color, float transparency);
float engine_ui_text_width(engine_ui_renderer_t* rendere, const char* text);
float engine_ui_tect_height(engine_ui_renderer_t* renderer);

void neko_init_ui_renderer(u32 shader);
void neko_deinit_ui_renderer();
i32 neko_ui_text_width(ui_font font, const char* test, i32 len);
i32 neko_ui_text_height(ui_font font);
void neko_update_ui(ui_context_t* context);
void neko_render_ui(ui_context_t* context, u32 width, u32 height);
engine_ui_renderer_t* neko_get_ui_renderer();
