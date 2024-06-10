

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
#include "engine/neko_platform.h"

// binding
#include "engine/neko_api.hpp"
#include "engine/neko_lua.hpp"
#include "engine/neko_reflection.hpp"

// game
#include "game_editor.h"
#include "game_main.h"
#include "game_physics_math.hpp"
#include "helper.h"

// opengl
#include <glad/glad.h>

#define NEKO_IMGUI_IMPL
#include "engine/neko_imgui.hpp"

using namespace neko;

namespace neko::reflection {
template <>
Type *type_of<neko_platform_running_desc_t>() {
    static Type type;
    type.name = "neko_platform_running_desc_t";
    type.destroy = [](void *obj) { delete static_cast<neko_platform_running_desc_t *>(obj); };
    type.copy = [](const void *obj) { return (void *)(new neko_platform_running_desc_t(*static_cast<const neko_platform_running_desc_t *>(obj))); };
    type.move = [](void *obj) { return (void *)(new neko_platform_running_desc_t(std::move(*static_cast<neko_platform_running_desc_t *>(obj)))); };

#define REFL_FIELDS(C, field) type.fields.insert({#field, {type_of<decltype(C::field)>(), offsetof(C, field)}})

    REFL_FIELDS(neko_platform_running_desc_t, title);
    REFL_FIELDS(neko_platform_running_desc_t, width);
    REFL_FIELDS(neko_platform_running_desc_t, height);
    REFL_FIELDS(neko_platform_running_desc_t, flags);
    REFL_FIELDS(neko_platform_running_desc_t, num_samples);
    REFL_FIELDS(neko_platform_running_desc_t, monitor_index);
    REFL_FIELDS(neko_platform_running_desc_t, vsync);
    REFL_FIELDS(neko_platform_running_desc_t, frame_rate);

    return &type;
};
}  // namespace neko::reflection

// user data

neko_client_userdata_t g_client_userdata = NEKO_DEFAULT_VAL();

void editor_dockspace(neko_ui_context_t *ctx);

NEKO_API_DECL void neko_console(neko_console_t *console, neko_ui_context_t *ctx, neko_ui_rect_t screen, const neko_ui_selector_desc_t *desc);

// test
void test_xml(const std::string &file);
void test_ut();
void test_se();
void test_containers();
void test_thd();
void test_fgd();
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

void app_load_style_sheet(bool destroy) {
    if (destroy) {
        neko_ui_style_sheet_destroy(&CL_GAME_USERDATA()->style_sheet);
    }
    CL_GAME_USERDATA()->style_sheet = neko_ui_style_sheet_load_from_file(&CL_GAME_USERDATA()->ui, game_assets("gamedir/style_sheets/gui.ss").c_str());
    neko_ui_set_style_sheet(&CL_GAME_USERDATA()->ui, &CL_GAME_USERDATA()->style_sheet);
}

void movement_system(ecs_view_t view, unsigned int row) {
    position_t *p = (position_t *)ecs_view(view, row, 0);
    velocity_t *v = (velocity_t *)ecs_view(view, row, 1);
    bounds_t *b = (bounds_t *)ecs_view(view, row, 2);
    color_t *c = (color_t *)ecs_view(view, row, 3);

    neko_vec2_t min = neko_vec2_add(*p, *v);
    neko_vec2_t max = neko_vec2_add(min, *b);

    auto ws = CL_GAME_USERDATA()->fbs;

    // Resolve collision and change velocity direction if necessary
    if (min.x < 0 || max.x >= ws.x) {
        v->x *= -1.f;
    }
    if (min.y < 0 || max.y >= ws.y) {
        v->y *= -1.f;
    }
    *p = neko_vec2_add(*p, *v);
}

void render_system(ecs_view_t view, unsigned int row) {
    position_t *p = (position_t *)ecs_view(view, row, 0);
    velocity_t *v = (velocity_t *)ecs_view(view, row, 1);
    bounds_t *b = (bounds_t *)ecs_view(view, row, 2);
    color_t *c = (color_t *)ecs_view(view, row, 3);

    auto pi = *p;
    auto bi = *b;
    neko_idraw_rectvd(&CL_GAME_USERDATA()->idraw, pi, bi, neko_v2(0.f, 0.f), neko_v2(1.f, 1.f), (neko_color_t)*c, R_PRIMITIVE_TRIANGLES);
}

void aseprite_render_system(ecs_view_t view, unsigned int row) { position_t *p = (position_t *)ecs_view(view, row, 0); }

#if 0

