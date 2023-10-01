

#include <filesystem>
#include <format>

// engine
#include "engine/neko.h"
#include "engine/neko_component.h"
#include "engine/neko_ecs.h"
#include "engine/neko_platform.h"
#include "engine/util/neko_gui.h"
#include "engine/util/neko_idraw.h"
#include "engine/util/neko_lua.h"

// game
#include "neko_hash.h"
#include "neko_scripting.h"
#include "neko_sprite.h"

// hpp
#include "hpp/neko_static_refl.hpp"

#define NEKO_IMGUI_IMPL
#include "neko_imgui.h"
#include "neko_imgui_utils.hpp"

neko_command_buffer_t g_cb = {0};
neko_gui_context_t g_gui = {0};
neko_imgui_context_t g_imgui = {};
neko_immediate_draw_t g_idraw = {0};

lua_State *L = nullptr;

neko_string data_path = {};
bool show_demo_window = false;

// Color picker
static float bg[3] = {90, 95, 100};

neko_global neko_sprite test_witch_spr = {0};

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
    };
};

void dockspace(neko_gui_context_t *ctx);

// test
void test_xml(const neko_string &file);
void test_sr();
void test_ut();
void test_se();

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

neko_ecs_decl_mask(MOVEMENT_SYSTEM, 3, COMPONENT_TRANSFORM, COMPONENT_VELOCITY, COMPONENT_SPRITE);
void movement_system(neko_ecs *ecs) {
    for (u32 i = 0; i < neko_ecs_for_count(ecs); i++) {
        neko_ecs_ent e = neko_ecs_get_ent(ecs, i);
        if (neko_ecs_ent_has_mask(ecs, e, neko_ecs_get_mask(MOVEMENT_SYSTEM))) {
            CTransform *xform = (CTransform *)neko_ecs_ent_get_component(ecs, e, COMPONENT_TRANSFORM);
            CVelocity *velocity = (CVelocity *)neko_ecs_ent_get_component(ecs, e, COMPONENT_VELOCITY);
            neko_sprite_renderer *sprite = (neko_sprite_renderer *)neko_ecs_ent_get_component(ecs, e, COMPONENT_SPRITE);

            // Grab global instance of engine
            neko_t *engine = neko_instance();

            neko_sprite_renderer_update(sprite, engine->ctx.platform->time.delta);

            xform->x += velocity->dx;
            xform->y += velocity->dy;

            velocity->dx /= 2.0f;
            velocity->dy /= 2.0f;
        }
    }
}

neko_ecs_decl_mask(SPRITE_RENDER_SYSTEM, 2, COMPONENT_TRANSFORM, COMPONENT_SPRITE);
void sprite_render_system(neko_ecs *ecs) {
    for (u32 i = 0; i < neko_ecs_for_count(ecs); i++) {
        neko_ecs_ent e = neko_ecs_get_ent(ecs, i);
        if (neko_ecs_ent_has_mask(ecs, e, neko_ecs_get_mask(SPRITE_RENDER_SYSTEM))) {
            CTransform *xform = (CTransform *)neko_ecs_ent_get_component(ecs, e, COMPONENT_TRANSFORM);
            neko_sprite_renderer *sprite = (neko_sprite_renderer *)neko_ecs_ent_get_component(ecs, e, COMPONENT_SPRITE);

            neko_graphics_t *gfx = neko_instance()->ctx.graphics;
            neko_command_buffer_t *cb = &g_cb;

            s32 index;
            if (sprite->loop) {
                index = sprite->loop->indices[sprite->current_frame];
            } else {
                index = sprite->current_frame;
            }

            neko_sprite *spr = sprite->sprite;
            neko_sprite_frame f = spr->frames[index];

            // gfx->immediate.draw_rect_textured_ext(cb, xform->x, xform->y, xform->x + spr->width * 4.f, xform->y + spr->height * 4.f, f.u0, f.v0, f.u1, f.v1, sprite->sprite->img.id,
            // neko_color_white);

            neko_idraw_rect_2d_textured_ext(&g_idraw, xform->x, xform->y, xform->x + spr->width * 4.f, xform->y + spr->height * 4.f, f.u0, f.v0, f.u1, f.v1, sprite->sprite->img.id, NEKO_COLOR_WHITE);

            // neko_idraw_texture(&gsi, sprite->sprite->img.id);
        }
    }
}

void register_components(neko_ecs *ecs) {
    // neko_ecs, component index, component pool size, size of component, and component free func
    neko_ecs_register_component(ecs, COMPONENT_TRANSFORM, 1000, sizeof(CTransform), NULL);
    neko_ecs_register_component(ecs, COMPONENT_VELOCITY, 200, sizeof(CVelocity), NULL);
    neko_ecs_register_component(ecs, COMPONENT_SPRITE, 1000, sizeof(neko_sprite_renderer), NULL);
    neko_ecs_register_component(ecs, COMPONENT_PARTICLE, 20, sizeof(neko_particle_renderer), NULL);
}

void register_systems(neko_ecs *ecs) {
    // ecs_run_systems will run the systems in the order they are registered
    // ecs_run_system is also available if you wish to handle each system seperately
    //
    // neko_ecs, function pointer to system (must take a parameter of neko_ecs), system type
    neko_ecs_register_system(ecs, movement_system, ECS_SYSTEM_UPDATE);
    neko_ecs_register_system(ecs, sprite_render_system, ECS_SYSTEM_RENDER_IMMEDIATE);
    // neko_ecs_register_system(ecs, particle_render_system, ECS_SYSTEM_RENDER_IMMEDIATE);
}

