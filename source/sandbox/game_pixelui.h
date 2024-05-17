#ifndef NEKO_GAME_PIXELUI_H
#define NEKO_GAME_PIXELUI_H

#include "engine/neko.h"
#include "engine/neko_engine.h"

// game
#include "magic_pixel.h"

#define window_width 1280
#define window_height 740

struct pixelui_t {

    // UI texture buffer
    neko_color_t* ui_buffer = {0};

    f32 brush_size = 0.3f;

    neko_global const s32 g_pixelui_scale = 4;
    neko_global const s32 g_pixelui_texture_width = window_width / g_pixelui_scale;
    neko_global const s32 g_pixelui_texture_height = window_height / g_pixelui_scale;

    // Frame counter
    u32 frame_counter = 0;

    u64 update_time = 0;

    bool show_material_selection_panel = true;
    bool show_frame_count = true;

    material_type material_selection = material_type::ROCK;

    neko_texture_t tex_ui = {0};
};

bool update_ui(pixelui_t* pui);
void pixelui_init(pixelui_t* fallsand);
void pixelui_destroy(pixelui_t* fallsand);
void pixelui_draw(pixelui_t* pui);

#endif