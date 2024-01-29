

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
#include "engine/neko_component.h"
#include "engine/neko_platform.h"
#include "engine/util/neko_ai.h"
#include "engine/util/neko_asset.h"
#include "engine/util/neko_gfxt.h"
#include "engine/util/neko_gui.h"
#include "engine/util/neko_idraw.h"
#include "engine/util/neko_imgui.h"
#include "engine/util/neko_sprite.h"
#include "engine/util/neko_tiled.h"
#include "engine/util/neko_vm.h"

// builtin
#include "engine/builtin/neko_aseprite.h"

// game
#include "game_editor.h"
#include "game_scripts.h"
#include "neko_client_ecs.h"
#include "neko_hash.h"
#include "neko_nui_auto.hpp"
#include "player.h"

// hpp
#include "hpp/neko_static_refl.hpp"
#include "hpp/neko_struct.hpp"

#define NEKO_CONSOLE_IMPL
#include "engine/util/neko_console.h"

#define NEKO_IMGUI_IMPL
#include "game_imgui.h"

#define SPRITEBATCH_IMPLEMENTATION
#include "engine/builtin/cute_spritebatch.h"

#define CUTE_PNG_IMPLEMENTATION
#include "engine/builtin/cute_png.h"

#define CUTE_FILEWATCH_IMPLEMENTATION
#define STRPOOL_IMPLEMENTATION
#define ASSETSYS_IMPLEMENTATION
#include "engine/builtin/cute_filewatch.h"

// opengl
#include "libs/glad/glad.h"

// stb
#include "libs/stb/stb_image.h"

NEKO_HIJACK_MAIN();

neko_command_buffer_t g_cb = neko_default_val();
neko_core_ui_context_t g_core_ui = neko_default_val();
neko_immediate_draw_t g_idraw = neko_default_val();
neko_client_userdata_t g_client_userdata = neko_default_val();
neko_asset_ascii_font_t g_font;
neko_core_ui_style_sheet_t style_sheet;
neko_gui_ctx_t g_gui = neko_default_val();
neko_imgui_context_t g_imgui = neko_default_val();

neko_handle(neko_graphics_renderpass_t) rp = neko_default_val();
neko_handle(neko_graphics_framebuffer_t) fbo = neko_default_val();
neko_handle(neko_graphics_texture_t) rt = neko_default_val();

neko_texture_t g_test_ase = neko_default_val();
neko_asset_manager_t g_am = neko_default_val();
neko_asset_t tex_hndl = neko_default_val();
neko_string data_path = neko_default_val();
neko_global neko_sprite test_witch_spr = neko_default_val();
neko_packreader_t g_pack = neko_default_val();

game_vm_t g_vm = neko_default_val();

game_csharp g_csharp;

// ai test
enum { AI_STATE_MOVE = 0x00, AI_STATE_HEAL };

typedef struct {
    neko_vqs xform;
    neko_vec3 target;
    f32 health;
    int16_t state;
} ai_t;

neko_camera_t camera;
neko_ai_bt_t bt;
ai_t ai;

// decl static reflection
template <>
struct neko::meta::static_refl::TypeInfo<neko_platform_running_desc_t> : TypeInfoBase<neko_platform_running_desc_t> {
    static constexpr AttrList attrs = {};
    static constexpr FieldList fields = {
            Field{TSTR("title"), &Type::title},                  // 窗口标题
            Field{TSTR("width"), &Type::width},                  //
            Field{TSTR("height"), &Type::height},                //
            Field{TSTR("flags"), &Type::flags},                  //
            Field{TSTR("num_samples"), &Type::num_samples},      //
            Field{TSTR("monitor_index"), &Type::monitor_index},  //
            Field{TSTR("vsync"), &Type::vsync},                  // 启用 vsync
            Field{TSTR("frame_rate"), &Type::frame_rate},        // 限制帧率
            Field{TSTR("engine_args"), &Type::engine_args},      // 引擎参数
    };
};

#define CVAR_TYPES() bool, s32, f32, f32 *

struct neko_engine_cvar_t {
    bool show_editor = false;

    bool show_demo_window = false;
    bool show_info_window = false;
    bool show_cvar_window = false;
    bool show_pack_editor = false;
    bool show_profiler_window = false;

    bool show_gui = false;

    bool hello_ai_shit = false;

    f32 bg[3] = {28, 28, 28};
};

neko_struct(neko_engine_cvar_t,                            //
            _Fs(show_editor, "Is show editor"),            //
            _Fs(show_demo_window, "Is show nui demo"),     //
            _Fs(show_info_window, "Is show info window"),  //
            _Fs(show_cvar_window, "cvar inspector"),       //
            _Fs(show_pack_editor, "pack editor"),          //
            _Fs(show_profiler_window, "profiler"),         //
            _Fs(show_gui, "neko gui"),                     //
            _Fs(hello_ai_shit, "Test AI"),                 //
            _Fs(bg, "bg color")                            //
);

neko_engine_cvar_t g_cvar = neko_default_val();

void editor_dockspace(neko_core_ui_context_t *ctx);

// test
void test_xml(const neko_string &file);
void test_sr();
void test_ut();
void test_se();
void test_containers();
void test_nbt();
void test_cs(std::string_view path);

NEKO_API_DECL void test_sexpr();
NEKO_API_DECL void test_ttf();
NEKO_API_DECL void test_sc();

neko_texture_t load_ase_texture_simple(const void *memory, int size) {

    ase_t *ase = neko_aseprite_load_from_memory(memory, size);
    neko_defer([&] { neko_aseprite_free(ase); });

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

neko_string game_assets(const neko_string &path) {
    if (data_path.empty()) {
        neko_assert(!data_path.empty());  // game data path not detected
        return path;
    }
    neko_string get_path(data_path);
    switch (neko::hash(path.substr(0, 7))) {
        case neko::hash("gamedir"):
            get_path.append(path);
            break;
        default:
            break;
    }
    return get_path;
}

void app_load_style_sheet(bool destroy) {
    if (destroy) {
        neko_core_ui_style_sheet_destroy(&style_sheet);
    }
    style_sheet = neko_core_ui_style_sheet_load_from_file(&g_core_ui, game_assets("gamedir/style_sheets/gui.ss").c_str());
    neko_core_ui_set_style_sheet(&g_core_ui, &style_sheet);
}

s32 button_custom(neko_core_ui_context_t *ctx, const char *label) {
    // Do original button call
    s32 res = neko_core_ui_button(ctx, label);

    // Draw inner shadows/highlights over button
    neko_color_t hc = NEKO_COLOR_WHITE, sc = neko_color(85, 85, 85, 255);
    neko_core_ui_rect_t r = ctx->last_rect;
    s32 w = 2;
    neko_core_ui_draw_rect(ctx, neko_core_ui_rect(r.x + w, r.y, r.w - 2 * w, w), hc);
    neko_core_ui_draw_rect(ctx, neko_core_ui_rect(r.x + w, r.y + r.h - w, r.w - 2 * w, w), sc);
    neko_core_ui_draw_rect(ctx, neko_core_ui_rect(r.x, r.y, w, r.h), hc);
    neko_core_ui_draw_rect(ctx, neko_core_ui_rect(r.x + r.w - w, r.y, w, r.h), sc);

    return res;
}

// Custom callback for immediate drawing directly into the gui window
void gui_cb(neko_core_ui_context_t *ctx, struct neko_core_ui_customcommand_t *cmd) {
    neko_immediate_draw_t *gui_idraw = &ctx->gui_idraw;  // Immediate draw list in gui context
    neko_vec2 fbs = ctx->framebuffer_size;               // Framebuffer size bound for gui context
    neko_color_t *color = (neko_color_t *)cmd->data;     // Grab custom data
    neko_asset_texture_t *tp = neko_assets_getp(&g_am, neko_asset_texture_t, tex_hndl);
    const f32 t = neko_platform_elapsed_time();

    // Set up an immedaite camera using our passed in cmd viewport (this is the clipped viewport of the gui window being drawn)
    neko_idraw_camera3D(gui_idraw, (u32)cmd->viewport.w, (u32)cmd->viewport.h);
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
    neko_idraw_camera2D(gui_idraw, (u32)fbs.x, (u32)fbs.y);

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
    neko_idraw_camera3D(gui_idraw, (u32)fbs.x, (u32)fbs.y);
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
    neko_idraw_camera3D(gui_idraw, (u32)fbs.x, (u32)fbs.y);
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

#pragma region AI

// Behavior tree functions
void ai_behavior_tree_frame(struct neko_ai_bt_t *ctx);
void ai_task_target_find(struct neko_ai_bt_t *ctx, struct neko_ai_bt_node_t *node);
void ai_task_target_move_to(struct neko_ai_bt_t *ctx, struct neko_ai_bt_node_t *node);
void ai_task_health_check(struct neko_ai_bt_t *ctx, struct neko_ai_bt_node_t *node);
void ai_task_heal(struct neko_ai_bt_t *ctx, struct neko_ai_bt_node_t *node);

void ai_behavior_tree_frame(struct neko_ai_bt_t *ctx) {
    ai_t *ai = (ai_t *)ctx->ctx.user_data;

    neko_ai_bt(ctx, {
        neko_ai_repeater(ctx, {
            neko_ai_selector(ctx, {
                // Heal
                neko_ai_sequence(ctx, {
                    neko_ai_leaf(ctx, ai_task_health_check);
                    neko_ai_leaf(ctx, ai_task_heal);
                });

                // Move to
                neko_ai_sequence(ctx, {
                    neko_ai_leaf(ctx, ai_task_target_find);
                    neko_ai_condition(ctx, (ai->health > 50.f), { neko_ai_leaf(ctx, ai_task_target_move_to); });
                });
            });
        });
    });
}

void ai_task_health_check(struct neko_ai_bt_t *ctx, struct neko_ai_bt_node_t *node) {
    ai_t *ai = (ai_t *)ctx->ctx.user_data;
    node->state = ai->health >= 50 ? NEKO_AI_BT_STATE_FAILURE : NEKO_AI_BT_STATE_SUCCESS;
}

void ai_task_heal(struct neko_ai_bt_t *ctx, struct neko_ai_bt_node_t *node) {
    ai_t *ai = (ai_t *)ctx->ctx.user_data;
    if (ai->health < 100.f) {
        ai->health += 1.f;
        node->state = NEKO_AI_BT_STATE_RUNNING;
        ai->state = AI_STATE_HEAL;
    } else {
        node->state = NEKO_AI_BT_STATE_SUCCESS;
    }
}

void ai_task_target_find(struct neko_ai_bt_t *ctx, struct neko_ai_bt_node_t *node) {
    ai_t *ai = (ai_t *)ctx->ctx.user_data;
    f32 dist = neko_vec3_dist(ai->xform.translation, ai->target);
    if (dist < 1.f) {
        neko_mt_rand_t rand = neko_rand_seed(time(NULL));
        neko_vec3 target = neko_v3(neko_rand_gen_range(&rand, -10.f, 10.f), 0.f, neko_rand_gen_range(&rand, -10.f, 10.f));
        ai->target = target;
    }
    node->state = NEKO_AI_BT_STATE_SUCCESS;
}

void ai_task_target_move_to(struct neko_ai_bt_t *ctx, struct neko_ai_bt_node_t *node) {
    ai_t *ai = (ai_t *)ctx->ctx.user_data;
    const f32 dt = neko_platform_time()->delta;
    f32 dist = neko_vec3_dist(ai->xform.translation, ai->target);
    f32 speed = 25.f * dt;
    if (dist > speed) {
        neko_vec3 dir = neko_vec3_norm(neko_vec3_sub(ai->target, ai->xform.translation));
        neko_vec3 vel = neko_vec3_scale(dir, speed);
        neko_vec3 np = neko_vec3_add(ai->xform.translation, vel);
        ai->xform.translation =
                neko_v3(neko_interp_linear(ai->xform.translation.x, np.x, 0.05f), neko_interp_linear(ai->xform.translation.y, np.y, 0.05f), neko_interp_linear(ai->xform.translation.z, np.z, 0.05f));
        // Look at target rotation
        ai->xform.rotation = neko_quat_look_rotation(ai->xform.translation, ai->target, NEKO_YAXIS);
        node->state = NEKO_AI_BT_STATE_RUNNING;
        ai->state = AI_STATE_MOVE;
    } else {
        node->state = NEKO_AI_BT_STATE_SUCCESS;
    }
}

#pragma endregion

#pragma region console

static bool window = 1, embeded;
static int summons;

static void toggle_window(int argc, char **argv);
static void toggle_embedded(int argc, char **argv);
static void help(int argc, char **argv);
static void echo(int argc, char **argv);
static void spam(int argc, char **argv);
static void crash(int argc, char **argv);
void summon(int argc, char **argv);
void nui_theme(int argc, char **argv);
void sz(int argc, char **argv);

neko_console_command_t commands[] = {
        {
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
                .func = nui_theme,
                .name = "nui_theme",
                .desc = "set neko nui theme",
        },
};

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

void nui_theme(int argc, char **argv) {
    if (argc != 2) return;
    set_style(&g_gui.neko_gui_ctx, (neko_gui_style_theme)atoi(argv[1]));
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
                neko_gui::gui_auto(var, name);
                neko_gui_labelf(&g_gui.neko_gui_ctx, NEKO_GUI_TEXT_LEFT, "    [%s]", std::get<0>(fields));
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
                    neko_gui::gui_auto((&neko_cv()->cvars[i])->value.s, (&neko_cv()->cvars[i])->name);
                    break;
                case __NEKO_CONFIG_TYPE_FLOAT:
                    neko_gui::gui_auto((&neko_cv()->cvars[i])->value.f, (&neko_cv()->cvars[i])->name);
                    break;
                case __NEKO_CONFIG_TYPE_INT:
                    neko_gui::gui_auto((&neko_cv()->cvars[i])->value.i, (&neko_cv()->cvars[i])->name);
                    break;
            };
        };
    }
}

