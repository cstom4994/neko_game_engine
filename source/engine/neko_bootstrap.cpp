
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "neko_api.hpp"
#include "neko_app.h"
#include "neko_asset.h"
#include "neko_base.h"
#include "neko_draw.h"
#include "neko_lua.h"
#include "neko_lua_wrap.h"
#include "neko_luabind.hpp"
#include "neko_math.h"
#include "neko_os.h"
#include "neko_prelude.h"
#include "neko_ui.h"

extern "C" {
#include <lua.h>
#include <lualib.h>
}

#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_glue.h>
#include <sokol_log.h>
#include <sokol_time.h>
#include <util/sokol_gfx_imgui.h>
#include <util/sokol_gl.h>

// gp
#include "deps/sokol_gp.h"

#if 0


static int __neko_boot_lua_create_ent(lua_State *L) {
    struct neko_boot_t *w = (struct neko_boot_t *)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    ecs_entity_t e = ecs_entity(w);
    lua_pushinteger(L, e);
    return 1;
}

const ecs_entity_t __neko_boot_lua_component_id_w(lua_State *L, const_str component_name) {
    lua_getfield(L, LUA_REGISTRYINDEX, "__NEKO_ECS_CORE");  // # -5
    lua_getiuservalue(L, -1, NEKO_ECS_COMPONENTS_NAME);     // # -4
    lua_getfield(L, -1, "comp_map");                        // # -3
    lua_pushinteger(L, neko_hash_str(component_name));      // 使用 32 位哈希以适应 Lua 数字范围
    lua_gettable(L, -2);                                    // # -2
    lua_getfield(L, -1, "id");                              // # -1
    const ecs_entity_t c = lua_tointeger(L, -1);
    lua_pop(L, 5);
    return c;
}

static int __neko_boot_lua_attach(lua_State *L) {
    int n = lua_gettop(L);
    luaL_argcheck(L, n >= 3, 1, "lost the component name");

    struct neko_boot_t *w = (struct neko_boot_t *)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    ecs_entity_t e = luaL_checkinteger(L, 2);

    for (int i = 3; i <= n; i++) {
        if (lua_isstring(L, i)) {
            const_str component_name = lua_tostring(L, i);
            const ecs_entity_t c = __neko_boot_lua_component_id_w(L, component_name);
            ecs_attach(w, e, c);
        } else {
            NEKO_WARN("argument %d is not a string", i);
        }
    }

    return 0;
}

static int __neko_boot_lua_get_com(lua_State *L) {
    struct neko_boot_t *w = (struct neko_ecs_world_t *)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    lua_getiuservalue(L, 1, NEKO_ECS_COMPONENTS_NAME);
    lua_getiuservalue(L, 1, NEKO_ECS_UPVAL_N);
    return 2;
}

static int __neko_boot_lua_gc(lua_State *L) {
    struct neko_boot_t *w = (struct neko_ecs_world_t *)luaL_checkudata(L, ECS_WORLD, ECS_WORLD_UDATA_NAME);
    ecs_fini_i(w);
    NEKO_INFO("ecs_lua_gc");
    return 0;
}

neko_boot_t *ecs_init(lua_State *L) {
    NEKO_ASSERT(L);
    neko_boot_t *registry = (neko_boot_t *)lua_newuserdatauv(L, sizeof(neko_boot_t), NEKO_ECS_UPVAL_N);  // # -1
    registry = ecs_init_i(registry);
    if (registry == NULL) {
        NEKO_ERROR("%s", "failed to initialize neko_boot_t");
        return NULL;
    }
    registry->L = L;

    if (luaL_getmetatable(L, ECS_WORLD_UDATA_NAME) == LUA_TNIL) {  // # -2

        // clang-format off
        luaL_Reg world_mt[] = {
            {"__gc", __neko_boot_lua_gc}, 
            {"create_ent", __neko_boot_lua_create_ent}, 
            {"attach", __neko_boot_lua_attach}, 
            {"get_com", __neko_boot_lua_get_com}, 
            {NULL, NULL}
        };
        // clang-format on

        lua_pop(L, 1);                  // # pop -2
        luaL_newlibtable(L, world_mt);  // # -2
        luaL_setfuncs(L, world_mt, 0);
        lua_pushvalue(L, -1);                                      // # -3
        lua_setfield(L, -2, "__index");                            // pop -3
        lua_pushliteral(L, ECS_WORLD_UDATA_NAME);                  // # -3
        lua_setfield(L, -2, "__name");                             // pop -3
        lua_pushvalue(L, -1);                                      // # -3
        lua_setfield(L, LUA_REGISTRYINDEX, ECS_WORLD_UDATA_NAME);  // pop -3
    }
    lua_setmetatable(L, -2);  // pop -2

    // lua_newtable(L);  // components name 表
    // lua_setfield(L, LUA_REGISTRYINDEX, MY_TABLE_KEY);
    //
    // lua_setiuservalue(L, -2, NEKO_ECS_COMPONENTS_NAME);

    lua_newtable(L);
    lua_pushstring(L, "comp_map");
    lua_createtable(L, 0, ENTITY_MAX_COMPONENTS);
    lua_settable(L, -3);
    lua_setiuservalue(L, -2, NEKO_ECS_COMPONENTS_NAME);

    const_str s = "Is man one of God's blunders? Or is God one of man's blunders?";
    lua_pushstring(L, s);
    lua_setiuservalue(L, -2, NEKO_ECS_UPVAL_N);

    // lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "__NEKO_ECS_CORE");

    // lua_setglobal(L, "__NEKO_ECS_CORE");

    return registry;
}

ecs_entity_t ecs_component_w(neko_boot_t *registry, const_str component_name, size_t component_size) {

    ecs_map_set(registry->component_index, (void *)registry->next_entity_id, &(size_t){component_size});
    ecs_entity_t id = registry->next_entity_id++;

    lua_State *L = registry->L;

    // lua_getglobal(L, "__NEKO_ECS_CORE");  // # 1
    lua_getfield(L, LUA_REGISTRYINDEX, "__NEKO_ECS_CORE");

    lua_getiuservalue(L, -1, NEKO_ECS_COMPONENTS_NAME);
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "comp_map");
        if (lua_istable(L, -1)) {
            lua_pushinteger(L, neko_hash_str(component_name));  // 使用 32 位哈希以适应 Lua 数字范围

            lua_createtable(L, 0, 2);
            lua_pushinteger(L, id);
            lua_setfield(L, -2, "id");
            lua_pushstring(L, component_name);
            lua_setfield(L, -2, "name");

            lua_settable(L, -3);
            lua_pop(L, 1);
        } else {
            NEKO_ERROR("%s", "failed to get comp_map");
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
    } else {
        NEKO_ERROR("%s", "failed to get upvalue NEKO_ECS_COMPONENTS_NAME");
        lua_pop(L, 1);
    }
    lua_pop(L, 1);  // pop 1

    return id;
}



//=============================
// NEKO_ENGINE
//=============================

using namespace neko;

i32 random_val(i32 lower, i32 upper) {
    if (upper < lower) {
        i32 tmp = lower;
        lower = upper;
        upper = tmp;
    }
    return (rand() % (upper - lower + 1) + lower);
}

