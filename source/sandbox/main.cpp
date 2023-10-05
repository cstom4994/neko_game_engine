

#include <filesystem>
#include <format>

// engine
#include "engine/neko.h"
#include "engine/neko_component.h"
#include "engine/neko_ecs.h"
#include "engine/neko_platform.h"
#include "engine/util/neko_ai.h"
#include "engine/util/neko_asset.h"
#include "engine/util/neko_gfxt.h"
#include "engine/util/neko_gui.h"
#include "engine/util/neko_idraw.h"
#include "engine/util/neko_lua.h"
#include "engine/util/neko_tiled.h"

// game
#include "neko_client_ecs.h"
#include "neko_hash.h"
#include "neko_scripting.h"
#include "neko_sprite.h"

// hpp
#include "hpp/neko_static_refl.hpp"
#include "hpp/neko_struct.hpp"

#define NEKO_IMGUI_IMPL
#include "neko_imgui.h"
#include "neko_imgui_utils.hpp"

#define NEKO_CONSOLE_IMPL
#include "neko_console.h"

// opengl
#include "libs/glad/glad.h"

neko_command_buffer_t g_cb = {0};
neko_gui_context_t g_gui = {0};
neko_imgui_context_t g_imgui = {};
neko_immediate_draw_t g_idraw = {0};
neko_client_ecs_userdata_t g_client_ecs_userdata = {};
neko_asset_font_t g_font;
neko_gui_style_sheet_t style_sheet;

neko_gfxt_pipeline_t pip;
neko_gfxt_material_t mat;
neko_gfxt_mesh_t mesh;
neko_gfxt_texture_t texture;

neko_asset_manager_t g_am = {0};

neko_asset_t tex_hndl = {0};

lua_State *L = nullptr;

neko_string data_path = {};

neko_global neko_sprite test_witch_spr = {0};

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
    bool show_demo_window = false;
    bool show_info_window = false;
    bool show_cvar_window = false;

    bool show_gui = false;

    bool hello_duck = false;

    f32 bg[3] = {90, 95, 100};
};

neko_struct(neko_engine_cvar_t, _Fs(show_demo_window, "Is show imgui demo"), _Fs(show_info_window, "Is show info window"), _Fs(show_cvar_window, "cvar inspector"), _Fs(show_gui, "neko gui"),
            _Fs(hello_duck, "Test gltf"), _Fs(bg, "bg color"));

neko_engine_cvar_t g_cvar = {};

void dockspace(neko_gui_context_t *ctx);

// test
void test_xml(const neko_string &file);
void test_sr();
void test_ut();
void test_se();
void test_containers();

NEKO_API_DECL void test_sexpr();

neko_texture_t load_ase_texture_simple(const std::string &path) {

    ase_t *ase = neko_aseprite_load_from_file(path.c_str());
    neko_defer([&] { neko_aseprite_free(ase); });

    if (NULL == ase) {
        neko_log_error("unable to load ase %s", path);
        return neko_default_val();
    }

    neko_assert(ase->frame_count == 1, "load_ase_texture_simple used to load simple aseprite");

    neko_graphics_t *gfx = neko_instance()->ctx.graphics;

    neko_log_info(std::format("load aseprite\n - frame_count {0}\n - palette.entry_count{1}\n - w={2} h={3}", ase->frame_count, ase->palette.entry_count, ase->w, ase->h).c_str());

    s32 bpp = 4;

    neko_graphics_texture_desc_t t_desc = {};

    t_desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    t_desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.num_mips = 0;
    t_desc.width = ase->w;
    t_desc.height = ase->h;
    // t_desc.num_comps = 4;
    t_desc.data[0] = ase->frames->pixels;

    neko_tex_flip_vertically(ase->w, ase->h, (u8 *)t_desc.data);

    neko_texture_t tex = neko_graphics_texture_create(&t_desc);

    return tex;
}

neko_string __neko_platform_get_path(const neko_string &path) {

    if (data_path.empty()) {
        neko_assert(!data_path.empty(), "gamepath not detected");
        return path;
    }

    neko_string get_path(data_path);

    switch (neko::hash(path.substr(0, 4))) {
        case neko::hash("data"):
            get_path.append(path);
            break;
        default:
            break;
    }

    return get_path;
}

