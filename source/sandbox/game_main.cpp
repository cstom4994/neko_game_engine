

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <format>
#include <functional>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

// engine
#include "engine/neko.h"
#include "engine/neko.hpp"
#include "engine/neko_asset.h"
#include "engine/neko_ecs.h"
#include "engine/neko_engine.h"
#include "engine/neko_filesystem.h"
#include "engine/neko_platform.h"

// binding
#include "neko_api.h"
#include "neko_refl.hpp"

// game
#include "game_editor.h"
#include "game_main.h"
#include "sandbox/game_physics_math.hpp"
#include "sandbox/mat_base.h"
#include "sandbox/neko_api.h"
#include "sandbox/simulation.h"

// lua
#include "engine/neko_lua.h"

// opengl
#include <glad/glad.h>

// hpp
#include "sandbox/hpp/neko_static_refl.hpp"
#include "sandbox/hpp/neko_struct.hpp"

#define NEKO_IMGUI_IMPL
#include "game_imgui.h"

NEKO_HIJACK_MAIN();

// user data

neko_client_userdata_t g_client_userdata = neko_default_val();

void editor_dockspace(neko_ui_context_t *ctx);

// test
void test_xml(const std::string &file);
void test_sr();
void test_ut();
void test_se();
void test_containers();
void test_thd();
void test_fgd();

NEKO_API_DECL void test_sexpr();
NEKO_API_DECL void test_ttf();

void print(const float &val) { std::printf("%f", val); }

void print(const std::string &val) { std::printf("%s", val.c_str()); }

// Register component types by wrapping the struct name in `Comp(...)`

struct Comp(Position) {
    // Register reflectable fields ('props') using the `Prop` macro. This can be used in any aggregate
    // `struct`, doesn't have to be a `Comp`.
    neko_prop(float, x) = 0;
    neko_prop(float, y) = 0;
};

struct Comp(Name) {
    neko_prop(std::string, first) = "First";

    // Props can have additional attributes, see the `neko_prop_attribs` type. You can customize and add
    // your own attributes there.
    neko_prop(std::string, last, .exampleFlag = true) = "Last";

    int internal = 42;  // This is a non-prop field that doesn't show up in reflection. IMPORTANT NOTE:
    // All such non-prop fields must be at the end of the struct, with all props at
    // the front. Mixing props and non-props in the order is not allowed.
};

// After all compnonent types are defined, this macro must be called to be able to use
// `forEachComponentType` to iterate all component types
UseComponentTypes();

bool SDL_PointInRect(const neko_vec2_t p, const neko_vec4_t r) {
    //    if (p == NULL || r == NULL) {
    //        return false;
    //    }
    return (p.x >= r.x && p.x < (r.x + r.w) && p.y >= r.y && p.y < (r.y + r.z));
}

void Graphics::DrawCircle(neko_immediate_draw_t *idraw, const neko_vec2_t center, const neko_vec4_t bounds, u16 radius) {
    if (!SDL_PointInRect(center, bounds)) {
        return;
    }

    s32 x = radius;
    s32 y = 0;
    s32 tx = 1;
    s32 ty = 1;
    s32 error = tx - radius * 2;

    while (x >= y) {
        s32 lookup[16] = {x, -y, x, y, -x, -y, -x, y, y, -x, y, x, -y, -x, -y, x};
        for (int i = 0; i < 8; ++i) {
            s32 drawX = std::clamp(center.x + lookup[i * 2], bounds.x, bounds.x + bounds.w);
            s32 drawY = std::clamp(center.y + lookup[i * 2 + 1], bounds.y, bounds.y + bounds.z - 1);
            //            SDL_RenderDrawPoint(renderer, drawX, drawY);
        }
        if (error <= 0) {
            ++y;
            ty += 2;
            error += ty;
        }
        if (error > 0) {
            --x;
            tx += 2;
            error += tx - radius * 2;
        }
    }
}

neko_texture_t load_ase_texture_simple(const void *memory, int size) {

    ase_t *ase = neko_aseprite_load_from_memory(memory, size);
    neko_defer({ neko_aseprite_free(ase); });

    if (NULL == ase) {
        neko_log_error("unable to load ase %p", memory);
        return neko_default_val();
    }

    neko_assert(ase->frame_count == 1);  // load_ase_texture_simple used to load simple aseprite

    neko_aseprite_default_blend_bind(ase);

    neko_graphics_t *gfx = neko_instance()->ctx.graphics;

    neko_log_trace(std::format("load aseprite - frame_count {0} - palette.entry_count{1} - w={2} h={3}", ase->frame_count, ase->palette.entry_count, ase->w, ase->h).c_str());

    s32 bpp = 4;

    neko_graphics_texture_desc_t t_desc = {};

    t_desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    t_desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.num_mips = 0;
    t_desc.width = ase->w;
    t_desc.height = ase->h;
    // t_desc.num_comps = 4;
    t_desc.data[0] = ase->frames->pixels[0];

    neko_tex_flip_vertically(ase->w, ase->h, (u8 *)(t_desc.data[0]));

    neko_texture_t tex = neko_graphics_texture_create(&t_desc);

    return tex;
}

std::string game_assets(const std::string &path) {
    if (CL_GAME_USERDATA()->data_path.empty()) {
        neko_assert(!CL_GAME_USERDATA()->data_path.empty());  // game data path not detected
        return path;
    }
    std::string get_path(CL_GAME_USERDATA()->data_path);

    if (path.substr(0, 7) == "gamedir") {
        get_path.append(path);
    } else if (path.substr(0, 11) == "lua_scripts") {
        std::string lua_path = path;
        get_path = std::filesystem::path(CL_GAME_USERDATA()->data_path).parent_path().string().append("/source/lua/game/").append(lua_path.replace(0, 12, ""));
    }

    return get_path;
}

// clang-format off
unsigned char random_table[256] = {
    246, 44, 11, 243, 99, 68, 235, 255, 40, 141, 188, 125, 228, 115, 33,
    248, 60, 235, 232, 58, 81, 140, 8, 68, 88, 143, 44, 100, 149, 214, 39,
    199, 52, 250, 217, 55, 231, 108, 68, 241, 50, 200, 121, 22, 51, 189,
    203, 193, 105, 72, 42, 235, 242, 142, 12, 139, 134, 87, 241, 239, 128,
    221, 225, 251, 248, 255, 123, 32, 148, 182, 193, 204, 79, 154, 182,
    26, 152, 54, 16, 162, 78, 220, 90, 65, 66, 242, 25, 79, 187, 74, 217,
    236, 56, 6, 125, 208, 132, 161, 193, 111, 52, 49, 218, 197, 111, 250,
    192, 230, 229, 204, 168, 230, 78, 58, 165, 131, 54, 94, 118, 29, 62,
    112, 180, 146, 71, 148, 5, 54, 158, 158, 91, 17, 95, 107, 38, 91, 198,
    47, 70, 228, 15, 175, 222, 225, 83, 80, 20, 253, 77, 163, 134, 158, 6,
    248, 111, 212, 81, 98, 168, 131, 158, 208, 30, 255, 208, 120, 175, 149,
    7, 124, 49, 142, 71, 211, 162, 26, 154, 194, 51, 128, 217, 162, 31,
    191, 158, 82, 223, 115, 108, 209, 62, 168, 51, 235, 212, 199, 151,
    11, 182, 245, 5, 10, 188, 231, 122, 166, 149, 54, 251, 153, 143, 107,
    98, 154, 90, 171, 78, 24, 41, 64, 187, 237, 58, 208, 106, 226, 176,
    228, 167, 17, 4, 148, 219, 18, 124, 70, 214, 105, 231, 206, 195, 127,
    182, 11, 208, 224, 56, 197, 11, 62, 138, 218, 18, 234, 245, 242
};
// clang-format on

neko_global int rnd_index = 0;

int g_random(void) {
    rnd_index = (rnd_index + 1) & 0xff;
    return random_table[rnd_index];
}

s32 random_val(s32 lower, s32 upper) {
    if (upper < lower) {
        s32 tmp = lower;
        lower = upper;
        upper = tmp;
    }
    return (rand() % (upper - lower + 1) + lower);
}

s32 random_tlb_val(s32 lower, s32 upper) {
    s32 rand_num = g_random();
    if (upper < lower) {
        s32 tmp = lower;
        lower = upper;
        upper = tmp;
    }
    return (rand_num % (upper - lower + 1) + lower);
}

void app_load_style_sheet(bool destroy) {
    if (destroy) {
        neko_ui_style_sheet_destroy(&CL_GAME_USERDATA()->style_sheet);
    }
    CL_GAME_USERDATA()->style_sheet = neko_ui_style_sheet_load_from_file(&CL_GAME_USERDATA()->ui, game_assets("gamedir/style_sheets/gui.ss").c_str());
    neko_ui_set_style_sheet(&CL_GAME_USERDATA()->ui, &CL_GAME_USERDATA()->style_sheet);
}

s32 button_custom(neko_ui_context_t *ctx, const char *label) {
    // Do original button call
    s32 res = neko_ui_button(ctx, label);

    // Draw inner shadows/highlights over button
    neko_color_t hc = NEKO_COLOR_WHITE, sc = neko_color(85, 85, 85, 255);
    neko_ui_rect_t r = ctx->last_rect;
    s32 w = 2;
    neko_ui_draw_rect(ctx, neko_ui_rect(r.x + w, r.y, r.w - 2 * w, w), hc);
    neko_ui_draw_rect(ctx, neko_ui_rect(r.x + w, r.y + r.h - w, r.w - 2 * w, w), sc);
    neko_ui_draw_rect(ctx, neko_ui_rect(r.x, r.y, w, r.h), hc);
    neko_ui_draw_rect(ctx, neko_ui_rect(r.x + r.w - w, r.y, w, r.h), sc);

    return res;
}

