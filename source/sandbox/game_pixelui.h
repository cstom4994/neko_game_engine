
#ifndef NEKO_GAME_PIXELUI_H
#define NEKO_GAME_PIXELUI_H

#include "engine/neko.h"
#include "engine/neko_engine.h"

#define NEKO_BUILTIN_IMPL
#include "old/neko_builtin_font.h"

neko_global const s32 g_pixelui_scale = 4;
neko_global const s32 g_pixelui_texture_width = g_window_width / g_pixelui_scale;
neko_global const s32 g_pixelui_texture_height = g_window_height / g_pixelui_scale;

s32 pixelui_compute_idx(s32 x, s32 y) { return (y * g_pixelui_texture_width + x); }

bool pixelui_in_bounds(s32 x, s32 y) {
    if (x < 0 || x > (g_pixelui_texture_width - 1) || y < 0 || y > (g_pixelui_texture_height - 1)) return false;
    return true;
}

void putpixel(s32 x, s32 y) {
    if (pixelui_in_bounds(x, y)) {
        g_fallsand.ui_buffer[pixelui_compute_idx(x, y)] = cell_color_t{255, 255, 255, 255};
    }
}

// Function to put pixels
// at subsequence points
void drawCircle(s32 xc, s32 yc, s32 x, s32 y) {
    putpixel(xc + x, yc + y);
    putpixel(xc - x, yc + y);
    putpixel(xc + x, yc - y);
    putpixel(xc - x, yc - y);
    putpixel(xc + y, yc + x);
    putpixel(xc - y, yc + x);
    putpixel(xc + y, yc - x);
    putpixel(xc - y, yc - x);
}

// Function for circle-generation
// using Bresenham's algorithm
void circleBres(s32 xc, s32 yc, s32 r) {
    s32 x = 0, y = r;
    s32 d = 3 - 2 * r;
    drawCircle(xc, yc, x, y);
    while (y >= x) {
        // For each pixel we will
        // draw all eight pixels
        x++;

        // Check for decision parameter
        // and correspondingly
        // update d, x, y
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else
            d = d + 4 * x + 6;
        drawCircle(xc, yc, x, y);
    }
}

neko_vec2 calculate_mouse_position() {
    neko_vec2 ws = neko_platform_framebuffer_sizev(neko_platform_main_window());
    neko_vec2 pmp = neko_platform_mouse_positionv();
    // Need to place mouse into frame
    f32 x_scale = pmp.x / (f32)ws.x;
    f32 y_scale = pmp.y / (f32)ws.y;
    return neko_vec2{x_scale * (f32)g_pixelui_texture_width, y_scale * (f32)g_pixelui_texture_height};
}

void draw_glyph_at(font_t* f, cell_color_t* buffer, s32 x, s32 y, char c, cell_color_t col) {
    u8* font_data = (u8*)f->data;
    font_glyph_t g = get_glyph(f, c);

    // How to accurately place? I have width and height of glyph in texture, but need to convert this to RGBA data for ui buffer
    for (s32 h = 0; h < g.height; ++h) {
        for (s32 w = 0; w < g.width; ++w) {
            s32 _w = w + g.x;
            s32 _h = h + g.y;
            u8 a = font_data[(_h * f->width + _w) * f->num_comps + 0] == 0 ? 0 : 255;
            cell_color_t c = {font_data[(_h * f->width + _w) * f->num_comps + 0], font_data[(_h * f->width + _w) * f->num_comps + 1], font_data[(_h * f->width + _w) * f->num_comps + 2], a};
            if (pixelui_in_bounds(x + w, y + h) && a) {
                buffer[pixelui_compute_idx(x + w, y + h)] = col;
            }
        }
    }
}

void draw_string_at(font_t* f, cell_color_t* buffer, s32 x, s32 y, const char* str, usize len, cell_color_t col) {
    u8* font_data = (u8*)f->data;
    for (u32 i = 0; i < len; ++i) {
        font_glyph_t g = get_glyph(f, str[i]);
        draw_glyph_at(f, buffer, x, y, str[i], col);
        x += g.width + f->glyph_advance;  // Move by glyph width + advance
    }
}

bool in_rect(neko_vec2 p, neko_vec2 ro, neko_vec2 rd) {
    if (p.x < ro.x || p.x > ro.x + rd.x || p.y < ro.y || p.y > ro.y + rd.y) return false;
    return true;
}

bool gui_rect(cell_color_t* buffer, s32 _x, s32 _y, s32 _w, s32 _h, cell_color_t c) {
    neko_vec2 mp = calculate_mouse_position();

    for (u32 h = 0; h < _h; ++h) {
        for (u32 w = 0; w < _w; ++w) {
            if (pixelui_in_bounds(_x + w, _y + h)) {
                buffer[pixelui_compute_idx(_x + w, _y + h)] = c;
            }
        }
    }

    bool clicked = neko_platform_mouse_down(NEKO_MOUSE_LBUTTON);

    return in_rect(mp, neko_vec2{(f32)_x, (f32)_y}, neko_vec2{(f32)_w, (f32)_h}) && clicked;
}