void app_load_style_sheet(bool destroy) {
    if (destroy) {
        neko_gui_style_sheet_destroy(&style_sheet);
    }
    style_sheet = neko_gui_style_sheet_load_from_file(&g_gui, __neko_platform_get_path("data/style_sheets/gui.ss").c_str());
    neko_gui_set_style_sheet(&g_gui, &style_sheet);
}

s32 button_custom(neko_gui_context_t *ctx, const char *label) {
    // Do original button call
    s32 res = neko_gui_button(ctx, label);

    // Draw inner shadows/highlights over button
    neko_color_t hc = NEKO_COLOR_WHITE, sc = neko_color(85, 85, 85, 255);
    neko_gui_rect_t r = ctx->last_rect;
    s32 w = 2;
    neko_gui_draw_rect(ctx, neko_gui_rect(r.x + w, r.y, r.w - 2 * w, w), hc);
    neko_gui_draw_rect(ctx, neko_gui_rect(r.x + w, r.y + r.h - w, r.w - 2 * w, w), sc);
    neko_gui_draw_rect(ctx, neko_gui_rect(r.x, r.y, w, r.h), hc);
    neko_gui_draw_rect(ctx, neko_gui_rect(r.x + r.w - w, r.y, w, r.h), sc);

    return res;
}

// Custom callback for immediate drawing directly into the gui window
void gui_cb(neko_gui_context_t *ctx, struct neko_gui_customcommand_t *cmd) {
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

static int window = 1, embeded, summons;

static void toggle_window(int argc, char **argv);
static void toggle_embedded(int argc, char **argv);
static void help(int argc, char **argv);
static void echo(int argc, char **argv);
static void spam(int argc, char **argv);
void summon(int argc, char **argv);
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
};

neko_console_t console = {
        .tb = "",
        .cb = "",
        .size = 0.4,
        .open_speed = 0.2,
        .close_speed = 0.3,
        .autoscroll = true,
        .commands = commands,
        .commands_len = neko_array_size(commands),
};