// Custom callback for immediate drawing directly into the gui window
void gui_cb(neko_ui_context_t *ctx, struct neko_ui_customcommand_t *cmd) {
    neko_immediate_draw_t *gui_idraw = &ctx->gui_idraw;  // Immediate draw list in gui context
    neko_vec2 fbs = ctx->framebuffer_size;               // Framebuffer size bound for gui context
    neko_color_t *color = (neko_color_t *)cmd->data;     // Grab custom data
    neko_asset_texture_t *tp = neko_assets_getp(&CL_GAME_USERDATA()->am, neko_asset_texture_t, CL_GAME_USERDATA()->tex_hndl);
    const f32 t = neko_platform_elapsed_time();

    // Set up an immedaite camera using our passed in cmd viewport (this is the clipped viewport of the gui window being drawn)
    neko_idraw_camera3d(gui_idraw, (u32)cmd->viewport.w, (u32)cmd->viewport.h);
    neko_idraw_blend_enabled(gui_idraw, true);
    neko_graphics_set_viewport(&gui_idraw->commands, cmd->viewport.x, fbs.y - cmd->viewport.h - cmd->viewport.y, cmd->viewport.w, cmd->viewport.h);
    neko_idraw_push_matrix(gui_idraw, NEKO_IDRAW_MATRIX_MODELVIEW);
    {
        neko_idraw_rotatev(gui_idraw, t * 0.001f, NEKO_YAXIS);
        neko_idraw_scalef(gui_idraw, 0.5f, 0.5f, 0.5f);
        neko_idraw_box(gui_idraw, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, color->r, color->g, color->b, color->a, NEKO_GRAPHICS_PRIMITIVE_LINES);
    }
    neko_idraw_pop_matrix(gui_idraw);

    // Set up 2D camera for projection matrix
    neko_idraw_camera2d(gui_idraw, (u32)fbs.x, (u32)fbs.y);

    // Rect
    neko_idraw_rectv(gui_idraw, neko_v2(500.f, 50.f), neko_v2(600.f, 100.f), NEKO_COLOR_RED, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
    neko_idraw_rectv(gui_idraw, neko_v2(650.f, 50.f), neko_v2(750.f, 100.f), NEKO_COLOR_RED, NEKO_GRAPHICS_PRIMITIVE_LINES);

    // Triangle
    neko_idraw_trianglev(gui_idraw, neko_v2(50.f, 50.f), neko_v2(100.f, 100.f), neko_v2(50.f, 100.f), NEKO_COLOR_WHITE, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
    neko_idraw_trianglev(gui_idraw, neko_v2(200.f, 50.f), neko_v2(300.f, 100.f), neko_v2(200.f, 100.f), NEKO_COLOR_WHITE, NEKO_GRAPHICS_PRIMITIVE_LINES);

    // Lines
    neko_idraw_linev(gui_idraw, neko_v2(50.f, 20.f), neko_v2(500.f, 20.f), neko_color(0, 255, 0, 255));

    // Circle
    neko_idraw_circle(gui_idraw, 350.f, 170.f, 50.f, 20, 100, 150, 220, 255, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
    neko_idraw_circle(gui_idraw, 250.f, 170.f, 50.f, 20, 100, 150, 220, 255, NEKO_GRAPHICS_PRIMITIVE_LINES);

    // Circle Sector
    neko_idraw_circle_sector(gui_idraw, 50.f, 150.f, 50.f, 0, 90, 32, 255, 255, 255, 255, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
    neko_idraw_circle_sector(gui_idraw, 150.f, 200.f, 50.f, 90, 270, 32, 255, 255, 255, 255, NEKO_GRAPHICS_PRIMITIVE_LINES);

    // Box (with texture)
    neko_idraw_depth_enabled(gui_idraw, true);
    neko_idraw_face_cull_enabled(gui_idraw, true);
    neko_idraw_camera3d(gui_idraw, (u32)fbs.x, (u32)fbs.y);
    neko_idraw_push_matrix(gui_idraw, NEKO_IDRAW_MATRIX_MODELVIEW);
    {
        neko_idraw_translatef(gui_idraw, -2.f, -1.f, -5.f);
        neko_idraw_rotatev(gui_idraw, neko_platform_elapsed_time() * 0.001f, NEKO_YAXIS);
        neko_idraw_rotatev(gui_idraw, neko_platform_elapsed_time() * 0.0005f, NEKO_ZAXIS);
        neko_idraw_texture(gui_idraw, tp->hndl);
        neko_idraw_scalef(gui_idraw, 1.5f, 1.5f, 1.5f);
        neko_idraw_box(gui_idraw, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 255, 255, 255, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
        neko_idraw_texture(gui_idraw, neko_handle(neko_graphics_texture_t){0});
    }
    neko_idraw_pop_matrix(gui_idraw);

    // Box (lines, no texture)
    neko_idraw_push_matrix(gui_idraw, NEKO_IDRAW_MATRIX_MODELVIEW);
    {
        neko_idraw_translatef(gui_idraw, 2.f, -1.f, -5.f);
        neko_idraw_rotatev(gui_idraw, neko_platform_elapsed_time() * 0.001f, NEKO_YAXIS);
        neko_idraw_rotatev(gui_idraw, neko_platform_elapsed_time() * 0.0008f, NEKO_ZAXIS);
        neko_idraw_rotatev(gui_idraw, neko_platform_elapsed_time() * 0.0009f, NEKO_XAXIS);
        neko_idraw_scalef(gui_idraw, 1.5f, 1.5f, 1.5f);
        neko_idraw_box(gui_idraw, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 200, 100, 255, NEKO_GRAPHICS_PRIMITIVE_LINES);
    }
    neko_idraw_pop_matrix(gui_idraw);

    // Sphere (triangles, no texture)
    neko_idraw_camera3d(gui_idraw, (u32)fbs.x, (u32)fbs.y);
    neko_idraw_push_matrix(gui_idraw, NEKO_IDRAW_MATRIX_MODELVIEW);
    {
        neko_idraw_translatef(gui_idraw, -2.f, -1.f, -5.f);
        neko_idraw_rotatev(gui_idraw, neko_platform_elapsed_time() * 0.001f, NEKO_YAXIS);
        neko_idraw_rotatev(gui_idraw, neko_platform_elapsed_time() * 0.0005f, NEKO_ZAXIS);
        neko_idraw_scalef(gui_idraw, 1.5f, 1.5f, 1.5f);
        neko_idraw_sphere(gui_idraw, 0.f, 0.f, 0.f, 1.0f, 255, 255, 255, 50, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
    }
    neko_idraw_pop_matrix(gui_idraw);

    // Sphere (lines)
    neko_idraw_push_matrix(gui_idraw, NEKO_IDRAW_MATRIX_MODELVIEW);
    {
        neko_idraw_translatef(gui_idraw, 2.f, -1.f, -5.f);
        neko_idraw_rotatev(gui_idraw, neko_platform_elapsed_time() * 0.001f, NEKO_YAXIS);
        neko_idraw_rotatev(gui_idraw, neko_platform_elapsed_time() * 0.0008f, NEKO_ZAXIS);
        neko_idraw_rotatev(gui_idraw, neko_platform_elapsed_time() * 0.0009f, NEKO_XAXIS);
        neko_idraw_scalef(gui_idraw, 1.5f, 1.5f, 1.5f);
        neko_idraw_sphere(gui_idraw, 0.f, 0.f, 0.f, 1.0f, 255, 255, 255, 50, NEKO_GRAPHICS_PRIMITIVE_LINES);
    }
    neko_idraw_pop_matrix(gui_idraw);

    // Text (custom and default fonts)
    // neko_idraw_camera2D(gui_idraw, (u32)ws.x, (u32)ws.y);
    // neko_idraw_defaults(gui_idraw);
    // neko_idraw_text(gui_idraw, 410.f, 150.f, "Custom Font", &font, false, 200, 100, 50, 255);
    // neko_idraw_text(gui_idraw, 450.f, 200.f, "Default Font", NULL, false, 50, 100, 255, 255);
}

neko_handle(neko_graphics_storage_buffer_t) u_voxels = {0};

f32 font_projection[16];

neko_graphics_batch_context_t *font_render;
neko_graphics_batch_shader_t font_shader;
neko_graphics_batch_renderable_t font_renderable;
f32 font_scale = 3.f;
int font_vert_count;
neko_font_vert_t *font_verts;
neko_font_u64 test_font_tex_id;
neko_font_t *test_font_bmfont;

void draw_text(neko_font_t *font, const char *text, float x, float y, float line_height, float clip_region, float wrap_x, f32 scale) {
    f32 text_w = (f32)neko_font_text_width(font, text);
    f32 text_h = (f32)neko_font_text_height(font, text);

    //    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());

    if (scale == 0.f) scale = font_scale;

    neko_font_rect_t clip_rect;
    clip_rect.left = -CL_GAME_USERDATA()->fbs.x / scale * clip_region;
    clip_rect.right = CL_GAME_USERDATA()->fbs.x / scale * clip_region + 0.5f;
    clip_rect.top = CL_GAME_USERDATA()->fbs.y / scale * clip_region + 0.5f;
    clip_rect.bottom = -CL_GAME_USERDATA()->fbs.y / scale * clip_region;

    f32 x0 = (x - CL_GAME_USERDATA()->fbs.x / 2.f) / scale /*+ -text_w / 2.f*/;
    f32 y0 = (CL_GAME_USERDATA()->fbs.y / 2.f - y) / scale + text_h / 2.f;
    f32 wrap_width = wrap_x - x0;

    neko_font_fill_vertex_buffer(font, text, x0, y0, wrap_width, line_height, &clip_rect, font_verts, 1024 * 2, &font_vert_count);

    if (font_vert_count) {
        neko_graphics_batch_draw_call_t call;
        call.textures[0] = (u32)font->atlas_id;
        call.texture_count = 1;
        call.r = &font_renderable;
        call.verts = font_verts;
        call.vert_count = font_vert_count;

        neko_graphics_batch_push_draw_call(font_render, call);
    }
}

u32 frame_count = 0;
u32 current_tick = 0;
u32 last_tick = 0;
neko_color_t kEmptyPixelValue = neko_color(0, 0, 0, 0);
neko_vec2_t cursor = {-1, -1};

thread_local std::mt19937 rng;

// int32_t random_val(int32_t lower, int32_t upper) { return ((rand() % (upper - lower + 1)) + lower); }

#if 0

// Move system
void move_system(ecs_iter_t *it) {
    const neko_vec2_t ws = neko_platform_window_sizev(neko_platform_main_window());

    // Get columns from system signature and cache local pointers to arrays
    //    ECS_COLUMN(it, position_t, p, 1);
    //    ECS_COLUMN(it, velocity_t, v, 2);
    //    ECS_COLUMN(it, bounds_t, b, 3);

    position_t *p = ecs_field(it, position_t, 1);
    velocity_t *v = ecs_field(it, velocity_t, 2);
    bounds_t *b = ecs_field(it, bounds_t, 3);

    for (int32_t i = 0; i < it->count; ++i) {
        neko_vec2_t min = neko_vec2_add(p[i], v[i]);
        neko_vec2_t max = neko_vec2_add(min, b[i]);

        // Resolve collision and change velocity direction if necessary
        if (min.x < 0 || max.x >= ws.x) {
            v[i].x *= -1.f;
        }
        if (min.y < 0 || max.y >= ws.y) {
            v[i].y *= -1.f;
        }
        p[i] = neko_vec2_add(p[i], v[i]);
    }
}

void render_system(ecs_iter_t *it) {
    //    app_data_t *app = gs_user_data(app_data_t);
    const neko_vec2_t fbs = neko_platform_window_sizev(neko_platform_main_window());

    // Grab position from column data
    //    ECS_COLUMN(it, position_t, p, 1);
    //    ECS_COLUMN(it, bounds_t, b, 2);
    //    ECS_COLUMN(it, color_t, c, 3);

    position_t *p = ecs_field(it, position_t, 1);
    bounds_t *b = ecs_field(it, bounds_t, 2);
    color_t *c = ecs_field(it, color_t, 3);

    neko_idraw_defaults(&CL_GAME_USERDATA()->idraw);
    neko_idraw_camera2d(&CL_GAME_USERDATA()->idraw, fbs.x, fbs.y);

    // Render all into immediate draw instance data
    for (int32_t i = 0; i < it->count; ++i) {
        auto pi = p[i];
        auto bi = b[i];
        neko_idraw_rectvd(&CL_GAME_USERDATA()->idraw, pi, bi, neko_v2(0.f, 0.f), neko_v2(1.f, 1.f), (neko_color_t)c[i], NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
    }
}

#endif

neko_ecs_decl_system(movement_system, MOVEMENT_SYSTEM, 4, COMPONENT_TRANSFORM, COMPONENT_VELOCITY, COMPONENT_BOUNDS, COMPONENT_COLOR) {

    const neko_vec2_t ws = neko_platform_window_sizev(neko_platform_main_window());

    for (u32 i = 0; i < neko_ecs_for_count(ecs); i++) {
        neko_ecs_ent e = neko_ecs_get_ent(ecs, i);
        if (neko_ecs_ent_has_mask(ecs, e, neko_ecs_get_mask(MOVEMENT_SYSTEM))) {
            position_t *p = (position_t *)neko_ecs_ent_get_component(ecs, e, COMPONENT_TRANSFORM);
            velocity_t *v = (velocity_t *)neko_ecs_ent_get_component(ecs, e, COMPONENT_VELOCITY);
            bounds_t *b = (bounds_t *)neko_ecs_ent_get_component(ecs, e, COMPONENT_BOUNDS);
            color_t *c = (color_t *)neko_ecs_ent_get_component(ecs, e, COMPONENT_COLOR);

            // Grab global instance of engine
            neko_t *engine = neko_instance();

            //            static u8 tick = 0;
            //
            //            if ((tick++ & 31) == 0)
            //                if (fabs(velocity->dx) >= 0.5 || fabs(velocity->dy) >= 0.5)
            //                    neko_sprite_renderer_play(sprite, "Run");
            //                else
            //                    neko_sprite_renderer_play(sprite, "Idle");
            //
            //            neko_sprite_renderer_update(sprite, engine->ctx.platform->time.delta);
            //
            //            neko_ecs_ent player = e;
            //            CVelocity *player_v = static_cast<CVelocity *>(neko_ecs_ent_get_component(CL_GAME_USERDATA()->ecs, player, COMPONENT_VELOCITY));

            neko_vec2_t min = neko_vec2_add(*p, *v);
            neko_vec2_t max = neko_vec2_add(min, *b);

            // Resolve collision and change velocity direction if necessary
            if (min.x < 0 || max.x >= ws.x) {
                v->x *= -1.f;
            }
            if (min.y < 0 || max.y >= ws.y) {
                v->y *= -1.f;
            }
            *p = neko_vec2_add(*p, *v);
        }
    }
}

neko_ecs_decl_system(render_system, RENDER_SYSTEM, 4, COMPONENT_TRANSFORM, COMPONENT_VELOCITY, COMPONENT_BOUNDS, COMPONENT_COLOR) {

    const neko_vec2_t ws = neko_platform_window_sizev(neko_platform_main_window());

    neko_idraw_defaults(&CL_GAME_USERDATA()->idraw);
    neko_idraw_camera2d(&CL_GAME_USERDATA()->idraw, ws.x, ws.y);

    for (u32 i = 0; i < neko_ecs_for_count(ecs); i++) {
        neko_ecs_ent e = neko_ecs_get_ent(ecs, i);
        if (neko_ecs_ent_has_mask(ecs, e, neko_ecs_get_mask(MOVEMENT_SYSTEM))) {
            position_t *p = (position_t *)neko_ecs_ent_get_component(ecs, e, COMPONENT_TRANSFORM);
            velocity_t *v = (velocity_t *)neko_ecs_ent_get_component(ecs, e, COMPONENT_VELOCITY);
            bounds_t *b = (bounds_t *)neko_ecs_ent_get_component(ecs, e, COMPONENT_BOUNDS);
            color_t *c = (color_t *)neko_ecs_ent_get_component(ecs, e, COMPONENT_COLOR);

            // Grab global instance of engine
            neko_t *engine = neko_instance();

            auto pi = *p;
            auto bi = *b;
            neko_idraw_rectvd(&CL_GAME_USERDATA()->idraw, pi, bi, neko_v2(0.f, 0.f), neko_v2(1.f, 1.f), (neko_color_t)*c, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);
        }
    }
}

void register_components(neko_ecs *ecs) {
    // neko_ecs, component index, component pool size, size of component, and component free func
    neko_ecs_register_component(ecs, COMPONENT_GAMEOBJECT, 1024 * 64, sizeof(CGameObjectTest), NULL);
    neko_ecs_register_component(ecs, COMPONENT_TRANSFORM, 1024 * 64, sizeof(position_t), NULL);
    neko_ecs_register_component(ecs, COMPONENT_VELOCITY, 1024 * 64, sizeof(velocity_t), NULL);
    neko_ecs_register_component(ecs, COMPONENT_BOUNDS, 1024 * 64, sizeof(bounds_t), NULL);
    neko_ecs_register_component(ecs, COMPONENT_COLOR, 1024 * 64, sizeof(color_t), NULL);
}

void register_systems(neko_ecs *ecs) {
    // ecs_run_systems 将按照系统注册的顺序运行系统
    // 如果希望单独处理每个系统 也可以使用 ecs_run_system

    neko_ecs_register_system(ecs, movement_system, ECS_SYSTEM_UPDATE);
    neko_ecs_register_system(ecs, render_system, ECS_SYSTEM_RENDER_IMMEDIATE);
    //    neko_ecs_register_system(ecs, fast_sprite_render_system, ECS_SYSTEM_RENDER_DEFERRED);
    //    neko_ecs_register_system(ecs, editor_system, ECS_SYSTEM_EDITOR);
}

sandbox_object::sandbox_object(neko_vec4_t rect) {

    this->rect = rect;

    this->draw_buffer = new neko_color_t[g_sim_chunk_width * g_sim_chunk_height];

    this->data = (unsigned char *)neko_safe_malloc(sizeof(unsigned char) * g_sim_chunk_width * g_sim_chunk_height);
    this->edgeSeen = (bool *)neko_safe_malloc(sizeof(bool) * g_sim_chunk_width * g_sim_chunk_height);

    neko_graphics_texture_desc_t t_desc = neko_default_val();

    t_desc = neko_default_val();
    t_desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    t_desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.num_mips = 0;

    t_desc.width = this->rect.w - this->rect.x;
    t_desc.height = this->rect.z - this->rect.y;

    t_desc.data[0] = this->draw_buffer;

    this->object_texture = neko_graphics_texture_create(&t_desc);
}

void sandbox_object::update() {}

void sandbox_object::render(f32 scale) {

    neko_graphics_texture_desc_t t_desc = neko_default_val();
    t_desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    t_desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.num_mips = 0;

    t_desc.width = this->rect.w - this->rect.x;
    t_desc.height = this->rect.z - this->rect.y;

    t_desc.data[0] = this->draw_buffer;

    neko_graphics_texture_update(this->object_texture, &t_desc);

    if (object_texture.id != 0) {

        //        neko_idraw_rect_textured_ext(sandbox_game::idraw, this->rect.x, this->rect.y, this->rect.x + this->rect.w * this->scale,
        //                                     this->rect.y + this->rect.z * this->scale, 0, 1, 1, 0, this->object_texture.id, NEKO_COLOR_WHITE);
        //
        //        neko_idraw_rectv(sandbox_game::idraw, neko_v2(this->rect.x,this->rect.y), neko_v2(this->rect.x + this->rect.w * this->scale, this->rect.y + this->rect.z * this->scale),
        //        NEKO_COLOR_WHITE,
        //                         NEKO_GRAPHICS_PRIMITIVE_LINES);

        neko_idraw_rect_textured_ext(sandbox_game::idraw, 0, 0, 512 * scale, 512 * scale, 0, 1, 1, 0, this->object_texture.id, NEKO_COLOR_WHITE);
    }

    if (CL_GAME_USERDATA()->game->debug_mode >= 1) this->chunk_mask_update_mesh();
}
// GameObject::~GameObject() {}

void sandbox_object::chunk_mask_update_mesh() {

    neko_immediate_draw_t *idraw = &CL_GAME_USERDATA()->idraw;

    // std::lock_guard<std::mutex> locker(g_mutex_updatechunkmesh);

    /*
    if (chunk->rb != nullptr) b2world->DestroyBody(chunk->rb->body);
    */

    // 区块坐标转化为世界坐标
    // int chTx = chunk->x * ch->chunk_w + loadZone.x;
    // int chTy = chunk->y * ch->chunk_h + loadZone.y;

    // if (chTx < 0 || chTy < 0 || chTx + ch->chunk_w >= width || chTy + kScreenHeight >= height) {
    //     return;
    // }

    auto sim = CL_GAME_USERDATA()->game->get_sim();

    // 优化掉无固态碰撞的区块
    bool foundAnything = false;
    for (int x = 0; x < g_sim_chunk_width; x++) {
        for (int y = 0; y < g_sim_chunk_height; y++) {

            auto type = sim->buffer_ptr_->GetMaterial(x, y);

            if (type == material_type::ROCK) {
                foundAnything = true;
                goto found;
            }

            // bool f = tiles[(x + meshZone.x) + (y + meshZone.y) * width].mat->physicsType == PhysicsType::SOLID;
            // foundAnything = foundAnything || f;
        }
    }

found:

    if (!foundAnything) {
        return;
    }

    // unsigned char data[ch->chunk_w * kScreenHeight] = neko_default_val();
    // bool edgeSeen[ch->chunk_w * kScreenHeight] = neko_default_val();

    for (int y = 0; y < g_sim_chunk_height; y++) {
        for (int x = 0; x < g_sim_chunk_width; x++) {
            // data[x + y * kScreenWidth] = real_tiles[(x + chTx) + (y + chTy) * width].mat->physicsType == PhysicsType::SOLID;
            //            u8 mat_id = get_particle_at(ch, x, y).mat_id;
            auto type = sim->buffer_ptr_->GetMaterial(x, y);
            this->data[x + y * g_sim_chunk_width] = (type == material_type::ROCK);
            this->edgeSeen[x + y * g_sim_chunk_width] = false;
        }
    }

    // TPPLPoly和MarchingSquares::Result都是非聚合类 需要构造函数 使用STL来存储
    std::vector<std::vector<neko_vec2>> world_meshes = {};
    std::list<neko::TPPLPoly> shapes;
    std::list<neko::marching_squares::ms_result> results;

    int inn = 0;
    int lookIndex = 0;

    // TODO: 23/10/19 优化一下啦
    u32 test_count = 0;

    while (true) {
        // inn++;
        int lookX = lookIndex % g_sim_chunk_width;
        int lookY = lookIndex / g_sim_chunk_width;
        /*if (inn == 1) {
            lookX = kScreenWidth / 2;
            lookY = kScreenHeight / 2;
        }*/

        int edgeX = -1;
        int edgeY = -1;
        int size = g_sim_chunk_width * g_sim_chunk_height;

        for (int i = lookIndex; i < size; i++) {
            if (this->data[i] != 0) {

                int numBorders = 0;
                // if (i % kScreenWidth - 1 >= 0) numBorders += data[(i % kScreenWidth - 1) + i / kScreenWidth * kScreenWidth];
                // if (i / kScreenWidth - 1 >= 0) numBorders += data[(i % kScreenWidth)+(i / kScreenWidth - 1) * kScreenWidth];
                if (i % g_sim_chunk_width + 1 < g_sim_chunk_width) numBorders += this->data[(i % g_sim_chunk_width + 1) + i / g_sim_chunk_width * g_sim_chunk_width];
                if (i / g_sim_chunk_width + 1 < g_sim_chunk_height) numBorders += this->data[(i % g_sim_chunk_width) + (i / g_sim_chunk_width + 1) * g_sim_chunk_width];
                if (i / g_sim_chunk_width + 1 < g_sim_chunk_height && i % g_sim_chunk_width + 1 < g_sim_chunk_width)
                    numBorders += this->data[(i % g_sim_chunk_width + 1) + (i / g_sim_chunk_width + 1) * g_sim_chunk_width];

                // int val = value(i % ch->render_w, i / ch->render_w, ch->render_w, height, data);
                if (numBorders != 3) {
                    edgeX = i % g_sim_chunk_width;
                    edgeY = i / g_sim_chunk_width;
                    break;
                }
            }
        }

        if (edgeX == -1) {
            break;
        }

        // marching_squares::ms_direction edge = marching_squares::find_edge(ch->render_w, ch->render_h, data, lookX, lookY);

        lookX = edgeX;
        lookY = edgeY;

        lookIndex = lookX + lookY * g_sim_chunk_width + 1;

        if (this->edgeSeen[lookX + lookY * g_sim_chunk_width]) {
            inn--;
            continue;
        }

        int val = neko::marching_squares::ms_value(lookX, lookY, g_sim_chunk_width, g_sim_chunk_height, this->data);

        if (val == 0 || val == 15) {
            inn--;
            continue;
        }

        neko::marching_squares::ms_result r = neko::marching_squares::find_perimeter(lookX, lookY, g_sim_chunk_width, g_sim_chunk_height, this->data);

        results.push_back(r);

        std::vector<neko_vec2> worldMesh;

        f32 lastX = (f32)r.initial_x;
        f32 lastY = (f32)r.initial_y;

        for (int i = 0; i < r.directions.size(); i++) {
            // if(r.directions[i].x != 0) r.directions[i].x = r.directions[i].x / abs(r.directions[i].x);
            // if(r.directions[i].y != 0) r.directions[i].y = r.directions[i].y / abs(r.directions[i].y);

            for (int ix = 0; ix < std::max(abs(r.directions[i].x), 1); ix++) {
                for (int iy = 0; iy < std::max(abs(r.directions[i].y), 1); iy++) {
                    int ilx = (int)(lastX + ix * (r.directions[i].x < 0 ? -1 : 1));
                    int ily = (int)(lastY - iy * (r.directions[i].y < 0 ? -1 : 1));

                    if (ilx < 0) ilx = 0;
                    if (ilx >= g_sim_chunk_width) ilx = g_sim_chunk_width - 1;

                    if (ily < 0) ily = 0;
                    if (ily >= g_sim_chunk_height) ily = g_sim_chunk_height - 1;

                    int ind = ilx + ily * g_sim_chunk_width;
                    if (ind >= size) {
                        continue;
                    }
                    this->edgeSeen[ind] = true;

                    test_count++;
                }
            }

            lastX += (f32)r.directions[i].x;
            lastY -= (f32)r.directions[i].y;
            worldMesh.emplace_back(neko_v2(lastX, lastY));
        }

        // 优化
        f32 simplify_tolerance = 1.f;
        if (test_count >= 800) simplify_tolerance = 3.f;
        worldMesh = neko::simplify(worldMesh, simplify_tolerance);

        // 优化单像素点
        if (worldMesh.size() < 3) continue;

        world_meshes.push_back(worldMesh);

        neko::TPPLPoly poly;

        poly.Init((long)worldMesh.size());

        for (int i = 0; i < worldMesh.size(); i++) {
            poly[(int)worldMesh.size() - i - 1] = {worldMesh[i].x, worldMesh[i].y};
        }

        if (poly.GetOrientation() == TPPL_CW) {
            poly.SetHole(true);
        }

        shapes.push_back(poly);
    }

    std::list<neko::TPPLPoly> result;
    std::list<neko::TPPLPoly> result2;

    neko::TPPLPartition part;
    neko::TPPLPartition part2;

    part.RemoveHoles(&shapes, &result2);

    part2.Triangulate_EC(&result2, &result);

    /*bool* solid = new bool[10 * 10];
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            solid[x + y * width] = rand() % 2 == 0;
        }
    }*/

    // Ps::marching_squares ms = Ps::marching_squares(solid, ch->render_w, ch->render_h);

    // Ps::marching_squares ms = Ps::marching_squares(texture);
    // worldMesh = ms.extract_simple(2);

    // chunk->polys.clear();

    //    neko_vec2 position = cam;

    std::for_each(result.begin(), result.end(), [&](neko::TPPLPoly cur) {
        // 修正重叠点
        if (cur[0].x == cur[1].x && cur[1].x == cur[2].x) {
            cur[0].x += 0.01f;
        }
        if (cur[0].y == cur[1].y && cur[1].y == cur[2].y) {
            cur[0].y += 0.01f;
        }

        std::vector<neko_vec2> vec = {neko_v2((f32)cur[0].x, (f32)cur[0].y), neko_v2((f32)cur[1].x, (f32)cur[1].y), neko_v2((f32)cur[2].x, (f32)cur[2].y)};

        std::for_each(vec.begin(), vec.end(), [&](neko_vec2 &v) {
            v.x *= CL_GAME_USERDATA()->game->draw_scale;
            v.y *= CL_GAME_USERDATA()->game->draw_scale;
            //            v.x += cam.x;
            //            v.y += cam.y;
        });

        //  worldTris.push_back(vec);
        //  b2PolygonShape sh;
        //  sh.Set(&vec[0], 3);
        //  chunk->polys.push_back(sh);

        neko_idraw_trianglev(idraw, vec[0], vec[1], vec[2], NEKO_COLOR_WHITE, NEKO_GRAPHICS_PRIMITIVE_LINES);
    });

    // neko_graphics_fc_text(std::to_string(test_count).c_str(), idraw->data->font_fc_default, 500, 100);
}

void sandbox_object::clean() {
    neko_graphics_texture_destroy(this->object_texture);
    delete[] this->draw_buffer;

    if (this->edgeSeen) neko_safe_free(this->edgeSeen);
    if (this->data) neko_safe_free(this->data);
}

neko_immediate_draw_t *sandbox_game::idraw = nullptr;

sandbox_game::sandbox_game(neko_immediate_draw_t *idraw) {
    init(idraw);
    create_simulation();
    create_viewport();
    create_ui();
    reset_variables();
}

sandbox_game::~sandbox_game() {}

void sandbox_game::init(neko_immediate_draw_t *idraw) {
    this->idraw = idraw;
    count = 0;
}

void sandbox_game::create_simulation() { simulation = new sandbox_simulation(); }

void sandbox_game::create_viewport() {
    neko_vec2 win_size = neko_platform_window_sizev(neko_platform_main_window());
    this->fbs_scale = (float)win_size.x / CL_GAME_USERDATA()->fbs.x;
    viewport = new sandbox_object(kViewportRect);
}

void sandbox_game::create_ui() {
    auto set_material = std::bind(&sandbox_game::set_material, this, std::placeholders::_1);
    auto pause = std::bind(&sandbox_game::pause, this, std::placeholders::_1);
    auto reset = std::bind(&sandbox_game::reset_simulation, this, std::placeholders::_1);
    //    ui_ = new UI(renderer_, {Helper::ScreenWidthPoint(11), 0, (int)kScreenWidth - Helper::ScreenWidthPoint(11), (int)kScreenHeight}, neko_color(128, 128, 128, 255));
    //    ui_->AddButtonGroup({0, 0, Helper::ScreenWidthPoint(4), Helper::ScreenWidthPoint(1)}, {0, 0, 120, 64}, "Pause", pause);
    //    ui_->AddButtonGroup({0, Helper::ScreenHeightPoint(1), Helper::ScreenWidthPoint(4), Helper::ScreenWidthPoint(2)}, {0, 0, 120, 64}, "Reset", reset);
    //    ui_->AddButtonGroup({0, Helper::ScreenWidthPoint(2), Helper::ScreenWidthPoint(4), Helper::ScreenWidthPoint(5)}, {0, 0, 120, 64}, "Empty Rock Sand Water Steam Wood Fire", set_material);

    pixelui_init(&this->pixelui);

    this->pixelui.show_material_selection_panel = true;
    this->pixelui.show_frame_count = true;
}

void sandbox_game::set_material(int m) { material = static_cast<material_type>(material); }

void sandbox_game::pause(int x) { paused = !paused; }

void sandbox_game::reset_simulation(int x) { simulation->reset(); }

void sandbox_game::reset_variables() {
    tick_count = 0;
    draw_radius = 15;
    material = material_type::ROCK;
    draw_scale = 3.f;
    debug_mode = 0;
}

void sandbox_game::pre_update() {
    current_tick = neko_platform_elapsed_time();
    frame_count++;
    //    delta_time = (current_tick - last_tick) / 1000.0f;

    try {
        neko_lua_wrap_call(CL_GAME_USERDATA()->L, "game_pre_update");
    } catch (std::exception &ex) {
        neko_log_error("lua exception %s", ex.what());
    }

    f32 dt = neko_platform_delta_time();

    if (neko_platform_key_pressed(NEKO_KEYCODE_ESC)) CL_GAME_USERDATA()->g_cvar.show_editor ^= true;

    {
        try {
            neko_lua_wrap_call<void, f32>(CL_GAME_USERDATA()->L, "game_loop", dt);
        } catch (std::exception &ex) {
            neko_log_error("lua exception %s", ex.what());
        }
    }

    if (neko_platform_was_key_down(NEKO_KEYCODE_W)) CL_GAME_USERDATA()->cam.y = CL_GAME_USERDATA()->cam.y - CL_GAME_USERDATA()->player_v * CL_GAME_USERDATA()->game->draw_scale * dt;
    if (neko_platform_was_key_down(NEKO_KEYCODE_S)) CL_GAME_USERDATA()->cam.y = CL_GAME_USERDATA()->cam.y + CL_GAME_USERDATA()->player_v * CL_GAME_USERDATA()->game->draw_scale * dt;
    if (neko_platform_was_key_down(NEKO_KEYCODE_A)) CL_GAME_USERDATA()->cam.x = CL_GAME_USERDATA()->cam.x - CL_GAME_USERDATA()->player_v * CL_GAME_USERDATA()->game->draw_scale * dt;
    if (neko_platform_was_key_down(NEKO_KEYCODE_D)) CL_GAME_USERDATA()->cam.x = CL_GAME_USERDATA()->cam.x + CL_GAME_USERDATA()->player_v * CL_GAME_USERDATA()->game->draw_scale * dt;

    if (neko_platform_key_pressed(NEKO_KEYCODE_F)) CL_GAME_USERDATA()->game->debug_mode = (CL_GAME_USERDATA()->game->debug_mode + 1) % 4;

    draw_radius = std::clamp<s16>(draw_radius, kMinDrawRadius, kMaxDrawRadius);
    neko_vec2 foo = neko_platform_mouse_positionv();
    neko_vec2 win_size = neko_platform_window_sizev(neko_platform_main_window());

    this->fbs_scale = (float)win_size.x / CL_GAME_USERDATA()->fbs.x;

    foo.x = foo.x / this->fbs_scale;
    foo.y = foo.y / this->fbs_scale;

    foo.x += CL_GAME_USERDATA()->cam.x;
    foo.y += CL_GAME_USERDATA()->cam.y;

    cursor = foo;

    this->material = this->pixelui.material_selection;
}

void sandbox_game::update() {
    pre_update();

    auto draw_pos = cursor;
    draw_pos.x = draw_pos.x / this->draw_scale;
    draw_pos.y = draw_pos.y / this->draw_scale;

    if (neko_platform_mouse_down(NEKO_MOUSE_LBUTTON) && !ImGui::GetIO().WantCaptureMouse) {
        simulation->SetCellInsideCircle(draw_pos, draw_radius, material);
    } else if (neko_platform_mouse_down(NEKO_MOUSE_RBUTTON) && !ImGui::GetIO().WantCaptureMouse) {
        simulation->SetCellInsideCircle(draw_pos, draw_radius, material_type::EMPTY);
    }

    neko_vec2 mw = neko_platform_mouse_wheelv();
    if (neko_platform_was_key_down(NEKO_KEYCODE_LEFT_CONTROL)) {
        if (mw.y > 0)
            this->draw_radius++;
        else if (mw.y < 0)
            this->draw_radius--;
    } else {
        if (mw.y > 0)
            this->pixelui.material_selection = (material_type)((u32)this->pixelui.material_selection - 1);
        else if (mw.y < 0)
            this->pixelui.material_selection = (material_type)((u32)this->pixelui.material_selection + 1);
    }

    if (!paused) {
        int pitch = (g_sim_chunk_width) * 8;
        //        SDL_LockTexture(viewport_->object_texture_ptr_->texture_, nullptr, (void **)&viewport_->draw_buffer_, &pitch);
        simulation->update();
        for (int i = 0; i < g_sim_chunk_width; i++) {
            for (int j = 0; j < g_sim_chunk_height; j++) {
                s64 foo = i + j * g_sim_chunk_width;
                viewport->draw_buffer[foo] = simulation->buffer_ptr_->GetCellColor(i, j);
            }
        }
        //        SDL_UnlockTexture(viewport_->object_texture_ptr_->texture_);
    }
    late_update();
}

void sandbox_game::late_update() {
    //    last_cursor_ = cursor;
    std::ostringstream stream;
    u32 last_update = neko_platform_elapsed_time();
    last_tick = current_tick;
    float frame_time = (last_update - current_tick) / 1000.0f;
    tick_count += last_update - current_tick;
    if (count++ == 60) {
        count = 0;
        stream << std::fixed << std::setprecision(2) << "Current: " << (1.0f / frame_time) << " fps Avg: " << (1000 / std::max<u32>(tick_count / frame_count, 1)) << " fps";
        //        performance_bar_->text_.back().text_ = stream.str();
        //        performance_bar_->CreateTexture();
    }
}

void sandbox_game::render() {

    neko_idraw_defaults(&CL_GAME_USERDATA()->idraw);
    neko_idraw_camera2d_ex(&CL_GAME_USERDATA()->idraw, CL_GAME_USERDATA()->cam.x, CL_GAME_USERDATA()->cam.x + CL_GAME_USERDATA()->fbs.x, CL_GAME_USERDATA()->cam.y,
                           CL_GAME_USERDATA()->cam.y + CL_GAME_USERDATA()->fbs.y);

    viewport->render(draw_scale);

    if (debug_mode >= 2) {
        for (int i = 0; i < kMaxChunk; i++) {
            (*simulation->chunks_ptr_)[i].draw_debug(sandbox_game::idraw, this->draw_scale);
        }
    }
    //    neko_idraw_circle(sandbox_game::idraw, cursor.x, cursor.y, draw_radius * this->draw_scale, 20, 100, 150, 220, 255, NEKO_GRAPHICS_PRIMITIVE_LINES);

    this->pixelui.brush_size = draw_radius;

    //    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());
    neko_idraw_defaults(sandbox_game::idraw);
    neko_idraw_camera2d(sandbox_game::idraw, (u32)CL_GAME_USERDATA()->fbs.x, (u32)CL_GAME_USERDATA()->fbs.y);

    update_ui(&this->pixelui);
    neko_idraw_rect_textured_ext(sandbox_game::idraw, 0, 0, CL_GAME_USERDATA()->fbs.x, CL_GAME_USERDATA()->fbs.y, 0, 1, 1, 0, this->pixelui.tex_ui.id, NEKO_COLOR_WHITE);
}

void sandbox_game::clean() {

    pixelui_destroy(&this->pixelui);

    delete simulation;
    //    delete[] viewport->draw_buffer;

    this->viewport->clean();

    delete viewport;
}

void neko_register(lua_State *L);

lua_State *neko_scripting_init() {
    neko::timer timer;
    timer.start();

    lua_State *L = neko_lua_wrap_create();

    try {
        lua_atpanic(
                L, +[](lua_State *L) {
                    auto msg = neko_lua_to<const_str>(L, -1);
                    neko_log_error("[lua] panic error: %s", msg);
                    return 0;
                });
        neko_register(L);

        neko_lua_wrap_run_string(L, std::format("package.path = "
                                                "'{1}/?.lua;{0}/?.lua;{0}/../libs/?.lua;{0}/../libs/?/init.lua;{0}/../libs/"
                                                "?/?.lua;' .. package.path",
                                                game_assets("lua_scripts"), neko::fs_normalize_path(std::filesystem::current_path().string()).c_str()));

        neko_lua_wrap_run_string(L, std::format("package.cpath = "
                                                "'{1}/?.{2};{0}/?.{2};{0}/../libs/?.{2};{0}/../libs/?/init.{2};{0}/../libs/"
                                                "?/?.{2};' .. package.cpath",
                                                game_assets("lua_scripts"), neko::fs_normalize_path(std::filesystem::current_path().string()).c_str(), "dll"));

        neko_lua_wrap_safe_dofile(L, "init");

    } catch (std::exception &ex) {
        neko_log_error("%s", ex.what());
    }

    timer.stop();
    neko_log_info(std::format("lua init done in {0:.3f} ms", timer.get()).c_str());

    return L;
}

void neko_scripting_end(lua_State *L) { neko_lua_wrap_destory(L); }

int init_work(void *user_data) {

    neko_thread_atomic_int_t *this_init_thread_flag = (neko_thread_atomic_int_t *)user_data;

    if (thread_atomic_int_load(this_init_thread_flag) == 0) {

        neko_thread_timer_t thread_timer;
        thread_timer_init(&thread_timer);

        neko::timer timer;
        timer.start();

        try {
            neko_lua_wrap_call(CL_GAME_USERDATA()->L, "game_init_thread");
        } catch (std::exception &ex) {
            neko_log_error("lua exception %s", ex.what());
        }

        timer.stop();
        neko_log_info(std::format("game_init_thread loading done in {0:.3f} ms", timer.get()).c_str());

        // thread_timer_wait(&thread_timer, 1000000000);  // sleep for a second

        thread_timer_term(&thread_timer);

        thread_atomic_int_store(this_init_thread_flag, 1);
    }

    return 0;
}

void game_init() {
    CL_GAME_USERDATA()->cb = neko_command_buffer_new();
    CL_GAME_USERDATA()->idraw = neko_immediate_draw_new();

    neko_graphics_info_t *info = neko_graphics_info();
    if (!info->compute.available) {
        neko_log_error("%s", "Compute shaders not available.");
        neko_quit();
        return;
    }

    CL_GAME_USERDATA()->am = neko_asset_manager_new();

    CL_GAME_USERDATA()->g_assetsys = neko_assetsys_create(0);

    neko_assetsys_mount(CL_GAME_USERDATA()->g_assetsys, "./gamedir", "/gamedir");

    //    g_client_userdata.cb = &CL_GAME_USERDATA()->cb;
    //    g_client_userdata.idraw = &CL_GAME_USERDATA()->idraw;
    //    CL_GAME_USERDATA()->core_ui = &CL_GAME_USERDATA()->ui;
    //    CL_GAME_USERDATA()->idraw_sd = neko_immediate_draw_static_data_get();

    //    g_client_userdata.pack = &CL_GAME_USERDATA()->lua_pack;

    neko_pack_result result = neko_pack_read(game_assets("gamedir/sc.pack").c_str(), 0, false, &CL_GAME_USERDATA()->lua_pack);
    neko_pack_check(result);

    CL_GAME_USERDATA()->L = neko_scripting_init();

    {
        neko_lua_wrap_register_t<>(CL_GAME_USERDATA()->L).def(+[](const_str path) -> std::string { return game_assets(path); }, "__neko_file_path");

        lua_newtable(CL_GAME_USERDATA()->L);
        for (int n = 0; n < neko_instance()->ctx.game.argc; ++n) {
            lua_pushstring(CL_GAME_USERDATA()->L, neko_instance()->ctx.game.argv[n]);
            lua_rawseti(CL_GAME_USERDATA()->L, -2, n);
        }
        lua_setglobal(CL_GAME_USERDATA()->L, "arg");

        neko_lua_wrap_safe_dofile(CL_GAME_USERDATA()->L, "main");

        // 获取 neko_game.table
        lua_getglobal(CL_GAME_USERDATA()->L, "neko_game");
        if (!lua_istable(CL_GAME_USERDATA()->L, -1)) {
            neko_log_error("%s", "neko_game is not a table");
        }

        neko_platform_running_desc_t t = {.title = "Neko Engine", .engine_args = ""};
        if (lua_getfield(CL_GAME_USERDATA()->L, -1, "app") == LUA_TNIL) throw std::exception("no app");
        if (lua_istable(CL_GAME_USERDATA()->L, -1)) {
            neko::static_refl::neko_type_info<neko_platform_running_desc_t>::ForEachVarOf(t, [](auto field, auto &&value) {
                static_assert(std::is_lvalue_reference_v<decltype(value)>);
                if (lua_getfield(CL_GAME_USERDATA()->L, -1, std::string(field.name).c_str()) != LUA_TNIL) value = neko_lua_to<std::remove_reference_t<decltype(value)>>(CL_GAME_USERDATA()->L, -1);
                lua_pop(CL_GAME_USERDATA()->L, 1);
            });
        } else {
            throw std::exception("no app table");
        }
        lua_pop(CL_GAME_USERDATA()->L, 1);

        neko_engine_cvar_t cvar;
        if (lua_getfield(CL_GAME_USERDATA()->L, -1, "cvar") == LUA_TNIL) throw std::exception("no cvar");
        if (lua_istable(CL_GAME_USERDATA()->L, -1)) {
            neko::static_refl::neko_type_info<neko_engine_cvar_t>::ForEachVarOf(cvar, [](auto field, auto &&value) {
                static_assert(std::is_lvalue_reference_v<decltype(value)>);
                if (lua_getfield(CL_GAME_USERDATA()->L, -1, std::string(field.name).c_str()) != LUA_TNIL) value = neko_lua_to<std::remove_reference_t<decltype(value)>>(CL_GAME_USERDATA()->L, -1);
                lua_pop(CL_GAME_USERDATA()->L, 1);
            });
            CL_GAME_USERDATA()->g_cvar = cvar;  // 复制到 g_cvar
        } else {
            throw std::exception("no cvar table");
        }
        lua_pop(CL_GAME_USERDATA()->L, 1);

        lua_pop(CL_GAME_USERDATA()->L, 1);  // 弹出 neko_game.table

        neko_log_info("load game: %s %d %d", t.title, t.width, t.height);

        neko_platform_set_window_title(neko_platform_main_window(), t.title);
    }

    try {
        neko_lua_wrap_call(CL_GAME_USERDATA()->L, "game_init");
    } catch (std::exception &ex) {
        neko_log_error("lua exception %s", ex.what());
    }

    result = neko_pack_read(game_assets("gamedir/res.pack").c_str(), 0, false, &CL_GAME_USERDATA()->pack);
    neko_pack_check(result);

    u8 *font_data, *cat_data;
    u32 font_data_size, cat_data_size;

    result = neko_pack_item_data(&CL_GAME_USERDATA()->pack, ".\\data\\assets\\fonts\\fusion-pixel-12px-monospaced.ttf", (const u8 **)&font_data, &font_data_size);
    neko_pack_check(result);

    result = neko_pack_item_data(&CL_GAME_USERDATA()->pack, ".\\data\\assets\\textures\\cat.aseprite", (const u8 **)&cat_data, &cat_data_size);
    neko_pack_check(result);

    CL_GAME_USERDATA()->test_ase = load_ase_texture_simple(cat_data, cat_data_size);

    neko_asset_texture_t tex0 = {0};
    neko_asset_texture_load_from_file(game_assets("gamedir/assets/textures/yzh.png").c_str(), &tex0, NULL, false, false);
    CL_GAME_USERDATA()->tex_hndl = neko_assets_create_asset(&CL_GAME_USERDATA()->am, neko_asset_texture_t, &tex0);

    neko_ui_init(&CL_GAME_USERDATA()->ui, neko_platform_main_window());

    // 加载自定义字体文件 初始化 gui font stash
    // neko_asset_ascii_font_load_from_memory(font_data, font_data_size, &CL_GAME_USERDATA()->font, 24);
    neko_asset_ascii_font_load_from_file(game_assets("gamedir/assets/fonts/monocraft.ttf").c_str(), &CL_GAME_USERDATA()->font, 24);

    neko_pack_item_free(&CL_GAME_USERDATA()->pack, font_data);
    neko_pack_item_free(&CL_GAME_USERDATA()->pack, cat_data);

    auto GUI_FONT_STASH = []() -> neko_ui_font_stash_desc_t * {
        static neko_ui_font_desc_t font_decl[] = {{.key = "mc_regular", .font = &CL_GAME_USERDATA()->font}};
        static neko_ui_font_stash_desc_t font_stash = {.fonts = font_decl, .size = 1 * sizeof(neko_ui_font_desc_t)};
        return &font_stash;
    }();

    // neko_ui_font_stash_desc_t font_stash = {.fonts = (neko_ui_font_desc_t[]){{.key = "mc_regular", .font = &font}, .size = 1 * sizeof(neko_ui_font_desc_t)};
    neko_ui_init_font_stash(&CL_GAME_USERDATA()->ui, GUI_FONT_STASH);

    // Load style sheet from file now
    app_load_style_sheet(false);

    // Dock windows before hand
    neko_ui_dock_ex(&CL_GAME_USERDATA()->ui, "Style_Editor", "Demo_Window", NEKO_UI_SPLIT_TAB, 0.5f);

    CL_GAME_USERDATA()->imgui = neko_imgui_new(&CL_GAME_USERDATA()->cb, neko_platform_main_window(), false);

    //    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());

    font_render = neko_graphics_batch_make_ctx(32);

    const char *font_vs = R"(
#version 330

uniform mat4 u_mvp;
in vec2 in_pos; in vec2 in_uv;
out vec2 v_uv;

void main() {
    v_uv = in_uv;
    gl_Position = u_mvp * vec4(in_pos, 0, 1);
})";

    const char *font_ps = R"(
#version 330
precision mediump float;

uniform sampler2D u_sprite_texture;
in vec2 v_uv; out vec4 out_col;

void main() { out_col = texture(u_sprite_texture, v_uv); }
)";

    neko_graphics_batch_vertex_data_t font_vd;
    neko_graphics_batch_make_vertex_data(&font_vd, 1024 * 1024, GL_TRIANGLES, sizeof(neko_font_vert_t), GL_DYNAMIC_DRAW);
    neko_graphics_batch_add_attribute(&font_vd, "in_pos", 2, NEKO_GL_CUSTOM_FLOAT, neko_offset(neko_font_vert_t, x));
    neko_graphics_batch_add_attribute(&font_vd, "in_uv", 2, NEKO_GL_CUSTOM_FLOAT, neko_offset(neko_font_vert_t, u));

    neko_graphics_batch_make_renderable(&font_renderable, &font_vd);
    neko_graphics_batch_load_shader(&font_shader, font_vs, font_ps);
    neko_graphics_batch_set_shader(&font_renderable, &font_shader);

    neko_graphics_batch_ortho_2d(CL_GAME_USERDATA()->fbs.x / font_scale, CL_GAME_USERDATA()->fbs.y / font_scale, 0, 0, font_projection);

    neko_graphics_batch_send_matrix(&font_shader, "u_mvp", font_projection);

    size_t test_font_size = 0;
    void *test_font_mem = neko_platform_read_file_contents(game_assets("gamedir/1.fnt").c_str(), "rb", &test_font_size);
    neko_png_image_t img = neko_png_load(game_assets("gamedir/1_0.png").c_str());
    test_font_tex_id = generate_texture_handle(img.pix, img.w, img.h, NULL);
    test_font_bmfont = neko_font_load_bmfont(test_font_tex_id, test_font_mem, test_font_size, 0);
    if (test_font_bmfont->atlas_w != img.w || test_font_bmfont->atlas_h != img.h) {
        neko_log_warning("failed to load font");
    }
    neko_safe_free(test_font_mem);
    neko_png_free(&img);

    g_client_userdata.test_font_bmfont = test_font_bmfont;

    font_verts = (neko_font_vert_t *)neko_safe_malloc(sizeof(neko_font_vert_t) * 1024 * 2);

    // Load a file
    neko_assetsys_file_t file;
    neko_assetsys_file(CL_GAME_USERDATA()->g_assetsys, "/gamedir/style_sheets/temp.ss", &file);
    int size = neko_assetsys_file_size(CL_GAME_USERDATA()->g_assetsys, file);
    // char *content = (char *)malloc(size + 1);  // extra space for '\0'
    // int loaded_size = 0;
    // neko_assetsys_file_load(g_assetsys, file, &loaded_size, content, size);
    // content[size] = '\0';  // zero terminate the text file
    // printf("%s\n", content);
    // free(content);

    // Construct frame buffer
    CL_GAME_USERDATA()->main_fbo = neko_graphics_framebuffer_create(NULL);

    neko_graphics_texture_desc_t main_rt_desc = {.width = (u32)CL_GAME_USERDATA()->fbs.x,
                                                 .height = (u32)CL_GAME_USERDATA()->fbs.y,
                                                 .format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8,
                                                 .wrap_s = NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT,
                                                 .wrap_t = NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT,
                                                 .min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST,
                                                 .mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST};
    CL_GAME_USERDATA()->main_rt = neko_graphics_texture_create(&main_rt_desc);

    neko_graphics_renderpass_desc_t main_rp_desc = {.fbo = CL_GAME_USERDATA()->main_fbo, .color = &CL_GAME_USERDATA()->main_rt, .color_size = sizeof(CL_GAME_USERDATA()->main_rt)};
    CL_GAME_USERDATA()->main_rp = neko_graphics_renderpass_create(&main_rp_desc);

    CL_GAME_USERDATA()->game = new sandbox_game(&CL_GAME_USERDATA()->idraw);

    const neko_vec2_t ws = neko_platform_window_sizev(neko_platform_main_window());

#if 0
    // Create world
    CL_GAME_USERDATA()->world = ecs_init_w_args(0, NULL);

    neko_ecs_com_init(CL_GAME_USERDATA()->world);

    // Register system with world
    ECS_SYSTEM(CL_GAME_USERDATA()->world, move_system, EcsOnUpdate, position_t, velocity_t, bounds_t);
    ECS_SYSTEM(CL_GAME_USERDATA()->world, render_system, EcsOnUpdate, position_t, bounds_t, color_t);

    auto ecs = flecs::world(CL_GAME_USERDATA()->world);

    // Create entities with random data
    for (uint32_t i = 0; i < 5; ++i) {

        //        auto e = ecs.entity().set([](Position &p, Velocity &v) {
        //            p = {10, 20};
        //            v = {1, 2};
        //        });
        //        ecs_entity_t e = ecs_new_w_type(CL_GAME_USERDATA()->world, 0);

        ecs_entity_t e = ecs_new_id(ecs);

        neko_vec2_t bounds = neko_v2((f32)random_val(10, 100), (f32)random_val(10, 100));

        // Set data for entity
        position_t p = {(f32)random_val(0, (int32_t)ws.x - (int32_t)bounds.x), (f32)random_val(0, (int32_t)ws.y - (int32_t)bounds.y)};
        ecs_set_id(CL_GAME_USERDATA()->world, e, ecs_id(position_t), sizeof(position_t), &p);
        velocity_t v = {(f32)random_val(1, 10), (f32)random_val(1, 10)};
        ecs_set_id(CL_GAME_USERDATA()->world, e, ecs_id(velocity_t), sizeof(velocity_t), &v);
        bounds_t b = {(f32)random_val(10, 100), (f32)random_val(10, 100)};
        ecs_set_id(CL_GAME_USERDATA()->world, e, ecs_id(bounds_t), sizeof(bounds_t), &b);
        color_t c = {(u8)random_val(50, 255), (u8)random_val(50, 255), (u8)random_val(50, 255), 255};
        ecs_set_id(CL_GAME_USERDATA()->world, e, ecs_id(color_t), sizeof(color_t), &c);
    }
#endif
    CL_GAME_USERDATA()->ecs = neko_ecs_make(1024 * 64, COMPONENT_COUNT, 2);

    register_components(CL_GAME_USERDATA()->ecs);
    register_systems(CL_GAME_USERDATA()->ecs);

    // 测试用
    neko_ecs_ent e1 = neko_ecs_ent_make(CL_GAME_USERDATA()->ecs);
    CGameObjectTest gameobj = neko_default_val();

#if 0

    neko_ui_renderer ui_render = neko_default_val();
    ui_render.type = neko_ui_renderer::type::LABEL;
    neko_snprintf(ui_render.text, 128, "%s", "啊对对对");

    // neko_fast_sprite_renderer test_fast_sprite = neko_default_val();
    // neko_fast_sprite_renderer_construct(&test_fast_sprite, 0, 0, NULL);

    gameobj.visible = true;
    gameobj.active = true;
    neko_snprintf(gameobj.name, 64, "%s", "AAA_ent");

    neko_ecs_ent_add_component(CL_GAME_USERDATA()->ecs, e1, COMPONENT_GAMEOBJECT, &gameobj);
    neko_ecs_ent_add_component(CL_GAME_USERDATA()->ecs, e1, COMPONENT_TRANSFORM, &xform);
    neko_ecs_ent_add_component(CL_GAME_USERDATA()->ecs, e1, COMPONENT_VELOCITY, &velocity);

#endif

    for (int i = 0; i < 1024 * 16; i++) {

        neko_vec2_t bounds = neko_v2((f32)random_val(10, 100), (f32)random_val(10, 100));

        position_t p = {(f32)random_val(0, (int32_t)ws.x - (int32_t)bounds.x), (f32)random_val(0, (int32_t)ws.y - (int32_t)bounds.y)};
        velocity_t v = {(f32)random_val(1, 10), (f32)random_val(1, 10)};
        bounds_t b = {(f32)random_val(10, 100), (f32)random_val(10, 100)};
        color_t c = {(u8)random_val(50, 255), (u8)random_val(50, 255), (u8)random_val(50, 255), 255};

        neko_snprintf(gameobj.name, 64, "%s_%d", "Test_ent", i);
        gameobj.visible = true;
        gameobj.active = false;

        neko_ecs_ent e = neko_ecs_ent_make(CL_GAME_USERDATA()->ecs);
        neko_ecs_ent_add_component(CL_GAME_USERDATA()->ecs, e, COMPONENT_GAMEOBJECT, &gameobj);
        neko_ecs_ent_add_component(CL_GAME_USERDATA()->ecs, e, COMPONENT_TRANSFORM, &p);
        neko_ecs_ent_add_component(CL_GAME_USERDATA()->ecs, e, COMPONENT_VELOCITY, &v);
        neko_ecs_ent_add_component(CL_GAME_USERDATA()->ecs, e, COMPONENT_BOUNDS, &b);
        neko_ecs_ent_add_component(CL_GAME_USERDATA()->ecs, e, COMPONENT_COLOR, &c);
    }

    // 初始化工作

    thread_atomic_int_store(&CL_GAME_USERDATA()->init_thread_flag, 0);

    CL_GAME_USERDATA()->init_work_thread = thread_init(init_work, &CL_GAME_USERDATA()->init_thread_flag, "init_work_thread", THREAD_STACK_SIZE_DEFAULT);
}

void game_loop() {

    int this_init_thread_flag = thread_atomic_int_load(&CL_GAME_USERDATA()->init_thread_flag);

    if (this_init_thread_flag == 0) {

        //        neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());
        const f32 t = neko_platform_elapsed_time();

        u8 tranp = 255;

        // tranp -= ((s32)t % 255);

        neko_graphics_clear_action_t clear = {.color = {CL_GAME_USERDATA()->g_cvar.bg[0] / 255, CL_GAME_USERDATA()->g_cvar.bg[1] / 255, CL_GAME_USERDATA()->g_cvar.bg[2] / 255, 1.f}};
        neko_graphics_renderpass_begin(&CL_GAME_USERDATA()->cb, neko_renderpass_t{0});
        { neko_graphics_clear(&CL_GAME_USERDATA()->cb, clear); }
        neko_graphics_renderpass_end(&CL_GAME_USERDATA()->cb);

        // Set up 2D camera for projection matrix
        neko_idraw_defaults(&CL_GAME_USERDATA()->idraw);
        neko_idraw_camera2d(&CL_GAME_USERDATA()->idraw, (u32)CL_GAME_USERDATA()->fbs.x, (u32)CL_GAME_USERDATA()->fbs.y);

        // 底层图片
        char background_text[64] = "Project: unknown";

        neko_vec2 td = neko_asset_ascii_font_text_dimensions(neko_idraw_default_font(), background_text, -1);
        neko_vec2 ts = neko_v2(512 + 128, 512 + 128);

        neko_idraw_text(&CL_GAME_USERDATA()->idraw, (CL_GAME_USERDATA()->fbs.x - td.x) * 0.5f, (CL_GAME_USERDATA()->fbs.y - td.y) * 0.5f + ts.y / 2.f - 100.f, background_text, NULL, false,
                        neko_color(255, 255, 255, 255));
        neko_idraw_texture(&CL_GAME_USERDATA()->idraw, CL_GAME_USERDATA()->test_ase);
        neko_idraw_rectvd(&CL_GAME_USERDATA()->idraw, neko_v2((CL_GAME_USERDATA()->fbs.x - ts.x) * 0.5f, (CL_GAME_USERDATA()->fbs.y - ts.y) * 0.5f - td.y - 50.f), ts, neko_v2(0.f, 1.f),
                          neko_v2(1.f, 0.f), neko_color(255, 255, 255, tranp), NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);

        neko_graphics_renderpass_begin(&CL_GAME_USERDATA()->cb, NEKO_GRAPHICS_RENDER_PASS_DEFAULT);
        {
            neko_graphics_set_viewport(&CL_GAME_USERDATA()->cb, 0, 0, (u32)CL_GAME_USERDATA()->fbs.x, (u32)CL_GAME_USERDATA()->fbs.y);
            neko_idraw_draw(&CL_GAME_USERDATA()->idraw, &CL_GAME_USERDATA()->cb);  // 立即模式绘制 idraw
        }
        neko_graphics_renderpass_end(&CL_GAME_USERDATA()->cb);

        // Submits to cb
        neko_graphics_command_buffer_submit(&CL_GAME_USERDATA()->cb);

    } else {

        static int init_retval = 1;
        if (init_retval) {
            init_retval = thread_join(CL_GAME_USERDATA()->init_work_thread);
            thread_term(CL_GAME_USERDATA()->init_work_thread);
            neko_log_trace("init_work_thread done");
        }

        //        neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());
        neko_vec2 win_size = neko_platform_window_sizev(neko_platform_main_window());
        const f64 t = neko_platform_elapsed_time();
        neko_vec2 mp = neko_platform_mouse_positionv();
        neko_vec2 mw = neko_platform_mouse_wheelv();
        neko_vec2 md = neko_platform_mouse_deltav();
        bool lock = neko_platform_mouse_locked();
        bool moved = neko_platform_mouse_moved();

        CL_GAME_USERDATA()->game->update();

        // Do rendering
        neko_graphics_clear_action_t clear = {.color = {CL_GAME_USERDATA()->g_cvar.bg[0] / 255, CL_GAME_USERDATA()->g_cvar.bg[1] / 255, CL_GAME_USERDATA()->g_cvar.bg[2] / 255, 1.f}};
        neko_graphics_clear_action_t b_clear = {.color = {0.0f, 0.0f, 0.0f, 1.f}};

        neko_graphics_renderpass_begin(&CL_GAME_USERDATA()->cb, NEKO_GRAPHICS_RENDER_PASS_DEFAULT);
        { neko_graphics_clear(&CL_GAME_USERDATA()->cb, clear); }
        neko_graphics_renderpass_end(&CL_GAME_USERDATA()->cb);

        // // 设置投影矩阵的 2D 相机
        // neko_idraw_defaults(&CL_GAME_USERDATA()->idraw);
        // neko_idraw_camera2d(&CL_GAME_USERDATA()->idraw, (u32)fbs.x, (u32)fbs.y);

        // // 底层图片
        // char background_text[64] = "Project: unknown";

        // neko_vec2 td = neko_asset_ascii_font_text_dimensions(neko_idraw_default_font(), background_text, -1);
        // neko_vec2 ts = neko_v2(512 + 128, 512 + 128);

        // neko_idraw_text(&CL_GAME_USERDATA()->idraw, (fbs.x - td.x) * 0.5f, (fbs.y - td.y) * 0.5f + ts.y / 2.f - 100.f, background_text, NULL, false, 255, 255, 255, 255);
        // neko_idraw_texture(&CL_GAME_USERDATA()->idraw, g_test_ase);
        // neko_idraw_rectvd(&CL_GAME_USERDATA()->idraw, neko_v2((fbs.x - ts.x) * 0.5f, (fbs.y - ts.y) * 0.5f - td.y - 50.f), ts, neko_v2(0.f, 1.f), neko_v2(1.f, 0.f), NEKO_COLOR_WHITE,
        // NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);

        neko_graphics_renderpass_begin(&CL_GAME_USERDATA()->cb, NEKO_GRAPHICS_RENDER_PASS_DEFAULT);
        {
            neko_graphics_set_viewport(&CL_GAME_USERDATA()->cb, 0, 0, (u32)CL_GAME_USERDATA()->fbs.x, (u32)CL_GAME_USERDATA()->fbs.y);
            neko_idraw_draw(&CL_GAME_USERDATA()->idraw, &CL_GAME_USERDATA()->cb);  // 立即模式绘制 idraw
        }
        neko_graphics_renderpass_end(&CL_GAME_USERDATA()->cb);

        neko_imgui_new_frame(&CL_GAME_USERDATA()->imgui);

        try {
            neko_lua_wrap_call(CL_GAME_USERDATA()->L, "game_render");
        } catch (std::exception &ex) {
            neko_log_error("lua exception %s", ex.what());
        }

#pragma region gui

        neko_ui_begin(&CL_GAME_USERDATA()->ui, NULL);
        {
            editor_dockspace(&CL_GAME_USERDATA()->ui);

            if (CL_GAME_USERDATA()->g_cvar.show_gui) {

                neko_ui_demo_window(&CL_GAME_USERDATA()->ui, neko_ui_rect(100, 100, 500, 500), NULL);
                neko_ui_style_editor(&CL_GAME_USERDATA()->ui, NULL, neko_ui_rect(350, 250, 300, 240), NULL);

                const neko_vec2 ws = neko_v2(600.f, 300.f);

#pragma region gui_ss

#if 1

                const neko_ui_style_sheet_t *ss = &CL_GAME_USERDATA()->style_sheet;

                const neko_vec2 ss_ws = neko_v2(500.f, 300.f);
                neko_ui_window_begin(&CL_GAME_USERDATA()->ui, "Window", neko_ui_rect((CL_GAME_USERDATA()->fbs.x - ss_ws.x) * 0.5f, (CL_GAME_USERDATA()->fbs.y - ss_ws.y) * 0.5f, ss_ws.x, ss_ws.y));
                {
                    // Cache the current container
                    neko_ui_container_t *cnt = neko_ui_get_current_container(&CL_GAME_USERDATA()->ui);

                    neko_ui_layout_row(&CL_GAME_USERDATA()->ui, 2, neko_ui_widths(200, 0), 0);

                    neko_ui_text(&CL_GAME_USERDATA()->ui, "A regular element button.");
                    neko_ui_button(&CL_GAME_USERDATA()->ui, "button");

                    neko_ui_text(&CL_GAME_USERDATA()->ui, "A regular element label.");
                    neko_ui_label(&CL_GAME_USERDATA()->ui, "label");

                    neko_ui_text(&CL_GAME_USERDATA()->ui, "Button with classes: {.c0 .btn}");

                    neko_ui_selector_desc_t selector_1 = {.classes = {"c0", "btn"}};
                    neko_ui_button_ex(&CL_GAME_USERDATA()->ui, "hello?##btn", &selector_1, 0x00);

                    neko_ui_text(&CL_GAME_USERDATA()->ui, "Label with id #lbl and class .c0");
                    neko_ui_selector_desc_t selector_2 = {.id = "lbl", .classes = {"c0"}};
                    neko_ui_label_ex(&CL_GAME_USERDATA()->ui, "label##lbl", &selector_2, 0x00);

                    const f32 m = cnt->body.w * 0.3f;
                    // neko_ui_layout_row(gui, 2, (int[]){m, -m}, 0);
                    // neko_ui_layout_next(gui); // Empty space at beginning
                    neko_ui_layout_row(&CL_GAME_USERDATA()->ui, 1, neko_ui_widths(0), 0);
                    neko_ui_selector_desc_t selector_3 = {.classes = {"reload_btn"}};
                    if (neko_ui_button_ex(&CL_GAME_USERDATA()->ui, "reload style sheet", &selector_3, 0x00)) {
                        app_load_style_sheet(true);
                    }

                    button_custom(&CL_GAME_USERDATA()->ui, "Hello?");
                }
                neko_ui_window_end(&CL_GAME_USERDATA()->ui);

#endif

#pragma endregion

#pragma region gui_idraw

                neko_ui_window_begin(&CL_GAME_USERDATA()->ui, "Idraw", neko_ui_rect((CL_GAME_USERDATA()->fbs.x - ws.x) * 0.2f, (CL_GAME_USERDATA()->fbs.y - ws.y) * 0.5f, ws.x, ws.y));
                {
                    // Cache the current container
                    neko_ui_container_t *cnt = neko_ui_get_current_container(&CL_GAME_USERDATA()->ui);

                    // 绘制到当前窗口中的转换对象的自定义回调
                    // 在这里，我们将容器的主体作为视口传递，但这可以是您想要的任何内容，
                    // 正如我们将在“分割”窗口示例中看到的那样。
                    // 我们还传递自定义数据（颜色），以便我们可以在回调中使用它。如果以下情况，这可以为 NULL
                    // 你不需要任何东西
                    neko_color_t color = neko_color_alpha(NEKO_COLOR_RED, (uint8_t)neko_clamp((sin(t * 0.001f) * 0.5f + 0.5f) * 255, 0, 255));
                    neko_ui_draw_custom(&CL_GAME_USERDATA()->ui, cnt->body, gui_cb, &color, sizeof(color));
                }
                neko_ui_window_end(&CL_GAME_USERDATA()->ui);

#pragma endregion
            }

            if (neko_platform_key_pressed(NEKO_KEYCODE_GRAVE_ACCENT)) {
                g_console.open = !g_console.open;
            } else if (neko_platform_key_pressed(NEKO_KEYCODE_TAB) && g_console.open) {
                g_console.autoscroll = !g_console.autoscroll;
            }

            neko_ui_layout_t l;

            if (CL_GAME_USERDATA()->g_cvar.show_gui) {

                if (neko_ui_window_begin(&CL_GAME_USERDATA()->ui, "App", neko_ui_rect(CL_GAME_USERDATA()->fbs.x - 210, 30, 200, 200))) {
                    l = *neko_ui_get_layout(&CL_GAME_USERDATA()->ui);
                    neko_ui_layout_row(&CL_GAME_USERDATA()->ui, 1, neko_ui_widths(-1), 0);

                    static f32 delta, fps = neko_default_val();

                    neko_timed_action(5, {
                        delta = neko_platform_delta_time();
                        fps = 1.f / delta;
                    });

                    neko_ui_labelf("FPS: %.2lf Delta: %.6lf", fps, delta * 1000.f);

                    // neko_ui_text_fc(&CL_GAME_USERDATA()->core_ui, "喵喵昂~");

                    // neko_ui_layout_row(&CL_GAME_USERDATA()->core_ui, 1, neko_ui_widths(-1), 0);

                    neko_ui_labelf("Position: <%.2f %.2f>", mp.x, mp.y);
                    neko_ui_labelf("Wheel: <%.2f %.2f>", mw.x, mw.y);
                    neko_ui_labelf("Delta: <%.2f %.2f>", md.x, md.y);
                    neko_ui_labelf("Lock: %zu", lock);
                    neko_ui_labelf("Moved: %zu", moved);
                    // neko_ui_labelf("Hover: %zu", g_gui.mouse_is_hover);
                    neko_ui_labelf("Time: %f", t);

                    struct {
                        const char *str;
                        s32 val;
                    } btns[] = {{"Left", NEKO_MOUSE_LBUTTON}, {"Right", NEKO_MOUSE_RBUTTON}, {"Middle", NEKO_MOUSE_MBUTTON}, {NULL}};

                    bool mouse_down[3] = {0};
                    bool mouse_pressed[3] = {0};
                    bool mouse_released[3] = {0};

                    // Query mouse held down states.
                    mouse_down[NEKO_MOUSE_LBUTTON] = neko_platform_mouse_down(NEKO_MOUSE_LBUTTON);
                    mouse_down[NEKO_MOUSE_RBUTTON] = neko_platform_mouse_down(NEKO_MOUSE_RBUTTON);
                    mouse_down[NEKO_MOUSE_MBUTTON] = neko_platform_mouse_down(NEKO_MOUSE_MBUTTON);

                    // Query mouse release states.
                    mouse_released[NEKO_MOUSE_LBUTTON] = neko_platform_mouse_released(NEKO_MOUSE_LBUTTON);
                    mouse_released[NEKO_MOUSE_RBUTTON] = neko_platform_mouse_released(NEKO_MOUSE_RBUTTON);
                    mouse_released[NEKO_MOUSE_MBUTTON] = neko_platform_mouse_released(NEKO_MOUSE_MBUTTON);

                    // Query mouse pressed states. Press is a single frame click.
                    mouse_pressed[NEKO_MOUSE_LBUTTON] = neko_platform_mouse_pressed(NEKO_MOUSE_LBUTTON);
                    mouse_pressed[NEKO_MOUSE_RBUTTON] = neko_platform_mouse_pressed(NEKO_MOUSE_RBUTTON);
                    mouse_pressed[NEKO_MOUSE_MBUTTON] = neko_platform_mouse_pressed(NEKO_MOUSE_MBUTTON);

                    neko_ui_layout_row(&CL_GAME_USERDATA()->ui, 7, neko_ui_widths(100, 100, 32, 100, 32, 100, 32), 0);
                    for (u32 i = 0; btns[i].str; ++i) {
                        neko_ui_labelf("%s: ", btns[i].str);
                        neko_ui_labelf("pressed: ");
                        neko_ui_labelf("%d", mouse_pressed[btns[i].val]);
                        neko_ui_labelf("down: ");
                        neko_ui_labelf("%d", mouse_down[btns[i].val]);
                        neko_ui_labelf("released: ");
                        neko_ui_labelf("%d", mouse_released[btns[i].val]);
                    }

                    neko_ui_layout_row(&CL_GAME_USERDATA()->ui, 1, neko_ui_widths(-1), 0);
                    {
                        static neko_memory_info_t meminfo = neko_default_val();

                        neko_timed_action(60, { meminfo = neko_platform_memory_info(); });

                        neko_ui_labelf("GC MemAllocInUsed: %.2lf mb", (f64)(neko_mem_bytes_inuse() / 1048576.0));
                        neko_ui_labelf("GC MemTotalAllocated: %.2lf mb", (f64)(g_allocation_metrics.total_allocated / 1048576.0));
                        neko_ui_labelf("GC MemTotalFree: %.2lf mb", (f64)(g_allocation_metrics.total_free / 1048576.0));

                        lua_Integer kb = lua_gc(CL_GAME_USERDATA()->L, LUA_GCCOUNT, 0);
                        lua_Integer bytes = lua_gc(CL_GAME_USERDATA()->L, LUA_GCCOUNTB, 0);

                        neko_ui_labelf("Lua MemoryUsage: %.2lf mb", ((f64)kb / 1024.0f));
                        neko_ui_labelf("Lua Remaining: %.2lf mb", ((f64)bytes / 1024.0f));

                        neko_ui_labelf("ImGui MemoryUsage: %.2lf mb", ((f64)__neko_imgui_meminuse() / 1048576.0));

                        neko_ui_labelf("Virtual MemoryUsage: %.2lf mb", ((f64)meminfo.virtual_memory_used / 1048576.0));
                        neko_ui_labelf("Real MemoryUsage: %.2lf mb", ((f64)meminfo.physical_memory_used / 1048576.0));
                        neko_ui_labelf("Virtual MemoryUsage Peak: %.2lf mb", ((f64)meminfo.peak_virtual_memory_used / 1048576.0));
                        neko_ui_labelf("Real MemoryUsage Peak: %.2lf mb", ((f64)meminfo.peak_physical_memory_used / 1048576.0));

                        neko_ui_labelf("GPU MemTotalAvailable: %.2lf mb", (f64)(meminfo.gpu_total_memory / 1024.0f));
                        neko_ui_labelf("GPU MemCurrentUsage: %.2lf mb", (f64)(meminfo.gpu_memory_used / 1024.0f));

                        neko_graphics_info_t *info = &neko_subsystem(graphics)->info;

                        neko_ui_labelf("OpenGL vendor: %s", info->vendor);
                        neko_ui_labelf("OpenGL version supported: %s", info->version);

                        neko_vec2 opengl_ver = neko_platform_opengl_ver();
                        neko_ui_labelf("OpenGL version: %d.%d", (s32)opengl_ver.x, (s32)opengl_ver.y);
                    }

                    neko_ui_window_end(&CL_GAME_USERDATA()->ui);
                }
            }

            neko_vec2 fb = (&CL_GAME_USERDATA()->ui)->framebuffer_size;
            neko_ui_rect_t screen;
            //            if (embeded)
            //                screen = l.body;
            //            else
            //                screen = neko_ui_rect(0, 0, fb.x, fb.y);
            screen = neko_ui_rect(0, 0, fb.x, fb.y);
            neko_console(&g_console, &CL_GAME_USERDATA()->ui, screen, NULL);
        }
        neko_ui_end(&CL_GAME_USERDATA()->ui, true);

        if (ImGui::Begin("Debug")) {
            neko_imgui::Auto<neko_vec2_t>(CL_GAME_USERDATA()->cam, "Cam");
        }
        ImGui::End();

        if (CL_GAME_USERDATA()->g_cvar.show_demo_window) ImGui::ShowDemoWindow();

        // neko_imgui::Auto(g_cvar.bg);
        // if (g_cvar.shader_inspect && ImGui::Begin("Shaders")) {
        //     inspect_shader("sprite_shader", sprite_shader.program);  // TEST
        //     ImGui::End();
        // }

        if (CL_GAME_USERDATA()->g_cvar.show_editor && ImGui::Begin("Cvar")) {
            try {
                neko_cvar_gui(CL_GAME_USERDATA()->g_cvar);
            } catch (const std::exception &ex) {
                neko_log_error("cvar exception %s", ex.what());
            }

            neko_imgui::toggle("帧检查器", &CL_GAME_USERDATA()->g_cvar.show_profiler_window);

            ImGui::DragFloat("Max FPS", &neko_subsystem(platform)->time.max_fps, 5.0f, 30.0f, 2000.0f);

            ImGui::Separator();

            if (ImGui::Button("test_xml")) test_xml(game_assets("gamedir/tests/test.xml"));
            if (ImGui::Button("test_fgd")) test_fgd();
            if (ImGui::Button("test_se")) test_se();
            if (ImGui::Button("test_sr")) test_sr();
            if (ImGui::Button("test_ut")) test_ut();
            if (ImGui::Button("test_backtrace")) __neko_inter_stacktrace();
            if (ImGui::Button("test_containers")) test_containers();
            if (ImGui::Button("test_sexpr")) test_sexpr();
            if (ImGui::Button("test_thread")) test_thd();
            if (ImGui::Button("test_ttf")) {
                neko_timer_do(t, neko_println("%llu", t), { test_ttf(); });
            }
            if (ImGui::Button("test_ecs_cpp")) {
                forEachComponentType([&]<typename T>() {
                    constexpr auto typeName = getTypeName<T>();
                    std::printf("component type: %.*s\n", int(typeName.size()), typeName.data());

                    T val{};
                    std::printf("  props:\n");
                    forEachProp(val, [&](auto propTag, auto &propVal) {
                        std::printf("    %s: ", propTag.attribs.name.data());
                        print(propVal);
                        if (propTag.attribs.exampleFlag) {
                            std::printf(" (example flag set)");
                        }
                        std::printf("\n");
                    });
                });
            }

            ImGui::End();
        }

        if (CL_GAME_USERDATA()->g_cvar.show_editor) {
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("engine")) {
                    if (ImGui::Checkbox("vsync", &CL_GAME_USERDATA()->g_cvar.vsync)) neko_platform_enable_vsync(CL_GAME_USERDATA()->g_cvar.vsync);
                    if (ImGui::MenuItem("quit")) {
                        ImGui::OpenPopup("Delete?");
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
        }

#pragma endregion

        //        sandbox_update(sb);

        CL_GAME_USERDATA()->game->render();

        neko_graphics_renderpass_begin(&CL_GAME_USERDATA()->cb, CL_GAME_USERDATA()->main_rp);
        {
            neko_graphics_set_viewport(&CL_GAME_USERDATA()->cb, 0, 0, (u32)CL_GAME_USERDATA()->fbs.x, (u32)CL_GAME_USERDATA()->fbs.y);
            neko_graphics_clear(&CL_GAME_USERDATA()->cb, clear);
            neko_idraw_draw(&CL_GAME_USERDATA()->idraw, &CL_GAME_USERDATA()->cb);  // 立即模式绘制 idraw
            neko_graphics_draw_batch(&CL_GAME_USERDATA()->cb, font_render, 0, 0, 0);
        }
        neko_graphics_renderpass_end(&CL_GAME_USERDATA()->cb);

        neko_idraw_defaults(&CL_GAME_USERDATA()->idraw);
        neko_idraw_camera2d(&CL_GAME_USERDATA()->idraw, (u32)win_size.x, (u32)win_size.y);
        neko_idraw_texture(&CL_GAME_USERDATA()->idraw, CL_GAME_USERDATA()->main_rt);
        neko_idraw_rectvd(&CL_GAME_USERDATA()->idraw, neko_v2(0.0, 0.0), neko_v2((u32)win_size.x, (u32)win_size.y), neko_v2(0.0, 1.0), neko_v2(1.0, 0.0), neko_color(255, 255, 255, 255),
                          NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);

        //        ecs_progress(CL_GAME_USERDATA()->world, 0);
        neko_ecs_run_systems(CL_GAME_USERDATA()->ecs, ECS_SYSTEM_UPDATE);
        neko_ecs_run_systems(CL_GAME_USERDATA()->ecs, ECS_SYSTEM_RENDER_IMMEDIATE);

        neko_graphics_renderpass_begin(&CL_GAME_USERDATA()->cb, NEKO_GRAPHICS_RENDER_PASS_DEFAULT);
        {
            neko_graphics_set_viewport(&CL_GAME_USERDATA()->cb, 0.0, 0.0, win_size.x, win_size.y);
            neko_idraw_draw(&CL_GAME_USERDATA()->idraw, &CL_GAME_USERDATA()->cb);  // 立即模式绘制 idraw
            neko_ui_render(&CL_GAME_USERDATA()->ui, &CL_GAME_USERDATA()->cb);
        }
        neko_graphics_renderpass_end(&CL_GAME_USERDATA()->cb);

        neko_imgui_render(&CL_GAME_USERDATA()->imgui);

        // Submits to cb
        neko_graphics_command_buffer_submit(&CL_GAME_USERDATA()->cb);

        neko_check_gl_error();
    }
}

void game_shutdown() {

    //    sandbox_destroy(sb);

    CL_GAME_USERDATA()->game->clean();

    neko_graphics_renderpass_destroy(CL_GAME_USERDATA()->main_rp);
    neko_graphics_texture_destroy(CL_GAME_USERDATA()->main_rt);
    neko_graphics_framebuffer_destroy(CL_GAME_USERDATA()->main_fbo);

    try {
        neko_lua_wrap_call(CL_GAME_USERDATA()->L, "game_shutdown");
    } catch (std::exception &ex) {
        neko_log_error("lua exception %s", ex.what());
    }

    neko_scripting_end(CL_GAME_USERDATA()->L);
    neko_safe_free(font_verts);

    neko_font_free(test_font_bmfont);
    neko_graphics_batch_free(font_render);

    neko_immediate_draw_free(&CL_GAME_USERDATA()->idraw);
    neko_command_buffer_free(&CL_GAME_USERDATA()->cb);

    destroy_texture_handle(test_font_tex_id, NULL);

    //    ecs_fini(CL_GAME_USERDATA()->world);
    neko_ecs_destroy(CL_GAME_USERDATA()->ecs);

    neko_imgui_shutdown(&CL_GAME_USERDATA()->imgui);
    neko_ui_free(&CL_GAME_USERDATA()->ui);
    neko_pack_destroy(&CL_GAME_USERDATA()->pack);
    neko_pack_destroy(&CL_GAME_USERDATA()->lua_pack);

    neko_assetsys_destroy(CL_GAME_USERDATA()->g_assetsys);
}

neko_game_desc_t neko_main(s32 argc, char **argv) {

    // 确定运行路径
    auto current_dir = std::filesystem::path(std::filesystem::current_path());
    for (int i = 0; i < 6; ++i)
        if (std::filesystem::exists(current_dir / "gamedir") &&  //
            std::filesystem::exists(current_dir / "gamedir" / "assets")) {
            CL_GAME_USERDATA()->data_path = neko::fs_normalize_path(current_dir.string());
            neko_log_info(std::format("gamedir: {0} (base: {1})", CL_GAME_USERDATA()->data_path, std::filesystem::current_path().string()).c_str());
            break;
        } else {
            current_dir = current_dir.parent_path();
        }

    return neko_game_desc_t{.init = game_init,
                            .update = game_loop,
                            .shutdown = game_shutdown,
                            .window = {.width = 1280, .height = 720, .vsync = false, .frame_rate = 60.f, .hdpi = false, .center = true, .running_background = true},
                            .argc = argc,
                            .argv = argv,
                            .console = &g_console};
}

void editor_dockspace(neko_ui_context_t *ctx) {
    u64 opt = NEKO_UI_OPT_NOCLIP | NEKO_UI_OPT_NOFRAME | NEKO_UI_OPT_FORCESETRECT | NEKO_UI_OPT_NOTITLE | NEKO_UI_OPT_DOCKSPACE | NEKO_UI_OPT_FULLSCREEN | NEKO_UI_OPT_NOMOVE |
              NEKO_UI_OPT_NOBRINGTOFRONT | NEKO_UI_OPT_NOFOCUS | NEKO_UI_OPT_NORESIZE;
    neko_ui_window_begin_ex(ctx, "Dockspace", neko_ui_rect(350, 40, 600, 500), NULL, NULL, opt);
    {
        // Editor dockspace
    }
    neko_ui_window_end(ctx);
}

///////////////////////////////////////////////
//
//  位图边缘检测算法

#if 0

static neko_vec2 last_point;

void render_test() {

    u8* alpha = neko_tex_rgba_to_alpha((u8*)g_texture_buffer, ch->chunk_w, ch->chunk_h);
    u8* thresholded = neko_tex_alpha_to_thresholded(alpha, ch->chunk_w, ch->chunk_h, 90);
    u8* outlined = neko_tex_thresholded_to_outlined(thresholded, ch->chunk_w, ch->chunk_h);
    neko_safe_free(alpha);
    neko_safe_free(thresholded);

    neko_tex_point* outline = neko_tex_extract_outline_path(outlined, ch->chunk_w, ch->chunk_h, &l, 0);
    while (l) {
        s32 l0 = l;
        neko_tex_distance_based_path_simplification(outline, &l, 0.5f);
        // printf("simplified outline: %d -> %d\n", l0, l);

        l_check = l;

        for (s32 i = 0; i < l; i++) {
            // gfx->immediate.draw_line_ext(cb, neko_vec2_mul(last_point, {4.f, 4.f}), neko_vec2_mul(neko_vec2{(f32)outline[i].x, (f32)outline[i].y}, {4.f, 4.f}), 2.f, neko_color_white);

            last_point = {(f32)outline[i].x, (f32)outline[i].y};
        }

        outline = neko_tex_extract_outline_path(outlined, ch->chunk_w, ch->chunk_h, &l, outline);
    };

    neko_safe_free(outline);
    neko_safe_free(outlined);
}

#endif

// image manipulation functions
u8 *neko_tex_rgba_to_alpha(const u8 *data, s32 w, s32 h) {
    u8 *result = (u8 *)neko_safe_malloc(w * h);
    s32 x, y;
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++) result[y * w + x] = data[(y * w + x) * 4 + 3];
    return result;
}

u8 *neko_tex_alpha_to_thresholded(const u8 *data, s32 w, s32 h, u8 threshold) {
    u8 *result = (u8 *)neko_safe_malloc(w * h);
    s32 x, y;
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++) result[y * w + x] = data[y * w + x] >= threshold ? 255 : 0;
    return result;
}

u8 *neko_tex_dilate_thresholded(const u8 *data, s32 w, s32 h) {
    s32 x, y, dx, dy, cx, cy;
    u8 *result = (u8 *)neko_safe_malloc(w * h);
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            result[y * w + x] = 0;
            for (dy = -1; dy <= 1; dy++) {
                for (dx = -1; dx <= 1; dx++) {
                    cx = x + dx;
                    cy = y + dy;
                    if (cx >= 0 && cx < w && cy >= 0 && cy < h) {
                        if (data[cy * w + cx]) {
                            result[y * w + x] = 255;
                            dy = 1;
                            break;
                        }
                    }
                }
            }
        }
    }
    return result;
}

u8 *neko_tex_thresholded_to_outlined(const u8 *data, s32 w, s32 h) {
    u8 *result = (u8 *)neko_safe_malloc(w * h);
    s32 x, y;
    for (x = 0; x < w; x++) {
        result[x] = data[x];
        result[(h - 1) * w + x] = data[(h - 1) * w + x];
    }
    for (y = 1; y < h - 1; y++) {
        result[y * w] = data[y * w];
        for (x = 1; x < w - 1; x++) {
            if (data[y * w + x] && (!data[y * w + x - 1] || !data[y * w + x + 1] || !data[y * w + x - w] || !data[y * w + x + w])) {
                result[y * w + x] = 255;
            } else {
                result[y * w + x] = 0;
            }
        }
        result[y * w + w - 1] = data[y * w + w - 1];
    }
    return result;
}

// outline path procedures
static neko_tex_bool neko_tex_find_first_filled_pixel(const u8 *data, s32 w, s32 h, neko_tex_point *first) {
    s32 x, y;
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            if (data[y * w + x]) {
                first->x = (short)x;
                first->y = (short)y;
                return 1;
            }
        }
    }
    return 0;
}

