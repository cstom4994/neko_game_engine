

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
#include "engine/neko_platform.h"
#include "engine/util/neko_ai.h"
#include "engine/util/neko_asset.h"
#include "engine/util/neko_gfxt.h"
#include "engine/util/neko_gui.h"
#include "engine/util/neko_idraw.h"
#include "engine/util/neko_imgui.h"
#include "engine/util/neko_script.h"
#include "engine/util/neko_sprite.h"
#include "engine/util/neko_tiled.h"

// binding
#include "neko_api.h"

// builtin
#include "engine/builtin/neko_aseprite.h"

// game
#include "game_editor.h"
#include "game_scripts.h"
#include "libs/imgui/imgui.h"
#include "neko_hash.h"

// hpp
#include "sandbox/hpp/neko_static_refl.hpp"
#include "sandbox/hpp/neko_struct.hpp"
#include "sandbox/neko_gui_auto.hpp"

#define NEKO_CONSOLE_IMPL
#include "engine/util/neko_console.h"

#define NEKO_IMGUI_IMPL
#include "game_imgui.h"

#define SPRITEBATCH_IMPLEMENTATION
#include "engine/builtin/cute_spritebatch.h"

#define NEKO_PNG_IMPLEMENTATION
#include "engine/builtin/neko_png.h"

// fs
#include "engine/builtin/neko_fs.h"

// opengl
#include "libs/glad/glad.h"

NEKO_HIJACK_MAIN();

neko_command_buffer_t g_cb = neko_default_val();
neko_core_ui_context_t g_core_ui = neko_default_val();
neko_immediate_draw_t g_idraw = neko_default_val();
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
neko_packreader_t g_pack = neko_default_val();

game_vm_t g_vm = neko_default_val();

#ifdef GAME_CSHARP_ENABLED
game_csharp g_csharp;
#endif

// user data

neko_client_userdata_t g_client_userdata = neko_default_val();

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
    bool show_info_window = false;
    bool show_cvar_window = false;
    bool show_pack_editor = false;
    bool show_profiler_window = false;

    bool show_gui = false;
    bool show_console = false;

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
            rf_field{TSTR("show_info_window"), &rf_type::show_info_window},          //
            rf_field{TSTR("show_cvar_window"), &rf_type::show_cvar_window},          //
            rf_field{TSTR("show_pack_editor"), &rf_type::show_pack_editor},          //
            rf_field{TSTR("show_profiler_window"), &rf_type::show_profiler_window},  //
            rf_field{TSTR("show_gui"), &rf_type::show_gui},                          //
            rf_field{TSTR("show_console"), &rf_type::show_console},                  //
            rf_field{TSTR("shader_inspect"), &rf_type::shader_inspect},              //
            rf_field{TSTR("hello_ai_shit"), &rf_type::hello_ai_shit},                //
            rf_field{TSTR("is_hotfix"), &rf_type::is_hotfix},                        //
            rf_field{TSTR("vsync"), &rf_type::vsync},                                //
            rf_field{TSTR("enable_nekolua"), &rf_type::enable_nekolua},              //
    };
};

