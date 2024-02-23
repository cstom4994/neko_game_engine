#ifndef NEKO_BINDING_ENGINE_H
#define NEKO_BINDING_ENGINE_H

#include "engine/neko.hpp"
#include "engine/neko_engine.h"
#include "engine/util/neko_gui.h"
#include "engine/util/neko_idraw.h"
#include "engine/util/neko_imgui.h"
#include "engine/util/neko_lua.hpp"

typedef struct neko_client_userdata_s {
    neko_command_buffer_t *cb;
    neko_immediate_draw_t *idraw;
    neko_immediate_draw_static_data_t *idraw_sd;
    neko_core_ui_context_t *igui;
    neko_gui_ctx_t *nui;
    neko_graphics_custom_batch_context_t *sprite_batch;

    lua_State *L;

    neko_font_t *test_font_bmfont;

    struct {
        // 热加载模块
        u32 module_count;
        void (*module_func[16])(void);
    } vm;
} neko_client_userdata_t;

// TODO:
neko_string game_assets(const neko_string &path);
void draw_text(neko_font_t *font, const char *text, float x, float y, float line_height, float clip_region, float wrap_x, f32 scale);

void neko_register(lua_State *L);

#endif