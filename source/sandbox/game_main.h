

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
#include "engine/neko_imgui.h"
#include "engine/neko_math.h"
#include "game_cvar.h"
#include "sound.h"

class sandbox_game;
class neko_filesystem_t;
struct game_font_render_batch;

typedef struct neko_client_userdata_s {

    neko_command_buffer_t cb = neko_default_val();
    neko_ui_context_t ui = neko_default_val();
    neko_ui_style_sheet_t style_sheet;
    neko_immediate_draw_t idraw = neko_default_val();
    neko_asset_ascii_font_t font;
    neko_imgui_context_t imgui = neko_default_val();
    neko_texture_t test_ase = neko_default_val();
    neko_asset_manager_t am = neko_default_val();
    neko_asset_t tex_hndl = neko_default_val();
    neko_packreader_t pack = neko_default_val();
    neko_packreader_t lua_pack = neko_default_val();

    // ecs_world_t* ecs_world;
    // neko_dyn_array(ecs_entity_t) entities;
    neko_ecs_t* ecs;

    neko_handle(neko_graphics_renderpass_t) main_rp = {0};
    neko_handle(neko_graphics_framebuffer_t) main_fbo = {0};
    neko_handle(neko_graphics_texture_t) main_rt = {0};

    lua_State* L;

    neko_fontbatch_t font_render_batch;

    neko_client_cvar_t cl_cvar = neko_default_val();

    neko_thread_atomic_int_t init_thread_flag;
    neko_thread_ptr_t init_work_thread;

    // neko_filesystem_t* assetsys;

    neko_vec2_t fbs = {640 * 1.5, 360 * 1.5};
    neko_vec2_t cam = {512, 512};

    u8 debug_mode;

    // sandbox_game* game = nullptr;

    f32 player_v = 100.f;

    neko_audio_engine audio_engine;

} neko_client_userdata_t;

extern neko_client_userdata_t g_client_userdata;

NEKO_INLINE neko_client_userdata_t* CL_GAME_USERDATA() { return &g_client_userdata; }

// TODO:
std::string game_assets(const std::string& path);