#if 0
namespace neko::ecs_component {

void movement_system(ecs_view_t view, unsigned int row) {

    position_t* p = (position_t*)ecs_view(view, row, 0);
    velocity_t* v = (velocity_t*)ecs_view(view, row, 1);
    bounds_t* b = (bounds_t*)ecs_view(view, row, 2);
    color_t* c = (color_t*)ecs_view(view, row, 3);

    neko_vec2_t min = neko_vec2_add(*p, *v);
    neko_vec2_t max = neko_vec2_add(min, *b);

    auto ws = neko_game()->DisplaySize;

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
    position_t* p = (position_t*)ecs_view(view, row, 0);
    velocity_t* v = (velocity_t*)ecs_view(view, row, 1);
    bounds_t* b = (bounds_t*)ecs_view(view, row, 2);
    color_t* c = (color_t*)ecs_view(view, row, 3);

    auto pi = *p;
    auto bi = *b;

    neko_idraw_rectvd(&ENGINE_INTERFACE()->idraw, pi, bi, neko_v2(0.f, 0.f), neko_v2(1.f, 1.f), (neko_color_t)*c, R_PRIMITIVE_TRIANGLES);
}

void aseprite_render_system(ecs_view_t view, unsigned int row) { position_t* p = (position_t*)ecs_view(view, row, 0); }

}  // namespace neko::ecs_component

#endif

void neko_register(lua_State *L);

lua_State *neko_scripting_init() {
    neko::timer timer;
    timer.start();

    lua_State *L = neko_lua_create();

    lua_atpanic(
            L, +[](lua_State *L) {
                auto msg = neko_lua_to<const_str>(L, -1);
                NEKO_ERROR("[lua] neko_panic error: %s", msg);
                return 0;
            });
    neko_register(L);

    timer.stop();
    NEKO_INFO(std::format("lua init done in {0:.3f} ms", timer.get()).c_str());

    return L;
}

void neko_scripting_end(lua_State *L) { neko_lua_fini(L); }

NEKO_API_DECL void neko_default_main_window_close_callback(void *window);

NEKO_API_DECL neko_instance_t *neko_instance() {
    static neko_instance_t g_neko_instance = NEKO_DEFAULT_VAL();
    return &g_neko_instance;
}

Neko_OnModuleLoad(Sound);
Neko_OnModuleLoad(Physics);
Neko_OnModuleLoad(Network);

NEKO_API_DECL lua_State *neko_lua_bootstrap(int argc, char **argv) {

    lua_State *L = neko_scripting_init();

    // neko_lua_safe_dofile(L, "startup");

    {
        
        lua_newtable(L);
        for (int n = 0; n < argc; ++n) {
            lua_pushstring(L, argv[n]);
            lua_rawseti(L, -2, n);
        }
        lua_setglobal(L, "arg");
    }

    if (argc != 0) {
        auto &module_list = ENGINE_INTERFACE()->modules;

        neko_module m = {};
        i32 sss;
        sss = Neko_OnModuleLoad_Sound(&m, ENGINE_INTERFACE());
        neko_dyn_array_push(module_list, m);
        sss = Neko_OnModuleLoad_Physics(&m, ENGINE_INTERFACE());
        neko_dyn_array_push(module_list, m);
        sss = Neko_OnModuleLoad_Network(&m, ENGINE_INTERFACE());
        neko_dyn_array_push(module_list, m);

        for (u32 i = 0; i < neko_dyn_array_size(module_list); ++i) {
            auto &module = module_list[i];
            module.func.OnInit(L);
        }
    } else {
    }


    return L;
}


using namespace neko;

namespace neko::reflection {
template <>
Type *type_of<neko_os_running_desc_t>() {
    static Type type;
    type.name = "neko_os_running_desc_t";
    type.destroy = [](void *obj) { delete static_cast<neko_os_running_desc_t *>(obj); };
    type.copy = [](const void *obj) { return (void *)(new neko_os_running_desc_t(*static_cast<const neko_os_running_desc_t *>(obj))); };
    type.move = [](void *obj) { return (void *)(new neko_os_running_desc_t(std::move(*static_cast<neko_os_running_desc_t *>(obj)))); };

#define REFL_FIELDS(C, field) type.fields.insert({#field, {type_of<decltype(C::field)>(), offsetof(C, field)}})

    REFL_FIELDS(neko_os_running_desc_t, title);
    REFL_FIELDS(neko_os_running_desc_t, width);
    REFL_FIELDS(neko_os_running_desc_t, height);
    REFL_FIELDS(neko_os_running_desc_t, flags);
    REFL_FIELDS(neko_os_running_desc_t, num_samples);
    REFL_FIELDS(neko_os_running_desc_t, monitor_index);
    REFL_FIELDS(neko_os_running_desc_t, vsync);
    REFL_FIELDS(neko_os_running_desc_t, frame_rate);

    return &type;
};
}  // namespace neko::reflection

// user data

void editor_dockspace(neko_ui_context_t *ctx);

// test
void test_xml(const std::string &file);
void test_se();
void test_containers();

void draw_gui();

