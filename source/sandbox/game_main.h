

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

    Neko_ModuleInterface module_interface{};

    neko_imgui_context_t imgui = NEKO_DEFAULT_VAL();

    neko_handle(neko_render_renderpass_t) main_rp = {0};
    neko_handle(neko_render_framebuffer_t) main_fbo = {0};
    neko_handle(neko_render_texture_t) main_rt = {0};

    neko_fontbatch_t font_render_batch;

    neko_client_cvar_t cl_cvar = NEKO_DEFAULT_VAL();

    neko_thread_atomic_int_t init_thread_flag;
    neko_thread_ptr_t init_work_thread;

    neko_vec2_t cam = {512, 512};

    neko_vec2_t DisplaySize;
    neko_vec2_t DisplayFramebufferScale;

    u8 debug_mode;

    // sandbox_game* game = nullptr;

    f32 player_v = 100.f;

} neko_client_userdata_t;

extern neko_client_userdata_t g_client_userdata;

NEKO_INLINE neko_client_userdata_t* CL_GAME_USERDATA() { return &g_client_userdata; }
NEKO_INLINE Neko_ModuleInterface* CL_GAME_INTERFACE() { return &g_client_userdata.module_interface; }

// TODO:
std::string game_assets(const std::string& path);