#define __gui_interaction(x, y, w, h, c, str, id)                                                                                                          \
    do {                                                                                                                                                   \
        if ((id) == g_fallsand.material_selection) {                                                                                                       \
            const s32 b = 2;                                                                                                                               \
            gui_rect(g_fallsand.ui_buffer, x - b / 2, y - b / 2, w + b, h + b, cell_color_t{200, 150, 20, 255});                                           \
        }                                                                                                                                                  \
        neko_vec2 mp = calculate_mouse_position();                                                                                                         \
        if (in_rect(mp, neko_vec2{(f32)(x), (f32)(y)}, neko_vec2{(w), (h)})) {                                                                             \
            interaction |= true;                                                                                                                           \
            char _str[] = (str);                                                                                                                           \
            cell_color_t col = cell_color_t{255, 255, 255, 255};                                                                                           \
            cell_color_t s_col = cell_color_t{10, 10, 10, 255};                                                                                            \
            cell_color_t r_col = cell_color_t{5, 5, 5, 170};                                                                                               \
            /*Draw rect around text as well for easier viewing*/                                                                                           \
            gui_rect(g_fallsand.ui_buffer, g_pixelui_texture_width / 2 - 50, 15, 100, 20, r_col);                                                          \
            draw_string_at(&pixel_font, g_fallsand.ui_buffer, g_pixelui_texture_width / 2 + 1 - (sizeof(str) * 5) / 2, 20 - 1, _str, sizeof(_str), s_col); \
            draw_string_at(&pixel_font, g_fallsand.ui_buffer, g_pixelui_texture_width / 2 - (sizeof(str) * 5) / 2, 20, _str, sizeof(_str), col);           \
        }                                                                                                                                                  \
        if (gui_rect(g_fallsand.ui_buffer, x, y, w, h, c)) {                                                                                               \
            g_fallsand.material_selection = id;                                                                                                            \
        }                                                                                                                                                  \
    } while (0)

bool update_ui() {
    bool interaction = false;
    // neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    // neko_platform_i* platform = neko_engine_instance()->ctx.platform;

    // Cache transformed mouse position
    neko_vec2 mp = calculate_mouse_position();

    // Do ui stuff
    memset(g_fallsand.ui_buffer, 0, g_pixelui_texture_width * g_pixelui_texture_height * sizeof(cell_color_t));

    // Material selection panel gui
    if (g_fallsand.show_material_selection_panel) {
        const s32 offset = 12;
        s32 xoff = 20;
        s32 base = 10;

        // Sand Selection
        __gui_interaction(g_pixelui_texture_width - xoff, base + offset * 0, 10, 10, mat_col_sand, "Sand", mat_sel_sand);
        __gui_interaction(g_pixelui_texture_width - xoff, base + offset * 1, 10, 10, mat_col_water, "Water", mat_sel_water);
        __gui_interaction(g_pixelui_texture_width - xoff, base + offset * 2, 10, 10, mat_col_smoke, "Smoke", mat_sel_smoke);
        __gui_interaction(g_pixelui_texture_width - xoff, base + offset * 3, 10, 10, mat_col_fire, "Fire", mat_sel_fire);
        __gui_interaction(g_pixelui_texture_width - xoff, base + offset * 4, 10, 10, mat_col_steam, "Steam", mat_sel_steam);
        __gui_interaction(g_pixelui_texture_width - xoff, base + offset * 5, 10, 10, mat_col_oil, "Oil", mat_sel_oil);
        __gui_interaction(g_pixelui_texture_width - xoff, base + offset * 6, 10, 10, mat_col_salt, "Salt", mat_sel_salt);
        __gui_interaction(g_pixelui_texture_width - xoff, base + offset * 7, 10, 10, mat_col_wood, "Wood", mat_sel_wood);
        __gui_interaction(g_pixelui_texture_width - xoff, base + offset * 8, 10, 10, mat_col_stone, "Stone", mat_sel_stone);
        __gui_interaction(g_pixelui_texture_width - xoff, base + offset * 9, 10, 10, mat_col_lava, "Lava", mat_sel_lava);
        __gui_interaction(g_pixelui_texture_width - xoff, base + offset * 10, 10, 10, mat_col_gunpowder, "GunPowder", mat_sel_gunpowder);
        __gui_interaction(g_pixelui_texture_width - xoff, base + offset * 11, 10, 10, mat_col_acid, "Acid", mat_sel_acid);
    }

    if (g_fallsand.show_frame_count) {

        char frame_time_str[256];
        neko_snprintf(frame_time_str, sizeof(frame_time_str), "frame: %6.2f ms %llu", neko_platform_frame_time(), g_fallsand.update_time);
        draw_string_at(&pixel_font, g_fallsand.ui_buffer, 10, 10, frame_time_str, strlen(frame_time_str), cell_color_t{255, 255, 255, 255});

        // char sim_state_str[256];
        // neko_snprintf(sim_state_str, sizeof(sim_state_str), "state: %s", "running");
        // draw_string_at(&pixel_font, g_ui_buffer, 10, 20, sim_state_str, strlen(sim_state_str), cell_color_t{255, 255, 255, 255});

        // gfx->fontcache_push_x_y(std::format("test: {0} {1}", l_check, neko_buildnum()), g_basic_font, 40, 160);
    }

    g_fallsand.update_time = 0;

    // Draw circle around mouse pointer
    s32 R = g_fallsand.brush_size * g_fallsand.render_scale;
    circleBres((s32)(mp.x), (s32)(mp.y), R);

    // Upload our updated texture data to GPU
    neko_graphics_texture_desc_t t_desc = neko_default_val();
    t_desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    t_desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.num_mips = 0;

    t_desc.width = g_pixelui_texture_width;
    t_desc.height = g_pixelui_texture_height;

    t_desc.data[0] = g_fallsand.ui_buffer;

    neko_graphics_texture_update(g_fallsand.tex_ui, &t_desc);

    return interaction;
}

#endif