void game_init() {

    neko_game()->cvar.bg[0] = neko_game()->cvar.bg[1] = neko_game()->cvar.bg[2] = 28.f;

    {

        neko_lua_safe_dofile(ENGINE_LUA(), "main");

        luax_get(ENGINE_LUA(), "neko", "__define_default_callbacks");
        luax_pcall(ENGINE_LUA(), 0, 0);

        // 获取 neko.conf.table
        luax_get(ENGINE_LUA(), "neko", "conf");
        if (!lua_istable(ENGINE_LUA(), -1)) {
            NEKO_ERROR("%s", "neko_game is not a table");
        }

        neko::reflection::Any v = neko_os_running_desc_t{.title = "Neko Engine"};
        neko::lua::checktable_refl(ENGINE_LUA(), "app", v);

        neko_client_cvar_t &cvar = neko_game()->cvar;
        // if (lua_getfield(ENGINE_L(), -1, "cvar") == LUA_TNIL) throw std::exception("no cvar");
        // if (lua_istable(ENGINE_L(), -1)) {
        //     neko::static_refl::neko_type_info<neko_client_cvar_t>::ForEachVarOf(cvar, [](auto field, auto &&value) {
        //         static_assert(std::is_lvalue_reference_v<decltype(value)>);
        //         if (lua_getfield(ENGINE_L(), -1, std::string(field.name).c_str()) != LUA_TNIL) value = neko_lua_to<std::remove_reference_t<decltype(value)>>(ENGINE_L(), -1);
        //         lua_pop(ENGINE_L(), 1);
        //     });
        // } else {
        //     throw std::exception("no cvar table");
        // }
        // lua_pop(ENGINE_L(), 1);

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

        lua_pop(ENGINE_LUA(), 1);  // 弹出 neko.conf.table

        NEKO_INFO("load game: %s %d %d", v.cast<neko_os_running_desc_t>().title, v.cast<neko_os_running_desc_t>().width, v.cast<neko_os_running_desc_t>().height);

        neko_os_set_window_title(neko_os_main_window(), v.cast<neko_os_running_desc_t>().title);
    }

    bool ok = ENGINE_INTERFACE()->pack.load("gamedir/res.pack", 0, false);
    NEKO_ASSERT(ok == true);

    u8 *font_data, *cat_data;
    u32 font_data_size, cat_data_size;

    ENGINE_INTERFACE()->pack.get_data(".\\data\\assets\\fonts\\fusion-pixel-12px-monospaced.ttf", (const u8 **)&font_data, &font_data_size);
    ENGINE_INTERFACE()->pack.get_data(".\\data\\assets\\textures\\cat.aseprite", (const u8 **)&cat_data, &cat_data_size);

    ENGINE_INTERFACE()->test_ase = neko_aseprite_simple(cat_data, cat_data_size);

    neko_asset_texture_t tex0 = {0};
    neko_asset_texture_load_from_file("gamedir/assets/textures/yzh.png", &tex0, NULL, false, false);
    ENGINE_INTERFACE()->tex_hndl = neko_assets_create_asset(&ENGINE_INTERFACE()->am, neko_asset_texture_t, &tex0);

    neko_ui_init(&ENGINE_INTERFACE()->ui, neko_os_main_window());

    // 加载自定义字体文件 初始化 gui font stash
    // neko_asset_font_load_from_memory(font_data, font_data_size, &CL_GAME_INTERFACE()->font, 24);
    neko_asset_font_load_from_file("gamedir/assets/fonts/monocraft.ttf", &ENGINE_INTERFACE()->font, 24);

    ENGINE_INTERFACE()->pack.free_item(font_data);
    ENGINE_INTERFACE()->pack.free_item(cat_data);

    auto GUI_FONT_STASH = []() -> neko_ui_font_stash_desc_t * {
        static neko_ui_font_desc_t font_decl[] = {{.key = "mc_regular", .font = &ENGINE_INTERFACE()->font}};
        static neko_ui_font_stash_desc_t font_stash = {.fonts = font_decl, .size = 1 * sizeof(neko_ui_font_desc_t)};
        return &font_stash;
    }();

    neko_ui_init_font_stash(&ENGINE_INTERFACE()->ui, GUI_FONT_STASH);

    neko_ui_dock_ex(&ENGINE_INTERFACE()->ui, "Style_Editor", "Demo_Window", NEKO_UI_SPLIT_TAB, 0.5f);

    neko_game()->imgui = neko_imgui_new(&ENGINE_INTERFACE()->cb, neko_os_main_window(), false);

    // Construct frame buffer
    neko_game()->r_main_fbo = neko_render_framebuffer_create({});

    neko_render_texture_desc_t main_rt_desc = {.width = (u32)neko_game()->DisplaySize.x,
                                               .height = (u32)neko_game()->DisplaySize.y,
                                               .format = R_TEXTURE_FORMAT_RGBA8,
                                               .wrap_s = R_TEXTURE_WRAP_REPEAT,
                                               .wrap_t = R_TEXTURE_WRAP_REPEAT,
                                               .min_filter = R_TEXTURE_FILTER_NEAREST,
                                               .mag_filter = R_TEXTURE_FILTER_NEAREST};
    neko_game()->r_main_rt = neko_render_texture_create(main_rt_desc);

    neko_render_renderpass_desc_t main_rp_desc = {.fbo = neko_game()->r_main_fbo, .color = &neko_game()->r_main_rt, .color_size = sizeof(neko_game()->r_main_rt)};
    neko_game()->r_main_rp = neko_render_renderpass_create(main_rp_desc);

    luax_get(ENGINE_LUA(), "neko", "game_init");
    luax_pcall(ENGINE_LUA(), 0, 0);

    neko::timer timer;
    timer.start();
    luax_get(ENGINE_LUA(), "neko", "game_init_thread");
    luax_pcall(ENGINE_LUA(), 0, 0);
    timer.stop();
    NEKO_INFO(std::format("game_init_thread loading done in {0:.3f} ms", timer.get()).c_str());
}

