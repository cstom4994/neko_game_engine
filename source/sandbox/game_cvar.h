//
// Created by KaoruXun on 24-5-17.
//

#ifndef NEKO_ENGINE_GAME_CVAR_H
#define NEKO_ENGINE_GAME_CVAR_H

#include <random>

#include "engine/neko.hpp"
#include "engine/neko_engine.h"
#include "engine/neko_math.h"

extern u32 frame_count;
extern u32 current_tick;
extern u32 last_tick;
extern neko_color_t kEmptyPixelValue;
extern neko_vec2_t cursor;
extern thread_local std::mt19937 rng;

constexpr u32 g_sim_chunk_width = 512;
constexpr u32 g_sim_chunk_height = 512;
constexpr u32 g_sim_chunk_width_Power2Expoent = 9;
constexpr u32 kMaxCell = g_sim_chunk_width * g_sim_chunk_height;
constexpr int kMaxChunk = 64;
constexpr int g_chunk_p2 = 6;
constexpr int kChunkBatchSize = 16;
constexpr neko_vec4_t kViewportRect = {0, 0, g_sim_chunk_width, g_sim_chunk_height};
constexpr u16 kMinDrawRadius = 0;
constexpr u16 kMaxDrawRadius = 75;

#define CVAR_TYPES() bool, s32, f32, f32 *

typedef struct neko_engine_cvar_s {
    bool show_editor = false;
    bool show_demo_window = false;
    bool show_pack_editor = false;
    bool show_profiler_window = false;
    bool show_test_window = false;
    bool show_gui = false;
    bool shader_inspect = false;
    bool hello_ai_shit = false;
    bool vsync = true;
    bool is_hotfix = true;

    // 实验性功能开关
    bool enable_nekolua = false;

    f32 bg[3] = {28, 28, 28};
} neko_client_cvar_t;

void neko_cvar_gui(neko_client_cvar_t &cvar);

extern neko_console_command_t commands[];
extern neko_console_t g_console;

NEKO_API_DECL void neko_console(neko_console_t *console, neko_ui_context_t *ctx, neko_ui_rect_t screen, const neko_ui_selector_desc_t *desc);

#endif  // NEKO_ENGINE_GAME_CVAR_H