// clang-format off
const_str image_names[] = {
        "gamedir/assets/textures/dragon_zombie.png",
        "gamedir/assets/textures/night_spirit.png",
};
// clang-format on

int images_count = sizeof(image_names) / sizeof(*image_names);
cp_image_t images[sizeof(image_names) / sizeof(*image_names)];

spritebatch_t sb;

neko_graphics_custom_batch_context_t *sprite_batch;
neko_graphics_custom_batch_shader_t sprite_shader;
neko_graphics_custom_batch_renderable_t sprite_renderable;
f32 sprite_projection[16];
f32 font_projection[16];

// example data for storing + transforming sprite vertices on CPU
typedef struct {
    f32 x, y;
    f32 u, v;
} vertex_t;

#define SPRITE_VERTS_MAX (1024 * 10)
int sprite_verts_count;
vertex_t sprite_verts[SPRITE_VERTS_MAX];

// example of a game sprite
typedef struct {
    SPRITEBATCH_U64 image_id;
    int depth;
    f32 x, y;
    f32 sx, sy;
    f32 c, s;
} neko_sprite_t;

neko_graphics_custom_batch_context_t *font_render;
neko_graphics_custom_batch_shader_t font_shader;
neko_graphics_custom_batch_renderable_t font_renderable;
f32 font_scale = 5.f;
int font_vert_count;
neko_font_vert_t *font_verts;
neko_font_u64 test_font_tex_id;
neko_font_t *test_font_bmfont;

spritebatch_config_t get_demo_config() {
    spritebatch_config_t config;
    spritebatch_set_default_config(&config);
    config.pixel_stride = sizeof(uint8_t) * 4;  // RGBA uint8_t from cute_png.h
    config.atlas_width_in_pixels = 1024;
    config.atlas_height_in_pixels = 1024;
    config.atlas_use_border_pixels = 0;
    config.ticks_to_decay_texture = 3;
    config.lonely_buffer_count_till_flush = 1;
    config.ratio_to_decay_atlas = 0.5f;
    config.ratio_to_merge_atlases = 0.25f;
    config.allocator_context = 0;
    return config;
}

neko_sprite_t make_sprite(SPRITEBATCH_U64 image_id, f32 x, f32 y, f32 scale, f32 angle_radians, int depth) {
    neko_sprite_t s;
    s.image_id = image_id;
    s.depth = depth;
    s.x = x;
    s.y = y;
    s.sx = (f32)images[s.image_id].w * 2.0f * scale;
    s.sy = (f32)images[s.image_id].h * 2.0f * scale;
    s.c = cosf(angle_radians);
    s.s = sinf(angle_radians);
    return s;
}

void push_sprite(neko_sprite_t sp) {
    spritebatch_sprite_t s;
    s.image_id = sp.image_id;
    s.w = images[sp.image_id].w;
    s.h = images[sp.image_id].h;
    s.x = sp.x;
    s.y = sp.y;
    s.sx = sp.sx;
    s.sy = sp.sy;
    s.c = sp.c;
    s.s = sp.s;
    s.sort_bits = (SPRITEBATCH_U64)sp.depth;
    spritebatch_push(&sb, s);
}

int call_count = 0;

// callbacks for cute_spritebatch.h
void batch_report(spritebatch_sprite_t *sprites, int count, int texture_w, int texture_h, void *udata) {
    ++call_count;

    (void)udata;
    (void)texture_w;
    (void)texture_h;
    // printf("begin batch\n");
    // for (int i = 0; i < count; ++i) printf("\t%llu\n", sprites[i].texture_id);
    // printf("end batch\n");

    // build the draw call
    neko_graphics_custom_batch_draw_call_t call;
    call.r = &sprite_renderable;
    call.textures[0] = (u32)sprites[0].texture_id;
    call.texture_count = 1;

    // set texture uniform in shader
    neko_graphics_custom_batch_send_texture(call.r->program, "u_sprite_texture", 0);

    // NOTE:
    // perform any additional sorting here

    // build vertex buffer of quads from all sprite transforms
    call.verts = sprite_verts + sprite_verts_count;
    call.vert_count = count * 6;
    sprite_verts_count += call.vert_count;
    assert(sprite_verts_count < SPRITE_VERTS_MAX);

    vertex_t *verts = (vertex_t *)call.verts;
    for (int i = 0; i < count; ++i) {
        spritebatch_sprite_t *s = sprites + i;

        neko_vec2 quad[] = {
                {-0.5f, 0.5f},
                {0.5f, 0.5f},
                {0.5f, -0.5f},
                {-0.5f, -0.5f},
        };

        for (int j = 0; j < 4; ++j) {
            f32 x = quad[j].x;
            f32 y = quad[j].y;

            // scale sprite about origin
            x *= s->sx;
            y *= s->sy;

            // rotate sprite about origin
            f32 x0 = s->c * x - s->s * y;
            f32 y0 = s->s * x + s->c * y;
            x = x0;
            y = y0;

            // translate sprite into the world
            x += s->x;
            y += s->y;

            quad[j].x = x;
            quad[j].y = y;
        }

        // output transformed quad into CPU buffer
        vertex_t *out_verts = verts + i * 6;

        out_verts[0].x = quad[0].x;
        out_verts[0].y = quad[0].y;
        out_verts[0].u = s->minx;
        out_verts[0].v = s->maxy;

        out_verts[1].x = quad[3].x;
        out_verts[1].y = quad[3].y;
        out_verts[1].u = s->minx;
        out_verts[1].v = s->miny;

        out_verts[2].x = quad[1].x;
        out_verts[2].y = quad[1].y;
        out_verts[2].u = s->maxx;
        out_verts[2].v = s->maxy;

        out_verts[3].x = quad[1].x;
        out_verts[3].y = quad[1].y;
        out_verts[3].u = s->maxx;
        out_verts[3].v = s->maxy;

        out_verts[4].x = quad[3].x;
        out_verts[4].y = quad[3].y;
        out_verts[4].u = s->minx;
        out_verts[4].v = s->miny;

        out_verts[5].x = quad[2].x;
        out_verts[5].y = quad[2].y;
        out_verts[5].u = s->maxx;
        out_verts[5].v = s->miny;
    }

    // submit call to cute_gl (does not get flushed to screen until `gl_flush` is called)
    neko_graphics_custom_batch_push_draw_call(sprite_batch, call);
}

void get_pixels(SPRITEBATCH_U64 image_id, void *buffer, int bytes_to_fill, void *udata) {
    (void)udata;
    memcpy(buffer, images[image_id].pix, bytes_to_fill);
}

SPRITEBATCH_U64 generate_texture_handle(void *pixels, int w, int h, void *udata) {
    (void)udata;
    GLuint location;
    glGenTextures(1, &location);
    glBindTexture(GL_TEXTURE_2D, location);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    return (SPRITEBATCH_U64)location;
}

void destroy_texture_handle(SPRITEBATCH_U64 texture_id, void *udata) {
    (void)udata;
    GLuint id = (GLuint)texture_id;
    glDeleteTextures(1, &id);
}