void game_loop() {

    PROFILE_FUNC();

    // int this_init_thread_flag = game_userdata->init_work_flag;

    // if (this_init_thread_flag == 0) {

    //     //        neko_vec2 fbs = neko_os_framebuffer_sizev(neko_os_main_window());
    //     const f32 t = neko_os_elapsed_time();

    //     u8 tranp = 255;

    //     // tranp -= ((i32)t % 255);

    //     neko_render_clear_action_t clear = {.color = {neko_game()->cvar.bg[0] / 255, neko_game()->cvar.bg[1] / 255, neko_game()->cvar.bg[2] / 255, 1.f}};
    //     neko_render_renderpass_begin(&ENGINE_INTERFACE()->cb, neko_renderpass_t{0});
    //     { neko_render_clear(&ENGINE_INTERFACE()->cb, clear); }
    //     neko_render_renderpass_end(&ENGINE_INTERFACE()->cb);

    //     // Set up 2D camera for projection matrix
    //     neko_idraw_defaults(&ENGINE_INTERFACE()->idraw);
    //     neko_idraw_camera2d(&ENGINE_INTERFACE()->idraw, (u32)neko_game()->DisplaySize.x, (u32)neko_game()->DisplaySize.y);

    //     // 底层图片
    //     char background_text[64] = "Project: unknown";

    //     neko_vec2 td = neko_asset_font_text_dimensions(neko_idraw_default_font(), background_text, -1);
    //     neko_vec2 ts = neko_v2(512 + 128, 512 + 128);

    //     neko_idraw_text(&ENGINE_INTERFACE()->idraw, (neko_game()->DisplaySize.x - td.x) * 0.5f, (neko_game()->DisplaySize.y - td.y) * 0.5f + ts.y / 2.f - 100.f, background_text, NULL, false,
    //                     neko_color(255, 255, 255, 255));
    //     neko_idraw_texture(&ENGINE_INTERFACE()->idraw, ENGINE_INTERFACE()->test_ase);
    //     neko_idraw_rectvd(&ENGINE_INTERFACE()->idraw, neko_v2((neko_game()->DisplaySize.x - ts.x) * 0.5f, (neko_game()->DisplaySize.y - ts.y) * 0.5f - td.y - 50.f), ts, neko_v2(0.f, 1.f),
    //                       neko_v2(1.f, 0.f), neko_color(255, 255, 255, tranp), R_PRIMITIVE_TRIANGLES);

    //     neko_render_renderpass_begin(&ENGINE_INTERFACE()->cb, R_RENDER_PASS_DEFAULT);
    //     {
    //         neko_render_set_viewport(&ENGINE_INTERFACE()->cb, 0, 0, (u32)neko_game()->DisplaySize.x, (u32)neko_game()->DisplaySize.y);
    //         neko_idraw_draw(&ENGINE_INTERFACE()->idraw, &ENGINE_INTERFACE()->cb);  // 立即模式绘制 idraw
    //     }
    //     neko_render_renderpass_end(&ENGINE_INTERFACE()->cb);

    // } else
    {

        // static int init_retval = 1;
        // if (init_retval) {
        //     // init_retval = thread_join(game_userdata->init_work_thread);
        //     // thread_term(game_userdata->init_work_thread);
        //     game_userdata->init_work_thread.join();
        //     // game_userdata->init_work_sema.wait();
        //     game_userdata->init_work_sema.trash();
        //     NEKO_TRACE("init_work_thread done");
        // }

        f32 dt = neko_os_delta_time();

        {
            PROFILE_BLOCK("lua_pre_update");
            luax_get(ENGINE_LUA(), "neko", "game_pre_update");
            luax_pcall(ENGINE_LUA(), 0, 0);
        }

        if (neko_os_key_pressed(NEKO_KEYCODE_ESC)) neko_game()->cvar.show_editor ^= true;

        {
            PROFILE_BLOCK("lua_loop");
            luax_get(ENGINE_LUA(), "neko", "game_loop");
            __lua_op_t<f32>::push_stack(ENGINE_LUA(), dt);
            luax_pcall(ENGINE_LUA(), 1, 0);
        }

        // Do rendering
        neko_render_clear_action_t clear = {.color = {neko_game()->cvar.bg[0] / 255, neko_game()->cvar.bg[1] / 255, neko_game()->cvar.bg[2] / 255, 1.f}};
        neko_render_clear_action_t b_clear = {.color = {0.0f, 0.0f, 0.0f, 1.f}};

        neko_render_renderpass_begin(&ENGINE_INTERFACE()->cb, R_RENDER_PASS_DEFAULT);
        { neko_render_clear(&ENGINE_INTERFACE()->cb, clear); }
        neko_render_renderpass_end(&ENGINE_INTERFACE()->cb);

        neko_imgui_new_frame(&neko_game()->imgui);

        draw_gui();

        if (neko_game()->cvar.show_demo_window) ImGui::ShowDemoWindow();

        if (neko_game()->cvar.show_editor && ImGui::Begin("Cvar")) {

            neko_cvar_gui(neko_game()->cvar);

            neko::imgui::toggle("帧检查器", &neko_game()->cvar.show_profiler_window);

            ImGui::DragFloat("Max FPS", &neko_subsystem(platform)->time.max_fps, 5.0f, 30.0f, 2000.0f);

            ImGui::Separator();

            if (ImGui::Button("test_xml")) test_xml("gamedir/tests/test.xml");
            if (ImGui::Button("test_se")) test_se();
            if (ImGui::Button("test_containers")) test_containers();
#if 0
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
#endif

            ImGui::End();
        }

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("engine")) {
                if (ImGui::Checkbox("vsync", &neko_game()->cvar.vsync)) neko_os_enable_vsync(neko_game()->cvar.vsync);
                if (ImGui::MenuItem("quit")) {
                    neko_quit();
                }

                ImGui::EndMenu();
            }

            ImGui::TextColored(ImVec4(0.19f, 1.f, 0.196f, 1.f), "nEKO");

            if (ImGui::BeginMenu("File")) {
                ImGui::EndMenu();
            }

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - 210 - ImGui::GetScrollX());
            ImGui::Text("%.1f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            ImGui::EndMainMenuBar();
        }

        //        sandbox_update(sb);

        neko_idraw_defaults(&ENGINE_INTERFACE()->idraw);
        // neko_idraw_camera2d_ex(&ENGINE_INTERFACE()->idraw, game_userdata->cam.x, game_userdata->cam.x + neko_game()->DisplaySize.x, game_userdata->cam.y,
        //                        game_userdata->cam.y + neko_game()->DisplaySize.y);

        neko_render_renderpass_begin(&ENGINE_INTERFACE()->cb, neko_game()->r_main_rp);
        {
            neko_render_set_viewport(&ENGINE_INTERFACE()->cb, 0, 0, (u32)neko_game()->DisplaySize.x, (u32)neko_game()->DisplaySize.y);
            neko_render_clear(&ENGINE_INTERFACE()->cb, clear);
            neko_idraw_draw(&ENGINE_INTERFACE()->idraw, &ENGINE_INTERFACE()->cb);  // 立即模式绘制 idraw
        }
        neko_render_renderpass_end(&ENGINE_INTERFACE()->cb);

        neko_idraw_defaults(&ENGINE_INTERFACE()->idraw);
        neko_idraw_camera2d(&ENGINE_INTERFACE()->idraw, (u32)neko_game()->DisplaySize.x, (u32)neko_game()->DisplaySize.y);
        neko_idraw_texture(&ENGINE_INTERFACE()->idraw, neko_game()->r_main_rt);
        neko_idraw_rectvd(&ENGINE_INTERFACE()->idraw, neko_v2(0.0, 0.0), neko_v2((u32)neko_game()->DisplaySize.x, (u32)neko_game()->DisplaySize.y), neko_v2(0.0, 1.0), neko_v2(1.0, 0.0),
                          neko_color(255, 255, 255, 255), R_PRIMITIVE_TRIANGLES);

        neko_render_renderpass_begin(&ENGINE_INTERFACE()->cb, R_RENDER_PASS_DEFAULT);
        {
            neko_render_set_viewport(&ENGINE_INTERFACE()->cb, 0.0, 0.0, neko_game()->DisplaySize.x, neko_game()->DisplaySize.y);
            neko_idraw_draw(&ENGINE_INTERFACE()->idraw, &ENGINE_INTERFACE()->cb);  // 立即模式绘制 idraw
            neko_ui_render(&ENGINE_INTERFACE()->ui, &ENGINE_INTERFACE()->cb);
        }
        neko_render_renderpass_end(&ENGINE_INTERFACE()->cb);
        {
            PROFILE_BLOCK("lua_render");
            luax_get(ENGINE_LUA(), "neko", "game_render");
            luax_pcall(ENGINE_LUA(), 0, 0);
        }

        auto &module_list = ENGINE_INTERFACE()->modules;
        for (u32 i = 0; i < neko_dyn_array_size(module_list); ++i) {
            auto &module = module_list[i];
            module.func.OnUpdate(ENGINE_LUA());
        }

        {
            PROFILE_BLOCK("imgui_submit");
            neko_imgui_render(&neko_game()->imgui);
        }
    }
}

void game_post_update() {}

void game_fini() {

    neko_render_renderpass_fini(neko_game()->r_main_rp);
    neko_render_texture_fini(neko_game()->r_main_rt);
    neko_render_framebuffer_fini(neko_game()->r_main_fbo);

    luax_get(ENGINE_LUA(), "neko", "game_fini");
    luax_pcall(ENGINE_LUA(), 0, 0);

    neko_imgui_shutdown(&neko_game()->imgui);
    neko_ui_free(&ENGINE_INTERFACE()->ui);
    ENGINE_INTERFACE()->pack.fini();

    // ecs_fini(game_userdata->w);

#ifdef USE_PROFILER
    neko::profile_shutdown();
#endif
}

