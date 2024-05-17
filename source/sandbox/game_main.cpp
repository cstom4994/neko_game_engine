

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
#include "engine/neko_engine.h"
#include "engine/neko_filesystem.h"
#include "engine/neko_platform.h"

// binding
#include "neko_api.h"

// game
#include "game_editor.h"
#include "game_main.h"
#include "sandbox/game_physics_math.hpp"
#include "sandbox/mat_base.h"
#include "sandbox/mat_custom.h"
#include "sandbox/neko_api.h"
#include "sandbox/simulation.h"

// opengl
#include <glad/glad.h>

// hpp
#include "sandbox/hpp/neko_static_refl.hpp"
#include "sandbox/hpp/neko_struct.hpp"

#define NEKO_IMGUI_IMPL
#include "game_imgui.h"

NEKO_HIJACK_MAIN();

neko_command_buffer_t g_cb = neko_default_val();
neko_ui_context_t g_ui = neko_default_val();
neko_ui_style_sheet_t style_sheet;
neko_immediate_draw_t g_idraw = neko_default_val();
neko_asset_ascii_font_t g_font;
neko_imgui_context_t g_imgui = neko_default_val();

neko_texture_t g_test_ase = neko_default_val();
neko_asset_manager_t g_am = neko_default_val();
neko_asset_t tex_hndl = neko_default_val();
std::string data_path = neko_default_val();
neko_packreader_t g_pack = neko_default_val();
neko_packreader_t g_lua_pack = neko_default_val();

neko_handle(neko_graphics_renderpass_t) main_rp = {0};
neko_handle(neko_graphics_framebuffer_t) main_fbo = {0};
neko_handle(neko_graphics_texture_t) main_rt = {0};

// user data

neko_client_userdata_t g_client_userdata = neko_default_val();

// decl static reflection
template <>
struct neko::static_refl::neko_type_info<neko_platform_running_desc_t> : neko_type_info_base<neko_platform_running_desc_t> {
    static constexpr AttrList attrs = {};
    static constexpr FieldList fields = {
            rf_field{TSTR("title"), &rf_type::title},                  // 窗口标题
            rf_field{TSTR("width"), &rf_type::width},                  //
            rf_field{TSTR("height"), &rf_type::height},                //
            rf_field{TSTR("flags"), &rf_type::flags},                  //
            rf_field{TSTR("num_samples"), &rf_type::num_samples},      //
            rf_field{TSTR("monitor_index"), &rf_type::monitor_index},  //
            rf_field{TSTR("vsync"), &rf_type::vsync},                  // 启用 vsync
            rf_field{TSTR("frame_rate"), &rf_type::frame_rate},        // 限制帧率
            rf_field{TSTR("engine_args"), &rf_type::engine_args},      // 引擎参数
    };
};

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
} neko_engine_cvar_t;

template <>
struct neko::static_refl::neko_type_info<neko_engine_cvar_t> : neko_type_info_base<neko_engine_cvar_t> {
    static constexpr AttrList attrs = {};
    static constexpr FieldList fields = {
            rf_field{TSTR("show_editor"), &rf_type::show_editor},                    //
            rf_field{TSTR("show_demo_window"), &rf_type::show_demo_window},          //
            rf_field{TSTR("show_pack_editor"), &rf_type::show_pack_editor},          //
            rf_field{TSTR("show_profiler_window"), &rf_type::show_profiler_window},  //
            rf_field{TSTR("show_gui"), &rf_type::show_gui},                          //
            rf_field{TSTR("shader_inspect"), &rf_type::shader_inspect},              //
            rf_field{TSTR("hello_ai_shit"), &rf_type::hello_ai_shit},                //
            rf_field{TSTR("is_hotfix"), &rf_type::is_hotfix},                        //
            rf_field{TSTR("vsync"), &rf_type::vsync},                                //
            rf_field{TSTR("enable_nekolua"), &rf_type::enable_nekolua},              //
    };
};

neko_struct(neko_engine_cvar_t,                         //
            _Fs(show_editor, "Is show editor"),         //
            _Fs(show_demo_window, "Is show nui demo"),  //
            _Fs(show_pack_editor, "pack editor"),       //
            _Fs(show_profiler_window, "profiler"),      //
            _Fs(show_gui, "neko gui"),                  //
            _Fs(shader_inspect, "shaders"),             //
            _Fs(hello_ai_shit, "Test AI"),              //
            _Fs(bg, "bg color")                         //
);

neko_engine_cvar_t g_cvar = neko_default_val();

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
    if (data_path.empty()) {
        neko_assert(!data_path.empty());  // game data path not detected
        return path;
    }
    std::string get_path(data_path);

    if (path.substr(0, 7) == "gamedir") {
        get_path.append(path);
    } else if (path.substr(0, 11) == "lua_scripts") {
        std::string lua_path = path;
        get_path = std::filesystem::path(data_path).parent_path().string().append("/source/lua/game/").append(lua_path.replace(0, 12, ""));
    }

    return get_path;
}

