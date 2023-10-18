#ifndef NEKO_NUI_H
#define NEKO_NUI_H

#include "engine/neko_engine.h"
#include "libs/nuklear.h"

#ifndef NEKO_NUI_TEXT_MAX
#define NEKO_NUI_TEXT_MAX 256
#endif

#define NEKO_NUI_MAX_VERTEX_BUFFER 512 * 1024
#define NEKO_NUI_MAX_INDEX_BUFFER 128 * 1024

typedef enum neko_nui_init_state { NEKO_NUI_STATE_DEFAULT = 0x00, NEKO_NUI_INSTALL_CALLBACKS } neko_nui_init_state;

typedef struct neko_nui_ctx_t {
    struct neko_nui_context neko_nui_ctx;
    struct neko_nui_font_atlas* atlas;
    struct neko_nui_vec2 fb_scale;
    uint32_t text[NEKO_NUI_TEXT_MAX];
    int32_t text_len;
    struct neko_nui_vec2 scroll;
    double last_button_click;
    int32_t is_double_click_down;
    struct neko_nui_vec2 double_click_pos;
    struct neko_nui_buffer cmds;
    struct neko_nui_draw_null_texture null;
    void* tmp_vertex_data;
    void* tmp_index_data;
    neko_handle(neko_graphics_pipeline_t) pip;
    neko_handle(neko_graphics_vertex_buffer_t) vbo;
    neko_handle(neko_graphics_index_buffer_t) ibo;
    neko_handle(neko_graphics_shader_t) shader;
    neko_handle(neko_graphics_texture_t) font_tex;
    neko_handle(neko_graphics_uniform_t) u_tex;
    neko_handle(neko_graphics_uniform_t) u_proj;
    uint32_t window_hndl;
    int32_t width, height;
    int32_t display_width, display_height;
} neko_nui_ctx_t;

NEKO_API_DECL struct neko_nui_context* neko_nui_init(neko_nui_ctx_t* neko, uint32_t win_hndl, enum neko_nui_init_state init_state);
NEKO_API_DECL void neko_nui_new_frame(neko_nui_ctx_t* neko);
NEKO_API_DECL void neko_nui_render(neko_nui_ctx_t* neko, neko_command_buffer_t* cb, enum neko_nui_anti_aliasing AA);
NEKO_API_DECL void neko_nui_device_upload_atlas(neko_nui_ctx_t* neko, const void* image, int32_t width, int32_t height);
NEKO_API_DECL void neko_nui_device_create(neko_nui_ctx_t* neko);

NEKO_API_DECL void neko_nui_font_stash_begin(struct neko_nui_ctx_t* neko, struct neko_nui_font_atlas** atlas);
NEKO_API_DECL void neko_nui_font_stash_end(struct neko_nui_ctx_t* neko);

NEKO_NUI_INTERN void neko_nui_clipboard_paste(neko_nui_handle usr, struct neko_nui_text_edit* edit);
NEKO_NUI_INTERN void neko_nui_clipboard_copy(neko_nui_handle usr, const char* text, int32_t len);

NEKO_API_DECL void set_style(struct neko_nui_context* ctx, enum neko_nui_style_theme theme);

NEKO_API_DECL int neko_nui_overview(struct neko_nui_context* ctx);

#endif  // NEKO_NUI_H