void game_init() {
    g_cb = neko_command_buffer_new();
    g_idraw = neko_immediate_draw_new();

    L = neko_scripting_init();

    {
        neko_lua_wrap_register_t<>(L).def(
                +[](const_str path) -> neko_string { return __neko_platform_get_path(path); }, "neko_file_path");

        // lua_newtable(L);
        // for (int n = 0; n < argc; ++n) {
        //     lua_pushstring(L, argv[n]);
        //     lua_rawseti(L, -2, n);
        // }
        // lua_setglobal(L, "arg");

        neko_lua_wrap_do_file(L, __neko_platform_get_path("data/scripts/main.lua"));

        neko_platform_running_desc_t t = {"Test", 1440, 840};

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

        neko_println("load game: %s %d %d", t.title, t.width, t.height);
    }

    try {
        neko_lua_wrap_call(L, "game_init");
    } catch (std::exception &ex) {
        neko_log_error("%s", ex.what());
    }

    neko_gui_init(&g_gui, neko_platform_main_window());

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
}

void game_update() {

    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());

    // Query for mouse position
    neko_vec2 mp = neko_platform_mouse_positionv();

    // Query for mouse wheel
    neko_vec2 mw = neko_platform_mouse_wheelv();

    // Query for mouse delta
    neko_vec2 md = neko_platform_mouse_deltav();

    // Query for mouse lock
    bool lock = neko_platform_mouse_locked();
    bool moved = neko_platform_mouse_moved();

    if (neko_platform_key_pressed(NEKO_KEYCODE_ESC)) {
        neko_quit();
    }

    neko_ecs_run_systems(neko_ecs(), ECS_SYSTEM_UPDATE);

    // Do rendering
    neko_graphics_clear_action_t clear_action = {.color = {bg[0] / 255, bg[1] / 255, bg[2] / 255, 1.f}};
    neko_graphics_clear_desc_t clear = {.actions = &clear_action};
    neko_graphics_renderpass_begin(&g_cb, neko_renderpass_t{0});
    { neko_graphics_clear(&g_cb, &clear); }
    neko_graphics_renderpass_end(&g_cb);

    // Set up 2D camera for projection matrix
    neko_idraw_camera2D(&g_idraw, (u32)fbs.x, (u32)fbs.y);

    neko_ecs_run_systems(neko_ecs(), ECS_SYSTEM_RENDER_IMMEDIATE);

    neko_graphics_renderpass_begin(&g_cb, neko_renderpass_t{0});
    {
        neko_graphics_set_viewport(&g_cb, 0, 0, (u32)fbs.x, (u32)fbs.y);
        neko_idraw_draw(&g_idraw, &g_cb);
    }
    neko_graphics_renderpass_end(&g_cb);

#pragma region gui

    neko_gui_begin(&g_gui, NULL);
    {
        dockspace(&g_gui);
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

#define GUI_WIDTHS(...)                           \
    []() -> const s32 * {                         \
        static s32 temp_widths[] = {__VA_ARGS__}; \
        return temp_widths;                       \
    }()

            neko_gui_layout_row(&g_gui, 1, GUI_WIDTHS(-1), 0);

            GUI_LABEL("Position: <%.2f %.2f>", mp.x, mp.y);
            GUI_LABEL("Wheel: <%.2f %.2f>", mw.x, mw.y);
            GUI_LABEL("Delta: <%.2f %.2f>", md.x, md.y);
            GUI_LABEL("Lock: %zu", lock);
            GUI_LABEL("Moved: %zu", moved);

            struct {
                const char *str;
                int32_t val;
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

            neko_gui_layout_row(&g_gui, 7, GUI_WIDTHS(100, 100, 32, 100, 32, 100, 32), 0);
            for (uint32_t i = 0; btns[i].str; ++i) {
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
    }
    neko_gui_end(&g_gui);

    // Do rendering

    neko_graphics_renderpass_begin(&g_cb, neko_renderpass_t{0});
    {
        neko_graphics_set_viewport(&g_cb, 0, 0, (int)fbs.x, (int)fbs.y);
        neko_gui_render(&g_gui, &g_cb);
    }
    neko_graphics_renderpass_end(&g_cb);

    neko_imgui_new_frame(&g_imgui);

    if (ImGui::Begin("Test")) {
        neko::imgui::Auto(show_demo_window, "ImGui Demo");
        if (ImGui::Button("test_xml")) test_xml(__neko_platform_get_path("data/test/test.xml"));
        if (ImGui::Button("test_se")) test_se();
        if (ImGui::Button("test_sr")) test_sr();
        if (ImGui::Button("test_ut")) test_ut();
    }
    ImGui::End();

    if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

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

    neko_scripting_end(L);
}

neko_game_desc_t neko_main(int32_t argc, char **argv) {

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

    return neko_game_desc_t{.init = game_init, .update = game_update, .shutdown = game_shutdown, .window = {.width = 1024, .height = 760}, .argc = argc, .argv = argv};
}

void dockspace(neko_gui_context_t *ctx) {
    int32_t opt = NEKO_GUI_OPT_NOCLIP | NEKO_GUI_OPT_NOFRAME | NEKO_GUI_OPT_FORCESETRECT | NEKO_GUI_OPT_NOTITLE | NEKO_GUI_OPT_DOCKSPACE | NEKO_GUI_OPT_FULLSCREEN | NEKO_GUI_OPT_NOMOVE |
                  NEKO_GUI_OPT_NOBRINGTOFRONT | NEKO_GUI_OPT_NOFOCUS | NEKO_GUI_OPT_NORESIZE;
    neko_gui_window_begin_ex(ctx, "Dockspace", neko_gui_rect(350, 40, 600, 500), NULL, NULL, opt);
    {
        // Empty dockspace...
    }
    neko_gui_window_end(ctx);
}