void app_load_style_sheet(bool destroy) {
    if (destroy) {
        neko_ui_style_sheet_destroy(&style_sheet);
    }
    style_sheet = neko_ui_style_sheet_load_from_file(&g_ui, game_assets("gamedir/style_sheets/gui.ss").c_str());
    neko_ui_set_style_sheet(&g_ui, &style_sheet);
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
    neko_asset_texture_t *tp = neko_assets_getp(&g_am, neko_asset_texture_t, tex_hndl);
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

#pragma region console

NEKO_API_DECL void neko_console(neko_console_t *console, neko_ui_context_t *ctx, neko_ui_rect_t screen, const neko_ui_selector_desc_t *desc) {
    if (console->open)
        console->y += (screen.h * console->size - console->y) * console->open_speed;
    else if (!console->open && console->y >= 1.0f)
        console->y += (0 - console->y) * console->close_speed;
    else
        return;

    const f32 sz = neko_min(console->y, 26);
    if (neko_ui_window_begin_ex(ctx, "neko_console_content", neko_ui_rect(screen.x, screen.y, screen.w, console->y - sz), NULL, NULL,
                                NEKO_UI_OPT_FORCESETRECT | NEKO_UI_OPT_NOTITLE | NEKO_UI_OPT_NORESIZE | NEKO_UI_OPT_NODOCK | NEKO_UI_OPT_FORCEFOCUS | NEKO_UI_OPT_HOLDFOCUS)) {
        neko_ui_layout_row(ctx, 1, neko_ui_widths(-1), 0);
        neko_ui_text(ctx, console->tb);
        // neko_imgui_draw_text(console->tb, NEKO_COLOR_WHITE, 10.f, 10.f, true, NEKO_COLOR_BLACK);
        if (console->autoscroll) neko_ui_get_current_container(ctx)->scroll.y = sizeof(console->tb) * 7 + 100;
        neko_ui_container_t *ctn = neko_ui_get_current_container(ctx);
        neko_ui_bring_to_front(ctx, ctn);
        neko_ui_window_end(ctx);
    }

    if (neko_ui_window_begin_ex(ctx, "neko_console_input", neko_ui_rect(screen.x, screen.y + console->y - sz, screen.w, sz), NULL, NULL,
                                NEKO_UI_OPT_FORCESETRECT | NEKO_UI_OPT_NOTITLE | NEKO_UI_OPT_NORESIZE | NEKO_UI_OPT_NODOCK | NEKO_UI_OPT_NOHOVER | NEKO_UI_OPT_NOINTERACT)) {
        int len = strlen(console->cb[0]);
        neko_ui_layout_row(ctx, 3, neko_ui_widths(14, len * 7 + 2, 10), 0);
        neko_ui_text(ctx, "$>");
        neko_ui_text(ctx, console->cb[0]);

        if (!console->open || !console->last_open_state) {
            goto console_input_handling_done;
        }

        // 处理文本输入
        int32_t n = neko_min(sizeof(*console->cb) - len - 1, (int32_t)strlen(ctx->input_text));

        if (neko_platform_key_pressed(NEKO_KEYCODE_UP)) {
            console->current_cb_idx++;
            if (console->current_cb_idx >= neko_arr_size(console->cb)) {
                console->current_cb_idx = neko_arr_size(console->cb) - 1;
            } else {
                memcpy(&console->cb[0], &console->cb[console->current_cb_idx], sizeof(*console->cb));
            }
        } else if (neko_platform_key_pressed(NEKO_KEYCODE_DOWN)) {
            console->current_cb_idx--;
            if (console->current_cb_idx <= 0) {
                console->current_cb_idx = 0;
                memset(&console->cb[0], 0, sizeof(*console->cb));
            } else {
                memcpy(&console->cb[0], &console->cb[console->current_cb_idx], sizeof(*console->cb));
            }
        } else if (neko_platform_key_pressed(NEKO_KEYCODE_ENTER)) {
            console->current_cb_idx = 0;
            neko_console_printf(console, "$ %s\n", console->cb[0]);

            memmove((uint8_t *)console->cb + sizeof(*console->cb), (uint8_t *)console->cb, sizeof(console->cb) - sizeof(*console->cb));

            if (console->cb[0][0] && console->commands) {
                char *tmp = console->cb[0];
                int argc = 1;
                while ((tmp = strchr(tmp, ' '))) {
                    argc++;
                    tmp++;
                }

                tmp = console->cb[0];
                char *last_pos = console->cb[0];
                char **argv = (char **)neko_safe_malloc(argc * sizeof(char *));
                int i = 0;
                while ((tmp = strchr(tmp, ' '))) {
                    *tmp = 0;
                    argv[i++] = last_pos;
                    last_pos = ++tmp;
                }
                argv[argc - 1] = last_pos;

                for (int i = 0; i < console->commands_len; i++) {
                    if (console->commands[i].name && console->commands[i].func && strcmp(argv[0], console->commands[i].name) == 0) {
                        console->commands[i].func(argc, argv);
                        goto console_command_found;
                    }
                }
                neko_console_printf(console, "[neko_console]: unrecognized command '%s'\n", argv[0]);
            console_command_found:
                console->cb[0][0] = '\0';
                neko_safe_free(argv);
            }
        } else if (neko_platform_key_pressed(NEKO_KEYCODE_BACKSPACE)) {
            console->current_cb_idx = 0;
            // 跳过 utf-8 连续字节
            while ((console->cb[0][--len] & 0xc0) == 0x80 && len > 0)
                ;
            console->cb[0][len] = '\0';
        } else if (n > 0 && !neko_platform_key_pressed(NEKO_KEYCODE_GRAVE_ACCENT)) {
            console->current_cb_idx = 0;
            if (len + n + 1 < sizeof(*console->cb)) {
                memcpy(console->cb[0] + len, ctx->input_text, n);
                len += n;
                console->cb[0][len] = '\0';
            }
        }

    console_input_handling_done:

        // 闪烁光标
        neko_ui_get_layout(ctx)->body.x += len * 7 - 5;
        if ((int)(neko_platform_elapsed_time() / 666.0f) & 1) neko_ui_text(ctx, "|");

        neko_ui_container_t *ctn = neko_ui_get_current_container(ctx);
        neko_ui_bring_to_front(ctx, ctn);

        neko_ui_window_end(ctx);
    }

    console->last_open_state = console->open;
}

static bool window = 1, embeded;
static int summons;

static void toggle_window(int argc, char **argv);
static void toggle_embedded(int argc, char **argv);
static void help(int argc, char **argv);
static void echo(int argc, char **argv);
static void spam(int argc, char **argv);
static void crash(int argc, char **argv);
void summon(int argc, char **argv);
void exec(int argc, char **argv);
void sz(int argc, char **argv);

neko_console_command_t commands[] = {{
                                             .func = echo,
                                             .name = "echo",
                                             .desc = "repeat what was entered",
                                     },
                                     {
                                             .func = spam,
                                             .name = "spam",
                                             .desc = "send the word arg1, arg2 amount of times",
                                     },
                                     {
                                             .func = help,
                                             .name = "help",
                                             .desc = "sends a list of commands",
                                     },
                                     {
                                             .func = toggle_window,
                                             .name = "window",
                                             .desc = "toggles gui window",
                                     },
                                     {
                                             .func = toggle_embedded,
                                             .name = "embed",
                                             .desc = "places the console inside the window",
                                     },
                                     {
                                             .func = summon,
                                             .name = "summon",
                                             .desc = "summons a gui window",
                                     },
                                     {
                                             .func = sz,
                                             .name = "sz",
                                             .desc = "change console size",
                                     },
                                     {
                                             .func = crash,
                                             .name = "crash",
                                             .desc = "test crashhhhhhhhh.....",
                                     },
                                     {
                                             .func = exec,
                                             .name = "exec",
                                             .desc = "run nekoscript",
                                     }};

neko_console_t console = {
        .tb = "",
        .cb = "",
        .size = 0.4,
        .open_speed = 0.2,
        .close_speed = 0.3,
        .autoscroll = true,
        .commands = commands,
        .commands_len = neko_arr_size(commands),
};

void sz(int argc, char **argv) {
    if (argc != 2) {
        neko_console_printf(&console, "[sz]: needs 1 argument!\n");
        return;
    }
    f32 sz = atof(argv[1]);
    if (sz > 1 || sz < 0) {
        neko_console_printf(&console, "[sz]: number needs to be between (0, 1)");
        return;
    }
    console.size = sz;

    neko_console_printf(&console, "console size is now %f\n", sz);
}

void toggle_window(int argc, char **argv) {
    if (window && embeded)
        neko_console_printf(&console, "Unable to turn off window, console is embeded!\n");
    else
        neko_console_printf(&console, "GUI Window turned %s\n", (window = !window) ? "on" : "off");
}

void toggle_embedded(int argc, char **argv) {
    if (!window && !embeded)
        neko_console_printf(&console, "Unable to embed into window, open window first!\n");
    else
        neko_console_printf(&console, "console embedded turned %s\n", (embeded = !embeded) ? "on" : "off");
}

void summon(int argc, char **argv) {
    neko_console_printf(&console, "A summoner has cast his spell! A window has appeared!!!!\n");
    summons++;
}

void crash(int argc, char **argv) {

    const_str trace_info = __neko_inter_stacktrace();

    neko_platform_msgbox(std::format("Crash...\n{0}", trace_info).c_str());
}

void spam(int argc, char **argv) {
    int count;
    if (argc != 3) goto spam_invalid_command;
    count = atoi(argv[2]);
    if (!count) goto spam_invalid_command;
    while (count--) neko_console_printf(&console, "%s\n", argv[1]);
    return;
spam_invalid_command:
    neko_console_printf(&console, "[spam]: invalid usage. It should be 'spam word [int count]''\n");
}

void echo(int argc, char **argv) {
    for (int i = 1; i < argc; i++) neko_console_printf(&console, "%s ", argv[i]);
    neko_console_printf(&console, "\n");
}

void exec(int argc, char **argv) {
    if (argc != 2) return;
    // neko_vm_interpreter(argv[1]);
}

void help(int argc, char **argv) {
    for (int i = 0; i < neko_arr_size(commands); i++) {
        if (commands[i].name) neko_console_printf(&console, "* Command: %s\n", commands[i].name);
        if (commands[i].desc) neko_console_printf(&console, "- desc: %s\n", commands[i].desc);
    }
}

#pragma endregion

template <typename T, typename Fields = std::tuple<>>
void __neko_cvar_gui_internal(T &&obj, int depth = 0, const char *fieldName = "", Fields &&fields = std::make_tuple()) {
    if constexpr (std::is_class_v<std::decay_t<T>>) {
        neko_struct_foreach(obj, [depth](auto &&fieldName, auto &&value, auto &&info) { __neko_cvar_gui_internal(value, depth + 1, fieldName, info); });
    } else {

        auto ff = [&]<typename S>(const char *name, auto &var, S &t) {
            if constexpr (std::is_same_v<std::decay_t<decltype(var)>, S>) {
                neko_imgui::Auto(var, name);
                ImGui::Text("    [%s]", std::get<0>(fields));
            }
        };

        neko::invoke::apply([&](auto &&...args) { (ff(fieldName, obj, args), ...); }, std::tuple<CVAR_TYPES()>());
    }
}

void neko_cvar_gui() {
    __neko_cvar_gui_internal(g_cvar);

    for (size_t i = 0; i < neko_dyn_array_size(neko_cv()->cvars); i++) {
        {
            switch ((&neko_cv()->cvars[i])->type) {
                default:
                case __NEKO_CONFIG_TYPE_STRING:
                    neko_imgui::Auto((&neko_cv()->cvars[i])->value.s, (&neko_cv()->cvars[i])->name);
                    break;
                case __NEKO_CONFIG_TYPE_FLOAT:
                    neko_imgui::Auto((&neko_cv()->cvars[i])->value.f, (&neko_cv()->cvars[i])->name);
                    break;
                case __NEKO_CONFIG_TYPE_INT:
                    neko_imgui::Auto((&neko_cv()->cvars[i])->value.i, (&neko_cv()->cvars[i])->name);
                    break;
            };
        };
    }
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

neko_assetsys_t *g_assetsys;

neko_vec2_t fbs = {640 * 1.5, 360 * 1.5};
neko_vec2_t cam = {512, 512};

Game *game = nullptr;

void draw_text(neko_font_t *font, const char *text, float x, float y, float line_height, float clip_region, float wrap_x, f32 scale) {
    f32 text_w = (f32)neko_font_text_width(font, text);
    f32 text_h = (f32)neko_font_text_height(font, text);

    //    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());

    if (scale == 0.f) scale = font_scale;

    neko_font_rect_t clip_rect;
    clip_rect.left = -fbs.x / scale * clip_region;
    clip_rect.right = fbs.x / scale * clip_region + 0.5f;
    clip_rect.top = fbs.y / scale * clip_region + 0.5f;
    clip_rect.bottom = -fbs.y / scale * clip_region;

    f32 x0 = (x - fbs.x / 2.f) / scale /*+ -text_w / 2.f*/;
    f32 y0 = (fbs.y / 2.f - y) / scale + text_h / 2.f;
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

sandbox_object::sandbox_object(neko_vec4_t rect) {

    this->rect = rect;

    this->draw_buffer = new neko_color_t[kScreenWidth * kScreenHeight];

    this->data = (unsigned char *)neko_safe_malloc(sizeof(unsigned char) * kScreenWidth * kScreenHeight);
    this->edgeSeen = (bool *)neko_safe_malloc(sizeof(bool) * kScreenWidth * kScreenHeight);

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

        //        neko_idraw_rect_textured_ext(Game::idraw, this->rect.x, this->rect.y, this->rect.x + this->rect.w * this->scale,
        //                                     this->rect.y + this->rect.z * this->scale, 0, 1, 1, 0, this->object_texture.id, NEKO_COLOR_WHITE);
        //
        //        neko_idraw_rectv(Game::idraw, neko_v2(this->rect.x,this->rect.y), neko_v2(this->rect.x + this->rect.w * this->scale, this->rect.y + this->rect.z * this->scale), NEKO_COLOR_WHITE,
        //                         NEKO_GRAPHICS_PRIMITIVE_LINES);

        neko_idraw_rect_textured_ext(Game::idraw, 0, 0, 512 * scale, 512 * scale, 0, 1, 1, 0, this->object_texture.id, NEKO_COLOR_WHITE);
    }

    this->chunk_mask_update_mesh();
}
// GameObject::~GameObject() {}

void sandbox_object::chunk_mask_update_mesh() {

    neko_immediate_draw_t *idraw = g_client_userdata.idraw;

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

    auto sim = game->get_sim();

    // 优化掉无固态碰撞的区块
    bool foundAnything = false;
    for (int x = 0; x < kScreenWidth; x++) {
        for (int y = 0; y < kScreenHeight; y++) {

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

    for (int y = 0; y < kScreenHeight; y++) {
        for (int x = 0; x < kScreenWidth; x++) {
            // data[x + y * kScreenWidth] = real_tiles[(x + chTx) + (y + chTy) * width].mat->physicsType == PhysicsType::SOLID;
            //            u8 mat_id = get_particle_at(ch, x, y).mat_id;
            auto type = sim->buffer_ptr_->GetMaterial(x, y);
            this->data[x + y * kScreenWidth] = (type == material_type::ROCK);
            this->edgeSeen[x + y * kScreenWidth] = false;
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
        int lookX = lookIndex % kScreenWidth;
        int lookY = lookIndex / kScreenWidth;
        /*if (inn == 1) {
            lookX = kScreenWidth / 2;
            lookY = kScreenHeight / 2;
        }*/

        int edgeX = -1;
        int edgeY = -1;
        int size = kScreenWidth * kScreenHeight;

        for (int i = lookIndex; i < size; i++) {
            if (this->data[i] != 0) {

                int numBorders = 0;
                // if (i % kScreenWidth - 1 >= 0) numBorders += data[(i % kScreenWidth - 1) + i / kScreenWidth * kScreenWidth];
                // if (i / kScreenWidth - 1 >= 0) numBorders += data[(i % kScreenWidth)+(i / kScreenWidth - 1) * kScreenWidth];
                if (i % kScreenWidth + 1 < kScreenWidth) numBorders += this->data[(i % kScreenWidth + 1) + i / kScreenWidth * kScreenWidth];
                if (i / kScreenWidth + 1 < kScreenHeight) numBorders += this->data[(i % kScreenWidth) + (i / kScreenWidth + 1) * kScreenWidth];
                if (i / kScreenWidth + 1 < kScreenHeight && i % kScreenWidth + 1 < kScreenWidth) numBorders += this->data[(i % kScreenWidth + 1) + (i / kScreenWidth + 1) * kScreenWidth];

                // int val = value(i % ch->render_w, i / ch->render_w, ch->render_w, height, data);
                if (numBorders != 3) {
                    edgeX = i % kScreenWidth;
                    edgeY = i / kScreenWidth;
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

        lookIndex = lookX + lookY * kScreenWidth + 1;

        if (this->edgeSeen[lookX + lookY * kScreenWidth]) {
            inn--;
            continue;
        }

        int val = neko::marching_squares::ms_value(lookX, lookY, kScreenWidth, kScreenHeight, this->data);

        if (val == 0 || val == 15) {
            inn--;
            continue;
        }

        neko::marching_squares::ms_result r = neko::marching_squares::find_perimeter(lookX, lookY, kScreenWidth, kScreenHeight, this->data);

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
                    if (ilx >= kScreenWidth) ilx = kScreenWidth - 1;

                    if (ily < 0) ily = 0;
                    if (ily >= kScreenHeight) ily = kScreenHeight - 1;

                    int ind = ilx + ily * kScreenWidth;
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
            v.x *= game->draw_scale;
            v.y *= game->draw_scale;
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

neko_immediate_draw_t *Game::idraw = nullptr;

Game::Game(neko_immediate_draw_t *idraw) {
    Init(idraw);
    CreateSimulation();
    CreateViewPort();
    CreateUI();
    ResetVariables();
}

Game::~Game() {}

void Game::Init(neko_immediate_draw_t *idraw) {
    this->idraw = idraw;
    count = 0;
}

void Game::CreateSimulation() { simulation = new Simulation(); }

void Game::CreateViewPort() {
    neko_vec2 win_size = neko_platform_window_sizev(neko_platform_main_window());
    this->fbs_scale = (float)win_size.x / fbs.x;
    viewport = new sandbox_object(kViewportRect);
}

void Game::CreateUI() {
    auto set_material = std::bind(&Game::SetMaterial, this, std::placeholders::_1);
    auto pause = std::bind(&Game::Pause, this, std::placeholders::_1);
    auto reset = std::bind(&Game::ResetSimulation, this, std::placeholders::_1);
    //    ui_ = new UI(renderer_, {Helper::ScreenWidthPoint(11), 0, (int)kScreenWidth - Helper::ScreenWidthPoint(11), (int)kScreenHeight}, neko_color(128, 128, 128, 255));
    //    ui_->AddButtonGroup({0, 0, Helper::ScreenWidthPoint(4), Helper::ScreenWidthPoint(1)}, {0, 0, 120, 64}, "Pause", pause);
    //    ui_->AddButtonGroup({0, Helper::ScreenHeightPoint(1), Helper::ScreenWidthPoint(4), Helper::ScreenWidthPoint(2)}, {0, 0, 120, 64}, "Reset", reset);
    //    ui_->AddButtonGroup({0, Helper::ScreenWidthPoint(2), Helper::ScreenWidthPoint(4), Helper::ScreenWidthPoint(5)}, {0, 0, 120, 64}, "Empty Rock Sand Water Steam Wood Fire", set_material);

    pixelui_init(&this->pixelui);

    this->pixelui.show_material_selection_panel = true;
    this->pixelui.show_frame_count = true;
    this->pixelui.brush_size = 0.5f;
}

void Game::SetMaterial(int m) { material = static_cast<material_type>(m); }

void Game::Pause(int x) { paused = !paused; }

void Game::ResetSimulation(int x) { simulation->Reset(); }

void Game::ResetVariables() {
    tick_count = 0;
    draw_radius = 15;
    material = material_type::ROCK;
    draw_scale = 3.f;
}

void Game::PreUpdate() {
    current_tick = neko_platform_elapsed_time();
    frame_count++;
    //    delta_time = (current_tick - last_tick) / 1000.0f;

    draw_radius = std::clamp<s16>(draw_radius, kMinDrawRadius, kMaxDrawRadius);
    neko_vec2 foo = neko_platform_mouse_positionv();
    neko_vec2 win_size = neko_platform_window_sizev(neko_platform_main_window());

    this->fbs_scale = (float)win_size.x / fbs.x;

    foo.x = foo.x / this->fbs_scale;
    foo.y = foo.y / this->fbs_scale;

    foo.x += cam.x;
    foo.y += cam.y;

    cursor = foo;

    this->material = this->pixelui.material_selection;
}

void Game::Update() {
    PreUpdate();

    auto draw_pos = cursor;
    draw_pos.x = draw_pos.x / this->draw_scale;
    draw_pos.y = draw_pos.y / this->draw_scale;

    if (neko_platform_mouse_down(NEKO_MOUSE_LBUTTON)) {
        simulation->SetCellInsideCircle(draw_pos, draw_radius, material);
    } else if (neko_platform_mouse_down(NEKO_MOUSE_RBUTTON)) {
        simulation->SetCellInsideCircle(draw_pos, draw_radius, material_type::EMPTY);
    }

    neko_vec2 mw = neko_platform_mouse_wheelv();
    if (mw.y > 0)
        this->draw_radius++;
    else if (mw.y < 0)
        this->draw_radius--;

    static int k = 0;
    if (neko_platform_key_released(NEKO_KEYCODE_1)) {
        SetMaterial(k++);
    }
    if (neko_platform_key_released(NEKO_KEYCODE_2)) {
        SetMaterial(k--);
    }

    //    if (InputManager::Instance()->key_states_.contains(SDLK_TAB) && InputManager::Instance()->key_states_[SDLK_TAB] == InputManager::KeyState::JUSTDOWN) {
    //        ui_mode_ = !ui_mode_;
    //    }
    //    if (InputManager::Instance()->key_states_.contains(SDLK_r) && InputManager::Instance()->key_states_[SDLK_r] == InputManager::KeyState::JUSTDOWN) {
    //        debug_mode_ = !debug_mode_;
    //    }
    //    if(InputManager::Instance()->key_states_.find(SDLK_s) != InputManager::Instance()->key_states_.end() && (InputManager::Instance()->key_states_[SDLK_s] == InputManager::KeyState::DOWN ||
    //    InputManager::Instance()->key_states_[SDLK_s] == InputManager::KeyState::JUSTDOWN)){
    //        viewport_->object_texture_ptr_->rect_.y = std::clamp(viewport_->object_texture_ptr_->rect_.y+1,0,(int)kScreenHeight-(int)kViewportHeight);
    //    }
    if (!paused) {
        int pitch = (kScreenWidth) * 8;
        //        SDL_LockTexture(viewport_->object_texture_ptr_->texture_, nullptr, (void **)&viewport_->draw_buffer_, &pitch);
        simulation->Update();
        for (int i = 0; i < kScreenWidth; i++) {
            for (int j = 0; j < kScreenHeight; j++) {
                s64 foo = i + j * kScreenWidth;
                viewport->draw_buffer[foo] = simulation->buffer_ptr_->GetCellColor(i, j);
            }
        }
        //        SDL_UnlockTexture(viewport_->object_texture_ptr_->texture_);
    }
    LateUpdate();
}

void Game::LateUpdate() {
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

void Game::Render() {

    neko_idraw_defaults(&g_idraw);
    neko_idraw_camera2d_ex(&g_idraw, cam.x, cam.x + fbs.x, cam.y, cam.y + fbs.y);

    viewport->render(draw_scale);

    if (debug_mode) {
        for (int i = 0; i < 64; i++) {
            //            (*simulation_->chunks_ptr_)[i].Debug(renderer_, viewport_->object_texture_ptr_->GetScale());
        }
    }
    neko_idraw_circle(Game::idraw, cursor.x, cursor.y, draw_radius * this->draw_scale, 20, 100, 150, 220, 255, NEKO_GRAPHICS_PRIMITIVE_LINES);

    //    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());
    neko_idraw_defaults(Game::idraw);
    neko_idraw_camera2d(Game::idraw, (u32)fbs.x, (u32)fbs.y);

    update_ui(&this->pixelui);
    neko_idraw_rect_textured_ext(Game::idraw, 0, 0, fbs.x, fbs.y, 0, 1, 1, 0, this->pixelui.tex_ui.id, NEKO_COLOR_WHITE);
}

void Game::Clean() {

    pixelui_destroy(&this->pixelui);

    delete simulation;
    //    delete[] viewport->draw_buffer;

    this->viewport->clean();

    delete viewport;
}

// lua
#include "engine/neko_lua.h"

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

lua_State *g_L = nullptr;

neko_thread_atomic_int_t init_thread_flag;
neko_thread_ptr_t init_work_thread;

// sandbox_t *sb = nullptr;

int init_work(void *user_data) {

    neko_thread_atomic_int_t *this_init_thread_flag = (neko_thread_atomic_int_t *)user_data;

    if (thread_atomic_int_load(this_init_thread_flag) == 0) {

        neko_thread_timer_t thread_timer;
        thread_timer_init(&thread_timer);

        neko::timer timer;
        timer.start();

        try {
            neko_lua_wrap_call(g_L, "game_init_thread");
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
    g_cb = neko_command_buffer_new();
    g_idraw = neko_immediate_draw_new();

    neko_graphics_info_t *info = neko_graphics_info();
    if (!info->compute.available) {
        neko_log_error("%s", "Compute shaders not available.");
        neko_quit();
        return;
    }

    g_am = neko_asset_manager_new();

    g_assetsys = neko_assetsys_create(0);

    neko_assetsys_mount(g_assetsys, "./gamedir", "/gamedir");

    g_client_userdata.cb = &g_cb;
    g_client_userdata.idraw = &g_idraw;
    g_client_userdata.core_ui = &g_ui;
    g_client_userdata.idraw_sd = neko_immediate_draw_static_data_get();

    g_client_userdata.pack = &g_lua_pack;

    neko_pack_result result = neko_pack_read(game_assets("gamedir/sc.pack").c_str(), 0, false, &g_lua_pack);
    neko_pack_check(result);

    g_L = neko_scripting_init();

    {
        neko_lua_wrap_register_t<>(g_L).def(+[](const_str path) -> std::string { return game_assets(path); }, "__neko_file_path");

        lua_newtable(g_L);
        for (int n = 0; n < neko_instance()->ctx.game.argc; ++n) {
            lua_pushstring(g_L, neko_instance()->ctx.game.argv[n]);
            lua_rawseti(g_L, -2, n);
        }
        lua_setglobal(g_L, "arg");

        neko_lua_wrap_safe_dofile(g_L, "main");

        // 获取 neko_game.table
        lua_getglobal(g_L, "neko_game");
        if (!lua_istable(g_L, -1)) {
            neko_log_error("%s", "neko_game is not a table");
        }

        neko_platform_running_desc_t t = {.title = "Neko Engine", .engine_args = ""};
        if (lua_getfield(g_L, -1, "app") == LUA_TNIL) throw std::exception("no app");
        if (lua_istable(g_L, -1)) {
            neko::static_refl::neko_type_info<neko_platform_running_desc_t>::ForEachVarOf(t, [](auto field, auto &&value) {
                static_assert(std::is_lvalue_reference_v<decltype(value)>);
                if (lua_getfield(g_L, -1, std::string(field.name).c_str()) != LUA_TNIL) value = neko_lua_to<std::remove_reference_t<decltype(value)>>(g_L, -1);
                lua_pop(g_L, 1);
            });
        } else {
            throw std::exception("no app table");
        }
        lua_pop(g_L, 1);

        neko_engine_cvar_t cvar;
        if (lua_getfield(g_L, -1, "cvar") == LUA_TNIL) throw std::exception("no cvar");
        if (lua_istable(g_L, -1)) {
            neko::static_refl::neko_type_info<neko_engine_cvar_t>::ForEachVarOf(cvar, [](auto field, auto &&value) {
                static_assert(std::is_lvalue_reference_v<decltype(value)>);
                if (lua_getfield(g_L, -1, std::string(field.name).c_str()) != LUA_TNIL) value = neko_lua_to<std::remove_reference_t<decltype(value)>>(g_L, -1);
                lua_pop(g_L, 1);
            });
            g_cvar = cvar;  // 复制到 g_cvar
        } else {
            throw std::exception("no cvar table");
        }
        lua_pop(g_L, 1);

        lua_pop(g_L, 1);  // 弹出 neko_game.table

        neko_log_info("load game: %s %d %d", t.title, t.width, t.height);

        neko_platform_set_window_title(neko_platform_main_window(), t.title);
    }

    try {
        neko_lua_wrap_call(g_L, "game_init");
    } catch (std::exception &ex) {
        neko_log_error("lua exception %s", ex.what());
    }

    result = neko_pack_read(game_assets("gamedir/res.pack").c_str(), 0, false, &g_pack);
    neko_pack_check(result);

    u8 *font_data, *cat_data;
    u32 font_data_size, cat_data_size;

    result = neko_pack_item_data(&g_pack, ".\\data\\assets\\fonts\\fusion-pixel-12px-monospaced.ttf", (const u8 **)&font_data, &font_data_size);
    neko_pack_check(result);

    result = neko_pack_item_data(&g_pack, ".\\data\\assets\\textures\\cat.aseprite", (const u8 **)&cat_data, &cat_data_size);
    neko_pack_check(result);

    g_test_ase = load_ase_texture_simple(cat_data, cat_data_size);

    neko_asset_texture_t tex0 = {0};
    neko_asset_texture_load_from_file(game_assets("gamedir/assets/textures/yzh.png").c_str(), &tex0, NULL, false, false);
    tex_hndl = neko_assets_create_asset(&g_am, neko_asset_texture_t, &tex0);

    neko_ui_init(&g_ui, neko_platform_main_window());

    // 加载自定义字体文件 初始化 gui font stash
    // neko_asset_ascii_font_load_from_memory(font_data, font_data_size, &g_font, 24);
    neko_asset_ascii_font_load_from_file(game_assets("gamedir/assets/fonts/monocraft.ttf").c_str(), &g_font, 24);

    neko_pack_item_free(&g_pack, font_data);
    neko_pack_item_free(&g_pack, cat_data);

    auto GUI_FONT_STASH = []() -> neko_ui_font_stash_desc_t * {
        static neko_ui_font_desc_t font_decl[] = {{.key = "mc_regular", .font = &g_font}};
        static neko_ui_font_stash_desc_t font_stash = {.fonts = font_decl, .size = 1 * sizeof(neko_ui_font_desc_t)};
        return &font_stash;
    }();

    // neko_ui_font_stash_desc_t font_stash = {.fonts = (neko_ui_font_desc_t[]){{.key = "mc_regular", .font = &font}, .size = 1 * sizeof(neko_ui_font_desc_t)};
    neko_ui_init_font_stash(&g_ui, GUI_FONT_STASH);

    // Load style sheet from file now
    app_load_style_sheet(false);

    // Dock windows before hand
    neko_ui_dock_ex(&g_ui, "Style_Editor", "Demo_Window", NEKO_UI_SPLIT_TAB, 0.5f);

    g_imgui = neko_imgui_new(neko_platform_main_window(), false);

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

    neko_graphics_batch_ortho_2d(fbs.x / font_scale, fbs.y / font_scale, 0, 0, font_projection);

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
    neko_assetsys_file(g_assetsys, "/gamedir/style_sheets/temp.ss", &file);
    int size = neko_assetsys_file_size(g_assetsys, file);
    // char *content = (char *)malloc(size + 1);  // extra space for '\0'
    // int loaded_size = 0;
    // neko_assetsys_file_load(g_assetsys, file, &loaded_size, content, size);
    // content[size] = '\0';  // zero terminate the text file
    // printf("%s\n", content);
    // free(content);

    neko_assetsys_destroy(g_assetsys);

    // Construct frame buffer
    main_fbo = neko_graphics_framebuffer_create(NULL);

    neko_graphics_texture_desc_t main_rt_desc = {.width = (u32)fbs.x,
                                                 .height = (u32)fbs.y,
                                                 .format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8,
                                                 .wrap_s = NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT,
                                                 .wrap_t = NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT,
                                                 .min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST,
                                                 .mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST};
    main_rt = neko_graphics_texture_create(&main_rt_desc);

    neko_graphics_renderpass_desc_t main_rp_desc = {.fbo = main_fbo, .color = &main_rt, .color_size = sizeof(main_rt)};
    main_rp = neko_graphics_renderpass_create(&main_rp_desc);

    //    sb = new sandbox_t;
    //    sandbox_init(sb);

    game = new Game(&g_idraw);

    // 初始化工作

    thread_atomic_int_store(&init_thread_flag, 0);

    init_work_thread = thread_init(init_work, &init_thread_flag, "init_work_thread", THREAD_STACK_SIZE_DEFAULT);
}

void game_loop() {

    int this_init_thread_flag = thread_atomic_int_load(&init_thread_flag);

    if (this_init_thread_flag == 0) {

        //        neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());
        const f32 t = neko_platform_elapsed_time();

        u8 tranp = 255;

        // tranp -= ((s32)t % 255);

        neko_graphics_clear_action_t clear = {.color = {g_cvar.bg[0] / 255, g_cvar.bg[1] / 255, g_cvar.bg[2] / 255, 1.f}};
        neko_graphics_renderpass_begin(&g_cb, neko_renderpass_t{0});
        { neko_graphics_clear(&g_cb, clear); }
        neko_graphics_renderpass_end(&g_cb);

        // Set up 2D camera for projection matrix
        neko_idraw_defaults(&g_idraw);
        neko_idraw_camera2d(&g_idraw, (u32)fbs.x, (u32)fbs.y);

        // 底层图片
        char background_text[64] = "Project: unknown";

        neko_vec2 td = neko_asset_ascii_font_text_dimensions(neko_idraw_default_font(), background_text, -1);
        neko_vec2 ts = neko_v2(512 + 128, 512 + 128);

        neko_idraw_text(&g_idraw, (fbs.x - td.x) * 0.5f, (fbs.y - td.y) * 0.5f + ts.y / 2.f - 100.f, background_text, NULL, false, neko_color(255, 255, 255, 255));
        neko_idraw_texture(&g_idraw, g_test_ase);
        neko_idraw_rectvd(&g_idraw, neko_v2((fbs.x - ts.x) * 0.5f, (fbs.y - ts.y) * 0.5f - td.y - 50.f), ts, neko_v2(0.f, 1.f), neko_v2(1.f, 0.f), neko_color(255, 255, 255, tranp),
                          NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);

        neko_graphics_renderpass_begin(&g_cb, NEKO_GRAPHICS_RENDER_PASS_DEFAULT);
        {
            neko_graphics_set_viewport(&g_cb, 0, 0, (u32)fbs.x, (u32)fbs.y);
            neko_idraw_draw(&g_idraw, &g_cb);  // 立即模式绘制 idraw
        }
        neko_graphics_renderpass_end(&g_cb);

        // Submits to cb
        neko_graphics_command_buffer_submit(&g_cb);

    } else {

        static int init_retval = 1;
        if (init_retval) {
            init_retval = thread_join(init_work_thread);
            thread_term(init_work_thread);
            neko_log_trace("init_work_thread done");
        }

        {
            neko_profiler_scope_begin(lua_pre_update);
            try {
                neko_lua_wrap_call(g_L, "game_pre_update");
            } catch (std::exception &ex) {
                neko_log_error("lua exception %s", ex.what());
            }
            neko_profiler_scope_end(lua_pre_update);
        }

        //        neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());
        neko_vec2 win_size = neko_platform_window_sizev(neko_platform_main_window());
        const f32 t = neko_platform_elapsed_time();
        neko_vec2 mp = neko_platform_mouse_positionv();
        neko_vec2 mw = neko_platform_mouse_wheelv();
        neko_vec2 md = neko_platform_mouse_deltav();
        bool lock = neko_platform_mouse_locked();
        bool moved = neko_platform_mouse_moved();
        f32 dt = neko_platform_delta_time();

        if (neko_platform_key_pressed(NEKO_KEYCODE_ESC)) g_cvar.show_editor ^= true;

        {
            neko_profiler_scope_auto("lua_update");
            try {
                neko_lua_wrap_call<void, f32>(g_L, "game_loop", dt);
            } catch (std::exception &ex) {
                neko_log_error("lua exception %s", ex.what());
            }
        }

        if (neko_platform_was_key_down(NEKO_KEYCODE_W)) cam.y = cam.y - 50 * game->draw_scale * dt;
        if (neko_platform_was_key_down(NEKO_KEYCODE_S)) cam.y = cam.y + 50 * game->draw_scale * dt;
        if (neko_platform_was_key_down(NEKO_KEYCODE_A)) cam.x = cam.x - 50 * game->draw_scale * dt;
        if (neko_platform_was_key_down(NEKO_KEYCODE_D)) cam.x = cam.x + 50 * game->draw_scale * dt;

        // Do rendering
        neko_graphics_clear_action_t clear = {.color = {g_cvar.bg[0] / 255, g_cvar.bg[1] / 255, g_cvar.bg[2] / 255, 1.f}};
        neko_graphics_clear_action_t b_clear = {.color = {0.0f, 0.0f, 0.0f, 1.f}};

        neko_graphics_renderpass_begin(&g_cb, NEKO_GRAPHICS_RENDER_PASS_DEFAULT);
        { neko_graphics_clear(&g_cb, clear); }
        neko_graphics_renderpass_end(&g_cb);

        // // 设置投影矩阵的 2D 相机
        // neko_idraw_defaults(&g_idraw);
        // neko_idraw_camera2d(&g_idraw, (u32)fbs.x, (u32)fbs.y);

        // // 底层图片
        // char background_text[64] = "Project: unknown";

        // neko_vec2 td = neko_asset_ascii_font_text_dimensions(neko_idraw_default_font(), background_text, -1);
        // neko_vec2 ts = neko_v2(512 + 128, 512 + 128);

        // neko_idraw_text(&g_idraw, (fbs.x - td.x) * 0.5f, (fbs.y - td.y) * 0.5f + ts.y / 2.f - 100.f, background_text, NULL, false, 255, 255, 255, 255);
        // neko_idraw_texture(&g_idraw, g_test_ase);
        // neko_idraw_rectvd(&g_idraw, neko_v2((fbs.x - ts.x) * 0.5f, (fbs.y - ts.y) * 0.5f - td.y - 50.f), ts, neko_v2(0.f, 1.f), neko_v2(1.f, 0.f), NEKO_COLOR_WHITE,
        // NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);

#pragma region AI

        if (g_cvar.hello_ai_shit) {

            // neko_idraw_defaults(&g_idraw);
            // neko_idraw_camera3d(&g_idraw, (u32)fbs.x, (u32)fbs.y);

            // // AI
            // ai_behavior_tree_frame(&bt);

            // // Update/render scene
            // neko_idraw_camera(&g_idraw, &camera, (u32)fbs.x, (u32)fbs.y);
            // neko_idraw_depth_enabled(&g_idraw, true);

            // // Render ground
            // neko_idraw_rect3Dv(&g_idraw, neko_v3(-15.f, -0.5f, -15.f), neko_v3(15.f, -0.5f, 15.f), neko_v2s(0.f), neko_v2s(1.f), neko_color(50, 50, 50, 255), NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);

            // // Render ai
            // neko_idraw_push_matrix(&g_idraw, NEKO_IDRAW_MATRIX_MODELVIEW);
            // neko_idraw_mul_matrix(&g_idraw, neko_vqs_to_mat4(&ai.xform));
            // neko_idraw_box(&g_idraw, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 255, 255, 255, NEKO_GRAPHICS_PRIMITIVE_LINES);
            // neko_idraw_line3Dv(&g_idraw, neko_v3s(0.f), NEKO_ZAXIS, NEKO_COLOR_BLUE);
            // neko_idraw_line3Dv(&g_idraw, neko_v3s(0.f), NEKO_XAXIS, NEKO_COLOR_RED);
            // neko_idraw_line3Dv(&g_idraw, neko_v3s(0.f), NEKO_YAXIS, NEKO_COLOR_GREEN);
            // neko_idraw_pop_matrix(&g_idraw);

            // // Render target
            // neko_idraw_push_matrix(&g_idraw, NEKO_IDRAW_MATRIX_MODELVIEW);
            // neko_idraw_translatev(&g_idraw, ai.target);
            // neko_idraw_box(&g_idraw, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 255, 0, 255, NEKO_GRAPHICS_PRIMITIVE_LINES);
            // neko_idraw_pop_matrix(&g_idraw);
        }

#pragma endregion

        neko_graphics_renderpass_begin(&g_cb, NEKO_GRAPHICS_RENDER_PASS_DEFAULT);
        {
            neko_graphics_set_viewport(&g_cb, 0, 0, (u32)fbs.x, (u32)fbs.y);
            neko_idraw_draw(&g_idraw, &g_cb);  // 立即模式绘制 idraw
        }
        neko_graphics_renderpass_end(&g_cb);

        neko_imgui_new_frame(&g_imgui);

        {
            neko_profiler_scope_begin(lua_render);
            try {
                neko_lua_wrap_call(g_L, "game_render");
            } catch (std::exception &ex) {
                neko_log_error("lua exception %s", ex.what());
            }
            neko_profiler_scope_end(lua_render);
        }

#pragma region gui

        neko_ui_begin(&g_ui, NULL);
        {
            editor_dockspace(&g_ui);

            if (g_cvar.show_gui) {

                neko_ui_demo_window(&g_ui, neko_ui_rect(100, 100, 500, 500), NULL);
                neko_ui_style_editor(&g_ui, NULL, neko_ui_rect(350, 250, 300, 240), NULL);

                const neko_vec2 ws = neko_v2(600.f, 300.f);

#pragma region gui_ss

#if 1

                const neko_ui_style_sheet_t *ss = &style_sheet;

                const neko_vec2 ss_ws = neko_v2(500.f, 300.f);
                neko_ui_window_begin(&g_ui, "Window", neko_ui_rect((fbs.x - ss_ws.x) * 0.5f, (fbs.y - ss_ws.y) * 0.5f, ss_ws.x, ss_ws.y));
                {
                    // Cache the current container
                    neko_ui_container_t *cnt = neko_ui_get_current_container(&g_ui);

                    neko_ui_layout_row(&g_ui, 2, neko_ui_widths(200, 0), 0);

                    neko_ui_text(&g_ui, "A regular element button.");
                    neko_ui_button(&g_ui, "button");

                    neko_ui_text(&g_ui, "A regular element label.");
                    neko_ui_label(&g_ui, "label");

                    neko_ui_text(&g_ui, "Button with classes: {.c0 .btn}");

                    neko_ui_selector_desc_t selector_1 = {.classes = {"c0", "btn"}};
                    neko_ui_button_ex(&g_ui, "hello?##btn", &selector_1, 0x00);

                    neko_ui_text(&g_ui, "Label with id #lbl and class .c0");
                    neko_ui_selector_desc_t selector_2 = {.id = "lbl", .classes = {"c0"}};
                    neko_ui_label_ex(&g_ui, "label##lbl", &selector_2, 0x00);

                    const f32 m = cnt->body.w * 0.3f;
                    // neko_ui_layout_row(gui, 2, (int[]){m, -m}, 0);
                    // neko_ui_layout_next(gui); // Empty space at beginning
                    neko_ui_layout_row(&g_ui, 1, neko_ui_widths(0), 0);
                    neko_ui_selector_desc_t selector_3 = {.classes = {"reload_btn"}};
                    if (neko_ui_button_ex(&g_ui, "reload style sheet", &selector_3, 0x00)) {
                        app_load_style_sheet(true);
                    }

                    button_custom(&g_ui, "Hello?");
                }
                neko_ui_window_end(&g_ui);

#endif

#pragma endregion

#pragma region gui_idraw

                neko_ui_window_begin(&g_ui, "Idraw", neko_ui_rect((fbs.x - ws.x) * 0.2f, (fbs.y - ws.y) * 0.5f, ws.x, ws.y));
                {
                    // Cache the current container
                    neko_ui_container_t *cnt = neko_ui_get_current_container(&g_ui);

                    // 绘制到当前窗口中的转换对象的自定义回调
                    // 在这里，我们将容器的主体作为视口传递，但这可以是您想要的任何内容，
                    // 正如我们将在“分割”窗口示例中看到的那样。
                    // 我们还传递自定义数据（颜色），以便我们可以在回调中使用它。如果以下情况，这可以为 NULL
                    // 你不需要任何东西
                    neko_color_t color = neko_color_alpha(NEKO_COLOR_RED, (uint8_t)neko_clamp((sin(t * 0.001f) * 0.5f + 0.5f) * 255, 0, 255));
                    neko_ui_draw_custom(&g_ui, cnt->body, gui_cb, &color, sizeof(color));
                }
                neko_ui_window_end(&g_ui);

#pragma endregion
            }

            if (neko_platform_key_pressed(NEKO_KEYCODE_GRAVE_ACCENT)) {
                console.open = !console.open;
            } else if (neko_platform_key_pressed(NEKO_KEYCODE_TAB) && console.open) {
                console.autoscroll = !console.autoscroll;
            }

            neko_ui_layout_t l;

            if (g_cvar.show_gui) {

                if (window && neko_ui_window_begin(&g_ui, "App", neko_ui_rect(fbs.x - 210, 30, 200, 200))) {
                    l = *neko_ui_get_layout(&g_ui);
                    neko_ui_layout_row(&g_ui, 1, neko_ui_widths(-1), 0);

                    static f32 delta, fps = neko_default_val();

                    neko_timed_action(5, {
                        delta = neko_platform_delta_time();
                        fps = 1.f / delta;
                    });

                    neko_ui_labelf("FPS: %.2lf Delta: %.6lf", fps, delta * 1000.f);

                    // neko_ui_text_fc(&g_core_ui, "喵喵昂~");

                    // neko_ui_layout_row(&g_core_ui, 1, neko_ui_widths(-1), 0);

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

                    neko_ui_layout_row(&g_ui, 7, neko_ui_widths(100, 100, 32, 100, 32, 100, 32), 0);
                    for (u32 i = 0; btns[i].str; ++i) {
                        neko_ui_labelf("%s: ", btns[i].str);
                        neko_ui_labelf("pressed: ");
                        neko_ui_labelf("%d", mouse_pressed[btns[i].val]);
                        neko_ui_labelf("down: ");
                        neko_ui_labelf("%d", mouse_down[btns[i].val]);
                        neko_ui_labelf("released: ");
                        neko_ui_labelf("%d", mouse_released[btns[i].val]);
                    }

                    neko_ui_layout_row(&g_ui, 1, neko_ui_widths(-1), 0);
                    {
                        static neko_memory_info_t meminfo = neko_default_val();

                        neko_timed_action(60, { meminfo = neko_platform_memory_info(); });

                        neko_ui_labelf("GC MemAllocInUsed: %.2lf mb", (f64)(neko_mem_bytes_inuse() / 1048576.0));
                        neko_ui_labelf("GC MemTotalAllocated: %.2lf mb", (f64)(g_allocation_metrics.total_allocated / 1048576.0));
                        neko_ui_labelf("GC MemTotalFree: %.2lf mb", (f64)(g_allocation_metrics.total_free / 1048576.0));

                        lua_Integer kb = lua_gc(g_L, LUA_GCCOUNT, 0);
                        lua_Integer bytes = lua_gc(g_L, LUA_GCCOUNTB, 0);

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

                    neko_ui_window_end(&g_ui);
                }
            }

            int s = summons;
            while (s--) {
                neko_ui_push_id(&g_ui, &s, sizeof(s));
                if (neko_ui_window_begin(&g_ui, "Summon", neko_ui_rect(100, 100, 200, 200))) {
                    neko_ui_layout_row(&g_ui, 1, neko_ui_widths(-1), 0);
                    neko_ui_text(&g_ui, "new window");
                    neko_ui_window_end(&g_ui);
                }
                neko_ui_pop_id(&g_ui);
            }

            neko_vec2 fb = (&g_ui)->framebuffer_size;
            neko_ui_rect_t screen;
            if (embeded)
                screen = l.body;
            else
                screen = neko_ui_rect(0, 0, fb.x, fb.y);
            neko_console(&console, &g_ui, screen, NULL);
        }
        neko_ui_end(&g_ui, true);

        if (ImGui::Begin("Debug")) {
            //            neko_imgui::Auto(cam, "Cam");
            ImGui::Text("%f %f", cam.x, cam.y);
        }
        ImGui::End();

        if (g_cvar.show_demo_window) ImGui::ShowDemoWindow();

        // neko_imgui::Auto(g_cvar.bg);
        // if (g_cvar.shader_inspect && ImGui::Begin("Shaders")) {
        //     inspect_shader("sprite_shader", sprite_shader.program);  // TEST
        //     ImGui::End();
        // }

        if (g_cvar.show_editor && ImGui::Begin("Cvar")) {
            try {
                neko_cvar_gui();
            } catch (const std::exception &ex) {
                neko_log_error("cvar exception %s", ex.what());
            }

            neko_imgui::toggle("帧检查器", &g_cvar.show_profiler_window);

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

        if (g_cvar.show_editor) {
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("engine")) {
                    if (ImGui::Checkbox("vsync", &g_cvar.vsync)) neko_platform_enable_vsync(g_cvar.vsync);
                    if (ImGui::MenuItem("quit")) {
                        ImGui::OpenPopup("Delete?");
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
        }

        if (g_cvar.show_profiler_window) {
            static neko_profiler_frame_t frame_data;
            neko_profiler_get_frame(&frame_data);
            // // if (g_multi) ProfilerDrawFrameNavigation(g_frameInfos.data(), g_frameInfos.size());
            static char buffer[10 * 1024];
            neko_profiler_draw_frame(&frame_data, buffer, 10 * 1024);
            // ProfilerDrawStats(&data);
        }

#pragma endregion

        //        sandbox_update(sb);

        game->Update();
        game->Render();

        neko_graphics_renderpass_begin(&g_cb, main_rp);
        {
            neko_graphics_set_viewport(&g_cb, 0, 0, (u32)fbs.x, (u32)fbs.y);
            neko_graphics_clear(&g_cb, clear);
            neko_idraw_draw(&g_idraw, &g_cb);  // 立即模式绘制 idraw
            neko_graphics_draw_batch(&g_cb, font_render, 0, 0, 0);
        }
        neko_graphics_renderpass_end(&g_cb);

        neko_idraw_defaults(&g_idraw);
        neko_idraw_camera2d(&g_idraw, (u32)win_size.x, (u32)win_size.y);
        neko_idraw_texture(&g_idraw, main_rt);
        neko_idraw_rectvd(&g_idraw, neko_v2(0.0, 0.0), neko_v2((u32)win_size.x, (u32)win_size.y), neko_v2(0.0, 1.0), neko_v2(1.0, 0.0), neko_color(255, 255, 255, 255),
                          NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);

        neko_graphics_renderpass_begin(&g_cb, NEKO_GRAPHICS_RENDER_PASS_DEFAULT);
        {
            neko_graphics_set_viewport(&g_cb, 0.0, 0.0, win_size.x, win_size.y);
            neko_idraw_draw(&g_idraw, &g_cb);  // 立即模式绘制 idraw
            neko_ui_render(&g_ui, &g_cb);
        }
        neko_graphics_renderpass_end(&g_cb);

        neko_imgui_render();

        // Submits to cb
        {
            neko_profiler_scope_begin(render_submit);
            neko_graphics_command_buffer_submit(&g_cb);
            neko_profiler_scope_end(render_submit);
        }

        neko_check_gl_error();
    }
}

void game_shutdown() {

    //    sandbox_destroy(sb);

    game->Clean();

    neko_graphics_renderpass_destroy(main_rp);
    neko_graphics_texture_destroy(main_rt);
    neko_graphics_framebuffer_destroy(main_fbo);

    try {
        neko_lua_wrap_call(g_L, "game_shutdown");
    } catch (std::exception &ex) {
        neko_log_error("lua exception %s", ex.what());
    }

    neko_scripting_end(g_L);
    neko_safe_free(font_verts);

    neko_font_free(test_font_bmfont);
    neko_graphics_batch_free(font_render);

    neko_immediate_draw_free(&g_idraw);
    neko_command_buffer_free(&g_cb);

    destroy_texture_handle(test_font_tex_id, NULL);

    neko_imgui_shutdown(&g_imgui);
    neko_ui_free(&g_ui);
    neko_pack_destroy(&g_pack);
    neko_pack_destroy(&g_lua_pack);
}

neko_game_desc_t neko_main(s32 argc, char **argv) {

    // 确定运行路径
    auto current_dir = std::filesystem::path(std::filesystem::current_path());
    for (int i = 0; i < 6; ++i)
        if (std::filesystem::exists(current_dir / "gamedir") &&  //
            std::filesystem::exists(current_dir / "gamedir" / "assets")) {
            data_path = neko::fs_normalize_path(current_dir.string());
            neko_log_info(std::format("gamedir: {0} (base: {1})", data_path, std::filesystem::current_path().string()).c_str());
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
                            .console = &console};
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