void sz(int argc, char **argv) {
    if (argc != 2) {
        neko_console_printf(&console, "[sz]: needs 1 argument!\n");
        return;
    }
    float sz = atof(argv[1]);
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

void spam(int argc, char **argv) {
    if (argc != 3) goto spam_invalid_command;
    int count = atoi(argv[2]);
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

void help(int argc, char **argv) {
    for (int i = 0; i < neko_array_size(commands); i++) {
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

        auto ff = [&]<typename T>(const char *name, auto &var, T &t) {
            if constexpr (std::is_same_v<std::decay_t<decltype(var)>, T>) {
                neko::imgui::Auto(var, name);
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
                    neko::imgui::Auto((&neko_cv()->cvars[i])->value.s, (&neko_cv()->cvars[i])->name);
                    break;
                case __NEKO_CONFIG_TYPE_FLOAT:
                    neko::imgui::Auto((&neko_cv()->cvars[i])->value.f, (&neko_cv()->cvars[i])->name);
                    break;
                case __NEKO_CONFIG_TYPE_INT:
                    neko::imgui::Auto((&neko_cv()->cvars[i])->value.i, (&neko_cv()->cvars[i])->name);
                    break;
            };
        };
    }
}

// Tiled
map_t map;

void game_init() {
    g_cb = neko_command_buffer_new();
    g_idraw = neko_immediate_draw_new();

    neko_graphics_info_t *info = neko_graphics_info();
    if (!info->compute.available) {
        neko_log_error("Compute shaders not available.");
        return;
    }

    g_am = neko_asset_manager_new();

    g_client_ecs_userdata.cb = &g_cb;
    g_client_ecs_userdata.idraw = &g_idraw;

    neko_ecs()->user_data = &g_client_ecs_userdata;

    L = neko_scripting_init();

    {
        neko_lua_wrap_register_t<>(L).def(
                +[](const_str path) -> neko_string { return __neko_platform_get_path(path); }, "neko_file_path");

        lua_newtable(L);
        for (int n = 0; n < neko_instance()->ctx.game.argc; ++n) {
            lua_pushstring(L, neko_instance()->ctx.game.argv[n]);
            lua_rawseti(L, -2, n);
        }
        lua_setglobal(L, "arg");

        neko_lua_wrap_do_file(L, __neko_platform_get_path("data/scripts/main.lua"));

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
        neko_log_error("%s", ex.what());
    }

    neko_asset_texture_t tex0 = {0};
    neko_asset_texture_load_from_file(__neko_platform_get_path("data/assets/textures/yzh.jpg").c_str(), &tex0, NULL, false, false);
    tex_hndl = neko_assets_create_asset(&g_am, neko_asset_texture_t, &tex0);

    // Load pipeline from resource file
    pip = neko_gfxt_pipeline_load_from_file(__neko_platform_get_path("data/assets/pipelines/simple.sf").c_str());

    // Create material using this pipeline
    neko_gfxt_material_desc_t mat_decl = {.pip_func = {.hndl = &pip}};
    mat = neko_gfxt_material_create(&mat_decl);

    // Create mesh that uses the layout from the pipeline's requested mesh layout
    neko_gfxt_mesh_import_options_t mesh_decl = {
            .layout = pip.mesh_layout, .size = neko_dyn_array_size(pip.mesh_layout) * sizeof(neko_gfxt_mesh_layout_t), .index_buffer_element_size = pip.desc.raster.index_buffer_element_size};
    mesh = neko_gfxt_mesh_load_from_file(__neko_platform_get_path("data/assets/meshes/Duck.gltf").c_str(), &mesh_decl);

    texture = neko_gfxt_texture_load_from_file(__neko_platform_get_path("data/assets/textures/DuckCM.png").c_str(), NULL, false, false);

    neko_gui_init(&g_gui, neko_platform_main_window());

    // Load in custom font file and then initialize gui font stash
    neko_asset_font_load_from_file(__neko_platform_get_path("data/assets/fonts/fusion-pixel-12px-monospaced.ttf").c_str(), &g_font, 24);

#define GUI_FONT_STASH(...)                                                                                            \
    []() -> neko_gui_font_stash_desc_t * {                                                                             \
        neko_gui_font_desc_t font_decl[] = {{.key = "mc_regular", .font = &g_font}};                                   \
        static neko_gui_font_stash_desc_t font_stash = {.fonts = font_decl, .size = 1 * sizeof(neko_gui_font_desc_t)}; \
        return &font_stash;                                                                                            \
    }()

    // neko_gui_font_stash_desc_t font_stash = {.fonts = (neko_gui_font_desc_t[]){{.key = "mc_regular", .font = &font}, .size = 1 * sizeof(neko_gui_font_desc_t)};
    neko_gui_init_font_stash(&g_gui, GUI_FONT_STASH());

    // Load style sheet from file now
    app_load_style_sheet(false);

    // Dock windows before hand
    neko_gui_dock_ex(&g_gui, "Style_Editor", "Demo_Window", NEKO_GUI_SPLIT_TAB, 0.5f);

    g_imgui = neko_imgui_new(neko_platform_main_window(), false);

    register_components(neko_ecs());
    register_systems(neko_ecs());

    // 测试用
    neko_ecs_ent e = neko_ecs_ent_make(neko_ecs());
    CTransform xform = {10, 10};
    CVelocity velocity = {0, 0};

    neko_sprite_load(&test_witch_spr, __neko_platform_get_path("data/assets/textures/B_witch.ase"));

    neko_sprite_renderer sprite_test = {.sprite = &test_witch_spr};
    neko_particle_renderer particle_render = {};

    neko_sprite_renderer_play(&sprite_test, "Charge_Loop");
    neko_particle_renderer_construct(&particle_render);

    neko_ecs_ent_add_component(neko_ecs(), e, COMPONENT_TRANSFORM, &xform);
    neko_ecs_ent_add_component(neko_ecs(), e, COMPONENT_VELOCITY, &velocity);
    neko_ecs_ent_add_component(neko_ecs(), e, COMPONENT_SPRITE, &sprite_test);
    neko_ecs_ent_add_component(neko_ecs(), e, COMPONENT_PARTICLE, &particle_render);

    neko_ecs_ent_print(neko_ecs(), e);

    // neko_ecs_ent_destroy(ecs, e);

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

#pragma region tiled

    neko_tiled_load(&map, "D:/Projects/Neko/Dev/data_tiled/map.tmx", NULL);

    char *vert_src = neko_read_file_contents_into_string_null_term("D:/Projects/Neko/Dev/data_tiled/shader/sprite.vert", "rb", NULL);
    char *frag_src = neko_read_file_contents_into_string_null_term("D:/Projects/Neko/Dev/data_tiled/shader/sprite.frag", "rb", NULL);

    neko_tiled_render_init(&g_cb, vert_src, frag_src);

    neko_free(vert_src);
    neko_free(frag_src);

#pragma endregion
}

void game_update() {

    struct {
        float x, y, w, h;
    } batch_tex_uvs[] = {
            {2, 2, 24, 24}, {58, 2, 24, 24}, {114, 2, 24, 24}, {170, 2, 24, 24}, {2, 30, 24, 24},
    };

    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());
    neko_vec2 mp = neko_platform_mouse_positionv();
    neko_vec2 mw = neko_platform_mouse_wheelv();
    neko_vec2 md = neko_platform_mouse_deltav();
    bool lock = neko_platform_mouse_locked();
    bool moved = neko_platform_mouse_moved();
    const f32 t = neko_platform_elapsed_time();

    if (neko_platform_key_pressed(NEKO_KEYCODE_ESC)) {
        neko_quit();
    }

    neko_ecs_run_systems(neko_ecs(), ECS_SYSTEM_UPDATE);

    // Do rendering
    neko_graphics_clear_action_t clear_action = {.color = {g_cvar.bg[0] / 255, g_cvar.bg[1] / 255, g_cvar.bg[2] / 255, 1.f}};
    neko_graphics_clear_desc_t clear = {.actions = &clear_action};
    neko_graphics_renderpass_begin(&g_cb, neko_renderpass_t{0});
    { neko_graphics_clear(&g_cb, &clear); }
    neko_graphics_renderpass_end(&g_cb);

    // Set up 2D camera for projection matrix
    neko_idraw_defaults(&g_idraw);
    neko_idraw_camera2D(&g_idraw, (u32)fbs.x, (u32)fbs.y);

    neko_ecs_run_systems(neko_ecs(), ECS_SYSTEM_RENDER_IMMEDIATE);

    neko_graphics_renderpass_begin(&g_cb, neko_renderpass_t{0});
    {
        neko_graphics_set_viewport(&g_cb, 0, 0, (u32)fbs.x, (u32)fbs.y);
        neko_idraw_draw(&g_idraw, &g_cb);
    }
    neko_graphics_renderpass_end(&g_cb);

    // Camera for scene
    neko_camera_t cam = neko_camera_perspective();
    cam.transform.position = neko_v3(0.f, 6.f, 20.f);
    neko_vqs trans = {.translation = neko_v3(0.f, 0.f, -10.f), .rotation = neko_quat_angle_axis(t * 0.001f, NEKO_YAXIS), .scale = neko_v3s(0.1f)};
    neko_mat4 model = neko_vqs_to_mat4(&trans);
    neko_mat4 vp = neko_camera_get_view_projection(&cam, fbs.x, fbs.y);
    neko_mat4 mvp = neko_mat4_mul(vp, model);

    if (g_cvar.hello_duck) {

        // Apply material uniforms
        neko_gfxt_material_set_uniform(&mat, "u_mvp", &mvp);
        neko_gfxt_material_set_uniform(&mat, "u_tex", &texture);

        // Rendering
        neko_graphics_renderpass_begin(&g_cb, neko_renderpass_t{0});
        {
            // Set view port
            neko_graphics_set_viewport(&g_cb, 0, 0, (int)fbs.x, (int)fbs.y);

            // Bind material
            neko_gfxt_material_bind(&g_cb, &mat);

            // Bind material uniforms
            neko_gfxt_material_bind_uniforms(&g_cb, &mat);

            // Render mesh
            neko_gfxt_mesh_draw_material(&g_cb, &mesh, &mat);
        }
        neko_graphics_renderpass_end(&g_cb);
    }

#pragma region AI

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

#pragma endregion

    neko_graphics_renderpass_begin(&g_cb, neko_renderpass_t{0});
    {
        neko_tiled_render_begin();

        // for (u32 i = 0; i < neko_dyn_array_size(map.layers); i++) {
        //     layer_t *layer = map.layers + i;
        //     for (u32 y = 0; y < layer->height; y++) {
        //         for (u32 x = 0; x < layer->width; x++) {
        //             tile_t *tile = layer->tiles + (x + y * layer->width);
        //             if (tile->id != 0) {
        //                 tileset_t *tileset = map.tilesets + tile->tileset_id;
        //                 u32 tsxx = (tile->id % (tileset->width / tileset->tile_width) - 1) * tileset->tile_width;
        //                 u32 tsyy = tileset->tile_height * ((tile->id - tileset->first_gid) / (tileset->width / tileset->tile_width));
        //                 neko_tiled_quad_t quad = {.texture = tileset->texture,
        //                                           .texture_size = {(f32)tileset->width, (f32)tileset->height},
        //                                           .position = {(f32)(x * tileset->tile_width * SPRITE_SCALE), (f32)(y * tileset->tile_height * SPRITE_SCALE)},
        //                                           .dimentions = {(f32)(tileset->tile_width * SPRITE_SCALE), (f32)(tileset->tile_height * SPRITE_SCALE)},
        //                                           .rectangle = {(f32)tsxx, (f32)tsyy, (f32)tileset->tile_width, (f32)tileset->tile_height},
        //                                           .color = layer->tint,
        //                                           .use_texture = true};
        //                 neko_tiled_render_push(&g_cb, &quad);
        //             }
        //         }
        //     }
        // }

        // for (u32 i = 0; i < neko_dyn_array_size(map.object_groups); i++) {
        //     object_group_t *group = map.object_groups + i;
        //     for (u32 ii = 0; ii < neko_dyn_array_size(map.object_groups[i].objects); ii++) {
        //         object_t *object = group->objects + ii;
        //         neko_tiled_quad_t quad = {.position = {(f32)(object->x * SPRITE_SCALE), (f32)(object->y * SPRITE_SCALE)},
        //                        .dimentions = {(f32)(object->width * SPRITE_SCALE), (f32)(object->height * SPRITE_SCALE)},
        //                        .color = group->color,
        //                        .use_texture = false};
        //         neko_tiled_render_push(&g_cb, &quad);
        //     }
        // }

        neko_tiled_render_flush(&g_cb);
    }
    neko_graphics_renderpass_end(&g_cb);

#pragma region gui

    ImGuiIO io = ImGui::GetIO();

    neko_gui_begin(&g_gui, NULL);
    {
        dockspace(&g_gui);

        if (g_cvar.show_gui) {

            neko_gui_demo_window(&g_gui, neko_gui_rect(100, 100, 500, 500), NULL);
            neko_gui_style_editor(&g_gui, NULL, neko_gui_rect(350, 250, 300, 240), NULL);

#pragma region gui_mouse

            const neko_vec2 ws = neko_v2(600.f, 300.f);
            neko_gui_window_begin(&g_gui, "Mouse", neko_gui_rect((fbs.x - ws.x) * 0.5f, (fbs.y - ws.y) * 0.5f, ws.x, ws.y));
            {
#define GUI_LABEL(STR, ...)                              \
    do {                                                 \
        neko_snprintfc(BUFFER, 256, STR, ##__VA_ARGS__); \
        neko_gui_label(&g_gui, BUFFER);                  \
    } while (0)

                neko_gui_layout_row(&g_gui, 1, neko_gui_widths(-1), 0);

                GUI_LABEL("Position: <%.2f %.2f>", mp.x, mp.y);
                GUI_LABEL("Wheel: <%.2f %.2f>", mw.x, mw.y);
                GUI_LABEL("Delta: <%.2f %.2f>", md.x, md.y);
                GUI_LABEL("Lock: %zu", lock);
                GUI_LABEL("Moved: %zu", moved);

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

                neko_gui_layout_row(&g_gui, 7, neko_gui_widths(100, 100, 32, 100, 32, 100, 32), 0);
                for (u32 i = 0; btns[i].str; ++i) {
                    GUI_LABEL("%s: ", btns[i].str);
                    GUI_LABEL("pressed: ");
                    GUI_LABEL("%d", mouse_pressed[btns[i].val]);
                    GUI_LABEL("down: ");
                    GUI_LABEL("%d", mouse_down[btns[i].val]);
                    GUI_LABEL("released: ");
                    GUI_LABEL("%d", mouse_released[btns[i].val]);
                }
            }
            neko_gui_window_end(&g_gui);

#pragma endregion

#pragma region gui_ss

            const neko_gui_style_sheet_t *ss = &style_sheet;

            const neko_vec2 ss_ws = neko_v2(500.f, 300.f);
            neko_gui_window_begin(&g_gui, "Window", neko_gui_rect((fbs.x - ss_ws.x) * 0.5f, (fbs.y - ss_ws.y) * 0.5f, ss_ws.x, ss_ws.y));
            {
                // Cache the current container
                neko_gui_container_t *cnt = neko_gui_get_current_container(&g_gui);

                neko_gui_layout_row(&g_gui, 2, neko_gui_widths(200, 0), 0);

                neko_gui_text(&g_gui, "A regular element button.");
                neko_gui_button(&g_gui, "button");

                neko_gui_text(&g_gui, "A regular element label.");
                neko_gui_label(&g_gui, "label");

                neko_gui_text(&g_gui, "Button with classes: {.c0 .btn}");

                neko_gui_selector_desc_t selector_1 = {.classes = {"c0", "btn"}};
                neko_gui_button_ex(&g_gui, "hello?##btn", &selector_1, 0x00);

                neko_gui_text(&g_gui, "Label with id #lbl and class .c0");
                neko_gui_selector_desc_t selector_2 = {.id = "lbl", .classes = {"c0"}};
                neko_gui_label_ex(&g_gui, "label##lbl", &selector_2, 0x00);

                const f32 m = cnt->body.w * 0.3f;
                // neko_gui_layout_row(gui, 2, (int[]){m, -m}, 0);
                // neko_gui_layout_next(gui); // Empty space at beginning
                neko_gui_layout_row(&g_gui, 1, neko_gui_widths(0), 0);
                neko_gui_selector_desc_t selector_3 = {.classes = {"reload_btn"}};
                if (neko_gui_button_ex(&g_gui, "reload style sheet", &selector_3, 0x00)) {
                    app_load_style_sheet(true);
                }

                button_custom(&g_gui, "Hello?");
            }
            neko_gui_window_end(&g_gui);

#pragma endregion

#pragma region gui_idraw

            neko_gui_window_begin(&g_gui, "Idraw", neko_gui_rect((fbs.x - ws.x) * 0.2f, (fbs.y - ws.y) * 0.5f, ws.x, ws.y));
            {
                // Cache the current container
                neko_gui_container_t *cnt = neko_gui_get_current_container(&g_gui);

                // 绘制到当前窗口中的转换对象的自定义回调
                // 在这里，我们将容器的主体作为视口传递，但这可以是您想要的任何内容，
                // 正如我们将在“分割”窗口示例中看到的那样。
                // 我们还传递自定义数据（颜色），以便我们可以在回调中使用它。如果以下情况，这可以为 NULL
                // 你不需要任何东西。
                neko_color_t color = neko_color_alpha(NEKO_COLOR_RED, (uint8_t)neko_clamp((sin(t * 0.001f) * 0.5f + 0.5f) * 255, 0, 255));
                neko_gui_draw_custom(&g_gui, cnt->body, gui_cb, &color, sizeof(color));
            }
            neko_gui_window_end(&g_gui);

#pragma endregion

#pragma region gui_ai

            neko_gui_window_begin(&g_gui, "AI", neko_gui_rect(10, 10, 350, 220));
            {
                neko_gui_layout_row(&g_gui, 1, neko_gui_widths(-1), 100);
                neko_gui_text(&g_gui,
                              " * The AI will continue to move towards a random location as long as its health is not lower than 50.\n\n"
                              " * If health drops below 50, the AI will pause to heal back up to 100 then continue moving towards its target.\n\n"
                              " * After it reaches its target, it will find another random location to move towards.");

                neko_gui_layout_row(&g_gui, 1, neko_gui_widths(-1), 0);
                neko_gui_label(&g_gui, "state: %s", ai.state == AI_STATE_HEAL ? "HEAL" : "MOVE");
                neko_gui_layout_row(&g_gui, 2, neko_gui_widths(55, 50), 0);
                neko_gui_label(&g_gui, "health: ");
                neko_gui_number(&g_gui, &ai.health, 0.1f);
            }
            neko_gui_window_end(&g_gui);

#pragma endregion
        }

        if (neko_platform_key_pressed(NEKO_KEYCODE_GRAVE_ACCENT)) {
            console.open = !console.open;
        } else if (neko_platform_key_pressed(NEKO_KEYCODE_TAB) && console.open) {
            console.autoscroll = !console.autoscroll;
        }

        neko_gui_layout_t l;
        if (window && neko_gui_window_begin(&g_gui, "App", neko_gui_rect(100, 100, 200, 200))) {
            l = *neko_gui_get_layout(&g_gui);
            neko_gui_layout_row(&g_gui, 1, neko_gui_widths(-1), 0);
            neko_gui_text(&g_gui, "Hello neko");
            neko_gui_window_end(&g_gui);
        }

        int s = summons;
        while (s--) {
            neko_gui_push_id(&g_gui, &s, sizeof(s));
            if (neko_gui_window_begin(&g_gui, "Summon", neko_gui_rect(100, 100, 200, 200))) {
                neko_gui_layout_row(&g_gui, 1, neko_gui_widths(-1), 0);
                neko_gui_text(&g_gui, "new window");
                neko_gui_window_end(&g_gui);
            }
            neko_gui_pop_id(&g_gui);
        }

        neko_vec2 fb = (&g_gui)->framebuffer_size;
        neko_gui_rect_t screen;
        if (embeded)
            screen = l.body;
        else
            screen = neko_gui_rect(0, 0, fb.x, fb.y);
        neko_console(&console, &g_gui, screen, NULL);
    }
    neko_gui_end(&g_gui, !(io.WantCaptureMouse || io.WantCaptureKeyboard));

    // Do rendering

    neko_graphics_renderpass_begin(&g_cb, neko_renderpass_t{0});
    {
        neko_graphics_set_viewport(&g_cb, 0, 0, (int)fbs.x, (int)fbs.y);
        neko_gui_render(&g_gui, &g_cb);
    }
    neko_graphics_renderpass_end(&g_cb);

    neko_imgui_new_frame(&g_imgui);

    if (ImGui::Begin("Test")) {
        neko::imgui::Auto(g_cvar.show_cvar_window, "CVar Window");
        if (ImGui::Button("test_xml")) test_xml(__neko_platform_get_path("data/test/test.xml"));
        if (ImGui::Button("test_se")) test_se();
        if (ImGui::Button("test_sr")) test_sr();
        if (ImGui::Button("test_ut")) test_ut();
        if (ImGui::Button("test_backtrace")) __neko_print_stacktrace();
        if (ImGui::Button("test_containers")) test_containers();
        if (ImGui::Button("test_sexpr")) test_sexpr();
    }
    ImGui::End();

    if (g_cvar.show_info_window) {
        if (ImGui::Begin("Info", &g_cvar.show_info_window)) {
            {

                ImGui::Text("GC MemAllocInUsed: %.2lf mb", (f64)(neko_mem_bytes_inuse() / 1048576.0));
                ImGui::Text("GC MemTotalAllocated: %.2lf mb", (f64)(g_allocation_metrics.total_allocated / 1048576.0));
                ImGui::Text("GC MemTotalFree: %.2lf mb", (f64)(g_allocation_metrics.total_free / 1048576.0));

#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049

                GLint total_mem_kb = 0;
                glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX, &total_mem_kb);

                GLint cur_avail_mem_kb = 0;
                glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX, &cur_avail_mem_kb);

                ImGui::Text("GPU MemTotalAvailable: %.2lf mb", (f64)(cur_avail_mem_kb / 1024.0f));
                ImGui::Text("GPU MemCurrentUsage: %.2lf mb", (f64)((total_mem_kb - cur_avail_mem_kb) / 1024.0f));

                lua_gc(L, LUA_GCCOLLECT, 0);
                lua_Integer kb = lua_gc(L, LUA_GCCOUNT, 0);
                lua_Integer bytes = lua_gc(L, LUA_GCCOUNTB, 0);

                ImGui::Text("Lua MemoryUsage: %.2lf mb", ((f64)kb / 1024.0f));
                ImGui::Text("Lua Remaining: %.2lf mb", ((f64)bytes / 1024.0f));

                // ImGui::Text("ImGui MemoryUsage: %.2lf mb", ((f64)__neko_imgui_meminuse() / 1048576.0));

                static neko_platform_meminfo_t meminfo;

                neko_timed_action(60, { meminfo = neko_platform_get_meminfo(); });

                ImGui::Text("Virtual MemoryUsage: %.2lf mb", ((f64)meminfo.virtual_memory_used / 1048576.0));
                ImGui::Text("Real MemoryUsage: %.2lf mb", ((f64)meminfo.physical_memory_used / 1048576.0));
                ImGui::Text("Virtual MemoryUsage Peak: %.2lf mb", ((f64)meminfo.peak_virtual_memory_used / 1048576.0));
                ImGui::Text("Real MemoryUsage Peak: %.2lf mb", ((f64)meminfo.peak_physical_memory_used / 1048576.0));

                ImGui::Text("Using renderer: %s", glGetString(GL_RENDERER));
                ImGui::Text("OpenGL version supported: %s", glGetString(GL_VERSION));

                neko_vec2 opengl_ver = neko_platform_get_opengl_ver();
                ImGui::Text("OpenGL Version: %d.%d", (s32)opengl_ver.x, (s32)opengl_ver.y);
            }
        }
        ImGui::End();
    }

    if (g_cvar.show_demo_window) ImGui::ShowDemoWindow(&g_cvar.show_demo_window);

    if (g_cvar.show_cvar_window) {
        if (ImGui::Begin("CVar", &g_cvar.show_cvar_window)) {
            try {
                neko_cvar_gui();
            } catch (const std::exception ex) {
                neko_log_error("[Exception] %s", ex.what());
            }
        }
        ImGui::End();
    }

    neko_imgui_render(&g_imgui, &g_cb);

    try {
        neko_lua_wrap_call(L, "test_update");
    } catch (std::exception &ex) {
        neko_log_error("%s", ex.what());
    }

#pragma endregion

    // Submits to cb
    neko_graphics_command_buffer_submit(&g_cb);
}