static neko_tex_bool neko_tex_find_next_filled_pixel(const u8 *data, s32 w, s32 h, neko_tex_point current, neko_tex_direction *dir, neko_tex_point *next) {
    // turn around 180°, then make a clockwise scan for a filled pixel
    *dir = __neko_tex_direction_opposite(*dir);
    s32 i;
    for (i = 0; i < 8; i++) {
        __neko_tex_point_add(*next, current, neko_tex_direction_to_pixel_offset[*dir]);

        if (__neko_tex_point_is_inside(*next, w, h) && data[next->y * w + next->x]) return 1;

        // move to next angle (clockwise)
        *dir = *dir - 1;
        if (*dir < 0) *dir = 7;
    }
    return 0;
}

neko_tex_point *neko_tex_extract_outline_path(u8 *data, s32 w, s32 h, s32 *point_count, neko_tex_point *reusable_outline) {
    neko_tex_point *outline = reusable_outline;
    if (!outline) outline = (neko_tex_point *)neko_safe_malloc(w * h * sizeof(neko_tex_point));

    neko_tex_point current, next;

restart:
    if (!neko_tex_find_first_filled_pixel(data, w, h, &current)) {
        *point_count = 0;
        return outline;
    }

    s32 count = 0;
    neko_tex_direction dir = 0;

    while (__neko_tex_point_is_inside(current, w, h)) {
        data[current.y * w + current.x] = 0;  // clear the visited path
        outline[count++] = current;           // add our current point to the outline
        if (!neko_tex_find_next_filled_pixel(data, w, h, current, &dir, &next)) {
            // find loop connection
            neko_tex_bool found = 0;
            s32 i;
            for (i = 0; i < count / 2; i++)  // only allow big loops
            {
                if (__neko_tex_point_is_next_to(current, outline[i])) {
                    found = 1;
                    break;
                }
            }

            if (found) {
                break;
            } else {
                // go backwards until we see outline pixels again
                dir = __neko_tex_direction_opposite(dir);
                count--;  // back up
                s32 prev;
                for (prev = count; prev >= 0; prev--) {
                    current = outline[prev];
                    outline[count++] = current;  // add our current point to the outline again
                    if (neko_tex_find_next_filled_pixel(data, w, h, current, &dir, &next)) break;
                }
            }
        }
        current = next;
    }

    if (count <= 2)  // too small, discard and try again!
        goto restart;
    *point_count = count;
    return outline;
}