void load_images() {
    for (int i = 0; i < images_count; ++i) images[i] = cp_load_png(game_assets(image_names[i]).c_str());
}

void unload_images() {
    for (int i = 0; i < images_count; ++i) cp_free_png(images + i);
}

void draw_text(neko_font_t *font, const char *text, float x, float y, float line_height, float clip_region, float wrap_x) {
    f32 text_w = (f32)neko_font_text_width(font, text);
    f32 text_h = (f32)neko_font_text_height(font, text);

    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());

    neko_font_rect_t clip_rect;
    clip_rect.left = -fbs.x / font_scale * clip_region;
    clip_rect.right = fbs.x / font_scale * clip_region + 0.5f;
    clip_rect.top = fbs.y / font_scale * clip_region + 0.5f;
    clip_rect.bottom = -fbs.y / font_scale * clip_region;

    f32 x0 = (x - fbs.x / 2.f) / font_scale /*+ -text_w / 2.f*/;
    f32 y0 = (fbs.y / 2.f - y) / font_scale + text_h / 2.f;
    f32 wrap_width = wrap_x - x0;

    neko_font_fill_vertex_buffer(font, text, x0, y0, wrap_width, line_height, &clip_rect, font_verts, 1024 * 2, &font_vert_count);

    if (font_vert_count) {
        neko_graphics_custom_batch_draw_call_t call;
        call.textures[0] = (u32)font->atlas_id;
        call.texture_count = 1;
        call.r = &font_renderable;
        call.verts = font_verts;
        call.vert_count = font_vert_count;

        neko_graphics_custom_batch_push_draw_call(font_render, call);
    }
}

neko_script_binary_t *ns_load_module(char *name) {
    FILE *fd;
    std::string fullname[] = {std::format("{0}/{1}.ns", game_assets("gamedir/tests"), name)};
    for (int i = 0; i < neko_arr_size(fullname); i++) {
        if ((fd = fopen(fullname[i].c_str(), "r")) != NULL) {
            fclose(fd);
            neko_script_binary_t *module = neko_script_compile_file(fullname[i].c_str());
            neko_script_vector_push(g_vm.modules, module);
            return module;
        }
    }
    return NULL;
}

void watch_callback(filewatch_update_t change, const char *virtual_path, void *udata) {
    const char *change_string = nullptr;
    switch (change) {
        case FILEWATCH_DIR_ADDED:
            change_string = "FILEWATCH_DIR_ADDED";
            break;
        case FILEWATCH_DIR_REMOVED:
            change_string = "FILEWATCH_DIR_REMOVED";
            break;
        case FILEWATCH_FILE_ADDED:
            change_string = "FILEWATCH_FILE_ADDED";
            break;
        case FILEWATCH_FILE_REMOVED:
            change_string = "FILEWATCH_FILE_REMOVED";
            break;
        case FILEWATCH_FILE_MODIFIED:
            change_string = "FILEWATCH_FILE_MODIFIED";
            break;
    }

    neko_println("%s at %s", change_string, virtual_path);

    if (!strcmp(virtual_path, "/maps/map.tmx") && change == FILEWATCH_FILE_MODIFIED) {
        neko_tiled_renderer *tiled = static_cast<neko_tiled_renderer *>(neko_ecs_ent_get_component(neko_ecs(), neko_ecs_get_ent(neko_ecs(), __neko_ecs_ent_index(0)), COMPONENT_TILED));
        neko_assert(tiled);

        neko_tiled_unload(&tiled->map);

        neko_tiled_load(&tiled->map, game_assets("gamedir/maps/map.tmx").c_str(), NULL);
    }
}

assetsys_t *assetsys;
filewatch_t *filewatch;

