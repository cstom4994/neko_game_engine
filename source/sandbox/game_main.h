

#pragma once

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>

#include "engine/neko.hpp"
#include "engine/neko_ecs.h"
#include "engine/neko_engine.h"
#include "engine/neko_imgui.hpp"
#include "engine/neko_math.h"

class sandbox_game;
class neko_filesystem_t;
struct game_font_render_batch;

typedef struct neko_client_userdata_t {

    neko_command_buffer_t cb = NEKO_DEFAULT_VAL();
    neko_ui_context_t ui = NEKO_DEFAULT_VAL();
    neko_ui_style_sheet_t style_sheet;
    neko_immediate_draw_t idraw = NEKO_DEFAULT_VAL();
    neko_asset_ascii_font_t font;
    neko_imgui_context_t imgui = NEKO_DEFAULT_VAL();
    neko_texture_t test_ase = NEKO_DEFAULT_VAL();
    neko_asset_manager_t am = NEKO_DEFAULT_VAL();
    neko_asset_t tex_hndl = NEKO_DEFAULT_VAL();
    neko_packreader_t pack = NEKO_DEFAULT_VAL();
    neko_packreader_t lua_pack = NEKO_DEFAULT_VAL();

    // ecs_world_t* ecs_world;
    // neko_dyn_array(ecs_entity_t) entities;
    neko_ecs_t* ecs;

    neko_handle(neko_render_renderpass_t) main_rp = {0};
    neko_handle(neko_render_framebuffer_t) main_fbo = {0};
    neko_handle(neko_render_texture_t) main_rt = {0};

    neko_fontbatch_t font_render_batch;

    neko_client_cvar_t cl_cvar = NEKO_DEFAULT_VAL();

    neko_thread_atomic_int_t init_thread_flag;
    neko_thread_ptr_t init_work_thread;

    // neko_filesystem_t* assetsys;

    neko_vec2_t fbs = {640 * 1.5, 360 * 1.5};
    neko_vec2_t cam = {512, 512};

    u8 debug_mode;

    // sandbox_game* game = nullptr;

    f32 player_v = 100.f;

} neko_client_userdata_t;

extern neko_client_userdata_t g_client_userdata;

NEKO_INLINE neko_client_userdata_t* CL_GAME_USERDATA() { return &g_client_userdata; }

// TODO:
std::string game_assets(const std::string& path);