void neko_tex_distance_based_path_simplification(neko_tex_point *outline, s32 *outline_length, f32 distance_threshold) {
    s32 length = *outline_length;
    s32 l;
    for (l = length / 2 /*length - 1*/; l > 1; l--) {
        s32 a, b = l;
        for (a = 0; a < length; a++) {
            neko_tex_point ab;
            __neko_tex_point_sub(ab, outline[b], outline[a]);
            f32 lab = sqrtf((f32)(ab.x * ab.x + ab.y * ab.y));
            f32 ilab = 1.0f / lab;
            f32 abnx = ab.x * ilab, abny = ab.y * ilab;

            if (lab != 0.0f) {
                neko_tex_bool found = 1;
                s32 i = (a + 1) % length;
                while (i != b) {
                    neko_tex_point ai;
                    __neko_tex_point_sub(ai, outline[i], outline[a]);
                    f32 t = (abnx * ai.x + abny * ai.y) * ilab;
                    f32 distance = -abny * ai.x + abnx * ai.y;
                    if (t < 0.0f || t > 1.0f || distance > distance_threshold || -distance > distance_threshold) {
                        found = 0;
                        break;
                    }

                    if (++i == length) i = 0;
                }

                if (found) {
                    s32 i;
                    if (a < b) {
                        for (i = 0; i < length - b; i++) outline[a + i + 1] = outline[b + i];
                        length -= b - a - 1;
                    } else {
                        length = a - b + 1;
                        for (i = 0; i < length; i++) outline[i] = outline[b + i];
                    }
                    if (l >= length) l = length - 1;
                }
            }

            if (++b >= length) b = 0;
        }
    }
    *outline_length = length;
}