neko_ecs_decl_system(movement_system, MOVEMENT_SYSTEM, 4, COMPONENT_TRANSFORM, COMPONENT_VELOCITY, COMPONENT_BOUNDS, COMPONENT_COLOR) {

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
            neko_idraw_rectvd(&CL_GAME_USERDATA()->idraw, pi, bi, neko_v2(0.f, 0.f), neko_v2(1.f, 1.f), (neko_color_t)*c, R_PRIMITIVE_TRIANGLES);
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

#endif

void neko_register(lua_State *L);

lua_State *neko_scripting_init() {
    neko::timer timer;
    timer.start();

    lua_State *L = neko_lua_create();

    lua_atpanic(
            L, +[](lua_State *L) {
                auto msg = neko_lua_to<const_str>(L, -1);
                NEKO_ERROR("[lua] panic error: %s", msg);
                return 0;
            });
    neko_register(L);

    neko_lua_run_string(L, std::format("package.path = "
                                       "'{1}/?.lua;{0}/?.lua;{0}/../libs/?.lua;{0}/../libs/?/init.lua;{0}/../libs/"
                                       "?/?.lua;' .. package.path",
                                       game_assets("lua_scripts"), neko::fs_normalize_path(std::filesystem::current_path().string()).c_str()));

    neko_lua_run_string(L, std::format("package.cpath = "
                                       "'{1}/?.{2};{0}/?.{2};{0}/../libs/?.{2};{0}/../libs/?/init.{2};{0}/../libs/"
                                       "?/?.{2};' .. package.cpath",
                                       game_assets("lua_scripts"), neko::fs_normalize_path(std::filesystem::current_path().string()).c_str(), "dll"));

    neko_lua_safe_dofile(L, "init");

    timer.stop();
    NEKO_INFO(std::format("lua init done in {0:.3f} ms", timer.get()).c_str());

    return L;
}

void neko_scripting_end(lua_State *L) { neko_lua_fini(L); }

void draw_gui();

int init_work(void *user_data) {

    neko_thread_atomic_int_t *this_init_thread_flag = (neko_thread_atomic_int_t *)user_data;

    if (thread_atomic_int_load(this_init_thread_flag) == 0) {

        neko_thread_timer_t thread_timer;
        thread_timer_init(&thread_timer);

        neko::timer timer;
        timer.start();

        neko_lua_call(neko_instance()->L, "game_init_thread");

        timer.stop();
        NEKO_INFO(std::format("game_init_thread loading done in {0:.3f} ms", timer.get()).c_str());

        // thread_timer_wait(&thread_timer, 1000000000);  // sleep for a second

        thread_timer_term(&thread_timer);

        thread_atomic_int_store(this_init_thread_flag, 1);
    }

    return 0;
}

void game_init() {

    neko_userdata() = CL_GAME_USERDATA();

    auto mount = neko::vfs_mount(game_assets("gamedir/../").c_str());
    neko_instance()->L = neko_scripting_init();

    CL_GAME_USERDATA()->cb = neko_command_buffer_new();
    CL_GAME_USERDATA()->idraw = neko_immediate_draw_new();

    CL_GAME_USERDATA()->am = neko_asset_manager_new();

    neko_pack_read(game_assets("gamedir/sc.pack").c_str(), 0, false, &CL_GAME_USERDATA()->lua_pack);

    {
        neko::lua_bind::bind("__neko_file_path", +[](const_str path) -> std::string { return game_assets(path); });

        lua_newtable(neko_instance()->L);
        for (int n = 0; n < neko_instance()->argc; ++n) {
            lua_pushstring(neko_instance()->L, neko_instance()->argv[n]);
            lua_rawseti(neko_instance()->L, -2, n);
        }
        lua_setglobal(neko_instance()->L, "arg");

        neko_lua_safe_dofile(neko_instance()->L, "main");

        // 获取 neko_game.table
        lua_getglobal(neko_instance()->L, "neko_game");
        if (!lua_istable(neko_instance()->L, -1)) {
            NEKO_ERROR("%s", "neko_game is not a table");
        }

        neko::reflection::Any v = neko_platform_running_desc_t{.title = "Neko Engine"};
        if (lua_getfield(neko_instance()->L, -1, "app") == LUA_TNIL) {
            NEKO_ERROR("[exception] %s", "no app");
        }
        if (lua_istable(neko_instance()->L, -1)) {
            // neko::static_refl::neko_type_info<neko_platform_running_desc_t>::ForEachVarOf(t, [](auto field, auto &&value) {
            //     static_assert(std::is_lvalue_reference_v<decltype(value)>);
            //     if (lua_getfield(neko_instance()->L, -1, std::string(field.name).c_str()) != LUA_TNIL) value = neko_lua_to<std::remove_reference_t<decltype(value)>>(neko_instance()->L, -1);
            //     lua_pop(neko_instance()->L, 1);
            // });

            auto f = [](std::string_view name, neko::reflection::Any &value) {
                static_assert(std::is_lvalue_reference_v<decltype(value)>);
                // if (value.GetType() == neko::reflection::type_of<std::string_view>()) {
                //     std::cout << name << " = " << value.cast<std::string_view>() << std::endl;
                // } else if (value.GetType() == neko::reflection::type_of<std::size_t>()) {
                //     std::cout << name << " = " << value.cast<std::size_t>() << std::endl;
                // }
                if (lua_getfield(neko_instance()->L, -1, std::string(name).c_str()) != LUA_TNIL) {

                    auto ff = [&]<typename S>(const char *name, neko::reflection::Any &var, S &t) {
                        if (value.GetType() == neko::reflection::type_of<S>()) {
                            S s = neko_lua_to<std::remove_reference_t<S>>(neko_instance()->L, -1);
                            value.cast<S>() = s;
                            NEKO_DEBUG_LOG("%s : %d", neko::reflection::name_v<std::remove_reference_t<S>>.data(), s);
                        }
                    };

#define FUCK_TYPES() u32, b32, f32, bool, const_str

                    std::apply([&](auto &&...args) { (ff(std::string(name).c_str(), value, args), ...); }, std::tuple<FUCK_TYPES()>());
                }
                lua_pop(neko_instance()->L, 1);
            };

            v.foreach (f);

        } else {
            // throw std::exception();
            NEKO_ERROR("[exception] %s", "no app table");
        }
        lua_pop(neko_instance()->L, 1);

        neko_client_cvar_t &cvar = CL_GAME_USERDATA()->cl_cvar;
        // if (lua_getfield(neko_instance()->L, -1, "cvar") == LUA_TNIL) throw std::exception("no cvar");
        // if (lua_istable(neko_instance()->L, -1)) {
        //     neko::static_refl::neko_type_info<neko_client_cvar_t>::ForEachVarOf(cvar, [](auto field, auto &&value) {
        //         static_assert(std::is_lvalue_reference_v<decltype(value)>);
        //         if (lua_getfield(neko_instance()->L, -1, std::string(field.name).c_str()) != LUA_TNIL) value = neko_lua_to<std::remove_reference_t<decltype(value)>>(neko_instance()->L, -1);
        //         lua_pop(neko_instance()->L, 1);
        //     });
        // } else {
        //     throw std::exception("no cvar table");
        // }
        // lua_pop(neko_instance()->L, 1);

        cvar.show_editor = false;
        cvar.show_demo_window = false;
        cvar.show_pack_editor = false;
        cvar.show_profiler_window = false;
        cvar.show_test_window = false;
        cvar.show_gui = false;
        cvar.shader_inspect = false;
        cvar.hello_ai_shit = false;
        cvar.vsync = false;
        cvar.is_hotfix = false;

        lua_pop(neko_instance()->L, 1);  // 弹出 neko_game.table

        NEKO_INFO("load game: %s %d %d", v.cast<neko_platform_running_desc_t>().title, v.cast<neko_platform_running_desc_t>().width, v.cast<neko_platform_running_desc_t>().height);

        neko_platform_set_window_title(neko_platform_main_window(), v.cast<neko_platform_running_desc_t>().title);
    }

    CL_GAME_USERDATA()->fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());

    neko_lua_call(neko_instance()->L, "game_init");

    neko_pack_read(game_assets("gamedir/res.pack").c_str(), 0, false, &CL_GAME_USERDATA()->pack);

    u8 *font_data, *cat_data;
    u32 font_data_size, cat_data_size;

    neko_pack_item_data(&CL_GAME_USERDATA()->pack, ".\\data\\assets\\fonts\\fusion-pixel-12px-monospaced.ttf", (const u8 **)&font_data, &font_data_size);
    neko_pack_item_data(&CL_GAME_USERDATA()->pack, ".\\data\\assets\\textures\\cat.aseprite", (const u8 **)&cat_data, &cat_data_size);

    CL_GAME_USERDATA()->test_ase = neko_aseprite_simple(cat_data, cat_data_size);

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

    neko_ui_init_font_stash(&CL_GAME_USERDATA()->ui, GUI_FONT_STASH);

    app_load_style_sheet(false);

    neko_ui_dock_ex(&CL_GAME_USERDATA()->ui, "Style_Editor", "Demo_Window", NEKO_UI_SPLIT_TAB, 0.5f);

    CL_GAME_USERDATA()->imgui = neko_imgui_new(&CL_GAME_USERDATA()->cb, neko_platform_main_window(), false);

    // Load a file

    neko::string contents = {};
    bool ok = vfs_read_entire_file(&contents, "gamedir/1.fnt");
    NEKO_ASSERT(ok);
    neko_fontbatch_init(&CL_GAME_USERDATA()->font_render_batch, CL_GAME_USERDATA()->fbs, game_assets("gamedir/1_0.png").c_str(), contents.data, (s32)contents.len);
    neko_defer(neko_safe_free(contents.data));

    // Construct frame buffer
    CL_GAME_USERDATA()->main_fbo = neko_render_framebuffer_create({});

    neko_render_texture_desc_t main_rt_desc = {.width = (u32)CL_GAME_USERDATA()->fbs.x,
                                               .height = (u32)CL_GAME_USERDATA()->fbs.y,
                                               .format = R_TEXTURE_FORMAT_RGBA8,
                                               .wrap_s = R_TEXTURE_WRAP_REPEAT,
                                               .wrap_t = R_TEXTURE_WRAP_REPEAT,
                                               .min_filter = R_TEXTURE_FILTER_NEAREST,
                                               .mag_filter = R_TEXTURE_FILTER_NEAREST};
    CL_GAME_USERDATA()->main_rt = neko_render_texture_create(main_rt_desc);

    neko_render_renderpass_desc_t main_rp_desc = {.fbo = CL_GAME_USERDATA()->main_fbo, .color = &CL_GAME_USERDATA()->main_rt, .color_size = sizeof(CL_GAME_USERDATA()->main_rt)};
    CL_GAME_USERDATA()->main_rp = neko_render_renderpass_create(main_rp_desc);

    const neko_vec2_t ws = neko_platform_window_sizev(neko_platform_main_window());

    CL_GAME_USERDATA()->ecs = ecs_init(neko_instance()->L);

    auto registry = CL_GAME_USERDATA()->ecs;
    const ecs_entity_t pos_component = ECS_COMPONENT(registry, position_t);
    const ecs_entity_t vel_component = ECS_COMPONENT(registry, velocity_t);
    const ecs_entity_t bou_component = ECS_COMPONENT(registry, bounds_t);
    const ecs_entity_t col_component = ECS_COMPONENT(registry, color_t);
    const ecs_entity_t obj_component = ECS_COMPONENT(registry, CGameObjectTest);

    for (int i = 0; i < 5; i++) {

        neko_vec2_t bounds = neko_v2((f32)random_val(10, 100), (f32)random_val(10, 100));

        position_t p = {(f32)random_val(0, (int32_t)ws.x - (int32_t)bounds.x), (f32)random_val(0, (int32_t)ws.y - (int32_t)bounds.y)};
        velocity_t v = {(f32)random_val(1, 10), (f32)random_val(1, 10)};
        bounds_t b = {(f32)random_val(10, 100), (f32)random_val(10, 100)};
        color_t c = {(u8)random_val(50, 255), (u8)random_val(50, 255), (u8)random_val(50, 255), 255};
        CGameObjectTest gameobj = NEKO_DEFAULT_VAL();

        neko_snprintf(gameobj.name, 64, "%s_%d", "Test_ent", i);
        gameobj.visible = true;
        gameobj.active = false;

        ecs_entity_t e = ecs_entity(registry);

        ecs_attach(registry, e, pos_component);
        ecs_attach(registry, e, vel_component);
        ecs_attach(registry, e, bou_component);
        ecs_attach(registry, e, col_component);
        ecs_attach(registry, e, obj_component);

        ecs_set(registry, e, pos_component, &p);
        ecs_set(registry, e, vel_component, &v);
        ecs_set(registry, e, bou_component, &b);
        ecs_set(registry, e, col_component, &c);
        ecs_set(registry, e, obj_component, &gameobj);
    }

    ECS_SYSTEM(registry, movement_system, 4, pos_component, vel_component, bou_component, col_component);
    ECS_SYSTEM(registry, render_system, 4, pos_component, vel_component, bou_component, col_component);

    // const ecs_entity_t test_component = ECS_COMPONENT(registry, uptr);
    // for (int i = 0; i < 1024; i++) {
    //     ecs_entity_t e = ecs_entity(registry);
    //     ecs_attach(registry, e, test_component);
    // }

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

        neko_render_clear_action_t clear = {.color = {CL_GAME_USERDATA()->cl_cvar.bg[0] / 255, CL_GAME_USERDATA()->cl_cvar.bg[1] / 255, CL_GAME_USERDATA()->cl_cvar.bg[2] / 255, 1.f}};
        neko_render_renderpass_begin(&CL_GAME_USERDATA()->cb, neko_renderpass_t{0});
        { neko_render_clear(&CL_GAME_USERDATA()->cb, clear); }
        neko_render_renderpass_end(&CL_GAME_USERDATA()->cb);

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
                          neko_v2(1.f, 0.f), neko_color(255, 255, 255, tranp), R_PRIMITIVE_TRIANGLES);

        neko_render_renderpass_begin(&CL_GAME_USERDATA()->cb, R_RENDER_PASS_DEFAULT);
        {
            neko_render_set_viewport(&CL_GAME_USERDATA()->cb, 0, 0, (u32)CL_GAME_USERDATA()->fbs.x, (u32)CL_GAME_USERDATA()->fbs.y);
            neko_idraw_draw(&CL_GAME_USERDATA()->idraw, &CL_GAME_USERDATA()->cb);  // 立即模式绘制 idraw
        }
        neko_render_renderpass_end(&CL_GAME_USERDATA()->cb);

        // Submits to cb
        neko_render_command_buffer_submit(&CL_GAME_USERDATA()->cb);

    } else {

        static int init_retval = 1;
        if (init_retval) {
            init_retval = thread_join(CL_GAME_USERDATA()->init_work_thread);
            thread_term(CL_GAME_USERDATA()->init_work_thread);
            NEKO_TRACE("init_work_thread done");
        }

        //        neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());
        neko_vec2 win_size = neko_platform_window_sizev(neko_platform_main_window());

        f32 dt = neko_platform_delta_time();

        neko_lua_call(neko_instance()->L, "game_pre_update");

        if (neko_platform_key_pressed(NEKO_KEYCODE_ESC)) CL_GAME_USERDATA()->cl_cvar.show_editor ^= true;

        neko_lua_call<void, f32>(neko_instance()->L, "game_loop", dt);

        // Do rendering
        neko_render_clear_action_t clear = {.color = {CL_GAME_USERDATA()->cl_cvar.bg[0] / 255, CL_GAME_USERDATA()->cl_cvar.bg[1] / 255, CL_GAME_USERDATA()->cl_cvar.bg[2] / 255, 1.f}};
        neko_render_clear_action_t b_clear = {.color = {0.0f, 0.0f, 0.0f, 1.f}};

        neko_render_renderpass_begin(&CL_GAME_USERDATA()->cb, R_RENDER_PASS_DEFAULT);
        { neko_render_clear(&CL_GAME_USERDATA()->cb, clear); }
        neko_render_renderpass_end(&CL_GAME_USERDATA()->cb);

        neko_imgui_new_frame(&CL_GAME_USERDATA()->imgui);

        draw_gui();

        if (ImGui::Begin("Debug")) {
            neko::imgui::Auto<neko_vec2_t>(CL_GAME_USERDATA()->cam, "Cam");
            if (ImGui::Button("ECS")) {
                ecs_inspect(CL_GAME_USERDATA()->ecs);
            }
            ImGui::Text("%lld", neko_lua_mem_usage());
            ImGui::End();
        }

        if (CL_GAME_USERDATA()->cl_cvar.show_demo_window) ImGui::ShowDemoWindow();

        // neko_imgui::Auto(g_cvar.bg);
        if (ImGui::Begin("Shaders")) {
            neko_gl_data_t *ogl = neko_render_userdata();
            for (neko_slot_array_iter it = neko_slot_array_iter_new(ogl->shaders); neko_slot_array_iter_valid(ogl->shaders, it); neko_slot_array_iter_advance(ogl->shaders, it)) {
                neko_gl_shader_t s = neko_slot_array_iter_get(ogl->shaders, it);
                inspect_shader(std::to_string(s).c_str(), s);
            }
            ImGui::End();
        }

        if (CL_GAME_USERDATA()->cl_cvar.show_editor && ImGui::Begin("Cvar")) {

            neko_cvar_gui(CL_GAME_USERDATA()->cl_cvar);

            neko::imgui::toggle("帧检查器", &CL_GAME_USERDATA()->cl_cvar.show_profiler_window);

            ImGui::DragFloat("Max FPS", &neko_subsystem(platform)->time.max_fps, 5.0f, 30.0f, 2000.0f);

            ImGui::Separator();

            if (ImGui::Button("test_xml")) test_xml(game_assets("gamedir/tests/test.xml"));
            if (ImGui::Button("test_fgd")) test_fgd();
            if (ImGui::Button("test_se")) test_se();
            if (ImGui::Button("test_ut")) test_ut();
            if (ImGui::Button("test_containers")) test_containers();
            if (ImGui::Button("test_thread")) test_thd();
            if (ImGui::Button("test_ttf")) {
                // neko_timer_do(t, neko_println("%llu", t), { test_ttf(); });
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

        if (CL_GAME_USERDATA()->cl_cvar.show_editor) {
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("engine")) {
                    if (ImGui::Checkbox("vsync", &CL_GAME_USERDATA()->cl_cvar.vsync)) neko_platform_enable_vsync(CL_GAME_USERDATA()->cl_cvar.vsync);
                    if (ImGui::MenuItem("quit")) {
                        ImGui::OpenPopup("Delete?");
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
        }

        //        sandbox_update(sb);

        neko_idraw_defaults(&CL_GAME_USERDATA()->idraw);
        neko_idraw_camera2d_ex(&CL_GAME_USERDATA()->idraw, CL_GAME_USERDATA()->cam.x, CL_GAME_USERDATA()->cam.x + CL_GAME_USERDATA()->fbs.x, CL_GAME_USERDATA()->cam.y,
                               CL_GAME_USERDATA()->cam.y + CL_GAME_USERDATA()->fbs.y);

        neko_render_renderpass_begin(&CL_GAME_USERDATA()->cb, CL_GAME_USERDATA()->main_rp);
        {
            neko_render_set_viewport(&CL_GAME_USERDATA()->cb, 0, 0, (u32)CL_GAME_USERDATA()->fbs.x, (u32)CL_GAME_USERDATA()->fbs.y);
            neko_render_clear(&CL_GAME_USERDATA()->cb, clear);
            neko_idraw_draw(&CL_GAME_USERDATA()->idraw, &CL_GAME_USERDATA()->cb);  // 立即模式绘制 idraw
            neko_render_draw_batch(&CL_GAME_USERDATA()->cb, CL_GAME_USERDATA()->font_render_batch.font_render, 0, 0, 0);
        }
        neko_render_renderpass_end(&CL_GAME_USERDATA()->cb);

        neko_idraw_defaults(&CL_GAME_USERDATA()->idraw);
        neko_idraw_camera2d(&CL_GAME_USERDATA()->idraw, (u32)win_size.x, (u32)win_size.y);
        neko_idraw_texture(&CL_GAME_USERDATA()->idraw, CL_GAME_USERDATA()->main_rt);
        neko_idraw_rectvd(&CL_GAME_USERDATA()->idraw, neko_v2(0.0, 0.0), neko_v2((u32)win_size.x, (u32)win_size.y), neko_v2(0.0, 1.0), neko_v2(1.0, 0.0), neko_color(255, 255, 255, 255),
                          R_PRIMITIVE_TRIANGLES);

        const neko_vec2_t ws = neko_platform_window_sizev(neko_platform_main_window());

        neko_idraw_defaults(&CL_GAME_USERDATA()->idraw);
        neko_idraw_camera2d(&CL_GAME_USERDATA()->idraw, ws.x, ws.y);

        // ecs_progress(CL_GAME_USERDATA()->ecs_world, 0);
        // neko_ecs_run_systems(CL_GAME_USERDATA()->ecs, ECS_SYSTEM_UPDATE);
        // neko_ecs_run_systems(CL_GAME_USERDATA()->ecs, ECS_SYSTEM_RENDER_IMMEDIATE);
        ecs_step(CL_GAME_USERDATA()->ecs);

        neko_render_renderpass_begin(&CL_GAME_USERDATA()->cb, R_RENDER_PASS_DEFAULT);
        {
            neko_render_set_viewport(&CL_GAME_USERDATA()->cb, 0.0, 0.0, win_size.x, win_size.y);
            neko_idraw_draw(&CL_GAME_USERDATA()->idraw, &CL_GAME_USERDATA()->cb);  // 立即模式绘制 idraw
            neko_ui_render(&CL_GAME_USERDATA()->ui, &CL_GAME_USERDATA()->cb);
        }
        neko_render_renderpass_end(&CL_GAME_USERDATA()->cb);

        neko_lua_call(neko_instance()->L, "game_render");

        neko_imgui_render(&CL_GAME_USERDATA()->imgui);

        // Submits to cb
        neko_render_command_buffer_submit(&CL_GAME_USERDATA()->cb);

        neko_check_gl_error();
    }
}

void game_shutdown() {

    neko_render_renderpass_destroy(CL_GAME_USERDATA()->main_rp);
    neko_render_texture_destroy(CL_GAME_USERDATA()->main_rt);
    neko_render_framebuffer_destroy(CL_GAME_USERDATA()->main_fbo);

    neko_lua_call(neko_instance()->L, "game_shutdown");

    neko_fontbatch_end(&CL_GAME_USERDATA()->font_render_batch);

    neko_immediate_draw_free(&CL_GAME_USERDATA()->idraw);
    neko_command_buffer_free(&CL_GAME_USERDATA()->cb);

    // ecs_fini(CL_GAME_USERDATA()->ecs_world);
    // ecs_fini(CL_GAME_USERDATA()->ecs);

    neko_imgui_shutdown(&CL_GAME_USERDATA()->imgui);
    neko_ui_free(&CL_GAME_USERDATA()->ui);
    neko_pack_destroy(&CL_GAME_USERDATA()->pack);
    neko_pack_destroy(&CL_GAME_USERDATA()->lua_pack);

    neko::vfs_trash();

    neko_scripting_end(neko_instance()->L);

#ifdef USE_PROFILER
    neko::profile_shutdown();
#endif
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

std::string game_assets(const std::string &path) {
    std::string get_path;

    if (path.substr(0, 11) == "lua_scripts") {
        std::string lua_path = path;
        get_path.append("./source/lua/game").append(lua_path.replace(0, 12, ""));
    } else {
        get_path.append(path);
    }

    return get_path;
}

///////////////////////////////////////////////
//
//  测试UI

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

void draw_gui() {

    const f64 t = neko_platform_elapsed_time();

    // Custom callback for immediate drawing directly into the gui window
    auto gui_cb = [](neko_ui_context_t *ctx, struct neko_ui_customcommand_t *cmd) {
        neko_immediate_draw_t *gui_idraw = &ctx->gui_idraw;  // Immediate draw list in gui context
        neko_vec2 fbs = ctx->framebuffer_size;               // Framebuffer size bound for gui context
        neko_color_t *color = (neko_color_t *)cmd->data;     // Grab custom data
        neko_asset_texture_t *tp = neko_assets_getp(&CL_GAME_USERDATA()->am, neko_asset_texture_t, CL_GAME_USERDATA()->tex_hndl);
        const f32 t = neko_platform_elapsed_time();

        // Set up an immedaite camera using our passed in cmd viewport (this is the clipped viewport of the gui window being drawn)
        neko_idraw_camera3d(gui_idraw, (u32)cmd->viewport.w, (u32)cmd->viewport.h);
        neko_idraw_blend_enabled(gui_idraw, true);
        neko_render_set_viewport(&gui_idraw->commands, cmd->viewport.x, fbs.y - cmd->viewport.h - cmd->viewport.y, cmd->viewport.w, cmd->viewport.h);
        neko_idraw_push_matrix(gui_idraw, NEKO_IDRAW_MATRIX_MODELVIEW);
        {
            neko_idraw_rotatev(gui_idraw, t * 0.001f, NEKO_YAXIS);
            neko_idraw_scalef(gui_idraw, 0.5f, 0.5f, 0.5f);
            neko_idraw_box(gui_idraw, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, color->r, color->g, color->b, color->a, R_PRIMITIVE_LINES);
        }
        neko_idraw_pop_matrix(gui_idraw);

        // Set up 2D camera for projection matrix
        neko_idraw_camera2d(gui_idraw, (u32)fbs.x, (u32)fbs.y);

        // Rect
        neko_idraw_rectv(gui_idraw, neko_v2(500.f, 50.f), neko_v2(600.f, 100.f), NEKO_COLOR_RED, R_PRIMITIVE_TRIANGLES);
        neko_idraw_rectv(gui_idraw, neko_v2(650.f, 50.f), neko_v2(750.f, 100.f), NEKO_COLOR_RED, R_PRIMITIVE_LINES);

        // Triangle
        neko_idraw_trianglev(gui_idraw, neko_v2(50.f, 50.f), neko_v2(100.f, 100.f), neko_v2(50.f, 100.f), NEKO_COLOR_WHITE, R_PRIMITIVE_TRIANGLES);
        neko_idraw_trianglev(gui_idraw, neko_v2(200.f, 50.f), neko_v2(300.f, 100.f), neko_v2(200.f, 100.f), NEKO_COLOR_WHITE, R_PRIMITIVE_LINES);

        // Lines
        neko_idraw_linev(gui_idraw, neko_v2(50.f, 20.f), neko_v2(500.f, 20.f), neko_color(0, 255, 0, 255));

        // Circle
        neko_idraw_circle(gui_idraw, 350.f, 170.f, 50.f, 20, 100, 150, 220, 255, R_PRIMITIVE_TRIANGLES);
        neko_idraw_circle(gui_idraw, 250.f, 170.f, 50.f, 20, 100, 150, 220, 255, R_PRIMITIVE_LINES);

        // Circle Sector
        neko_idraw_circle_sector(gui_idraw, 50.f, 150.f, 50.f, 0, 90, 32, 255, 255, 255, 255, R_PRIMITIVE_TRIANGLES);
        neko_idraw_circle_sector(gui_idraw, 150.f, 200.f, 50.f, 90, 270, 32, 255, 255, 255, 255, R_PRIMITIVE_LINES);

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
            neko_idraw_box(gui_idraw, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 255, 255, 255, R_PRIMITIVE_TRIANGLES);
            neko_idraw_texture(gui_idraw, neko_handle(neko_render_texture_t){0});
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
            neko_idraw_box(gui_idraw, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 200, 100, 255, R_PRIMITIVE_LINES);
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
            neko_idraw_sphere(gui_idraw, 0.f, 0.f, 0.f, 1.0f, 255, 255, 255, 50, R_PRIMITIVE_TRIANGLES);
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
            neko_idraw_sphere(gui_idraw, 0.f, 0.f, 0.f, 1.0f, 255, 255, 255, 50, R_PRIMITIVE_LINES);
        }
        neko_idraw_pop_matrix(gui_idraw);

        // Text (custom and default fonts)
        // neko_idraw_camera2D(gui_idraw, (u32)ws.x, (u32)ws.y);
        // neko_idraw_defaults(gui_idraw);
        // neko_idraw_text(gui_idraw, 410.f, 150.f, "Custom Font", &font, false, 200, 100, 50, 255);
        // neko_idraw_text(gui_idraw, 450.f, 200.f, "Default Font", NULL, false, 50, 100, 255, 255);
    };

    neko_ui_begin(&CL_GAME_USERDATA()->ui, NULL);
    {
        editor_dockspace(&CL_GAME_USERDATA()->ui);

        if (CL_GAME_USERDATA()->cl_cvar.show_gui) {

            neko_ui_demo_window(&CL_GAME_USERDATA()->ui, neko_ui_rect(100, 100, 500, 500), NULL);
            neko_ui_style_editor(&CL_GAME_USERDATA()->ui, NULL, neko_ui_rect(350, 250, 300, 240), NULL);

            const neko_vec2 ws = neko_v2(600.f, 300.f);

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

            neko_ui_window_begin(&CL_GAME_USERDATA()->ui, "Idraw", neko_ui_rect((CL_GAME_USERDATA()->fbs.x - ws.x) * 0.2f, (CL_GAME_USERDATA()->fbs.y - ws.y) * 0.5f, ws.x, ws.y));
            {
                // Cache the current container
                neko_ui_container_t *cnt = neko_ui_get_current_container(&CL_GAME_USERDATA()->ui);

                // 绘制到当前窗口中的转换对象的自定义回调
                // 在这里，我们将容器的主体作为视口传递，但这可以是您想要的任何内容，
                // 正如我们将在“分割”窗口示例中看到的那样。
                // 我们还传递自定义数据（颜色），以便我们可以在回调中使用它。如果以下情况，这可以为 NULL
                // 你不需要任何东西
                neko_color_t color = neko_color_alpha(NEKO_COLOR_RED, (uint8_t)NEKO_CLAMP((sin(t * 0.001f) * 0.5f + 0.5f) * 255, 0, 255));
                neko_ui_draw_custom(&CL_GAME_USERDATA()->ui, cnt->body, gui_cb, &color, sizeof(color));
            }
            neko_ui_window_end(&CL_GAME_USERDATA()->ui);
        }

        if (neko_platform_key_pressed(NEKO_KEYCODE_GRAVE_ACCENT)) {
            g_console.open = !g_console.open;
        } else if (neko_platform_key_pressed(NEKO_KEYCODE_TAB) && g_console.open) {
            g_console.autoscroll = !g_console.autoscroll;
        }

        neko_ui_layout_t l;

        if (CL_GAME_USERDATA()->cl_cvar.show_gui) {

            neko_vec2 mp = neko_platform_mouse_positionv();
            neko_vec2 mw = neko_platform_mouse_wheelv();
            neko_vec2 md = neko_platform_mouse_deltav();
            bool lock = neko_platform_mouse_locked();
            bool moved = neko_platform_mouse_moved();

            if (neko_ui_window_begin(&CL_GAME_USERDATA()->ui, "App", neko_ui_rect(CL_GAME_USERDATA()->fbs.x - 210, 30, 200, 200))) {
                l = *neko_ui_get_layout(&CL_GAME_USERDATA()->ui);
                neko_ui_layout_row(&CL_GAME_USERDATA()->ui, 1, neko_ui_widths(-1), 0);

                static f32 delta, fps = NEKO_DEFAULT_VAL();

                NEKO_TIMED_ACTION(5, {
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
                    static neko_memory_info_t meminfo = NEKO_DEFAULT_VAL();

                    NEKO_TIMED_ACTION(60, { meminfo = neko_platform_memory_info(); });

                    neko_ui_labelf("GC MemAllocInUsed: %.2lf mb", (f64)(neko_mem_bytes_inuse() / 1048576.0));
                    neko_ui_labelf("GC MemTotalAllocated: %.2lf mb", (f64)(g_allocation_metrics.total_allocated / 1048576.0));
                    neko_ui_labelf("GC MemTotalFree: %.2lf mb", (f64)(g_allocation_metrics.total_free / 1048576.0));

                    lua_Integer kb = lua_gc(neko_instance()->L, LUA_GCCOUNT, 0);
                    lua_Integer bytes = lua_gc(neko_instance()->L, LUA_GCCOUNTB, 0);

                    neko_ui_labelf("Lua MemoryUsage: %.2lf mb", ((f64)kb / 1024.0f));
                    neko_ui_labelf("Lua Remaining: %.2lf mb", ((f64)bytes / 1024.0f));

                    neko_ui_labelf("ImGui MemoryUsage: %.2lf mb", ((f64)__neko_imgui_meminuse() / 1048576.0));

                    neko_ui_labelf("Virtual MemoryUsage: %.2lf mb", ((f64)meminfo.virtual_memory_used / 1048576.0));
                    neko_ui_labelf("Real MemoryUsage: %.2lf mb", ((f64)meminfo.physical_memory_used / 1048576.0));
                    neko_ui_labelf("Virtual MemoryUsage Peak: %.2lf mb", ((f64)meminfo.peak_virtual_memory_used / 1048576.0));
                    neko_ui_labelf("Real MemoryUsage Peak: %.2lf mb", ((f64)meminfo.peak_physical_memory_used / 1048576.0));

                    neko_ui_labelf("GPU MemTotalAvailable: %.2lf mb", (f64)(meminfo.gpu_total_memory / 1024.0f));
                    neko_ui_labelf("GPU MemCurrentUsage: %.2lf mb", (f64)(meminfo.gpu_memory_used / 1024.0f));

                    neko_render_info_t *info = &neko_subsystem(render)->info;

                    neko_ui_labelf("OpenGL vendor: %s", info->vendor);
                    neko_ui_labelf("OpenGL version supported: %s", info->version);
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
}

// neko_game_desc_t *neko_main() {
//     static neko_game_desc_t gd = {.init = game_init,
//                                   .update = game_loop,
//                                   .shutdown = game_shutdown,
//                                   .window = {.width = 1280, .height = 720, .vsync = false, .frame_rate = 60.f, .hdpi = false, .center = true, .running_background = true},
//                                   .console = &g_console};
//     return &gd;
// }

void neko_app() {

    CL_GAME_USERDATA()->cl_cvar.bg[0] = CL_GAME_USERDATA()->cl_cvar.bg[1] = CL_GAME_USERDATA()->cl_cvar.bg[2] = 28.f;

    neko_instance()->update = &game_loop;
    neko_instance()->shutdown = &game_shutdown;
    neko_instance()->init = &game_init;
}