void game_shutdown() {
    try {
        neko_lua_wrap_call(L, "game_shutdown");
    } catch (std::exception &ex) {
        neko_log_error("%s", ex.what());
    }

    for (u32 i = 0; i < neko_dyn_array_size(map.tilesets); i++) {
        neko_graphics_texture_destroy(map.tilesets[i].texture);
    }

    for (u32 i = 0; i < neko_dyn_array_size(map.layers); i++) {
        neko_free(map.layers[i].tiles);
    }

    neko_dyn_array_free(map.layers);
    neko_dyn_array_free(map.tilesets);

    neko_tiled_render_deinit(&g_cb);

    neko_scripting_end(L);

    neko_gui_free(&g_gui);
}

neko_game_desc_t neko_main(s32 argc, char **argv) {

    // 确定运行路径
    auto current_dir = std::filesystem::path(std::filesystem::current_path());
    for (int i = 0; i < 3; ++i)
        if (std::filesystem::exists(current_dir / "data") && std::filesystem::exists(current_dir / "data" / "scripts")) {
            data_path = neko_fs_normalize_path(current_dir.string());
            neko_log_info(std::format("game data path detected: {0} (base: {1})", data_path, std::filesystem::current_path().string()).c_str());
            break;
        } else {
            current_dir = current_dir.parent_path();
        }

    return neko_game_desc_t{.init = game_init, .update = game_update, .shutdown = game_shutdown, .window = {.width = 1440, .height = 880}, .argc = argc, .argv = argv};
}

void dockspace(neko_gui_context_t *ctx) {
    s32 opt = NEKO_GUI_OPT_NOCLIP | NEKO_GUI_OPT_NOFRAME | NEKO_GUI_OPT_FORCESETRECT | NEKO_GUI_OPT_NOTITLE | NEKO_GUI_OPT_DOCKSPACE | NEKO_GUI_OPT_FULLSCREEN | NEKO_GUI_OPT_NOMOVE |
              NEKO_GUI_OPT_NOBRINGTOFRONT | NEKO_GUI_OPT_NOFOCUS | NEKO_GUI_OPT_NORESIZE;
    neko_gui_window_begin_ex(ctx, "Dockspace", neko_gui_rect(350, 40, 600, 500), NULL, NULL, opt);
    {
        // Empty dockspace...
    }
    neko_gui_window_end(ctx);
}