NEKO_API_DECL neko_instance_t *neko_create(int argc, char **argv) {
    if (neko_instance() != NULL) {

#ifndef NDEBUG
        g_allocator = new DebugAllocator();
#else
        g_allocator = new HeapAllocator();
#endif

        g_allocator->make();

        neko_tm_init();

        profile_setup();
        PROFILE_FUNC();

        neko_instance()->console = &g_console;

        NEKO_INFO("neko engine build %d", neko_buildnum());

        {

            // 初始化应用程序并设置为运行
            mount_result mount;

#if defined(NEKO_DEBUG_BUILD)
            mount = neko::vfs_mount(NEKO_PACKS::GAMEDATA, "./");
            mount = neko::vfs_mount(NEKO_PACKS::LUACODE, "./");
#else
            mount = neko::vfs_mount(NEKO_PACKS::GAMEDATA, "./gamedata.zip");
            mount = neko::vfs_mount(NEKO_PACKS::LUACODE, "./luacode.zip");
#endif

            ENGINE_LUA() = neko_lua_bootstrap(argc, argv);

            neko_w_init();

            lua_channels_setup();

        }

        auto L = ENGINE_LUA();

        // 初始化 cvars
        CVAR(settings_window_width, 1280.f);
        CVAR(settings_window_height, 720.f);
        CVAR(settings_window_vsync, false);
        CVAR(settings_window_frame_rate, 60.f);
        CVAR(settings_window_hdpi, false);
        CVAR(settings_window_center, true);
        CVAR(settings_window_running_background, true);
        CVAR(settings_window_monitor_index, 0);
        CVAR(settings_video_render_debug, 0);
        CVAR(settings_video_render_hdpi, false);

        // CVAR(dwadaw, "nihao");

        CVAR(pack_buffer_size, 128);

        // 需要从用户那里传递视频设置
        neko_subsystem(platform) = neko_os_create();

        // 此处平台的默认初始化
        neko_os_init(neko_subsystem(platform));

        // 设置应用程序的帧速率
        neko_subsystem(platform)->time.max_fps = settings_window_frame_rate.get<f32>();

        neko_os_running_desc_t window = {.title = "Neko Engine", .width = 1280, .height = 720, .vsync = false, .frame_rate = 60.f, .hdpi = false, .center = true, .running_background = true};

        // 构建主窗口
        neko_os_window_create(&window);

        // 设置视频垂直同步
        neko_os_enable_vsync(settings_window_vsync.get<bool>());

        // 构建图形API
        neko_subsystem(render) = neko_render_create();

        // 初始化图形
        neko_render_init(neko_subsystem(render));

        neko_module_interface_init(ENGINE_INTERFACE());

        {
            u32 w, h;
            u32 display_w, display_h;

            neko_os_window_size(neko_os_main_window(), &w, &h);
            neko_os_framebuffer_size(neko_os_main_window(), &display_w, &display_h);

            neko_game()->DisplaySize = neko_v2((float)w, (float)h);
            neko_game()->DisplayFramebufferScale = neko_v2((float)display_w / w, (float)display_h / h);
        }

        lua_register(
                L, "sandbox_init", +[](lua_State *L) {
                    // auto ud = neko::lua::udata_new<neko_client_userdata_t>(L, 1);
                    PROFILE_FUNC();
                    game_init();
                    return 0;
                });

        lua_register(
                L, "sandbox_update", +[](lua_State *L) {
                    // auto ud = neko::lua::toudata_ptr<neko_client_userdata_t>(L, 1);
                    PROFILE_FUNC();
                    game_loop();
                    return 0;
                });

        lua_register(
                L, "sandbox_fini", +[](lua_State *L) {
                    // auto ud = neko::lua::toudata_ptr<neko_client_userdata_t>(L, 1);
                    PROFILE_FUNC();
                    game_fini();
                    return 0;
                });

        // neko_lua_safe_dofile(ENGINE_L(), "boot");

        std::string boot_code = R"lua(
function boot_init()
    sandbox_init()
end

function boot_update()
    sandbox_update()
end

function boot_fini()
    sandbox_fini()
end
)lua";

        neko_lua_run_string(ENGINE_LUA(), boot_code.c_str());

        neko_lua_call(ENGINE_LUA(), "boot_init");
        neko_instance()->game.is_running = true;

        // 设置按下主窗口关闭按钮时的默认回调
        neko_os_set_window_close_callback(neko_os_main_window(), &neko_default_main_window_close_callback);
    }

    return neko_instance();
}

// 主框架函数
NEKO_API_DECL void neko_frame() {

    PROFILE_FUNC();

    {
        PROFILE_BLOCK("main_loop");

        neko_os_t *platform = neko_subsystem(platform);

        neko_os_window_t *win = (neko_slot_array_getp(platform->windows, neko_os_main_window()));

        // 帧开始时的缓存时间
        platform->time.elapsed = (f32)neko_os_elapsed_time();
        platform->time.update = platform->time.elapsed - platform->time.previous;
        platform->time.previous = platform->time.elapsed;

        // 更新平台和流程输入
        neko_os_update(platform);

        if (win->focus /*|| neko_instance()->game.window.running_background*/) {

            PROFILE_BLOCK("main_update");

            neko_lua_call(ENGINE_LUA(), "boot_update");

            // neko_idraw_defaults(&ENGINE_INTERFACE()->idraw);
            // neko_idraw_camera2d(&ENGINE_INTERFACE()->idraw, neko_game()->DisplaySize.x, neko_game()->DisplaySize.y);
            // ecs_step(ENGINE_INTERFACE()->ecs);

            // neko_instance()->post_update();
            {
                PROFILE_BLOCK("cmd_submit");
                neko_render_cmd_submit(&ENGINE_INTERFACE()->cb);
                neko_check_gl_error();
            }

            auto &module_list = ENGINE_INTERFACE()->modules;
            for (u32 i = 0; i < neko_dyn_array_size(module_list); ++i) {
                auto &module = module_list[i];
                module.func.OnPostUpdate(ENGINE_LUA());
            }
        }

        // 清除所有平台事件
        neko_dyn_array_clear(platform->events);

        {
            PROFILE_BLOCK("swap_buffer");
            for (neko_slot_array_iter it = 0; neko_slot_array_iter_valid(platform->windows, it); neko_slot_array_iter_advance(platform->windows, it)) {
                neko_os_window_swap_buffer(it);
            }
            // neko_os_window_swap_buffer(neko_os_main_window());
        }

        // 帧锁定
        platform->time.elapsed = (f32)neko_os_elapsed_time();
        platform->time.render = platform->time.elapsed - platform->time.previous;
        platform->time.previous = platform->time.elapsed;
        platform->time.frame = platform->time.update + platform->time.render;  // 总帧时间
        platform->time.delta = platform->time.frame / 1000.f;

        f32 target = (1000.f / platform->time.max_fps);

        if (platform->time.frame < target) {
            PROFILE_BLOCK("wait");
            neko_os_sleep((f32)(target - platform->time.frame));
            platform->time.elapsed = (f32)neko_os_elapsed_time();
            double wait_time = platform->time.elapsed - platform->time.previous;
            platform->time.previous = platform->time.elapsed;
            platform->time.frame += wait_time;
            platform->time.delta = platform->time.frame / 1000.f;
        }

        //        neko_profiler_end_scope(profile_id_engine);
    }
}

void neko_fini() {
    PROFILE_FUNC();

    // Shutdown application
    neko_lua_call(ENGINE_LUA(), "boot_fini");
    neko_instance()->game.is_running = false;

    lua_channels_shutdown();

    auto &module_list = ENGINE_INTERFACE()->modules;
    for (u32 i = 0; i < neko_dyn_array_size(module_list); ++i) {
        auto &module = module_list[i];
        module.func.OnFini(ENGINE_LUA());
    }

    neko_module_interface_fini(ENGINE_INTERFACE());

    neko_scripting_end(ENGINE_LUA());

    neko::vfs_fini({});

    neko_render_fini(neko_subsystem(render));

    neko_os_fini(neko_subsystem(platform));

    // __neko_config_free();

    profile_fini();

    // 在 app 结束后进行内存检查
#ifndef NDEBUG
    DebugAllocator *allocator = dynamic_cast<DebugAllocator *>(g_allocator);
    if (allocator != nullptr) {
        allocator->dump_allocs();
    }
#endif

    g_allocator->trash();
    operator delete(g_allocator);
}

NEKO_API_DECL void neko_default_main_window_close_callback(void *window) { neko_instance()->game.is_running = false; }

void neko_quit() {
#ifndef NEKO_IS_WEB
    neko_instance()->game.is_running = false;
#endif
}

void neko_module_interface_init(Neko_ModuleInterface *module_interface) {
    module_interface->cb = neko_command_buffer_new();
    module_interface->idraw = neko_immediate_draw_new();
    module_interface->am = neko_asset_manager_new();
}

void neko_module_interface_fini(Neko_ModuleInterface *module_interface) {

    neko_immediate_draw_free(&module_interface->idraw);
    neko_command_buffer_free(&module_interface->cb);
    neko_asset_manager_free(&module_interface->am);
}

//=============================
// MAIN
//=============================

int main(int argv, char **argc) {
    neko_instance_t *inst = neko_create(argv, argc);
    while (neko_instance()->game.is_running) neko_frame();
    neko_fini();
    return 0;
}

#endif

