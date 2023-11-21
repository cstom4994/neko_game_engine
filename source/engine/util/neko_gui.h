#ifndef NEKO_GUI_H
#define NEKO_GUI_H

#include "engine/neko_engine.h"
#include "engine/util/neko_asset.h"

// gui implement
#include "engine/builtin/neko_gui_impl.h"

#ifndef NEKO_GUI_TEXT_MAX
#define NEKO_GUI_TEXT_MAX 256
#endif

#define NEKO_GUI_MAX_VERTEX_BUFFER 512 * 1024
#define NEKO_GUI_MAX_INDEX_BUFFER 128 * 1024

typedef enum neko_gui_init_state { NEKO_GUI_STATE_DEFAULT = 0x00, NEKO_GUI_INSTALL_CALLBACKS } neko_gui_init_state;

typedef struct neko_gui_ctx_t {
    struct neko_gui_context neko_gui_ctx;
    struct neko_gui_font_atlas* atlas;
    struct neko_gui_vec2 fb_scale;
    uint32_t text[NEKO_GUI_TEXT_MAX];
    int32_t text_len;
    struct neko_gui_vec2 scroll;
    double last_button_click;
    int32_t is_double_click_down;
    struct neko_gui_vec2 double_click_pos;
    struct neko_gui_buffer cmds;
    struct neko_gui_draw_null_texture null;
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
    bool mouse_is_hover;
    neko_nbt_tag_t* gui_layout_nbt_tags;
} neko_gui_ctx_t;

NEKO_API_DECL struct neko_gui_context* neko_gui_init(neko_gui_ctx_t* neko, uint32_t win_hndl, enum neko_gui_init_state init_state);
NEKO_API_DECL void neko_gui_new_frame(neko_gui_ctx_t* neko);
NEKO_API_DECL void neko_gui_render(neko_gui_ctx_t* neko, neko_command_buffer_t* cb, enum neko_gui_anti_aliasing AA);
NEKO_API_DECL void neko_gui_device_upload_atlas(neko_gui_ctx_t* neko, const void* image, int32_t width, int32_t height);
NEKO_API_DECL void neko_gui_device_create(neko_gui_ctx_t* neko);

NEKO_API_DECL void neko_gui_font_stash_begin(struct neko_gui_ctx_t* neko, struct neko_gui_font_atlas** atlas);
NEKO_API_DECL void neko_gui_font_stash_end(struct neko_gui_ctx_t* neko);

NEKO_GUI_INTERN void neko_gui_clipboard_paste(neko_gui_handle usr, struct neko_gui_text_edit* edit);
NEKO_GUI_INTERN void neko_gui_clipboard_copy(neko_gui_handle usr, const char* text, int32_t len);

NEKO_API_DECL struct neko_gui_rect neko_gui_layout_get_bounds(neko_gui_ctx_t* neko_nui, const char* name);
NEKO_API_DECL struct neko_gui_rect neko_gui_layout_get_bounds_ex(neko_gui_ctx_t* neko_nui, const char* name, struct neko_gui_rect default_bounds);
NEKO_API_DECL void neko_gui_layout_save(neko_gui_ctx_t* neko_nui);

enum neko_gui_style_theme { THEME_BLACK, THEME_WHITE, THEME_RED, THEME_BLUE, THEME_DARK };

NEKO_API_DECL void set_style(struct neko_gui_context* ctx, enum neko_gui_style_theme theme);

NEKO_API_DECL int neko_gui_overview(struct neko_gui_context* ctx);

#endif  // NEKO_GUI_H
