
#include "game_pixelui.h"

#include "neko_api.h"

#define NEKO_BUILTIN_IMPL
#include "../old/neko_builtin_font.h"

// user data
extern neko_client_userdata_t g_client_userdata;

s32 pixelui_compute_idx(pixelui_t* pui, s32 x, s32 y) { return (y * pui->g_pixelui_texture_width + x); }

bool pixelui_in_bounds(pixelui_t* pui, s32 x, s32 y) {
    if (x < 0 || x > (pui->g_pixelui_texture_width - 1) || y < 0 || y > (pui->g_pixelui_texture_height - 1)) return false;
    return true;
}

void putpixel(pixelui_t* pui, s32 x, s32 y) {
    if (pixelui_in_bounds(pui, x, y)) {
        pui->ui_buffer[pixelui_compute_idx(pui, x, y)] = neko_color_t{255, 255, 255, 255};
    }
}

// 圆生成函数
// 使用 Bresenham 算法
void circle_bres(pixelui_t* pui, s32 xc, s32 yc, s32 r) {
    auto drawCircle = [](pixelui_t* pui, s32 xc, s32 yc, s32 x, s32 y) {
        putpixel(pui, xc + x, yc + y);
        putpixel(pui, xc - x, yc + y);
        putpixel(pui, xc + x, yc - y);
        putpixel(pui, xc - x, yc - y);
        putpixel(pui, xc + y, yc + x);
        putpixel(pui, xc - y, yc + x);
        putpixel(pui, xc + y, yc - x);
        putpixel(pui, xc - y, yc - x);
    };

    s32 x = 0, y = r;
    s32 d = 3 - 2 * r;
    drawCircle(pui, xc, yc, x, y);
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
        drawCircle(pui, xc, yc, x, y);
    }
}

neko_vec2 calculate_mouse_position(pixelui_t* pui) {
    neko_vec2 ws = neko_platform_framebuffer_sizev(neko_platform_main_window());
    neko_vec2 pmp = neko_platform_mouse_positionv();
    // Need to place mouse into frame
    f32 x_scale = pmp.x / (f32)ws.x;
    f32 y_scale = pmp.y / (f32)ws.y;
    return neko_vec2{x_scale * (f32)pui->g_pixelui_texture_width, y_scale * (f32)pui->g_pixelui_texture_height};
}

void draw_glyph_at(pixelui_t* pui, font_t* f, neko_color_t* buffer, s32 x, s32 y, char c, neko_color_t col) {
    u8* font_data = (u8*)f->data;
    font_glyph_t g = get_glyph(f, c);

    // How to accurately place? I have width and height of glyph in texture, but need to convert this to RGBA data for ui buffer
    for (s32 h = 0; h < g.height; ++h) {
        for (s32 w = 0; w < g.width; ++w) {
            s32 _w = w + g.x;
            s32 _h = h + g.y;
            u8 a = font_data[(_h * f->width + _w) * f->num_comps + 0] == 0 ? 0 : 255;
            neko_color_t c = {font_data[(_h * f->width + _w) * f->num_comps + 0], font_data[(_h * f->width + _w) * f->num_comps + 1], font_data[(_h * f->width + _w) * f->num_comps + 2], a};
            if (pixelui_in_bounds(pui, x + w, y + h) && a) {
                buffer[pixelui_compute_idx(pui, x + w, y + h)] = col;
            }
        }
    }
}

void draw_string_at(pixelui_t* pui, font_t* f, neko_color_t* buffer, s32 x, s32 y, const char* str, usize len, neko_color_t col) {
    u8* font_data = (u8*)f->data;
    for (u32 i = 0; i < len; ++i) {
        font_glyph_t g = get_glyph(f, str[i]);
        draw_glyph_at(pui, f, buffer, x, y, str[i], col);
        x += g.width + f->glyph_advance;  // Move by glyph width + advance
    }
}

bool in_rect(neko_vec2 p, neko_vec2 ro, neko_vec2 rd) {
    if (p.x < ro.x || p.x > ro.x + rd.x || p.y < ro.y || p.y > ro.y + rd.y) return false;
    return true;
}

bool gui_rect(pixelui_t* pui, neko_color_t* buffer, s32 _x, s32 _y, s32 _w, s32 _h, neko_color_t c) {
    neko_vec2 mp = calculate_mouse_position(pui);

    for (u32 h = 0; h < _h; ++h) {
        for (u32 w = 0; w < _w; ++w) {
            if (pixelui_in_bounds(pui, _x + w, _y + h)) {
                buffer[pixelui_compute_idx(pui, _x + w, _y + h)] = c;
            }
        }
    }

    bool clicked = neko_platform_mouse_down(NEKO_MOUSE_LBUTTON);

    return in_rect(mp, neko_vec2{(f32)_x, (f32)_y}, neko_vec2{(f32)_w, (f32)_h}) && clicked;
}