i32 neko_buildnum(void) {
    static const char *__build_date = __DATE__;
    static const char *mon[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    static const char mond[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    i32 m = 0, d = 0, y = 0;
    static i32 b = 0;
    if (b != 0) return b;  // 优化
    for (m = 0; m < 11; m++) {
        if (!strncmp(&__build_date[0], mon[m], 3)) break;
        d += mond[m];
    }
    d += atoi(&__build_date[4]) - 1;
    y = atoi(&__build_date[7]) - 2022;
    b = d + (i32)((y - 1) * 365.25f);
    if (((y % 4) == 0) && m > 1) b += 1;
    b -= 151;
    return b;
}

static Mutex g_init_mtx;
static sgl_pipeline g_pipeline;

static sgimgui_t sgimgui;

static void *__neko_imgui_malloc(size_t sz, void *user_data) { return mem_alloc(sz); }

static void __neko_imgui_free(void *ptr, void *user_data) { return mem_free(ptr); }

// just for test
// static map_t map;

static void init() {
    PROFILE_FUNC();
    LockGuard lock(&g_init_mtx);

    neko::timer tm_init;
    tm_init.start();

    {
        PROFILE_BLOCK("sokol");

        sg_desc sg = {};
        sg.logger.func = slog_func;
        // sg.context = sapp_sgcontext();
        sg.environment = sglue_environment();
        sg_setup(sg);

        sgl_desc_t sgl = {};
        sgl.logger.func = slog_func;
        sgl.max_vertices = 128 * 1024;
        sgl_setup(sgl);

        sgp_desc sgpdesc = {0};
        sgp_setup(&sgpdesc);
        if (!sgp_is_valid()) {
            fprintf(stderr, "failed to create sokol_gp context: %s\n", sgp_get_error_message(sgp_get_last_error()));
            exit(-1);
        }

        ImGui::SetAllocatorFunctions(__neko_imgui_malloc, __neko_imgui_free);

        const sgimgui_desc_t desc = {};
        sgimgui_init(&sgimgui, &desc);

        simgui_desc_t simgui_desc = {};
        simgui_desc.ini_filename = "imgui.ini";
        simgui_desc.logger.func = slog_func;
        simgui_desc.no_default_font = true;
        simgui_desc.allocator = {
                .alloc_fn = +[](size_t size, void *user_data) { return mem_alloc(size); },
                .free_fn = +[](void *ptr, void *user_data) { return mem_free(ptr); },
        };
        simgui_setup(&simgui_desc);

        CVAR_REF(conf_imgui_font, String);

        if (neko_strlen(conf_imgui_font.data.str) > 0) {
            auto &io = ImGui::GetIO();

            ImFontConfig config;
            // config.PixelSnapH = 1;

            String ttf_file;
            vfs_read_entire_file(NEKO_PACKS::GAMEDATA, &ttf_file, conf_imgui_font.data.str);
            neko_defer(mem_free(ttf_file.data));
            void *ttf_data = ::mem_alloc(ttf_file.len);  // TODO:: imgui 内存方法接管
            memcpy(ttf_data, ttf_file.data, ttf_file.len);
            io.Fonts->AddFontFromMemoryTTF(ttf_data, ttf_file.len, 12.0f, &config, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());

            // 为字体创建字体纹理和线性过滤采样器
            simgui_font_tex_desc_t font_texture_desc = {};
            font_texture_desc.min_filter = SG_FILTER_LINEAR;
            font_texture_desc.mag_filter = SG_FILTER_LINEAR;
            simgui_create_fonts_texture(&font_texture_desc);
        }

        sg_pipeline_desc sg_pipline = {};
        sg_pipline.depth.write_enabled = true;
        sg_pipline.colors[0].blend.enabled = true;
        sg_pipline.colors[0].blend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
        sg_pipline.colors[0].blend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        g_pipeline = sgl_make_pipeline(sg_pipline);
    }

    {
        PROFILE_BLOCK("miniaudio");

        g_app->miniaudio_vfs = vfs_for_miniaudio();

        ma_engine_config ma_config = ma_engine_config_init();
        ma_config.channels = 2;
        ma_config.sampleRate = 44100;
        ma_config.pResourceManagerVFS = g_app->miniaudio_vfs;
        ma_result res = ma_engine_init(&ma_config, &g_app->audio_engine);
        if (res != MA_SUCCESS) {
            fatal_error("failed to initialize audio engine");
        }
    }

    microui_init();

    renderer_reset();

    g_app->time.startup = stm_now();
    g_app->time.last = stm_now();

    // just for test
    // neko_tiled_load(&map, "assets/maps/map.tmx", NULL);

    {
        PROFILE_BLOCK("neko.start");

        lua_State *L = g_app->L;

        if (!g_app->error_mode.load()) {
            luax_neko_get(L, "start");

            Slice<String> args = g_app->args;
            lua_createtable(L, args.len - 1, 0);
            for (u64 i = 1; i < args.len; i++) {
                lua_pushlstring(L, args[i].data, args[i].len);
                lua_rawseti(L, -2, i);
            }

            luax_pcall(L, 1, 0);
        }
    }

    g_app->gpu_mtx.lock();

    lua_channels_setup();
    assets_start_hot_reload();

    tm_init.stop();

    NEKO_INFO("end of init in %.3f ms", tm_init.get());
}

static void event(const sapp_event *e) {
    if (simgui_handle_event(e)) return;

    microui_sokol_event(e);

    switch (e->type) {
        case SAPP_EVENTTYPE_KEY_DOWN:
            g_app->key_state[e->key_code] = true;
            break;
        case SAPP_EVENTTYPE_KEY_UP:
            g_app->key_state[e->key_code] = false;
            break;
        case SAPP_EVENTTYPE_MOUSE_DOWN:
            g_app->mouse_state[e->mouse_button] = true;
            break;
        case SAPP_EVENTTYPE_MOUSE_UP:
            g_app->mouse_state[e->mouse_button] = false;
            break;
        case SAPP_EVENTTYPE_MOUSE_MOVE:
            g_app->mouse_x = e->mouse_x;
            g_app->mouse_y = e->mouse_y;
            break;
        case SAPP_EVENTTYPE_MOUSE_SCROLL:
            g_app->scroll_x = e->scroll_x;
            g_app->scroll_y = e->scroll_y;
            break;
        default:
            break;
    }
}

static void render() {
    PROFILE_FUNC();

    {
        PROFILE_BLOCK("begin render pass");

        sg_pass_action pass = {};
        pass.colors[0].load_action = SG_LOADACTION_CLEAR;
        pass.colors[0].store_action = SG_STOREACTION_STORE;
        if (g_app->error_mode.load()) {
            pass.colors[0].clear_value = {0.0f, 0.0f, 0.0f, 1.0f};
        } else {
            float rgba[4];
            renderer_get_clear_color(rgba);
            pass.colors[0].clear_value.r = rgba[0];
            pass.colors[0].clear_value.g = rgba[1];
            pass.colors[0].clear_value.b = rgba[2];
            pass.colors[0].clear_value.a = rgba[3];
        }

        {
            LockGuard lock{&g_app->gpu_mtx};
            // sg_begin_default_pass(pass, sapp_width(), sapp_height());
            sg_pass ps = {.action = {.colors = {{.load_action = SG_LOADACTION_CLEAR, .clear_value = {0.f, 0.f, 0.f, 1.f}}}}, .swapchain = sglue_swapchain()};
            sg_begin_pass(&ps);
        }

        sgl_defaults();
        sgl_load_pipeline(g_pipeline);

        sgl_viewport(0, 0, sapp_width(), sapp_height(), true);
        sgl_ortho(0, sapp_widthf(), sapp_heightf(), 0, -1, 1);
    }

    if (g_app->error_mode.load()) {
        if (g_app->default_font == nullptr) {
            g_app->default_font = (FontFamily *)mem_alloc(sizeof(FontFamily));
            g_app->default_font->load_default();
        }

        renderer_reset();

        float x = 10;
        float y = 25;
        u64 font_size = 28;

        if (LockGuard lock{&g_app->error_mtx}) {
            y = draw_font(g_app->default_font, font_size, x, y, "-- ! Neko Error ! --");
            y += font_size;

            y = draw_font_wrapped(g_app->default_font, font_size, x, y, g_app->fatal_error, sapp_widthf() - x);
            y += font_size;

            if (g_app->traceback.data) {
                draw_font(g_app->default_font, font_size, x, y, g_app->traceback);
            }
        }
    } else {
        const int width = sapp_width();
        const int height = sapp_height();
        simgui_new_frame({width, height, sapp_frame_duration(), sapp_dpi_scale()});

        microui_begin();

        lua_State *L = g_app->L;

        luax_neko_get(L, "__timer_update");
        lua_pushnumber(L, g_app->time.delta);
        luax_pcall(L, 1, 0);

        {
            PROFILE_BLOCK("neko.frame");

            luax_neko_get(L, "frame");
            lua_pushnumber(L, g_app->time.delta);
            luax_pcall(L, 1, 0);
        }

        assert(lua_gettop(L) == 1);

        microui_end_and_present();

        if (ImGui::BeginMainMenuBar()) {
            ImGui::TextColored(ImVec4(0.19f, 1.f, 0.196f, 1.f), "Neko %d", neko_buildnum());

            sgimgui_draw_menu(&sgimgui, "gfx");

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - 275 - ImGui::GetScrollX());
            ImGui::Text("%.2f Mb %.2f Mb %.1lf ms/frame (%.1lf FPS)", lua_gc(L, LUA_GCCOUNT, 0) / 1024.f, (f32)g_allocator->alloc_size / (1024 * 1024), g_app->time.delta * 1000.f,
                        1.f / g_app->time.delta);

            ImGui::EndMainMenuBar();
        }
        sgimgui_draw(&sgimgui);
    }

    {
        PROFILE_BLOCK("gp render pass");
        int width = sapp_width(), height = sapp_height();
        float ratio = width / (float)height;

        sgp_begin(width, height);
        sgp_viewport(0, 0, width, height);

        // 绘图坐标空间
        sgp_project(-ratio, ratio, 1.0f, -1.0f);

        // 清除帧缓冲区
        sgp_set_color(0.1f, 0.1f, 0.1f, 1.0f);
        sgp_clear();

#if 0
        // 绘制一个可以旋转并改变颜色的动画矩形
        float time = sapp_frame_count() * sapp_frame_duration();
        float r = sinf(time) * 0.5 + 0.5, g = cosf(time) * 0.5 + 0.5;
        sgp_set_color(r, g, 0.3f, 1.0f);
        sgp_rotate_at(time, 0.0f, 0.0f);
        sgp_draw_filled_rect(-0.5f, -0.5f, 1.0f, 1.0f);
#endif
    }

    {
        PROFILE_BLOCK("end render pass");
        LockGuard lock{&g_app->gpu_mtx};

        // 将所有绘制命令分派至 Sokol GFX
        sgp_flush();
        // 完成绘制命令队列
        sgp_end();

        sgl_draw();

        sgl_error_t sgl_err = sgl_error();
        if (sgl_err != SGL_NO_ERROR) {
            neko_panic("a draw error occurred: %d", sgl_err);
        }

        simgui_render();

        sg_end_pass();
        sg_commit();
    }
}

static void frame() {
    PROFILE_FUNC();

    {
        AppTime *time = &g_app->time;
        u64 lap = stm_laptime(&time->last);
        time->delta = stm_sec(lap);
        time->accumulator += lap;

#if !defined(NEKO_IS_WEB)
        if (time->target_ticks > 0) {
            u64 TICK_MS = 1000000;
            u64 TICK_US = 1000;

            u64 target = time->target_ticks;

            if (time->accumulator < target) {
                u64 ms = (target - time->accumulator) / TICK_MS;
                if (ms > 0) {
                    PROFILE_BLOCK("sleep");
                    os_sleep(ms - 1);
                }

                {
                    PROFILE_BLOCK("spin loop");

                    u64 lap = stm_laptime(&time->last);
                    time->delta += stm_sec(lap);
                    time->accumulator += lap;

                    while (time->accumulator < target) {
                        os_yield();

                        u64 lap = stm_laptime(&time->last);
                        time->delta += stm_sec(lap);
                        time->accumulator += lap;
                    }
                }
            }

            u64 fuzz = TICK_US * 100;
            while (time->accumulator >= target - fuzz) {
                if (time->accumulator < target + fuzz) {
                    time->accumulator = 0;
                } else {
                    time->accumulator -= target + fuzz;
                }
            }
        }
#endif
    }

    g_app->gpu_mtx.unlock();
    render();
    assets_perform_hot_reload_changes();
    g_app->gpu_mtx.lock();

    memcpy(g_app->prev_key_state, g_app->key_state, sizeof(g_app->key_state));
    memcpy(g_app->prev_mouse_state, g_app->mouse_state, sizeof(g_app->mouse_state));
    g_app->prev_mouse_x = g_app->mouse_x;
    g_app->prev_mouse_y = g_app->mouse_y;
    g_app->scroll_x = 0;
    g_app->scroll_y = 0;

    Array<Sound *> &sounds = g_app->garbage_sounds;
    for (u64 i = 0; i < sounds.len;) {
        Sound *sound = sounds[i];

        if (sound->dead_end) {
            assert(sound->zombie);
            sound->trash();
            mem_free(sound);

            sounds[i] = sounds[sounds.len - 1];
            sounds.len--;
        } else {
            i++;
        }
    }
}

static void actually_cleanup() {
    PROFILE_FUNC();

    g_app->gpu_mtx.unlock();

    // just for test
    // neko_tiled_unload(&map);

    lua_State *L = g_app->L;

    {
        PROFILE_BLOCK("before quit");

        luax_neko_get(L, "before_quit");
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            String err = luax_check_string(L, -1);
            neko_panic("%s", err.data);
        }
    }

    microui_trash();

    {
        PROFILE_BLOCK("lua close");

        lua_pop(L, 1);  // luax_msgh

        neko::neko_lua_fini(L);
        // lua_close(L);
        // luaalloc_delete(g_app->LA);
    }

    {
        PROFILE_BLOCK("destroy assets");

        lua_channels_shutdown();

        if (g_app->default_font != nullptr) {
            g_app->default_font->trash();
            mem_free(g_app->default_font);
        }

        for (Sound *sound : g_app->garbage_sounds) {
            sound->trash();
        }
        g_app->garbage_sounds.trash();

        assets_shutdown();
    }

    {
        PROFILE_BLOCK("audio uninit");
        ma_engine_uninit(&g_app->audio_engine);
        mem_free(g_app->miniaudio_vfs);
    }

    {
        PROFILE_BLOCK("destory sokol");
        sgimgui_discard(&sgimgui);
        simgui_shutdown();

        sgp_shutdown();

        sgl_destroy_pipeline(g_pipeline);
        sgl_shutdown();
        sg_shutdown();
    }

    vfs_fini();

    mem_free(g_app->fatal_error.data);
    mem_free(g_app->traceback.data);

    for (String arg : g_app->args) {
        mem_free(arg.data);
    }
    mem_free(g_app->args.data);

    mem_free(g_app);

    g_init_mtx.trash();
}