void game_init() {
    g_cb = neko_command_buffer_new();
    g_idraw = neko_immediate_draw_new();

    neko_graphics_info_t *info = neko_graphics_info();
    if (!info->compute.available) {
        neko_log_error("%s", "Compute shaders not available.");
        return;
    }

    g_am = neko_asset_manager_new();

    g_client_userdata.cb = &g_cb;
    g_client_userdata.idraw = &g_idraw;
    g_client_userdata.igui = &g_core_ui;
    g_client_userdata.idraw_sd = neko_immediate_draw_static_data_get();

    neko_ecs()->user_data = &g_client_userdata;

    // ctx.userdata = &g_client_userdata;

    g_vm.ctx = neko_script_ctx_new(nullptr);

    // 测试 csharp wrapper
#ifdef NEKO_DEBUG
    test_cs(game_assets("gamedir/managed/debug"));
#else
    test_cs(game_assets("gamedir/managed/release"));
#endif

    //
#ifdef NEKO_DEBUG
    std::string managed_path(game_assets("gamedir/managed/debug"));
#else
    std::string managed_path(game_assets("gamedir/managed/release"));
#endif

    g_csharp.init(managed_path);

    /*
    {
        neko_lua_wrap_register_t<>(L).def(
                +[](const_str path) -> neko_string { return __neko_game_get_path(path); }, "neko_file_path");

        lua_newtable(L);
        for (int n = 0; n < neko_instance()->ctx.game.argc; ++n) {
            lua_pushstring(L, neko_instance()->ctx.game.argv[n]);
            lua_rawseti(L, -2, n);
        }
        lua_setglobal(L, "arg");

        neko_lua_wrap_do_file(L, __neko_game_get_path("gamedir/scripts/main.lua"));

        neko_platform_running_desc_t t = {.title = "Neko Engine", .engine_args = ""};

        // neko_lua_auto_push(L, neko_application_dedsc_t, &t);

        if (lua_getglobal(L, "neko_app") == LUA_TNIL) throw std::exception("no app");

        if (lua_istable(L, -1)) {
            neko::meta::static_refl::TypeInfo<neko_platform_running_desc_t>::ForEachVarOf(t, [](auto field, auto &&value) {
                static_assert(std::is_lvalue_reference_v<decltype(value)>);
                if (lua_getfield(L, -1, std::string(field.name).c_str()) != LUA_TNIL) value = neko_lua_to<std::remove_reference_t<decltype(value)>>(L, -1);
                lua_pop(L, 1);
            });
        } else {
            throw std::exception("no app table");
        }

        lua_pop(L, 1);

        neko_log_info("load game: %s %d %d", t.title, t.width, t.height);

        neko_platform_set_window_title(neko_platform_main_window(), t.title);
    }

    try {
        neko_lua_wrap_call(L, "game_init");
    } catch (std::exception &ex) {
        neko_log_error("lua exception %s", ex.what());
    }
    */

    neko_pack_result result = neko_pack_read(game_assets("gamedir/res.pack").c_str(), 0, false, &g_pack);
    neko_pack_check(result);

    u8 *font_data, *cat_data;
    u32 font_data_size, cat_data_size;

    result = neko_pack_item_data(&g_pack, ".\\data\\assets\\fonts\\fusion-pixel-12px-monospaced.ttf", (const u8 **)&font_data, &font_data_size);
    neko_pack_check(result);

    result = neko_pack_item_data(&g_pack, ".\\data\\assets\\textures\\cat.aseprite", (const u8 **)&cat_data, &cat_data_size);
    neko_pack_check(result);

    g_test_ase = load_ase_texture_simple(cat_data, cat_data_size);

    neko_asset_texture_t tex0 = {0};
    neko_asset_texture_load_from_file(game_assets("gamedir/assets/textures/yzh.jpg").c_str(), &tex0, NULL, false, false);
    tex_hndl = neko_assets_create_asset(&g_am, neko_asset_texture_t, &tex0);

    neko_core_ui_init(&g_core_ui, neko_platform_main_window());

    // 加载自定义字体文件 初始化 gui font stash
    neko_asset_ascii_font_load_from_memory(font_data, font_data_size, &g_font, 24);
    // neko_asset_ascii_font_load_from_file(__neko_game_get_path("gamedir/assets/fonts/fusion-pixel-12px-monospaced.ttf").c_str(), &g_font, 24);

    neko_safe_free(font_data);
    neko_safe_free(cat_data);

    auto GUI_FONT_STASH = []() -> neko_core_ui_font_stash_desc_t * {
        static neko_core_ui_font_desc_t font_decl[] = {{.key = "mc_regular", .font = &g_font}};
        static neko_core_ui_font_stash_desc_t font_stash = {.fonts = font_decl, .size = 1 * sizeof(neko_core_ui_font_desc_t)};
        return &font_stash;
    }();

    // neko_core_ui_font_stash_desc_t font_stash = {.fonts = (neko_core_ui_font_desc_t[]){{.key = "mc_regular", .font = &font}, .size = 1 * sizeof(neko_core_ui_font_desc_t)};
    neko_core_ui_init_font_stash(&g_core_ui, GUI_FONT_STASH);

    // Load style sheet from file now
    // app_load_style_sheet(false);

    // Dock windows before hand
    neko_core_ui_dock_ex(&g_core_ui, "Style_Editor", "Demo_Window", NEKO_CORE_UI_SPLIT_TAB, 0.5f);

    // Initialize neko_nui context
    neko_gui_init(&g_gui, neko_platform_main_window(), NEKO_GUI_STATE_DEFAULT);

    // struct neko_gui_font_config config = neko_gui_font_config(20);
    // config.oversample_h = 1;
    // config.oversample_v = 1;
    // config.range = neko_gui_font_chinese_glyph_ranges();

    // 字体加载
    neko_gui_font_stash_begin(&g_gui, NULL);
    // neko_gui_font *font = neko_gui_font_atlas_add_from_file(g_nui.atlas, __neko_game_get_path("gamedir/assets/fonts/ark-pixel-12px-monospaced-zh_cn.ttf").c_str(), 20, &config);
    neko_gui_font_stash_end(&g_gui);

    // neko_gui_style_set_font(&g_nui.neko_gui_ctx, &font->handle);

    g_imgui = neko_imgui_new(neko_platform_main_window(), false);

    neko_gui::ctx = &g_gui.neko_gui_ctx;

    g_client_userdata.nui = &g_gui;

    register_components(neko_ecs());
    register_systems(neko_ecs());

    // 测试用
    neko_ecs_ent e1 = neko_ecs_ent_make(neko_ecs());
    neko_ecs_ent e2 = neko_ecs_ent_make(neko_ecs());
    neko_ecs_ent e3 = neko_ecs_ent_make(neko_ecs());
    neko_ecs_ent e4 = neko_ecs_ent_make(neko_ecs());
    CTransform xform = {10, 10};
    CVelocity velocity = {0, 0};
    CGameObject gameobj = neko_default_val();

#if 1

    neko_sprite_load(&test_witch_spr, game_assets("gamedir/assets/textures/B_witch.ase").c_str());

    neko_tiled_renderer tiled = neko_default_val();

    neko_tiled_load(&tiled.map, game_assets("gamedir/maps/map.tmx").c_str(), NULL);

    neko_tiled_render_init(&g_cb, &tiled, game_assets("gamedir/shaders/sprite_vs.glsl").c_str(), game_assets("gamedir/shaders/sprite_fs.glsl").c_str());

    neko_ui_renderer ui_render = neko_default_val();
    ui_render.type = neko_ui_renderer::type::LABEL;
    neko_snprintf(ui_render.text, 128, "%s", "啊对对对");

    neko_fast_sprite_renderer test_fast_sprite = neko_default_val();
    neko_fast_sprite_renderer_construct(&test_fast_sprite, 0, 0, NULL);

    gameobj.visible = true;
    gameobj.active = true;
    neko_snprintf(gameobj.name, 64, "%s", "AAA_ent");

    neko_ecs_ent_add_component(neko_ecs(), e1, COMPONENT_GAMEOBJECT, &gameobj);
    neko_ecs_ent_add_component(neko_ecs(), e1, COMPONENT_TRANSFORM, &xform);
    neko_ecs_ent_add_component(neko_ecs(), e1, COMPONENT_VELOCITY, &velocity);
    neko_ecs_ent_add_component(neko_ecs(), e1, COMPONENT_TILED, &tiled);

    neko_sprite_renderer sprite_test = {.sprite = &test_witch_spr};
    neko_particle_renderer particle_render = neko_default_val();

    neko_sprite_renderer_play(&sprite_test, "Idle");
    neko_particle_renderer_construct(&particle_render);

    gameobj.visible = true;
    gameobj.active = true;
    neko_snprintf(gameobj.name, 64, "%s", "Player_ent");

    neko_ecs_ent_add_component(neko_ecs(), e4, COMPONENT_GAMEOBJECT, &gameobj);
    neko_ecs_ent_add_component(neko_ecs(), e4, COMPONENT_TRANSFORM, &xform);
    neko_ecs_ent_add_component(neko_ecs(), e4, COMPONENT_VELOCITY, &velocity);
    neko_ecs_ent_add_component(neko_ecs(), e4, COMPONENT_SPRITE, &sprite_test);
    neko_ecs_ent_add_component(neko_ecs(), e4, COMPONENT_PARTICLE, &particle_render);

#endif

    neko_snprintf(gameobj.name, 64, "%s", "BBB_ent");
    gameobj.visible = true;
    gameobj.active = false;

    neko_ecs_ent_add_component(neko_ecs(), e2, COMPONENT_GAMEOBJECT, &gameobj);
    neko_ecs_ent_add_component(neko_ecs(), e2, COMPONENT_TRANSFORM, &xform);
    neko_ecs_ent_add_component(neko_ecs(), e2, COMPONENT_GFXT, NULL);
    // neko_ecs_ent_add_component(neko_ecs(), e2, COMPONENT_UI, &ui_render);
    //  neko_ecs_ent_add_component(neko_ecs(), e, COMPONENT_FAST_SPRITE, &test_fast_sprite);

    neko_gfxt_renderer *gfxt_render = (neko_gfxt_renderer *)neko_ecs_ent_get_component(neko_ecs(), e2, COMPONENT_GFXT);

    // Load pipeline from resource file
    gfxt_render->pip = neko_gfxt_pipeline_load_from_file(game_assets("gamedir/assets/pipelines/simple.sf").c_str());

    // Create material using this pipeline
    neko_gfxt_material_desc_t mat_decl = {.pip_func = {.hndl = &gfxt_render->pip}};
    gfxt_render->mat = neko_gfxt_material_create(&mat_decl);

    // Create mesh that uses the layout from the pipeline's requested mesh layout
    neko_gfxt_mesh_import_options_t mesh_decl = {.layout = gfxt_render->pip.mesh_layout,
                                                 .size = neko_dyn_array_size(gfxt_render->pip.mesh_layout) * sizeof(neko_gfxt_mesh_layout_t),
                                                 .index_buffer_element_size = gfxt_render->pip.desc.raster.index_buffer_element_size};

    gfxt_render->mesh = neko_gfxt_mesh_load_from_file(game_assets("gamedir/assets/meshes/Duck.gltf").c_str(), &mesh_decl);

    gfxt_render->texture = neko_gfxt_texture_load_from_file(game_assets("gamedir/assets/textures/DuckCM.png").c_str(), NULL, false, false);

    neko_snprintf(gameobj.name, 64, "%s", "chunk_test");
    gameobj.visible = true;
    gameobj.active = false;

    neko_ecs_ent_add_component(neko_ecs(), e3, COMPONENT_GAMEOBJECT, &gameobj);
    neko_ecs_ent_add_component(neko_ecs(), e3, COMPONENT_TRANSFORM, &xform);
    neko_ecs_ent_add_component(neko_ecs(), e3, COMPONENT_FALLSAND, NULL);

    neko_fallsand_render *fallsand = (neko_fallsand_render *)neko_ecs_ent_get_component(neko_ecs(), e3, COMPONENT_FALLSAND);

    game_chunk_init(fallsand);

    for (int i = 0; i < 10; i++) {
        neko_snprintf(gameobj.name, 64, "%s_%d", "Test_ent", i);
        gameobj.visible = true;
        gameobj.active = false;

        neko_ecs_ent e = neko_ecs_ent_make(neko_ecs());
        neko_ecs_ent_add_component(neko_ecs(), e, COMPONENT_GAMEOBJECT, &gameobj);
        neko_ecs_ent_add_component(neko_ecs(), e, COMPONENT_TRANSFORM, &xform);
    }

    // neko_ecs_ent_print(neko_ecs(), e1);
    // neko_ecs_ent_print(neko_ecs(), e2);
    // neko_ecs_ent_print(neko_ecs(), e3);

    // AI
    // Set up camera
    camera = neko_camera_perspective();
    camera.transform = neko_vqs{.translation = neko_v3(22.52f, 15.52f, 18.18f), .rotation = neko_quat{-0.23f, 0.42f, 0.11f, 0.87f}, .scale = neko_v3s(1.f)};

    // Initialize ai information
    ai = ai_t{.xform = neko_vqs{.translation = neko_v3s(0.f), .rotation = neko_quat_default(), .scale = neko_v3s(1.f)}, .target = neko_v3s(0.f), .health = 100.f};

    // 行为树上下文保留内部 ai 上下文
    // 该上下文可以保存 BT 节点可以读/写的全局/共享信息
    // 我们将把 BT 的内部上下文用户数据设置为 AI 信息的地址
    bt.ctx.user_data = &ai;

    // Construct frame buffer
    fbo = neko_graphics_framebuffer_create(NULL);

    // Construct color render target
    rt = neko_graphics_texture_create(neko_c_ref(neko_graphics_texture_desc_t,
                                                 {
                                                         .width = neko_platform_framebuffer_width(neko_platform_main_window()),    // Width of texture in pixels
                                                         .height = neko_platform_framebuffer_height(neko_platform_main_window()),  // Height of texture in pixels
                                                         .format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8,       // Format of texture data (rgba32, rgba8, rgba32f, r8, depth32f, etc...)
                                                         .wrap_s = NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT,        // Wrapping type for s axis of texture
                                                         .wrap_t = NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT,        // Wrapping type for t axis of texture
                                                         .min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_LINEAR,  // Minification filter for texture
                                                         .mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_LINEAR   // Magnification filter for texture
                                                 }));

    // Construct render pass for offscreen render pass
    rp = neko_graphics_renderpass_create(neko_c_ref(neko_graphics_renderpass_desc_t, {
                                                                                             .fbo = fbo,               // Frame buffer to bind for render pass
                                                                                             .color = &rt,             // Color buffer array to bind to frame buffer
                                                                                             .color_size = sizeof(rt)  // Size of color attachment array in bytes
                                                                                     }));

    sprite_batch = neko_graphics_custom_batch_make_ctx(32);

    g_client_userdata.sprite_batch = sprite_batch;

    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());

    const char *vs = R"(
#version 330

uniform mat4 u_mvp;

in vec2 in_pos; in vec2 in_uv;

out vec2 v_uv;

void main() {
    v_uv = in_uv;
    gl_Position = u_mvp * vec4(in_pos, 0, 1);
}
)";

    const char *ps = R"(