#if 1

// color brightness as perceived:
f32 brightness(neko_color_t c) { return ((f32)c.r * 0.299f + (f32)c.g * 0.587f + (f32)c.b * 0.114f) / 256.f; }

f32 color_num(neko_color_t c) {
    const f32 bright_factor = 100.0f;
    const f32 sat_factor = 0.1f;
    neko_hsv_t hsv = neko_rgb_to_hsv(c);
    return hsv.s * sat_factor + brightness(c) * bright_factor;
}

// particle_t get_closest_particle_from_color(neko_color_t c) {
//     particle_t p = particle_empty();
//     f32 min_dist = f32_max;
//     neko_vec4 c_vec = neko_vec4{(f32)c.r, (f32)c.g, (f32)c.b, (f32)c.a};
//     u8 id = mat_id_empty;
//
//     __check_dist_euclidean(c, mat_col_sand, particle_sand);
//     __check_dist_euclidean(c, mat_col_water, particle_water);
//     __check_dist_euclidean(c, mat_col_salt, particle_salt);
//     __check_dist_euclidean(c, mat_col_wood, particle_wood);
//     __check_dist_euclidean(c, mat_col_fire, particle_fire);
//     __check_dist_euclidean(c, mat_col_smoke, particle_smoke);
//     __check_dist_euclidean(c, mat_col_steam, particle_steam);
//     __check_dist_euclidean(c, mat_col_gunpowder, particle_gunpowder);
//     __check_dist_euclidean(c, mat_col_oil, particle_oil);
//     __check_dist_euclidean(c, mat_col_lava, particle_lava);
//     __check_dist_euclidean(c, mat_col_stone, particle_stone);
//     __check_dist_euclidean(c, mat_col_acid, particle_acid);
//
//     return p;
// }