#define __gui_interaction(x, y, w, h, c, str, id)                                                                                                              \
    do {                                                                                                                                                       \
        if ((id) == pui->material_selection) {                                                                                                                 \
            const s32 b = 2;                                                                                                                                   \
            gui_rect(pui, pui->ui_buffer, x - b / 2, y - b / 2, w + b, h + b, neko_color_t{200, 150, 20, 255});                                                \
        }                                                                                                                                                      \
        neko_vec2 mp = calculate_mouse_position(pui);                                                                                                          \
        if (in_rect(mp, neko_vec2{(f32)(x), (f32)(y)}, neko_vec2{(w), (h)})) {                                                                                 \
            interaction |= true;                                                                                                                               \
            char _str[] = (str);                                                                                                                               \
            neko_color_t col = neko_color_t{255, 255, 255, 255};                                                                                               \
            neko_color_t s_col = neko_color_t{10, 10, 10, 255};                                                                                                \
            neko_color_t r_col = neko_color_t{5, 5, 5, 170};                                                                                                   \
            /*Draw rect around text as well for easier viewing*/                                                                                               \
            gui_rect(pui, pui->ui_buffer, pui->g_pixelui_texture_width / 2 - 50, 15, 100, 20, r_col);                                                          \
            draw_string_at(pui, &pixel_font, pui->ui_buffer, pui->g_pixelui_texture_width / 2 + 1 - (sizeof(str) * 5) / 2, 20 - 1, _str, sizeof(_str), s_col); \
            draw_string_at(pui, &pixel_font, pui->ui_buffer, pui->g_pixelui_texture_width / 2 - (sizeof(str) * 5) / 2, 20, _str, sizeof(_str), col);           \
        }                                                                                                                                                      \
        if (gui_rect(pui, pui->ui_buffer, x, y, w, h, c)) {                                                                                                    \
            pui->material_selection = id;                                                                                                                      \
        }                                                                                                                                                      \
    } while (0)

bool update_ui(pixelui_t* pui) {
    bool interaction = false;
    // neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    // neko_platform_i* platform = neko_engine_instance()->ctx.platform;

    // Cache transformed mouse position
    neko_vec2 mp = calculate_mouse_position(pui);

    // Do ui stuff
    memset(pui->ui_buffer, 0, pui->g_pixelui_texture_width * pui->g_pixelui_texture_height * sizeof(neko_color_t));

    // Material selection panel gui
    if (pui->show_material_selection_panel) {
        const s32 offset = 12;
        s32 xoff = 20;
        s32 base = 10;

        auto& mat_map = MagicPixelFactory::Instance()->get_registy();
        for (auto& mat_reg : mat_map) {
            auto mat = mat_reg.second();
            __gui_interaction(pui->g_pixelui_texture_width - xoff, base + offset * (u32)mat->material_, 10, 10, mat->color_, "Sand", mat->material_);
        }
    }

    if (pui->show_frame_count) {

        char frame_time_str[256];
        neko_snprintf(frame_time_str, sizeof(frame_time_str), "frame: %6.2f ms %llu", neko_platform_frame_time(), pui->update_time);
        draw_string_at(pui, &pixel_font, pui->ui_buffer, 10, 10, frame_time_str, strlen(frame_time_str), neko_color_t{255, 255, 255, 255});

        // gfx->fontcache_push_x_y(std::format("test: {0} {1}", l_check, neko_buildnum()), g_basic_font, 40, 160);
    }

    pui->update_time = 0;

    // 围绕鼠标指针绘制圆圈
    s32 R = pui->brush_size * pui->g_pixelui_scale;
    circle_bres(pui, (s32)(mp.x), (s32)(mp.y), R);

    // 将更新后的纹理数据上传到 GPU
    neko_graphics_texture_desc_t t_desc = neko_default_val();
    t_desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    t_desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.num_mips = 0;

    t_desc.width = pui->g_pixelui_texture_width;
    t_desc.height = pui->g_pixelui_texture_height;

    t_desc.data[0] = pui->ui_buffer;

    neko_graphics_texture_update(pui->tex_ui, &t_desc);

    return interaction;
}

void pixelui_init(pixelui_t* pui) {

    pui->ui_buffer = (neko_color_t*)neko_safe_malloc(pui->g_pixelui_texture_width * pui->g_pixelui_texture_height * sizeof(neko_color_t));
    memset(pui->ui_buffer, 0, pui->g_pixelui_texture_width * pui->g_pixelui_texture_height);

    neko_graphics_texture_desc_t t_desc = neko_default_val();
    t_desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    t_desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.num_mips = 0;
    t_desc.width = pui->g_pixelui_texture_width;
    t_desc.height = pui->g_pixelui_texture_height;
    t_desc.data[0] = pui->ui_buffer;

    pui->tex_ui = neko_graphics_texture_create(&t_desc);

    // Load UI font texture data from file
    construct_font_data(g_font_data);
}

void pixelui_destroy(pixelui_t* pui) {
    neko_graphics_texture_destroy(pui->tex_ui);
    neko_safe_free(pui->ui_buffer);
}

void pixelui_draw(pixelui_t* pui) {
    neko_immediate_draw_t* idraw = g_client_userdata.idraw;
    const neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());
    neko_idraw_rect_textured_ext(idraw, 0, 0, fbs.x, fbs.y, 0, 1, 1, 0, pui->tex_ui.id, NEKO_COLOR_WHITE);
}