#version 330
precision mediump float;
uniform sampler2D u_sprite_texture;
in vec2 v_uv; out vec4 out_col;
void main() { out_col = texture(u_sprite_texture, v_uv); }
)";

    neko_graphics_custom_batch_vertex_data_t vd;
    neko_graphics_custom_batch_make_vertex_data(&vd, 1024 * 1024, GL_TRIANGLES, sizeof(vertex_t), GL_DYNAMIC_DRAW);
    neko_graphics_custom_batch_add_attribute(&vd, "in_pos", 2, NEKO_GL_CUSTOM_FLOAT, neko_offset(vertex_t, x));
    neko_graphics_custom_batch_add_attribute(&vd, "in_uv", 2, NEKO_GL_CUSTOM_FLOAT, neko_offset(vertex_t, u));

    neko_graphics_custom_batch_make_renderable(&sprite_renderable, &vd);
    neko_graphics_custom_batch_load_shader(&sprite_shader, vs, ps);
    neko_graphics_custom_batch_set_shader(&sprite_renderable, &sprite_shader);

    // gl_make_frame_buffer(&sb_fb, &sprite_shader, (s32)fbs.x, (s32)fbs.y, 0);

    neko_graphics_custom_batch_ortho_2d(fbs.x, fbs.y, 0, 0, sprite_projection);
    // glViewport(0, 0, (s32)fbs.x, (s32)fbs.y);

    neko_graphics_custom_batch_send_matrix(&sprite_shader, "u_mvp", sprite_projection);

    load_images();

    // setup cute_spritebatch configuration
    // this configuration is specialized to test out the demo. don't use these settings
    // in your own project. Instead, start with `spritebatch_set_default_config`.
    spritebatch_config_t sb_config = get_demo_config();
    // spritebatch_set_default_config(&config); // turn default config off to test out demo

    // assign the 4 callbacks
    sb_config.batch_callback = batch_report;                        // report batches of sprites from `spritebatch_flush`
    sb_config.get_pixels_callback = get_pixels;                     // used to retrieve image pixels from `spritebatch_flush` and `spritebatch_defrag`
    sb_config.generate_texture_callback = generate_texture_handle;  // used to generate a texture handle from `spritebatch_flush` and `spritebatch_defrag`
    sb_config.delete_texture_callback = destroy_texture_handle;     // used to destroy a texture handle from `spritebatch_defrag`

    // initialize cute_spritebatch
    spritebatch_init(&sb, &sb_config, NULL);

    font_render = neko_graphics_custom_batch_make_ctx(32);

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

    neko_graphics_custom_batch_vertex_data_t font_vd;
    neko_graphics_custom_batch_make_vertex_data(&font_vd, 1024 * 1024, GL_TRIANGLES, sizeof(neko_font_vert_t), GL_DYNAMIC_DRAW);
    neko_graphics_custom_batch_add_attribute(&font_vd, "in_pos", 2, NEKO_GL_CUSTOM_FLOAT, neko_offset(neko_font_vert_t, x));
    neko_graphics_custom_batch_add_attribute(&font_vd, "in_uv", 2, NEKO_GL_CUSTOM_FLOAT, neko_offset(neko_font_vert_t, u));

    neko_graphics_custom_batch_make_renderable(&font_renderable, &vd);
    neko_graphics_custom_batch_load_shader(&font_shader, font_vs, font_ps);
    neko_graphics_custom_batch_set_shader(&font_renderable, &font_shader);

    neko_graphics_custom_batch_ortho_2d(fbs.x / font_scale, fbs.y / font_scale, 0, 0, font_projection);

    // neko_graphics_custom_batch_ortho_2d((float)640, (float)480, 0, 0, projection);
    // glViewport(0, 0, 640, 480);

    neko_graphics_custom_batch_send_matrix(&font_shader, "u_mvp", font_projection);

    size_t test_font_size = 0;
    void *test_font_mem = neko_platform_read_file_contents(game_assets("gamedir/1.fnt").c_str(), "rb", &test_font_size);
    cp_image_t img = cp_load_png(game_assets("gamedir/1_0.png").c_str());
    test_font_tex_id = generate_texture_handle(img.pix, img.w, img.h, NULL);
    test_font_bmfont = neko_font_load_bmfont(test_font_tex_id, test_font_mem, test_font_size, 0);
    if (test_font_bmfont->atlas_w != img.w || test_font_bmfont->atlas_h != img.h) {
        neko_log_warning("failed to load font");
    }
    neko_safe_free(test_font_mem);
    cp_free_png(&img);

    font_verts = (neko_font_vert_t *)neko_safe_malloc(sizeof(neko_font_vert_t) * 1024 * 2);

    assetsys = assetsys_create(0);
    filewatch = filewatch_create(assetsys, 0);

    // filewatch_mount(filewatch, "./source", "/data");
    filewatch_mount(filewatch, game_assets("gamedir/style_sheets").c_str(), "/style_sheets");
    filewatch_mount(filewatch, game_assets("gamedir/maps").c_str(), "/maps");
    filewatch_start_watching(filewatch, "/maps", watch_callback, 0);
}

void game_update() {

    //    if (is_hotfix_loaded) neko_hotload_module_update(ctx);

    neko_timed_action(250, {
        filewatch_update(filewatch);
        filewatch_notify(filewatch);
    });

    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());
    neko_vec2 mp = neko_platform_mouse_positionv();
    neko_vec2 mw = neko_platform_mouse_wheelv();
    neko_vec2 md = neko_platform_mouse_deltav();
    bool lock = neko_platform_mouse_locked();
    bool moved = neko_platform_mouse_moved();
    const f32 t = neko_platform_elapsed_time();
    struct neko_gui_context *nui_ctx = &g_gui.neko_gui_ctx;

    if (neko_platform_key_pressed(NEKO_KEYCODE_ESC)) {
        // neko_quit();

        g_cvar.show_editor ^= true;
    }

#if 0
    // 开屏动画
    if (t <= 2000.f) {

        u8 tranp = 255;

        if (t >= 1000) {
            tranp -= ((t - 1000) / 4);
        }

        neko_graphics_clear_action_t clear_action = {.color = {g_cvar.bg[0] / 255, g_cvar.bg[1] / 255, g_cvar.bg[2] / 255, 1.f}};
        neko_graphics_clear_desc_t clear = {.actions = &clear_action};
        neko_graphics_renderpass_begin(&g_cb, neko_renderpass_t{0});
        { neko_graphics_clear(&g_cb, &clear); }
        neko_graphics_renderpass_end(&g_cb);

        // Set up 2D camera for projection matrix
        neko_idraw_defaults(&g_idraw);
        neko_idraw_camera2D(&g_idraw, (u32)fbs.x, (u32)fbs.y);

        // 底层图片
        char background_text[64] = "Project: unknown";

        neko_vec2 td = neko_asset_ascii_font_text_dimensions(neko_idraw_default_font(), background_text, -1);
        neko_vec2 ts = neko_v2(512 + 128, 512 + 128);

        neko_idraw_text(&g_idraw, (fbs.x - td.x) * 0.5f, (fbs.y - td.y) * 0.5f + ts.y / 2.f - 100.f, background_text, NULL, false, 255, 255, 255, 255);
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

        return;
    }
#endif

    neko_ecs_run_systems(neko_ecs(), ECS_SYSTEM_UPDATE);

    // try {
    //     neko_lua_wrap_call(L, "test_update");
    // } catch (std::exception &ex) {
    //     neko_log_error("lua exception %s", ex.what());
    // }

    // Do rendering
    neko_graphics_clear_action_t clear_action = {.color = {g_cvar.bg[0] / 255, g_cvar.bg[1] / 255, g_cvar.bg[2] / 255, 1.f}};
    neko_graphics_clear_desc_t clear = {.actions = &clear_action};
    neko_graphics_renderpass_begin(&g_cb, neko_renderpass_t{0});
    { neko_graphics_clear(&g_cb, &clear); }
    neko_graphics_renderpass_end(&g_cb);

    // 设置投影矩阵的 2D 相机
    neko_idraw_defaults(&g_idraw);
    neko_idraw_camera2D(&g_idraw, (u32)fbs.x, (u32)fbs.y);

    // 底层图片
    char background_text[64] = "Project: unknown";

    neko_vec2 td = neko_asset_ascii_font_text_dimensions(neko_idraw_default_font(), background_text, -1);
    neko_vec2 ts = neko_v2(512 + 128, 512 + 128);

    neko_idraw_text(&g_idraw, (fbs.x - td.x) * 0.5f, (fbs.y - td.y) * 0.5f + ts.y / 2.f - 100.f, background_text, NULL, false, 255, 255, 255, 255);
    neko_idraw_texture(&g_idraw, g_test_ase);
    neko_idraw_rectvd(&g_idraw, neko_v2((fbs.x - ts.x) * 0.5f, (fbs.y - ts.y) * 0.5f - td.y - 50.f), ts, neko_v2(0.f, 1.f), neko_v2(1.f, 0.f), NEKO_COLOR_WHITE, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);

#pragma region AI

    if (g_cvar.hello_ai_shit) {

        neko_idraw_defaults(&g_idraw);
        neko_idraw_camera3D(&g_idraw, (u32)fbs.x, (u32)fbs.y);

        // AI
        ai_behavior_tree_frame(&bt);

        // Update/render scene
        neko_idraw_camera(&g_idraw, &camera, (u32)fbs.x, (u32)fbs.y);
        neko_idraw_depth_enabled(&g_idraw, true);

        // Render ground
        neko_idraw_rect3Dv(&g_idraw, neko_v3(-15.f, -0.5f, -15.f), neko_v3(15.f, -0.5f, 15.f), neko_v2s(0.f), neko_v2s(1.f), neko_color(50, 50, 50, 255), NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);

        // Render ai
        neko_idraw_push_matrix(&g_idraw, NEKO_IDRAW_MATRIX_MODELVIEW);
        neko_idraw_mul_matrix(&g_idraw, neko_vqs_to_mat4(&ai.xform));
        neko_idraw_box(&g_idraw, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 255, 255, 255, NEKO_GRAPHICS_PRIMITIVE_LINES);
        neko_idraw_line3Dv(&g_idraw, neko_v3s(0.f), NEKO_ZAXIS, NEKO_COLOR_BLUE);
        neko_idraw_line3Dv(&g_idraw, neko_v3s(0.f), NEKO_XAXIS, NEKO_COLOR_RED);
        neko_idraw_line3Dv(&g_idraw, neko_v3s(0.f), NEKO_YAXIS, NEKO_COLOR_GREEN);
        neko_idraw_pop_matrix(&g_idraw);

        // Render target
        neko_idraw_push_matrix(&g_idraw, NEKO_IDRAW_MATRIX_MODELVIEW);
        neko_idraw_translatev(&g_idraw, ai.target);
        neko_idraw_box(&g_idraw, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 255, 0, 255, NEKO_GRAPHICS_PRIMITIVE_LINES);
        neko_idraw_pop_matrix(&g_idraw);
    }

#pragma endregion

    neko_graphics_renderpass_begin(&g_cb, NEKO_GRAPHICS_RENDER_PASS_DEFAULT);
    {
        neko_graphics_set_viewport(&g_cb, 0, 0, (u32)fbs.x, (u32)fbs.y);
        neko_idraw_draw(&g_idraw, &g_cb);  // 立即模式绘制 idraw
    }
    neko_graphics_renderpass_end(&g_cb);

    // 设置投影矩阵的 2D 相机
    neko_idraw_defaults(&g_idraw);
    neko_idraw_camera2D(&g_idraw, (u32)fbs.x, (u32)fbs.y);

    neko_ecs_run_systems(neko_ecs(), ECS_SYSTEM_RENDER_IMMEDIATE);

    if (g_client_userdata.module_count) {
        if (g_client_userdata.module_func[0]) {
            g_client_userdata.module_func[0]();
        }
    }

    neko_ecs_run_systems(neko_ecs(), ECS_SYSTEM_RENDER_DEFERRED);

    call_count = 0;
    neko_sprite_t dragon_zombie = make_sprite(0, -250, -200, 5, neko_rad2deg(t / 100000.f), 0);
    neko_sprite_t night_spirit = make_sprite(1, -225, 100, 1, 0, 0);
    push_sprite(dragon_zombie);
    push_sprite(night_spirit);
    neko_sprite_t polish = make_sprite(1, 50, 180, 1, 0, 0);
    neko_sprite_t translated = polish;
    for (int i = 0; i < 4; ++i) {
        translated.x = polish.x + polish.sx * i;
        for (int j = 0; j < 6; ++j) {
            translated.y = polish.y - polish.sy * j;
            push_sprite(translated);
        }
    }

    // 运行cute_spriteBatch查找sprite批次
    // 这是cute_psriteBatch最基本的用法 每个游戏循环进行一次defrag tick flush
    // 也可以每N帧只使用一次defrag(碎片整理)
    // tick也可以在不同的时间间隔调用(例如每次游戏更新一次
    // 但不一定每次屏幕呈现一次
    spritebatch_defrag(&sb);
    spritebatch_tick(&sb);
    spritebatch_flush(&sb);
    sprite_verts_count = 0;

    draw_text(test_font_bmfont, "中文渲染测试 日本語レンダリングテスト Hello World! ", 50, 50, 1, 1.f, 800.f);

