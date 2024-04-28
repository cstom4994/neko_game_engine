#ifndef NEKO_GAME_PIXELUI_H
#define NEKO_GAME_PIXELUI_H

#include "engine/neko.h"
#include "engine/neko_engine.h"

#define window_width 1280
#define window_height 740

typedef neko_color_t cell_color_t;

typedef u32 mat_id_t;

// For now, all particle information will simply be a value to determine its material mat_id
#define mat_id_empty (mat_id_t)0
#define mat_id_sand (mat_id_t)1
#define mat_id_water (mat_id_t)2
#define mat_id_salt (mat_id_t)3
#define mat_id_wood (mat_id_t)4
#define mat_id_fire (mat_id_t)5
#define mat_id_smoke (mat_id_t)6
#define mat_id_ember (mat_id_t)7
#define mat_id_steam (mat_id_t)8
#define mat_id_gunpowder (mat_id_t)9
#define mat_id_oil (mat_id_t)10
#define mat_id_lava (mat_id_t)11
#define mat_id_stone (mat_id_t)12
#define mat_id_acid (mat_id_t)13

// Colors
#define mat_col_empty \
    cell_color_t { 0, 0, 0, 0 }
#define mat_col_sand \
    cell_color_t { 150, 100, 50, 255 }
#define mat_col_salt \
    cell_color_t { 200, 180, 190, 255 }
#define mat_col_water \
    cell_color_t { 20, 100, 170, 200 }
#define mat_col_stone \
    cell_color_t { 120, 110, 120, 255 }
#define mat_col_wood \
    cell_color_t { 60, 40, 20, 255 }
#define mat_col_fire \
    cell_color_t { 150, 20, 0, 255 }
#define mat_col_smoke \
    cell_color_t { 50, 50, 50, 255 }
#define mat_col_ember \
    cell_color_t { 200, 120, 20, 255 }
#define mat_col_steam \
    cell_color_t { 220, 220, 250, 255 }
#define mat_col_gunpowder \
    cell_color_t { 60, 60, 60, 255 }
#define mat_col_oil \
    cell_color_t { 80, 70, 60, 255 }
#define mat_col_lava \
    cell_color_t { 200, 50, 0, 255 }
#define mat_col_acid \
    cell_color_t { 90, 200, 60, 255 }

typedef enum material_selection {
    mat_sel_sand = 0x00,
    mat_sel_water,
    mat_sel_salt,
    mat_sel_wood,
    mat_sel_fire,
    mat_sel_smoke,
    mat_sel_steam,
    mat_sel_gunpowder,
    mat_sel_oil,
    mat_sel_lava,
    mat_sel_stone,
    mat_sel_acid
} material_selection;

struct pixelui_t {

    // UI texture buffer
    neko_color_t* ui_buffer = {0};

    neko_global const s32 g_pixelui_scale = 4;
    neko_global const s32 g_pixelui_texture_width = window_width / g_pixelui_scale;
    neko_global const s32 g_pixelui_texture_height = window_height / g_pixelui_scale;

    // Frame counter
    u32 frame_counter = 0;

    u64 update_time = 0;

    bool show_material_selection_panel = true;
    bool show_frame_count = true;

    // Material selection for "painting" / default to sand
    material_selection material_selection = mat_sel_sand;

    neko_texture_t tex_ui = {0};
};

bool update_ui(pixelui_t* pui);
void pixelui_init(pixelui_t* fallsand);
void pixelui_destroy(pixelui_t* fallsand);
void pixelui_draw(pixelui_t* pui);

#endif