#if 0
void drop_file_callback(void* platform_window, s32 count, const char** file_paths) {
    if (count < 1) return;

    // Just do first one for now...
    if (count > 1) count = 1;

    // We'll place at the mouse position as well, for shiggles
    neko_vec2 mp = calculate_mouse_position();

    for (s32 i = 0; i < count; ++i) {
        // Need to verify this IS an image first.
        char temp_file_extension_buffer[16] = {0};
        neko_util_get_file_extension(temp_file_extension_buffer, sizeof(temp_file_extension_buffer), file_paths[0]);
        if (neko_string_compare_equal(temp_file_extension_buffer, "png") || neko_string_compare_equal(temp_file_extension_buffer, "jpg") ||
            neko_string_compare_equal(temp_file_extension_buffer, "jpeg") || neko_string_compare_equal(temp_file_extension_buffer, "bmp")) {
            // Load texture into memory
            s32 _w, _h;
            u32 _n;
            void* texture_data = NULL;

            // Force texture data to 3 components
            neko_util_load_texture_data_from_file(file_paths[i], &_w, &_h, &_n, &texture_data, false);
            _n = 3;

            // Not sure what the format should be, so this is ...blah. Need to find a way to determine this beforehand.
            u8* data = (u8*)texture_data;

            s32 sx = (ch->render_w - _w) / 2;
            s32 sy = (ch->render_h - _h) / 2;

            // Now we need to process the data and place it into our particle/color buffers
            for (u32 h = 0; h < _h; ++h) {
                for (u32 w = 0; w < _w; ++w) {
                    neko_color_t c = {data[(h * _w + w) * _n + 0], data[(h * _w + w) * _n + 1], data[(h * _w + w) * _n + 2], 255};

                    // Get color of this pixel in the image
                    particle_t p = get_closest_particle_from_color(c);

                    // chunk_update_mask_t* chunk = neko_hash_table_getp(ch->chunk_data, 0);

                    // Let's place this thing in the center instead...
                    if (in_bounds(sx + w, sy + h)) {

                        u32 idx = compute_idx(sx + w, sy + h);

                        chunk_write_data(ch,idx, p);
                    }
                }
            }

            // Free texture data
            free(texture_data);
        }
    }
}
#endif