static void cleanup() {
    actually_cleanup();

#ifdef USE_PROFILER
    profile_shutdown();
#endif

#ifndef NDEBUG
    DebugAllocator *allocator = dynamic_cast<DebugAllocator *>(g_allocator);
    if (allocator != nullptr) {
        allocator->dump_allocs();
    }
#endif

    g_allocator->trash();
    operator delete(g_allocator);

    neko_println("see ya");
}

static void neko_setup_w() {
    PROFILE_FUNC();

    lua_State *L = neko::neko_lua_create();

    // g_app->LA = LA;
    g_app->L = L;

    open_neko_api(L);

#if 0
    ENGINE_ECS() = ecs_init();
    ECS_IMPORT(ENGINE_ECS(), FlecsLua);
    ecs_lua_set_state(ENGINE_ECS(), ENGINE_LUA());
#endif

    open_luasocket(L);

    PRELOAD("enet", luaopen_enet);  // test
    PRELOAD("__neko.imgui", luaopen_imgui);  // test

    neko::lua::luax_run_bootstrap(L);

    lua_pushcfunction(L, luax_msgh);  // 添加错误消息处理程序 始终位于堆栈底部

    luax_neko_get(L, "__define_default_callbacks");
    luax_pcall(L, 0, 0);
}

static void load_all_lua_scripts(lua_State *L) {
    PROFILE_FUNC();

    Array<String> files = {};
    neko_defer({
        for (String str : files) {
            mem_free(str.data);
        }
        files.trash();
    });

    bool ok = vfs_list_all_files(NEKO_PACKS::GAMEDATA, &files);
    if (!ok) {
        neko_panic("failed to list all files");
    }
    std::qsort(files.data, files.len, sizeof(String), [](const void *a, const void *b) -> int {
        String *lhs = (String *)a;
        String *rhs = (String *)b;
        return std::strcmp(lhs->data, rhs->data);
    });

    for (String file : files) {
        if (file != "main.lua" && file.ends_with(".lua")) {
            NEKO_DEBUG_LOG("load_all_lua_scripts(\"%s\")", file.data);
            asset_load_kind(AssetKind_LuaRef, file, nullptr);
        } else {
        }
    }
}