#pragma region gui

    neko_core_ui_begin(&g_core_ui, NULL);
    {
        editor_dockspace(&g_core_ui);

        if (g_cvar.show_info_window) {
            neko_core_ui_window_begin_ex(&g_core_ui, "Info", neko_core_ui_rect(100, 100, 500, 500), &g_cvar.show_info_window, NULL, NULL);
            {
                neko_core_ui_layout_row(&g_core_ui, 1, neko_core_ui_widths(-1), 0);
                {

                    static f32 delta, fps = neko_default_val();
                    static neko_memory_info_t meminfo = neko_default_val();

                    neko_timed_action(60, {
                        delta = neko_platform_delta_time();
                        fps = 1.f / delta;
                        meminfo = neko_platform_memory_info();
                    });

                    neko_core_ui_labelf("Delta: %.6lf", delta);
                    neko_core_ui_labelf("FPS: %.2lf", fps);

                    neko_core_ui_labelf("GC MemAllocInUsed: %.2lf mb", (f64)(neko_mem_bytes_inuse() / 1048576.0));
                    neko_core_ui_labelf("GC MemTotalAllocated: %.2lf mb", (f64)(g_allocation_metrics.total_allocated / 1048576.0));
                    neko_core_ui_labelf("GC MemTotalFree: %.2lf mb", (f64)(g_allocation_metrics.total_free / 1048576.0));

                    u64 max, used;
                    neko_script_gc_count(&max, &used);

                    neko_core_ui_labelf("NekoScript Alloc: %.2lf mb", ((f64)max / 1024.0f));
                    neko_core_ui_labelf("NekoScript UsedMemory: %.2lf mb", ((f64)used / 1024.0f));

                    // neko_core_ui_labelf("ImGui MemoryUsage: %.2lf mb", ((f64)__neko_core_ui_meminuse() / 1048576.0));

                    neko_core_ui_labelf("Virtual MemoryUsage: %.2lf mb", ((f64)meminfo.virtual_memory_used / 1048576.0));
                    neko_core_ui_labelf("Real MemoryUsage: %.2lf mb", ((f64)meminfo.physical_memory_used / 1048576.0));
                    neko_core_ui_labelf("Virtual MemoryUsage Peak: %.2lf mb", ((f64)meminfo.peak_virtual_memory_used / 1048576.0));
                    neko_core_ui_labelf("Real MemoryUsage Peak: %.2lf mb", ((f64)meminfo.peak_physical_memory_used / 1048576.0));

                    neko_core_ui_labelf("GPU MemTotalAvailable: %.2lf mb", (f64)(meminfo.gpu_total_memory / 1024.0f));
                    neko_core_ui_labelf("GPU MemCurrentUsage: %.2lf mb", (f64)(meminfo.gpu_memory_used / 1024.0f));

                    neko_graphics_info_t *info = &neko_subsystem(graphics)->info;

                    neko_core_ui_labelf("OpenGL vendor: %s", info->vendor);
                    neko_core_ui_labelf("OpenGL version supported: %s", info->version);

                    neko_vec2 opengl_ver = neko_platform_opengl_ver();
                    neko_core_ui_labelf("OpenGL version: %d.%d", (s32)opengl_ver.x, (s32)opengl_ver.y);
                }
            }
            neko_core_ui_window_end(&g_core_ui);
        }

        if (g_cvar.show_gui) {

            neko_core_ui_demo_window(&g_core_ui, neko_core_ui_rect(100, 100, 500, 500), NULL);
            neko_core_ui_style_editor(&g_core_ui, NULL, neko_core_ui_rect(350, 250, 300, 240), NULL);

            const neko_vec2 ws = neko_v2(600.f, 300.f);

#pragma region gui_ss

#if 0

            const neko_core_ui_style_sheet_t *ss = &style_sheet;

            const neko_vec2 ss_ws = neko_v2(500.f, 300.f);
            neko_core_ui_window_begin(&g_gui, "Window", neko_core_ui_rect((fbs.x - ss_ws.x) * 0.5f, (fbs.y - ss_ws.y) * 0.5f, ss_ws.x, ss_ws.y));
            {
                // Cache the current container
                neko_core_ui_container_t *cnt = neko_core_ui_get_current_container(&g_gui);

                neko_core_ui_layout_row(&g_gui, 2, neko_core_ui_widths(200, 0), 0);

                neko_core_ui_text(&g_gui, "A regular element button.");
                neko_core_ui_button(&g_gui, "button");

                neko_core_ui_text(&g_gui, "A regular element label.");
                neko_core_ui_label(&g_gui, "label");

                neko_core_ui_text(&g_gui, "Button with classes: {.c0 .btn}");

                neko_core_ui_selector_desc_t selector_1 = {.classes = {"c0", "btn"}};
                neko_core_ui_button_ex(&g_gui, "hello?##btn", &selector_1, 0x00);

                neko_core_ui_text(&g_gui, "Label with id #lbl and class .c0");
                neko_core_ui_selector_desc_t selector_2 = {.id = "lbl", .classes = {"c0"}};
                neko_core_ui_label_ex(&g_gui, "label##lbl", &selector_2, 0x00);

                const f32 m = cnt->body.w * 0.3f;
                // neko_core_ui_layout_row(gui, 2, (int[]){m, -m}, 0);
                // neko_core_ui_layout_next(gui); // Empty space at beginning
                neko_core_ui_layout_row(&g_gui, 1, neko_core_ui_widths(0), 0);
                neko_core_ui_selector_desc_t selector_3 = {.classes = {"reload_btn"}};
                if (neko_core_ui_button_ex(&g_gui, "reload style sheet", &selector_3, 0x00)) {
                    app_load_style_sheet(true);
                }

                button_custom(&g_gui, "Hello?");
            }
            neko_core_ui_window_end(&g_gui);

#endif

#pragma endregion

#pragma region gui_idraw

            neko_core_ui_window_begin(&g_core_ui, "Idraw", neko_core_ui_rect((fbs.x - ws.x) * 0.2f, (fbs.y - ws.y) * 0.5f, ws.x, ws.y));
            {
                // Cache the current container
                neko_core_ui_container_t *cnt = neko_core_ui_get_current_container(&g_core_ui);

                // 绘制到当前窗口中的转换对象的自定义回调
                // 在这里，我们将容器的主体作为视口传递，但这可以是您想要的任何内容，
                // 正如我们将在“分割”窗口示例中看到的那样。
                // 我们还传递自定义数据（颜色），以便我们可以在回调中使用它。如果以下情况，这可以为 NULL
                // 你不需要任何东西
                neko_color_t color = neko_color_alpha(NEKO_COLOR_RED, (uint8_t)neko_clamp((sin(t * 0.001f) * 0.5f + 0.5f) * 255, 0, 255));
                neko_core_ui_draw_custom(&g_core_ui, cnt->body, gui_cb, &color, sizeof(color));
            }
            neko_core_ui_window_end(&g_core_ui);

#pragma endregion

#pragma region gui_ai

            neko_core_ui_window_begin(&g_core_ui, "AI", neko_core_ui_rect(10, 10, 350, 220));
            {
                neko_core_ui_layout_row(&g_core_ui, 1, neko_core_ui_widths(-1), 100);
                neko_core_ui_text(&g_core_ui,
                                  " * The AI will continue to move towards a random location as long as its health is not lower than 50.\n\n"
                                  " * If health drops below 50, the AI will pause to heal back up to 100 then continue moving towards its target.\n\n"
                                  " * After it reaches its target, it will find another random location to move towards.");

                neko_core_ui_layout_row(&g_core_ui, 1, neko_core_ui_widths(-1), 0);
                neko_core_ui_label(&g_core_ui, "state: %s", ai.state == AI_STATE_HEAL ? "HEAL" : "MOVE");
                neko_core_ui_layout_row(&g_core_ui, 2, neko_core_ui_widths(55, 50), 0);
                neko_core_ui_label(&g_core_ui, "health: ");
                neko_core_ui_number(&g_core_ui, &ai.health, 0.1f);
            }
            neko_core_ui_window_end(&g_core_ui);

#pragma endregion
        }

        if (neko_platform_key_pressed(NEKO_KEYCODE_GRAVE_ACCENT)) {
            console.open = !console.open;
        } else if (neko_platform_key_pressed(NEKO_KEYCODE_TAB) && console.open) {
            console.autoscroll = !console.autoscroll;
        }

        neko_core_ui_layout_t l;
        if (window && neko_core_ui_window_begin(&g_core_ui, "App", neko_core_ui_rect(fbs.x - 210, 30, 200, 200))) {
            l = *neko_core_ui_get_layout(&g_core_ui);
            neko_core_ui_layout_row(&g_core_ui, 1, neko_core_ui_widths(-1), 0);

            neko_core_ui_text(&g_core_ui, "Hello neko");

            // neko_core_ui_text_fc(&g_gui, "喵喵昂~");

            // neko_core_ui_layout_row(&g_gui, 1, neko_core_ui_widths(-1), 0);

            neko_core_ui_labelf("Position: <%.2f %.2f>", mp.x, mp.y);
            neko_core_ui_labelf("Wheel: <%.2f %.2f>", mw.x, mw.y);
            neko_core_ui_labelf("Delta: <%.2f %.2f>", md.x, md.y);
            neko_core_ui_labelf("Lock: %zu", lock);
            neko_core_ui_labelf("Moved: %zu", moved);
            neko_core_ui_labelf("Hover: %zu", g_gui.mouse_is_hover);
            neko_core_ui_labelf("Time: %f", t);

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

            neko_core_ui_layout_row(&g_core_ui, 7, neko_core_ui_widths(100, 100, 32, 100, 32, 100, 32), 0);
            for (u32 i = 0; btns[i].str; ++i) {
                neko_core_ui_labelf("%s: ", btns[i].str);
                neko_core_ui_labelf("pressed: ");
                neko_core_ui_labelf("%d", mouse_pressed[btns[i].val]);
                neko_core_ui_labelf("down: ");
                neko_core_ui_labelf("%d", mouse_down[btns[i].val]);
                neko_core_ui_labelf("released: ");
                neko_core_ui_labelf("%d", mouse_released[btns[i].val]);
            }

            neko_core_ui_window_end(&g_core_ui);
        }

        int s = summons;
        while (s--) {
            neko_core_ui_push_id(&g_core_ui, &s, sizeof(s));
            if (neko_core_ui_window_begin(&g_core_ui, "Summon", neko_core_ui_rect(100, 100, 200, 200))) {
                neko_core_ui_layout_row(&g_core_ui, 1, neko_core_ui_widths(-1), 0);
                neko_core_ui_text(&g_core_ui, "new window");
                neko_core_ui_window_end(&g_core_ui);
            }
            neko_core_ui_pop_id(&g_core_ui);
        }

        neko_vec2 fb = (&g_core_ui)->framebuffer_size;
        neko_core_ui_rect_t screen;
        if (embeded)
            screen = l.body;
        else
            screen = neko_core_ui_rect(0, 0, fb.x, fb.y);
        neko_console(&console, &g_core_ui, screen, NULL);
    }
    neko_core_ui_end(&g_core_ui, !g_gui.mouse_is_hover);

    neko_imgui_new_frame(&g_imgui);

    if (g_cvar.show_demo_window) {
        ImGui::ShowDemoWindow();
    }

    if (g_cvar.show_editor) {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("engine")) {
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    if (g_cvar.show_profiler_window) {
        static profiler_frame frame_data;
        neko_profiler_get_frame(&frame_data);
        // // if (g_multi) ProfilerDrawFrameNavigation(g_frameInfos.data(), g_frameInfos.size());
        static char buffer[10 * 1024];
        profiler_draw_frame(&frame_data, buffer, 10 * 1024);
        // ProfilerDrawStats(&data);
    }

#pragma endregion

    // Immediate rendering for back buffer
    // neko_idraw_defaults(&g_idraw);
    // neko_idraw_camera3D(&g_idraw, (uint32_t)fbs.x, (uint32_t)fbs.y);
    // neko_idraw_depth_enabled(&g_idraw, true);
    // neko_idraw_face_cull_enabled(&g_idraw, true);
    // neko_idraw_translatef(&g_idraw, 0.f, 0.f, -1.f);
    // neko_idraw_texture(&g_idraw, rt);
    // neko_idraw_rotatev(&g_idraw, neko_platform_elapsed_time() * 0.0001f, NEKO_YAXIS);
    // neko_idraw_rotatev(&g_idraw, neko_platform_elapsed_time() * 0.0002f, NEKO_XAXIS);
    // neko_idraw_rotatev(&g_idraw, neko_platform_elapsed_time() * 0.0003f, NEKO_ZAXIS);
    // neko_idraw_box(&g_idraw, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 255, 255, 255, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);

    neko_graphics_renderpass_begin(&g_cb, NEKO_GRAPHICS_RENDER_PASS_DEFAULT);
    {
        neko_graphics_set_viewport(&g_cb, 0, 0, (u32)fbs.x, (u32)fbs.y);
        neko_idraw_draw(&g_idraw, &g_cb);  // 立即模式绘制 idraw
    }
    neko_graphics_renderpass_end(&g_cb);

    neko_graphics_renderpass_begin(&g_cb, NEKO_GRAPHICS_RENDER_PASS_DEFAULT);
    {
        neko_graphics_set_viewport(&g_cb, 0, 0, (u32)fbs.x, (u32)fbs.y);
        neko_graphics_draw_batch(&g_cb, sprite_batch, 0, 0, 0);
        neko_graphics_draw_batch(&g_cb, font_render, 0, 0, 0);
    }
    neko_graphics_renderpass_end(&g_cb);

    neko_gui_new_frame(&g_gui);

    neko_ecs_run_systems(neko_ecs(), ECS_SYSTEM_EDITOR);

    if (g_cvar.show_demo_window) neko_gui_overview(nui_ctx);

    auto pack_editor_func = [nui_ctx]() {
        if (neko_gui_begin(nui_ctx, "PackEditor", neko_gui_layout_get_bounds_ex(&g_gui, "PackEditor", neko_gui_rect(400, 200, 450, 400)),
                           NEKO_GUI_WINDOW_BORDER | NEKO_GUI_WINDOW_MOVABLE | NEKO_GUI_WINDOW_SCALABLE | NEKO_GUI_WINDOW_MINIMIZABLE | NEKO_GUI_WINDOW_TITLE)) {

            // pack editor
            neko_private(neko_packreader_t) pack_reader;
            neko_private(neko_pack_result) result;
            neko_private(bool) pack_reader_is_loaded = false;
            neko_private(u8) pack_version;
            neko_private(bool) isLittleEndian;
            neko_private(u64) itemCount;

            neko_private(neko_string) file = game_assets("gamedir/res.pack");
            neko_private(bool) filebrowser = false;

            neko_gui_layout_row_static(nui_ctx, 30, 400, 1);

            if (neko_gui_button_label(nui_ctx, "Open") && !pack_reader_is_loaded) filebrowser = true;

            if (neko_gui_button_label(nui_ctx, "Close")) {
                neko_pack_destroy(&pack_reader);
                pack_reader_is_loaded = false;
                filebrowser = true;
            }

            if (filebrowser) {
                // imgui::file_browser(file);

                if (!file.empty() && !std::filesystem::is_directory(file)) {

                    if (pack_reader_is_loaded) {

                    } else {
                        result = neko_pack_info(file.c_str(), &pack_version, &isLittleEndian, &itemCount);
                        if (result != 0) return;

                        result = neko_pack_read(file.c_str(), 0, false, &pack_reader);
                        if (result != 0) return;

                        if (result == 0) itemCount = neko_pack_item_count(&pack_reader);

                        pack_reader_is_loaded = true;
                    }

                    // 复位变量
                    filebrowser = false;
                    file = game_assets("gamedir");
                }
            }

            if (pack_reader_is_loaded && result == 0) {

                static s32 item_current_idx = -1;

                neko_gui_labelf_wrap(nui_ctx, "version: %d", pack_version);
                neko_gui_labelf_wrap(nui_ctx, "little endian mode: %s", neko_bool_str(isLittleEndian));
                neko_gui_labelf_wrap(nui_ctx, "file counts: %llu", (long long unsigned int)itemCount);

                neko_gui_layout_row_dynamic(nui_ctx, 30, 1);

                for (u64 i = 0; i < itemCount; ++i) {
                    neko_gui_labelf_wrap(nui_ctx, "%llu  %s %u", (long long unsigned int)i, neko_pack_item_path(&pack_reader, i), neko_pack_item_size(&pack_reader, i));

                    if (neko_gui_button_label(nui_ctx, "check")) {
                        item_current_idx = i;
                    }
                }

                neko_gui_layout_row_dynamic(nui_ctx, 30, 1);

                if (item_current_idx >= 0) {
                    neko_gui_labelf_wrap(nui_ctx, "File index: %u", item_current_idx);
                    neko_gui_labelf_wrap(nui_ctx, "Path within package: %s", neko_pack_item_path(&pack_reader, item_current_idx));
                    neko_gui_labelf_wrap(nui_ctx, "File size: %u", neko_pack_item_size(&pack_reader, item_current_idx));
                }

            } else if (!neko_pack_check(result)) {
            }
        }
        neko_gui_end(nui_ctx);
    };

    if (g_cvar.show_pack_editor) pack_editor_func();

    if (neko_gui_begin(nui_ctx, "Hello Neko", neko_gui_layout_get_bounds_ex(&g_gui, "Hello Neko", neko_gui_rect(1050, 200, 350, 700)),
                       NEKO_GUI_WINDOW_BORDER | NEKO_GUI_WINDOW_MOVABLE | NEKO_GUI_WINDOW_SCALABLE | NEKO_GUI_WINDOW_MINIMIZABLE | NEKO_GUI_WINDOW_TITLE)) {

        enum { EASY, HARD };
        static int op = EASY;
        static int property = 20;
        static std::string str = "Hello shit";

        static std::vector<int> vec = {op, property};
        static std::map<std::string, int> mp = {{"AA", op}, {"BB", property}};
        static std::pair<std::string, int> pp = {"CC", property};
        static std::string *p_str = NULL;

        neko_gui_layout_row_static(nui_ctx, 30, 125, 1);

        try {
            neko_cvar_gui();
        } catch (const std::exception &ex) {
            neko_log_error("cvar exception %s", ex.what());
        }

        neko_gui::gui_auto(property, "test_auto_1");
        neko_gui::gui_auto(vec, "test_auto_2");
        neko_gui::gui_auto(str, "test_auto_3");
        neko_gui::gui_auto(mp, "test_auto_4");
        neko_gui::gui_auto(pp, "test_auto_5");
        neko_gui::gui_auto(p_str, "test_auto_6");

        neko_gui_layout_row_dynamic(nui_ctx, 300, 1);
        if (neko_gui_group_begin(nui_ctx, "ECS", NEKO_GUI_WINDOW_TITLE | NEKO_GUI_WINDOW_BORDER | NEKO_GUI_WINDOW_SCROLL_AUTO_HIDE)) {

            neko_gui_layout_row_static(nui_ctx, 18, 125, 1);

            for (neko_ecs_ent_view v = neko_ecs_ent_view_single(neko_ecs(), COMPONENT_GAMEOBJECT); neko_ecs_ent_view_valid(&v); neko_ecs_ent_view_next(&v)) {
                CGameObject *gameobj = (CGameObject *)neko_ecs_ent_get_component(neko_ecs(), v.ent, COMPONENT_GAMEOBJECT);
                // neko_println("%d %d %d %d %s", v.i, v.ent, v.valid, v.view_type, gameobj->name);

                neko_gui_selectable_label(nui_ctx, gameobj->name, NEKO_GUI_TEXT_CENTERED, (neko_gui_bool *)&gameobj->selected);
            }

            neko_gui_group_end(nui_ctx);
        }

        neko_gui_layout_row_dynamic(nui_ctx, 300, 1);
        if (neko_gui_group_begin(nui_ctx, "CVars", NEKO_GUI_WINDOW_TITLE | NEKO_GUI_WINDOW_BORDER | NEKO_GUI_WINDOW_SCROLL_AUTO_HIDE)) {

            neko_gui_layout_row_static(nui_ctx, 30, 150, 2);

            if (neko_gui_button_label(nui_ctx, "test_fnt")) {
                FILE *file = fopen("1.fnt", "rb");
                neko_fnt *fnt = neko_fnt_read(file);
                fclose(file);
                if (NULL != fnt && fnt->num_glyphs > 0) {
                    neko_println("Got FNT! %i glyphs", fnt->num_glyphs);
                    // printf("First glyph is '%c' in page '%s'.\n", fnt->glyphs[0].ch, fnt->page_names[fnt->glyphs[0].page]);

                    for (int i = 0; i < fnt->num_glyphs; i += 10) {
                        neko_println("glyph '%u' in '%s'", fnt->glyphs[i].ch, fnt->page_names[fnt->glyphs[i].page]);
                    }
                }
                neko_fnt_free(fnt);
            }
            if (neko_gui_button_label(nui_ctx, "test_xml")) test_xml(game_assets("gamedir/tests/test.xml"));
            if (neko_gui_button_label(nui_ctx, "test_se")) test_se();
            if (neko_gui_button_label(nui_ctx, "test_sr")) test_sr();
            if (neko_gui_button_label(nui_ctx, "test_ut")) test_ut();
            if (neko_gui_button_label(nui_ctx, "test_backtrace")) __neko_inter_stacktrace();
            if (neko_gui_button_label(nui_ctx, "test_containers")) test_containers();
            if (neko_gui_button_label(nui_ctx, "test_sexpr")) test_sexpr();
            if (neko_gui_button_label(nui_ctx, "test_nbt")) test_nbt();
            if (neko_gui_button_label(nui_ctx, "test_sc")) {
                neko_timer_do(t, neko_println("%llu", t), { test_sc(); });
            }
            if (neko_gui_button_label(nui_ctx, "test_ttf")) {
                neko_timer_do(t, neko_println("%llu", t), { test_ttf(); });
            }
            if (neko_gui_button_label(nui_ctx, "test_hotload")) {

                std::string command = "build_hotload.bat";

                //                FILE *pipe = _popen(command.c_str(), "r");
                //                if (!pipe) {
                //                    neko_log_error("%s", "Unable to create pipeline");
                //                } else {
                //                    char buffer[128];
                //                    while (!feof(pipe)) {
                //                        if (fgets(buffer, 128, pipe) != nullptr) {
                //                            std::cout << buffer;
                //                        }
                //                    }
                //                    _pclose(pipe);
                //                }
            }
            if (neko_gui_button_label(nui_ctx, "test_ecs_view")) {
                for (neko_ecs_ent_view v = neko_ecs_ent_view_single(neko_ecs(), COMPONENT_GAMEOBJECT); neko_ecs_ent_view_valid(&v); neko_ecs_ent_view_next(&v)) {
                    CGameObject *gameobj = (CGameObject *)neko_ecs_ent_get_component(neko_ecs(), v.ent, COMPONENT_GAMEOBJECT);
                    neko_println("%d %d %d %d %s", v.i, v.ent, v.valid, v.view_type, gameobj->name);
                }
            }
            if (neko_gui_button_label(nui_ctx, "test_ecs_cpp")) {
                forEachComponentType([&]<typename T>() {
                    // 打印类型名称 它是一个更大字符串的 std::string_view 因此我们使用“%.*s”
                    // `std::printf` 的形式
                    constexpr auto typeName = getTypeName<T>();
                    std::printf("component type: %.*s\n", int(typeName.size()), typeName.data());

                    // 设置组件的默认值并打印其 props
                    T val{};
                    std::printf("  props:\n");
                    forEachProp(val, [&](auto propTag, auto &propVal) {
                        // `propTag` 每个 prop 都有不同的类型 并保存包括名称在内的属性 我们打印
                        // 首先是名称。名称是 std::string_view  但它指向的整个字符串是
                        // 名称 以便我们可以直接打印其数据
                        std::printf("    %s: ", propTag.attribs.name.data());

                        // 调用上面重载的print函数之一来打印值本身
                        print(propVal);

                        // 现在我们打印是否设置了`exampleFlag`请注意 我们可以检查属性`constexpr`
                        if (propTag.attribs.exampleFlag) {
                            std::printf(" (example flag set)");
                        }

                        std::printf("\n");
                    });
                });
            }

            neko_gui_group_end(nui_ctx);
        }

        neko_gui_layout_row_dynamic(nui_ctx, 30, 2);
        if (neko_gui_option_label(nui_ctx, "easy", op == EASY)) op = EASY;
        if (neko_gui_option_label(nui_ctx, "hard", op == HARD)) op = HARD;

        neko_gui_layout_row_dynamic(nui_ctx, 25, 1);
        neko_gui_property_int(nui_ctx, "Compression:", 0, &property, 100, 10, 1);

        auto custom_draw_widget = [](struct neko_gui_context *ctx) {
            struct neko_gui_command_buffer *canvas;
            struct neko_gui_input *input = &ctx->input;
            canvas = neko_gui_window_get_canvas(ctx);

            struct neko_gui_rect space;
            enum neko_gui_widget_layout_states state;
            state = neko_gui_widget(&space, ctx);
            if (!state) return;

            if (state != NEKO_GUI_WIDGET_ROM) {
                //...
            }
            // neko_gui_fill_rect(canvas, space, 0, neko_gui_rgb(144, 0, 144));

            neko_gui_draw_text(canvas, space, "hhhhh", 5, ctx->style.font, neko_gui_rgb(0, 0, 0), neko_gui_rgb(255, 0, 0));
        };

        custom_draw_widget(nui_ctx);

        // neko_gui_layout_row_dynamic(ctx, 20, 1);
        // neko_gui_label(ctx, "background:", NEKO_GUI_TEXT_LEFT);
        // neko_gui_layout_row_dynamic(ctx, 25, 1);
        //  if (neko_gui_combo_begin_color(ctx, neko_gui_rgb_cf(bg), neko_gui_vec2(neko_gui_widget_width(ctx), 400))) {
        //      neko_gui_layout_row_dynamic(ctx, 120, 1);
        //      bg = neko_gui_color_picker(ctx, bg, NEKO_GUI_RGBA);
        //      neko_gui_layout_row_dynamic(ctx, 25, 1);
        //      bg.r = neko_gui_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f, 0.005f);
        //      bg.g = neko_gui_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f, 0.005f);
        //      bg.b = neko_gui_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f, 0.005f);
        //      bg.a = neko_gui_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f, 0.005f);
        //      neko_gui_combo_end(ctx);
        //  }
    }
    neko_gui_end(nui_ctx);

    g_gui.mouse_is_hover = neko_gui_window_is_any_hovered(nui_ctx) || neko_gui_item_is_any_active(nui_ctx);

    // Render neko_nui commands into graphics command buffer
    neko_gui_render(&g_gui, &g_cb, NEKO_GUI_ANTI_ALIASING_ON);

    neko_imgui_render(&g_imgui, &g_cb);

    neko_graphics_renderpass_begin(&g_cb, NEKO_GRAPHICS_RENDER_PASS_DEFAULT);
    {
        neko_graphics_set_viewport(&g_cb, 0, 0, (u32)fbs.x, (u32)fbs.y);
        neko_core_ui_render(&g_core_ui, &g_cb);
    }
    neko_graphics_renderpass_end(&g_cb);

    // Submits to cb
    neko_graphics_command_buffer_submit(&g_cb);

    NEKO_GL_CUSTOM_PRINT_GL_ERRORS();
}