#endif

#if 0

#define STB_HBWANG_RAND() neko_rand_xorshf32()
#define STB_HBWANG_IMPLEMENTATION
#include "deps/stb/stb_herringbone_wang_tile.h"
#include "deps/stb/stb_image.h"
#include "deps/stb/stb_image_write.h"

void genwang(std::string filename, unsigned char* data, s32 xs, s32 ys, s32 w, s32 h) {
    stbhw_tileset ts;
    if (xs < 1 || xs > 1000) {
        fprintf(stderr, "xsize invalid or out of range\n");
        exit(1);
    }
    if (ys < 1 || ys > 1000) {
        fprintf(stderr, "ysize invalid or out of range\n");
        exit(1);
    }

    stbhw_build_tileset_from_image(&ts, data, w * 3, w, h);
    // allocate a buffer to create the final image to
    s32 yimg = ys + 4;
    auto buff = static_cast<unsigned char*>(malloc(3 * xs * yimg));
    stbhw_generate_image(&ts, NULL, buff, xs * 3, xs, yimg);
    stbi_write_png(filename.c_str(), xs, yimg, 3, buff, xs * 3);
    stbhw_free_tileset(&ts);
    free(buff);
}

void test_wang() {

    // mapgen {tile-file} {xsize} {ysize} {seed} [n]

    s32 xs = 128;
    s32 ys = 128;

    s32 n = 1;

    s32 w, h;

    unsigned char* data = stbi_load(neko_file_path("gamedir/assets/textures/wang_test.png"), &w, &h, NULL, 3);

    neko_assert(data);

    printf("Output size: %dx%d\n", xs, ys);

    {
        u32 seed = neko_rand_xorshf32();

        // s32 num = seed + xs + 11 * (xs / -11) - 12 * (seed / 12);
        s32 num = xs % 11 + seed % 12;

        for (s32 i = 0; i < n; ++i) {

            auto filename = std::string("output_wang");
            filename = filename.substr(0, filename.size() - 4);
            filename = filename + std::to_string(seed) + "#" + std::to_string(i) + ".png";

            genwang(filename, data, xs, ys, w, h);
        }
    }

    free(data);
}

#endif