App *g_app;
Allocator *g_allocator;

void do_test() {
    extern void test_containers();
    // test_containers();
}

sapp_desc sokol_main(int argc, char **argv) {
    g_init_mtx.make();
    LockGuard lock(&g_init_mtx);

#ifndef NDEBUG
    g_allocator = new DebugAllocator();
#else
    g_allocator = new HeapAllocator();
#endif

    g_allocator->make();

    os_high_timer_resolution();
    stm_setup();

    profile_setup();
    PROFILE_FUNC();

    // const char *mount_path = nullptr;

    // for (i32 i = 1; i < argc; i++) {
    //     if (argv[i][0] != '-') {
    //         mount_path = argv[i];
    //         break;
    //     }
    // }

    g_app = (App *)mem_alloc(sizeof(App));
    memset(g_app, 0, sizeof(App));

    g_app->args.resize(argc);
    for (i32 i = 0; i < argc; i++) {
        g_app->args[i] = to_cstr(argv[i]);
    }

#if defined(NDEBUG)
    NEKO_INFO("neko %d", neko_buildnum());
#else
    NEKO_INFO("neko %d (debug build)", neko_buildnum());
#endif

    neko_setup_w();
    lua_State *L = g_app->L;

#if defined(_DEBUG)
    MountResult mount = vfs_mount(NEKO_PACKS::GAMEDATA, "./gamedir");
#else
    MountResult mount = vfs_mount(NEKO_PACKS::GAMEDATA, nullptr);
#endif

    g_app->is_fused.store(mount.is_fused);

    if (!g_app->error_mode.load() && mount.ok) {
        asset_load_kind(AssetKind_LuaRef, "main.lua", nullptr);
    }

    if (!g_app->error_mode.load()) {
        luax_neko_get(L, "arg");

        lua_createtable(L, argc - 1, 0);
        for (i32 i = 1; i < argc; i++) {
            lua_pushstring(L, argv[i]);
            lua_rawseti(L, -2, i);
        }

        if (lua_pcall(L, 1, 0, 1) != LUA_OK) {
            lua_pop(L, 1);
        }
    }

    lua_newtable(L);
    i32 conf_table = lua_gettop(L);

    if (!g_app->error_mode.load()) {
        luax_neko_get(L, "conf");
        lua_pushvalue(L, conf_table);
        luax_pcall(L, 1, 0);
    }

    g_app->win_console = g_app->win_console || luax_boolean_field(L, -1, "win_console", true);

    bool hot_reload = luax_boolean_field(L, -1, "hot_reload", true);
    bool startup_load_scripts = luax_boolean_field(L, -1, "startup_load_scripts", true);
    bool fullscreen = luax_boolean_field(L, -1, "fullscreen", false);
    lua_Number reload_interval = luax_opt_number_field(L, -1, "reload_interval", 0.1);
    lua_Number swap_interval = luax_opt_number_field(L, -1, "swap_interval", 1);
    lua_Number target_fps = luax_opt_number_field(L, -1, "target_fps", 0);
    lua_Number width = luax_opt_number_field(L, -1, "window_width", 800);
    lua_Number height = luax_opt_number_field(L, -1, "window_height", 600);
    String title = luax_opt_string_field(L, -1, "window_title", "NekoEngine");
    String imgui_font = luax_opt_string_field(L, -1, "imgui_font", "");
    bool debug_on = luax_boolean_field(L, -1, "debug_on", true);

    lua_pop(L, 1);  // conf table

    if (!g_app->error_mode.load() && startup_load_scripts && mount.ok) {
        load_all_lua_scripts(L);
    }

    CVAR(conf_hot_reload, hot_reload);
    CVAR(conf_startup_load_scripts, startup_load_scripts);
    CVAR(conf_fullscreen, fullscreen);
    CVAR(conf_reload_interval, reload_interval);
    CVAR(conf_swap_interval, swap_interval);
    CVAR(conf_target_fps, target_fps);
    CVAR(conf_width, width);
    CVAR(conf_height, height);
    CVAR(conf_title, title);
    CVAR(conf_imgui_font, imgui_font);
    CVAR(conf_debug_on, debug_on);

    g_app->hot_reload_enabled.store(mount.can_hot_reload && hot_reload);
    g_app->reload_interval.store((u32)(reload_interval * 1000));

    if (target_fps != 0) {
        g_app->time.target_ticks = 1000000000 / target_fps;
    }

#ifdef NEKO_IS_WIN32
    if (!g_app->win_console) {
        FreeConsole();
    }
#endif

    sapp_desc sapp = {};
    sapp.init_cb = init;
    sapp.frame_cb = frame;
    sapp.cleanup_cb = cleanup;
    sapp.event_cb = event;
    sapp.width = (i32)width;
    sapp.height = (i32)height;
    sapp.window_title = title.data;
    sapp.logger.func = slog_func;
    sapp.swap_interval = (i32)swap_interval;
    sapp.fullscreen = fullscreen;
    sapp.allocator = sapp_allocator{
            .alloc_fn = +[](size_t size, void *user_data) { return mem_alloc(size); },
            .free_fn = +[](void *ptr, void *user_data) { return mem_free(ptr); },
    };
    sapp.win32_console_utf8 = true;

    return sapp;
}