void game_shutdown() {
    // try {
    //     neko_lua_wrap_call(L, "game_shutdown");
    // } catch (std::exception &ex) {
    //     neko_log_error("lua exception %s", ex.what());
    // }

    g_csharp.shutdown();

    spritebatch_term(&sb);
    unload_images();

    neko_graphics_custom_batch_free(font_render);
    neko_graphics_custom_batch_free(sprite_batch);

    destroy_texture_handle(test_font_tex_id, NULL);

    // gl_free_frame_buffer(&sb_fb);

    neko_gui_layout_save(&g_gui);

    neko_dyn_array_free(test_witch_spr.frames);

    //    if (is_hotfix_loaded) neko_hotload_module_close(ctx);

    neko_script_gc_freeall();
    for (int i = 0; i < neko_script_vector_size(g_vm.modules); i++) {
        neko_script_binary_free(g_vm.modules[i]);
    }
    neko_script_vector_free(g_vm.modules);

    neko_core_ui_free(&g_core_ui);

    neko_pack_destroy(&g_pack);

    filewatch_stop_watching(filewatch, "/gamedir");

    filewatch_free(filewatch);
    assetsys_destroy(assetsys);
}

neko_game_desc_t neko_main(s32 argc, char **argv) {

    // 确定运行路径
    auto current_dir = std::filesystem::path(std::filesystem::current_path());
    for (int i = 0; i < 6; ++i)
        if (std::filesystem::exists(current_dir / "gamedir") && std::filesystem::exists(current_dir / "gamedir" / "scripts")) {
            data_path = neko_fs_normalize_path(current_dir.string());
            neko_log_info(std::format("gamedir detected: {0} (base: {1})", data_path, std::filesystem::current_path().string()).c_str());
            break;
        } else {
            current_dir = current_dir.parent_path();
        }

    return neko_game_desc_t{
            .init = game_init, .update = game_update, .shutdown = game_shutdown, .window = {.width = 1280, .height = 740, .vsync = true}, .argc = argc, .argv = argv, .console = &console};
}

void editor_dockspace(neko_core_ui_context_t *ctx) {
    s32 opt = NEKO_CORE_UI_OPT_NOCLIP | NEKO_CORE_UI_OPT_NOFRAME | NEKO_CORE_UI_OPT_FORCESETRECT | NEKO_CORE_UI_OPT_NOTITLE | NEKO_CORE_UI_OPT_DOCKSPACE | NEKO_CORE_UI_OPT_FULLSCREEN |
              NEKO_CORE_UI_OPT_NOMOVE | NEKO_CORE_UI_OPT_NOBRINGTOFRONT | NEKO_CORE_UI_OPT_NOFOCUS | NEKO_CORE_UI_OPT_NORESIZE;
    neko_core_ui_window_begin_ex(ctx, "Dockspace", neko_core_ui_rect(350, 40, 600, 500), NULL, NULL, opt);
    {
        // Editor dockspace
    }
    neko_core_ui_window_end(ctx);
}