neko_struct(neko_engine_cvar_t,                            //
            _Fs(show_editor, "Is show editor"),            //
            _Fs(show_demo_window, "Is show nui demo"),     //
            _Fs(show_info_window, "Is show info window"),  //
            _Fs(show_cvar_window, "cvar inspector"),       //
            _Fs(show_pack_editor, "pack editor"),          //
            _Fs(show_profiler_window, "profiler"),         //
            _Fs(show_gui, "neko gui"),                     //
            _Fs(show_console, "neko console"),             //
            _Fs(shader_inspect, "shaders"),                //
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
void test_thd(void);

NEKO_API_DECL void test_sexpr();
NEKO_API_DECL void test_ttf();
NEKO_API_DECL void test_sc();

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

neko_string game_assets(const neko_string &path) {
    if (data_path.empty()) {
        neko_assert(!data_path.empty());  // game data path not detected
        return path;
    }
    neko_string get_path(data_path);

    if (path.substr(0, 7) == "gamedir") {
        get_path.append(path);
    } else if (path.substr(0, 11) == "lua_scripts") {
        neko_string lua_path = path;
        get_path = std::filesystem::path(data_path).parent_path().string().append("/source/lua/game/").append(lua_path.replace(0, 12, ""));
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

neko_handle(neko_graphics_uniform_t) u_roll = {0};
neko_handle(neko_graphics_texture_t) cmptex = {0};
neko_handle(neko_graphics_pipeline_t) cmdpip = {0};
neko_handle(neko_graphics_shader_t) cmpshd = {0};
neko_handle(neko_graphics_storage_buffer_t) u_voxels = {0};

#define TEX_WIDTH 512
#define TEX_HEIGHT 512

const char *comp_src =
        "#version 430 core\n"
        "uniform float u_roll;\n"
        "layout(rgba32f, binding = 0) uniform image2D destTex;\n"
        "layout (std430, binding = 1) readonly buffer u_voxels {\n"
        "   vec4 data;\n"
        "};\n"
        "layout (local_size_x = 16, local_size_y = 16) in;\n"
        "void main() {\n"
        "ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);\n"
        "float localCoef = length(vec2(ivec2(gl_LocalInvocationID.xy) - 8 ) / 8.0);\n"
        "float globalCoef = sin(float(gl_WorkGroupID.x + gl_WorkGroupID.y) * 0.1 + u_roll) * 0.5;\n"
        "vec4 rc = vec4(1.0 - globalCoef * localCoef, globalCoef * localCoef, 0.0, 1.0);\n"
        "vec4 color = mix(rc, data, 0.5f);\n"
        "imageStore(destTex, storePos, color);\n"
        "}";

// clang-format off
const_str image_names[] = {
        "gamedir/assets/textures/dragon_zombie.png",
        "gamedir/assets/textures/night_spirit.png",
};
// clang-format on

int images_count = sizeof(image_names) / sizeof(*image_names);
neko_png_image_t images[sizeof(image_names) / sizeof(*image_names)];

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
    NEKO_SPRITEBATCH_U64 image_id;
    int depth;
    f32 x, y;
    f32 sx, sy;
    f32 c, s;
} neko_sprite_t;

neko_graphics_custom_batch_context_t *font_render;
neko_graphics_custom_batch_shader_t font_shader;
neko_graphics_custom_batch_renderable_t font_renderable;
f32 font_scale = 3.f;
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

neko_sprite_t make_sprite(NEKO_SPRITEBATCH_U64 image_id, f32 x, f32 y, f32 scale, f32 angle_radians, int depth) {

    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());

    f32 x0 = (x - fbs.x / 2.f) / scale /*+ -text_w / 2.f*/;
    f32 y0 = (fbs.y / 2.f - y) / scale / 2.f;

    neko_sprite_t s;
    s.image_id = image_id;
    s.depth = depth;
    s.x = x0;
    s.y = y0;
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
    s.sort_bits = (NEKO_SPRITEBATCH_U64)sp.depth;
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

void get_pixels(NEKO_SPRITEBATCH_U64 image_id, void *buffer, int bytes_to_fill, void *udata) {
    (void)udata;
    memcpy(buffer, images[image_id].pix, bytes_to_fill);
}

NEKO_SPRITEBATCH_U64 generate_texture_handle(void *pixels, int w, int h, void *udata) {
    (void)udata;
    GLuint location;
    glGenTextures(1, &location);
    glBindTexture(GL_TEXTURE_2D, location);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    return (NEKO_SPRITEBATCH_U64)location;
}

void destroy_texture_handle(NEKO_SPRITEBATCH_U64 texture_id, void *udata) {
    (void)udata;
    GLuint id = (GLuint)texture_id;
    glDeleteTextures(1, &id);
}

void load_images() {
    for (s32 i = 0; i < images_count; ++i) images[i] = neko_png_load(game_assets(image_names[i]).c_str());
}

void unload_images() {
    for (s32 i = 0; i < images_count; ++i) neko_png_free(images + i);
}

void draw_text(neko_font_t *font, const char *text, float x, float y, float line_height, float clip_region, float wrap_x, f32 scale) {
    f32 text_w = (f32)neko_font_text_width(font, text);
    f32 text_h = (f32)neko_font_text_height(font, text);

    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());

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

// neko_assetsys_t *g_assetsys;
// neko_filewatch_t *g_filewatch;

// 内部音频数据的资源句柄 由于音频必须在单独的线程上运行 因此这是必要的
cs_audio_source_t *piano;
cs_sound_params_t params = cs_sound_params_default();

game_editor_console console_new;

// lua
#include "neko_scripting.h"

lua_State *g_L = nullptr;

thread_atomic_int_t init_thread_flag;
thread_ptr_t init_work_thread;

int init_work(void *user_data) {

    thread_atomic_int_t *this_init_thread_flag = (thread_atomic_int_t *)user_data;

    if (thread_atomic_int_load(this_init_thread_flag) == 0) {

        thread_timer_t thread_timer;
        thread_timer_init(&thread_timer);

        neko_timer timer;
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

    g_client_userdata.cb = &g_cb;
    g_client_userdata.idraw = &g_idraw;
    g_client_userdata.igui = &g_core_ui;
    g_client_userdata.idraw_sd = neko_immediate_draw_static_data_get();

    g_vm.ctx = neko_script_ctx_new(nullptr);

    g_L = neko_scripting_init();

    // 测试 csharp wrapper
#ifdef GAME_CSHARP_ENABLED

#ifdef NEKO_DEBUG
    std::string managed_path(game_assets("gamedir/managed/debug"));
#else
    std::string managed_path(game_assets("gamedir/managed/release"));
#endif

    test_cs(managed_path);

    g_csharp.init(managed_path);
#endif

    {
        neko_lua_wrap_register_t<>(g_L).def(
                +[](const_str path) -> neko_string { return game_assets(path); }, "__neko_file_path");

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
    neko_asset_texture_load_from_file(game_assets("gamedir/assets/textures/yzh.png").c_str(), &tex0, NULL, false, false);
    tex_hndl = neko_assets_create_asset(&g_am, neko_asset_texture_t, &tex0);

    // 构建实例源
    piano = cs_load_wav(game_assets("gamedir/assets/audio/otoha.wav").c_str(), NULL);

    neko_core_ui_init(&g_core_ui, neko_platform_main_window());

    // 加载自定义字体文件 初始化 gui font stash
    // neko_asset_ascii_font_load_from_memory(font_data, font_data_size, &g_font, 24);
    neko_asset_ascii_font_load_from_file(game_assets("gamedir/assets/fonts/monocraft.ttf").c_str(), &g_font, 24);

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

    struct neko_gui_font_config config = neko_gui_font_config(20);
    config.oversample_h = 1;
    config.oversample_v = 1;
    config.range = neko_gui_font_chinese_glyph_ranges();

    // 字体加载
    neko_gui_font_stash_begin(&g_gui, NULL);
    neko_gui_font *font = neko_gui_font_atlas_add_from_file(g_gui.atlas, game_assets("gamedir/assets/fonts/VonwaonBitmap-12px.ttf").c_str(), 20, &config);
    neko_gui_font_stash_end(&g_gui);

    neko_gui_style_set_font(&g_gui.neko_gui_ctx, &font->handle);

    g_imgui = neko_imgui_new(neko_platform_main_window(), false);

    neko_gui::ctx = &g_gui.neko_gui_ctx;

    g_client_userdata.nui = &g_gui;

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

    // Compute shader
    neko_graphics_shader_source_desc_t sources[] = {neko_graphics_shader_source_desc_t{.type = NEKO_GRAPHICS_SHADER_STAGE_COMPUTE, .source = comp_src}};

    // Create shader
    cmpshd = neko_graphics_shader_create(neko_c_ref(neko_graphics_shader_desc_t, {.sources = sources, .size = sizeof(sources), .name = "compute"}));

    // Create uniform
    u_roll = neko_graphics_uniform_create(
            neko_c_ref(neko_graphics_uniform_desc_t, {.name = "u_roll", .layout = neko_c_ref(neko_graphics_uniform_layout_desc_t, {.type = NEKO_GRAPHICS_UNIFORM_FLOAT})}));

    // Texture for compute shader output
    cmptex = neko_graphics_texture_create(neko_c_ref(neko_graphics_texture_desc_t, {
                                                                                           .width = TEX_WIDTH,
                                                                                           .height = TEX_HEIGHT,
                                                                                           .format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA32F,
                                                                                           .wrap_s = NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT,
                                                                                           .wrap_t = NEKO_GRAPHICS_TEXTURE_WRAP_REPEAT,
                                                                                           .min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST,
                                                                                           .mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST,
                                                                                   }));

    cmdpip = neko_graphics_pipeline_create(neko_c_ref(neko_graphics_pipeline_desc_t, {.compute = {.shader = cmpshd}}));

    neko_vec4 data = neko_v4(0.f, 1.f, 1.f, 1.f);

    u_voxels = neko_graphics_storage_buffer_create(
            neko_c_ref(neko_graphics_storage_buffer_desc_t, {.data = &data, .size = sizeof(neko_vec4), .name = "u_voxels", .usage = NEKO_GRAPHICS_BUFFER_USAGE_DYNAMIC}));

    // 初始化工作

    thread_atomic_int_store(&init_thread_flag, 0);

    init_work_thread = thread_init(init_work, &init_thread_flag, "init_work_thread", THREAD_STACK_SIZE_DEFAULT);
}

void game_update() {
    //    if (is_hotfix_loaded) neko_hotload_module_update(ctx);

    int this_init_thread_flag = thread_atomic_int_load(&init_thread_flag);

    if (this_init_thread_flag == 0) {

        neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());
        const f32 t = neko_platform_elapsed_time();

        u8 tranp = 255;

        // tranp -= ((s32)t % 255);

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

        neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());
        const f32 t = neko_platform_elapsed_time();
        neko_vec2 mp = neko_platform_mouse_positionv();
        neko_vec2 mw = neko_platform_mouse_wheelv();
        neko_vec2 md = neko_platform_mouse_deltav();
        bool lock = neko_platform_mouse_locked();
        bool moved = neko_platform_mouse_moved();
        struct neko_gui_context *gui_ctx = &g_gui.neko_gui_ctx;

        if (neko_platform_key_pressed(NEKO_KEYCODE_ESC)) g_cvar.show_editor ^= true;
        if (neko_platform_key_pressed(NEKO_KEYCODE_SLASH)) g_cvar.show_console ^= true;

#ifdef GAME_CSHARP_ENABLED
        {
            neko_profiler_scope_begin(csharp_update);
            g_csharp.update();
            neko_profiler_scope_end(csharp_update);
        }
#endif

        {
            neko_profiler_scope_begin(lua_update);
            try {
                neko_lua_wrap_call(g_L, "game_update");
                neko_lua_wrap_call(g_L, "test_update");
            } catch (std::exception &ex) {
                neko_log_error("lua exception %s", ex.what());
            }
            neko_profiler_scope_end(lua_update);
        }

        // Do rendering
        neko_graphics_clear_action_t clear_action = {.color = {g_cvar.bg[0] / 255, g_cvar.bg[1] / 255, g_cvar.bg[2] / 255, 1.f}};
        neko_graphics_clear_desc_t clear = {.actions = &clear_action};

        neko_graphics_clear_action_t b_clear_action = {.color = {0.0f, 0.0f, 0.0f, 1.f}};
        neko_graphics_clear_desc_t b_clear = {.actions = &b_clear_action};

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

#pragma region FrameBuffer

        neko_idraw_defaults(&g_idraw);

        // Immediate rendering for offscreen buffer
        neko_idraw_camera3D(&g_idraw, (uint32_t)fbs.x, (uint32_t)fbs.y);
        neko_idraw_translatef(&g_idraw, 0.f, 0.f, -2.f);
        neko_idraw_rotatev(&g_idraw, neko_platform_elapsed_time() * 0.0001f, NEKO_YAXIS);
        neko_idraw_rotatev(&g_idraw, neko_platform_elapsed_time() * 0.0002f, NEKO_XAXIS);
        neko_idraw_rotatev(&g_idraw, neko_platform_elapsed_time() * 0.0005f, NEKO_ZAXIS);
        neko_idraw_box(&g_idraw, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 200, 100, 50, 255, NEKO_GRAPHICS_PRIMITIVE_LINES);

        // Bind render pass for offscreen rendering then draw to buffer
        neko_graphics_renderpass_begin(&g_cb, rp);
        neko_graphics_set_viewport(&g_cb, 0, 0, (uint32_t)fbs.x, (uint32_t)fbs.y);
        neko_graphics_clear(&g_cb, &b_clear);
        neko_idraw_draw(&g_idraw, &g_cb);
        neko_graphics_renderpass_end(&g_cb);

        // Immediate rendering for back buffer
        neko_idraw_camera3D(&g_idraw, (uint32_t)fbs.x, (uint32_t)fbs.y);
        neko_idraw_depth_enabled(&g_idraw, true);
        neko_idraw_face_cull_enabled(&g_idraw, true);
        neko_idraw_translatef(&g_idraw, 0.f, 0.f, -1.f);
        neko_idraw_texture(&g_idraw, rt);
        neko_idraw_rotatev(&g_idraw, neko_platform_elapsed_time() * 0.0001f, NEKO_YAXIS);
        neko_idraw_rotatev(&g_idraw, neko_platform_elapsed_time() * 0.0002f, NEKO_XAXIS);
        neko_idraw_rotatev(&g_idraw, neko_platform_elapsed_time() * 0.0003f, NEKO_ZAXIS);
        neko_idraw_box(&g_idraw, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 255, 255, 255, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);

        // Render to back buffer
        neko_graphics_renderpass_begin(&g_cb, NEKO_GRAPHICS_RENDER_PASS_DEFAULT);
        neko_graphics_set_viewport(&g_cb, 0, 0, (int32_t)fbs.x, (int32_t)fbs.y);
        // neko_graphics_clear(&g_cb, &clear);
        neko_idraw_draw(&g_idraw, &g_cb);
        neko_graphics_renderpass_end(&g_cb);

#pragma endregion

        neko_imgui_new_frame(&g_imgui);

        // 设置投影矩阵的 2D 相机
        neko_idraw_defaults(&g_idraw);
        neko_idraw_camera2D(&g_idraw, (u32)fbs.x, (u32)fbs.y);

        {
            neko_profiler_scope_begin(lua_render);
            try {
                neko_lua_wrap_call(g_L, "game_render");
            } catch (std::exception &ex) {
                neko_log_error("lua exception %s", ex.what());
            }
            neko_profiler_scope_end(lua_render);
        }

        if (g_client_userdata.vm.module_count) {
            if (g_client_userdata.vm.module_func[0]) {
                g_client_userdata.vm.module_func[0]();
            }
        }

        call_count = 0;

        // neko_sprite_t dragon_zombie = make_sprite(0, 650, 500, 1, neko_rad2deg(t / 100000.f), 0);
        // neko_sprite_t night_spirit = make_sprite(1, 50, 250, 1, 0, 0);
        // push_sprite(dragon_zombie);
        // push_sprite(night_spirit);
        // neko_sprite_t polish = make_sprite(1, 250, 180, 1, neko_rad2deg(t / 50000.f), 0);
        // neko_sprite_t translated = polish;
        // for (int i = 0; i < 4; ++i) {
        //     translated.x = polish.x + polish.sx * i;
        //     for (int j = 0; j < 6; ++j) {
        //         translated.y = polish.y - polish.sy * j;
        //         push_sprite(translated);
        //     }
        // }

        // 运行cute_spriteBatch查找sprite批次
        // 这是cute_psriteBatch最基本的用法 每个游戏循环进行一次defrag tick flush
        // 也可以每N帧只使用一次defrag(碎片整理)
        // tick也可以在不同的时间间隔调用(例如每次游戏更新一次
        // 但不一定每次屏幕呈现一次
        {
            neko_profiler_scope_begin(render_spritebatch);
            spritebatch_defrag(&sb);
            spritebatch_tick(&sb);
            spritebatch_flush(&sb);
            sprite_verts_count = 0;
            neko_profiler_scope_end(render_spritebatch);
        }

#pragma region gui

        neko_core_ui_begin(&g_core_ui, NULL);
        {
            editor_dockspace(&g_core_ui);

            if (g_cvar.show_info_window) {
                neko_core_ui_window_begin_ex(&g_core_ui, "Info", neko_core_ui_rect(100, 100, 500, 500), &g_cvar.show_info_window, NULL, NULL);
                {
                    neko_core_ui_layout_row(&g_core_ui, 1, neko_core_ui_widths(-1), 0);
                    {
                        static neko_memory_info_t meminfo = neko_default_val();

                        neko_timed_action(60, { meminfo = neko_platform_memory_info(); });

                        neko_core_ui_labelf("GC MemAllocInUsed: %.2lf mb", (f64)(neko_mem_bytes_inuse() / 1048576.0));
                        neko_core_ui_labelf("GC MemTotalAllocated: %.2lf mb", (f64)(g_allocation_metrics.total_allocated / 1048576.0));
                        neko_core_ui_labelf("GC MemTotalFree: %.2lf mb", (f64)(g_allocation_metrics.total_free / 1048576.0));

                        u64 max, used;
                        neko_script_gc_count(&max, &used);

                        neko_core_ui_labelf("NekoScript Alloc: %.2lf mb", ((f64)max / 1024.0f));
                        neko_core_ui_labelf("NekoScript UsedMemory: %.2lf mb", ((f64)used / 1024.0f));

                        lua_gc(g_L, LUA_GCCOLLECT, 0);
                        lua_Integer kb = lua_gc(g_L, LUA_GCCOUNT, 0);
                        lua_Integer bytes = lua_gc(g_L, LUA_GCCOUNTB, 0);

                        neko_core_ui_labelf("Lua MemoryUsage: %.2lf mb", ((f64)kb / 1024.0f));
                        neko_core_ui_labelf("Lua Remaining: %.2lf mb", ((f64)bytes / 1024.0f));

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

                static f32 delta, fps = neko_default_val();

                neko_timed_action(5, {
                    delta = neko_platform_delta_time();
                    fps = 1.f / delta;
                });

                neko_core_ui_labelf("FPS: %.2lf Delta: %.6lf", fps, delta * 1000.f);

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

        if (g_cvar.show_demo_window) ImGui::ShowDemoWindow();

        bool bInteractingWithTextbox = false;
        if (g_cvar.show_console) console_new.display_full(&bInteractingWithTextbox);

        // neko_imgui::Auto(g_cvar.bg);
        if (g_cvar.shader_inspect && ImGui::Begin("Shaders")) {
            inspect_shader("sprite_shader", sprite_shader.program);  // TEST
            ImGui::End();
        }

        if (g_cvar.show_editor) {
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("engine")) {
                    if (ImGui::Checkbox("cvar", &g_cvar.show_cvar_window))
                        ;
                    if (ImGui::Checkbox("vsync", &g_cvar.vsync)) neko_platform_enable_vsync(g_cvar.vsync);
                    if (ImGui::MenuItem("quit")) neko_quit();
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

// Compute pass
#if 0
    {
        neko_profiler_scope_auto("Compute_pass");

        float roll = neko_platform_elapsed_time() * .001f;

        // Bindings for compute shader
        neko_graphics_bind_desc_t binds = {
                .uniforms = {.desc = neko_c_ref(neko_graphics_bind_uniform_desc_t, {.uniform = u_roll, .data = &roll})},
                .image_buffers = {.desc = neko_c_ref(neko_graphics_bind_image_buffer_desc_t, {.tex = cmptex, .binding = 0, .access = NEKO_GRAPHICS_ACCESS_WRITE_ONLY})},
                .storage_buffers = {neko_c_ref(neko_graphics_bind_storage_buffer_desc_t, {.buffer = u_voxels, .binding = 1})},
        };

        // Bind compute pipeline
        neko_graphics_pipeline_bind(&g_cb, cmdpip);
        // Bind compute bindings
        neko_graphics_apply_bindings(&g_cb, &binds);
        // Dispatch compute shader
        neko_graphics_dispatch_compute(&g_cb, TEX_WIDTH / 16, TEX_HEIGHT / 16, 1);
    }

    neko_idraw_texture(&g_idraw, cmptex);
    neko_idraw_rectvd(&g_idraw, neko_v2(0.f, 0.f), neko_v2((float)TEX_WIDTH, (float)TEX_HEIGHT), neko_v2(0.f, 0.f), neko_v2(1.f, 1.f), NEKO_COLOR_WHITE, NEKO_GRAPHICS_PRIMITIVE_TRIANGLES);

#endif

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

        if (g_cvar.show_demo_window) neko_gui_overview(gui_ctx);

        auto pack_editor_func = [gui_ctx]() {
            if (neko_gui_begin(gui_ctx, "PackEditor", neko_gui_layout_get_bounds_ex(&g_gui, "PackEditor", neko_gui_rect(400, 200, 450, 400)),
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

                neko_gui_layout_row_static(gui_ctx, 30, 400, 1);

                if (neko_gui_button_label(gui_ctx, "Open") && !pack_reader_is_loaded) filebrowser = true;

                if (neko_gui_button_label(gui_ctx, "Close")) {
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

                    neko_gui_labelf_wrap(gui_ctx, "version: %d", pack_version);
                    neko_gui_labelf_wrap(gui_ctx, "little endian mode: %s", neko_bool_str(isLittleEndian));
                    neko_gui_labelf_wrap(gui_ctx, "file counts: %llu", (long long unsigned int)itemCount);

                    neko_gui_layout_row_dynamic(gui_ctx, 30, 1);

                    for (u64 i = 0; i < itemCount; ++i) {
                        neko_gui_labelf_wrap(gui_ctx, "%llu  %s %u", (long long unsigned int)i, neko_pack_item_path(&pack_reader, i), neko_pack_item_size(&pack_reader, i));

                        if (neko_gui_button_label(gui_ctx, "check")) {
                            item_current_idx = i;
                        }
                    }

                    neko_gui_layout_row_dynamic(gui_ctx, 30, 1);

                    if (item_current_idx >= 0) {
                        neko_gui_labelf_wrap(gui_ctx, "File index: %u", item_current_idx);
                        neko_gui_labelf_wrap(gui_ctx, "Path within package: %s", neko_pack_item_path(&pack_reader, item_current_idx));
                        neko_gui_labelf_wrap(gui_ctx, "File size: %u", neko_pack_item_size(&pack_reader, item_current_idx));
                    }

                } else if (!neko_pack_check(result)) {
                }
            }
            neko_gui_end(gui_ctx);
        };

        auto loading_window_func = [&]() {
            static u8 alpha = 0;

            alpha = alpha + 2;

            struct neko_gui_color bg_color = neko_gui_rgba(255, 255, 255, alpha);  // 128 表示透明度
            neko_gui_style_push_color(gui_ctx, &gui_ctx->style.window.fixed_background.data.color, bg_color);

            if (neko_gui_begin(gui_ctx, "Loading", neko_gui_layout_get_bounds_ex(&g_gui, "Loading", neko_gui_rect(fbs.x - 200, fbs.y - 80, 150, 40)),
                               NEKO_GUI_WINDOW_NO_SAVE | NEKO_GUI_WINDOW_NO_SCROLLBAR | NEKO_GUI_WINDOW_NO_INPUT | NEKO_GUI_WINDOW_NOT_INTERACTIVE)) {

                auto custom_draw_widget = [](struct neko_gui_context *ctx) {
                    struct neko_gui_command_buffer *canvas;
                    struct neko_gui_input *input = &ctx->input;
                    canvas = neko_gui_window_get_canvas(ctx);

                    struct neko_gui_rect space;
                    enum neko_gui_widget_layout_states state;
                    state = neko_gui_widget(&space, ctx);
                    if (!state) return;

                    neko_gui_fill_rect(canvas, space, 0, neko_gui_rgba(144, 0, 144, alpha));

                    neko_gui_draw_text(canvas, space, "加载中...", 9, ctx->style.font, neko_gui_rgb(0, 0, 0), neko_gui_rgb(255, 255, 255));
                };

                neko_gui_layout_row_static(gui_ctx, 30, 125, 1);

                custom_draw_widget(gui_ctx);
            }
            neko_gui_end(gui_ctx);

            neko_gui_style_pop_color(gui_ctx);
        };

        auto cvar_func = [gui_ctx]() {
            if (neko_gui_begin(gui_ctx, "Hello Neko", neko_gui_layout_get_bounds_ex(&g_gui, "Hello Neko", neko_gui_rect(30, 30, 350, 700)),
                               NEKO_GUI_WINDOW_BORDER | NEKO_GUI_WINDOW_MOVABLE | NEKO_GUI_WINDOW_SCALABLE | NEKO_GUI_WINDOW_MINIMIZABLE | NEKO_GUI_WINDOW_TITLE)) {

                enum { EASY, HARD };
                static int op = EASY;
                static int property = 20;
                static std::string str = "Hello shit";

                static std::vector<int> vec = {op, property};
                static std::map<std::string, int> mp = {{"AA", op}, {"BB", property}};
                static std::pair<std::string, int> pp = {"CC", property};
                static std::string *p_str = NULL;

                neko_gui_layout_row_static(gui_ctx, 30, 125, 1);

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

                neko_gui_layout_row_dynamic(gui_ctx, 300, 1);
                if (neko_gui_group_begin(gui_ctx, "ECS", NEKO_GUI_WINDOW_TITLE | NEKO_GUI_WINDOW_BORDER | NEKO_GUI_WINDOW_SCROLL_AUTO_HIDE)) {

                    neko_gui_layout_row_static(gui_ctx, 18, 125, 1);
                    // for (neko_ecs_ent_view v = neko_ecs_ent_view_single(neko_ecs(), COMPONENT_GAMEOBJECT); neko_ecs_ent_view_valid(&v); neko_ecs_ent_view_next(&v)) {
                    //     CGameObject *gameobj = (CGameObject *)neko_ecs_ent_get_component(neko_ecs(), v.ent, COMPONENT_GAMEOBJECT);
                    //     // neko_println("%d %d %d %d %s", v.i, v.ent, v.valid, v.view_type, gameobj->name);
                    //     neko_gui_selectable_label(gui_ctx, gameobj->name, NEKO_GUI_TEXT_CENTERED, (neko_gui_bool *)&gameobj->selected);
                    // }
                    neko_gui_group_end(gui_ctx);
                }

                neko_gui_layout_row_dynamic(gui_ctx, 300, 1);
                if (neko_gui_group_begin(gui_ctx, "CVars", NEKO_GUI_WINDOW_TITLE | NEKO_GUI_WINDOW_BORDER | NEKO_GUI_WINDOW_SCROLL_AUTO_HIDE)) {

                    neko_gui_layout_row_static(gui_ctx, 30, 150, 2);

                    if (neko_gui_button_label(gui_ctx, "test_fnt")) {
                        FILE *file = fopen("1.fnt", "rb");
                        neko_fnt *fnt = neko_font_fnt_read(file);
                        fclose(file);
                        if (NULL != fnt && fnt->num_glyphs > 0) {
                            neko_println("Got FNT! %i glyphs", fnt->num_glyphs);
                            // printf("First glyph is '%c' in page '%s'.\n", fnt->glyphs[0].ch, fnt->page_names[fnt->glyphs[0].page]);

                            for (int i = 0; i < fnt->num_glyphs; i += 10) {
                                neko_println("glyph '%u' in '%s'", fnt->glyphs[i].ch, fnt->page_names[fnt->glyphs[i].page]);
                            }
                        }
                        neko_font_fnt_free(fnt);
                    }

                    static cs_playing_sound_t pl;

                    if (neko_gui_button_label(gui_ctx, "test_audio")) pl = cs_play_sound(piano, params);

                    if (neko_gui_button_label(gui_ctx, "test_xml")) test_xml(game_assets("gamedir/tests/test.xml"));
                    if (neko_gui_button_label(gui_ctx, "test_se")) test_se();
                    if (neko_gui_button_label(gui_ctx, "test_sr")) test_sr();
                    if (neko_gui_button_label(gui_ctx, "test_ut")) test_ut();
                    if (neko_gui_button_label(gui_ctx, "test_backtrace")) __neko_inter_stacktrace();
                    if (neko_gui_button_label(gui_ctx, "test_containers")) test_containers();
                    if (neko_gui_button_label(gui_ctx, "test_sexpr")) test_sexpr();
                    if (neko_gui_button_label(gui_ctx, "test_nbt")) test_nbt();
                    if (neko_gui_button_label(gui_ctx, "test_thread")) test_thd();
                    if (neko_gui_button_label(gui_ctx, "test_sc")) {
                        neko_timer_do(t, neko_println("%llu", t), { test_sc(); });
                    }
                    if (neko_gui_button_label(gui_ctx, "test_ttf")) {
                        neko_timer_do(t, neko_println("%llu", t), { test_ttf(); });
                    }
                    if (neko_gui_button_label(gui_ctx, "test_hotload")) {

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
                    if (neko_gui_button_label(gui_ctx, "test_ecs_view")) {
                        // for (neko_ecs_ent_view v = neko_ecs_ent_view_single(neko_ecs(), COMPONENT_GAMEOBJECT); neko_ecs_ent_view_valid(&v); neko_ecs_ent_view_next(&v)) {
                        //     CGameObject *gameobj = (CGameObject *)neko_ecs_ent_get_component(neko_ecs(), v.ent, COMPONENT_GAMEOBJECT);
                        //     neko_println("%d %d %d %d %s", v.i, v.ent, v.valid, v.view_type, gameobj->name);
                        // }
                    }
                    if (neko_gui_button_label(gui_ctx, "test_ecs_cpp")) {
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

                    neko_gui_group_end(gui_ctx);
                }

                neko_gui_layout_row_dynamic(gui_ctx, 30, 2);
                if (neko_gui_option_label(gui_ctx, "easy", op == EASY)) op = EASY;
                if (neko_gui_option_label(gui_ctx, "hard", op == HARD)) op = HARD;

                neko_gui_layout_row_dynamic(gui_ctx, 25, 1);
                neko_gui_property_int(gui_ctx, "Compression:", 0, &property, 100, 10, 1);

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

                custom_draw_widget(gui_ctx);

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
            neko_gui_end(gui_ctx);
        };

        if (g_cvar.show_pack_editor) pack_editor_func();

        if (g_cvar.show_cvar_window) cvar_func();

        // loading_window_func();

        g_gui.mouse_is_hover = neko_gui_window_is_any_hovered(gui_ctx) || neko_gui_item_is_any_active(gui_ctx);

        // Render neko_nui commands into graphics command buffer
        neko_gui_render(&g_gui, &g_cb, NEKO_GUI_ANTI_ALIASING_ON);

        neko_graphics_renderpass_begin(&g_cb, NEKO_GRAPHICS_RENDER_PASS_DEFAULT);
        {
            neko_graphics_set_viewport(&g_cb, 0, 0, (u32)fbs.x, (u32)fbs.y);
            neko_core_ui_render(&g_core_ui, &g_cb);
        }
        neko_graphics_renderpass_end(&g_cb);

        neko_imgui_render(&g_imgui, &g_cb);

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
    try {
        neko_lua_wrap_call(g_L, "game_shutdown");
    } catch (std::exception &ex) {
        neko_log_error("lua exception %s", ex.what());
    }

    neko_scripting_end(g_L);

#ifdef GAME_CSHARP_ENABLED
    g_csharp.shutdown();
#endif

    neko_safe_free(font_verts);

    neko_font_free(test_font_bmfont);

    spritebatch_term(&sb);
    unload_images();

    neko_graphics_custom_batch_free(font_render);
    neko_graphics_custom_batch_free(sprite_batch);

    neko_immediate_draw_free(&g_idraw);

    neko_command_buffer_free(&g_cb);

    destroy_texture_handle(test_font_tex_id, NULL);

    // gl_free_frame_buffer(&sb_fb);

    neko_imgui_shutdown(&g_imgui);

    neko_gui_layout_save(&g_gui);

    //    if (is_hotfix_loaded) neko_hotload_module_close(ctx);

    neko_script_gc_freeall();
    for (int i = 0; i < neko_script_vector_size(g_vm.modules); i++) {
        neko_script_binary_free(g_vm.modules[i]);
    }
    neko_script_vector_free(g_vm.modules);

    neko_core_ui_free(&g_core_ui);

    neko_pack_destroy(&g_pack);
}

#define NEKO_ARGS_IMPL
#include "neko_args.h"

// interpreter
#include "interpreter.h"

neko_game_desc_t neko_main(s32 argc, char **argv) {

    neko_args_desc args = {
            .argc = argc,
            .argv = argv,
    };
    neko_args_setup(&args);

#define ARGS_STR(s0, s1) (0 == strcmp(s0, s1))

    if (ARGS_STR(neko_args_key_at(0), "packer")) {
    }

    if (ARGS_STR(neko_args_key_at(0), "vm")) {
        neko_vm_interpreter(neko_args_key_at(1));

        return neko_game_desc_t{.window = {.width = 1280, .height = 740, .vsync = true}, .argc = argc, .argv = argv, .console = &console};
    }

#undef ARGS_STR

    // 确定运行路径
    auto current_dir = std::filesystem::path(std::filesystem::current_path());
    for (int i = 0; i < 6; ++i)
        if (std::filesystem::exists(current_dir / "gamedir") &&  //
            std::filesystem::exists(current_dir / "gamedir" / "assets")) {
            data_path = neko_fs_normalize_path(current_dir.string());
            neko_log_info(std::format("gamedir: {0} (base: {1})", data_path, std::filesystem::current_path().string()).c_str());
            break;
        } else {
            current_dir = current_dir.parent_path();
        }

    return neko_game_desc_t{.init = game_init,
                            .update = game_update,
                            .shutdown = game_shutdown,
                            .window = {.width = 800, .height = 600, .vsync = false, .frame_rate = 60.f, .center = true},
                            .argc = argc,
                            .argv = argv,
                            .console = &console};
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
