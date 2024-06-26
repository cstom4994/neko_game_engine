
#include "engine/neko.hpp"
#include "engine/neko_lua.hpp"
#include "neko.h"
#include "neko_engine.h"

//=============================
// NEKO_ENGINE
//=============================

using namespace neko;

s32 random_val(s32 lower, s32 upper) {
    if (upper < lower) {
        s32 tmp = lower;
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

    auto ws = neko_game().DisplaySize;

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

void neko_register(lua_State* L);

lua_State* neko_scripting_init() {
    neko::timer timer;
    timer.start();

    lua_State* L = neko_lua_create();

    lua_atpanic(
            L, +[](lua_State* L) {
                auto msg = neko_lua_to<const_str>(L, -1);
                NEKO_ERROR("[lua] panic error: %s", msg);
                return 0;
            });
    neko_register(L);

    // neko_lua_run_string(L, std::format("package.path = "
    //                                    "'{1}/?.lua;{0}/?.lua;{0}/../libs/?.lua;{0}/../libs/?/init.lua;{0}/../libs/"
    //                                    "?/?.lua;' .. package.path",
    //                                    game_assets("lua_scripts"), neko::fs_normalize_path(std::filesystem::current_path().string()).c_str()));

    // neko_lua_run_string(L, std::format("package.cpath = "
    //                                    "'{1}/?.{2};{0}/?.{2};{0}/../libs/?.{2};{0}/../libs/?/init.{2};{0}/../libs/"
    //                                    "?/?.{2};' .. package.cpath",
    //                                    game_assets("lua_scripts"), neko::fs_normalize_path(std::filesystem::current_path().string()).c_str(), "dll"));

    timer.stop();
    NEKO_INFO(std::format("lua init done in {0:.3f} ms", timer.get()).c_str());

    return L;
}

void neko_scripting_end(lua_State* L) { neko_lua_fini(L); }

NEKO_API_DECL void neko_default_main_window_close_callback(void* window);

NEKO_API_DECL neko_t* neko_instance() {
    NEKO_STATIC neko_t g_neko_instance = NEKO_DEFAULT_VAL();
    return &g_neko_instance;
}

Neko_OnModuleLoad(Sound);
Neko_OnModuleLoad(Physics);

NEKO_API_DECL neko_t* neko_create(int argc, char** argv) {
    if (neko_instance() != NULL) {

        __neko_mem_init(argc, argv);

        neko_instance()->console = &g_console;

        // 初始化 cvars
        __neko_config_init();

        neko_cvar_lnew("settings.window.width", __NEKO_CONFIG_TYPE_INT, 1280);
        neko_cvar_lnew("settings.window.height", __NEKO_CONFIG_TYPE_INT, 720);
        neko_cvar_lnew("settings.window.vsync", __NEKO_CONFIG_TYPE_INT, 0);
        neko_cvar_lnew("settings.window.frame_rate", __NEKO_CONFIG_TYPE_FLOAT, 60.f);
        neko_cvar_lnew("settings.window.hdpi", __NEKO_CONFIG_TYPE_INT, 0);
        neko_cvar_lnew("settings.window.center", __NEKO_CONFIG_TYPE_INT, 1);
        neko_cvar_lnew("settings.window.running_background", __NEKO_CONFIG_TYPE_INT, 1);
        neko_cvar_lnew("settings.window.monitor_index", __NEKO_CONFIG_TYPE_INT, 0);
        neko_cvar_lnew("settings.video.render.debug", __NEKO_CONFIG_TYPE_INT, 0);
        neko_cvar_lnew("settings.video.render.hdpi", __NEKO_CONFIG_TYPE_INT, 0);

        // 需要从用户那里传递视频设置
        neko_subsystem(platform) = neko_pf_create();

        // 此处平台的默认初始化
        neko_pf_init(neko_subsystem(platform));

        // 设置应用程序的帧速率
        neko_subsystem(platform)->time.max_fps = neko_cvar("settings.window.frame_rate")->value.f;

        neko_pf_running_desc_t window = {.title = "Neko Engine", .width = 1280, .height = 720, .vsync = false, .frame_rate = 60.f, .hdpi = false, .center = true, .running_background = true};

        // 构建主窗口
        neko_pf_window_create(&window);

        // 设置视频垂直同步
        neko_pf_enable_vsync(neko_cvar("settings.window.vsync")->value.i);

        // 构建图形API
        neko_subsystem(render) = neko_render_create();

        // 初始化图形
        neko_render_init(neko_subsystem(render));

        // 初始化应用程序并设置为运行

        mount_result mount;

#if defined(NEKO_DEBUG_BUILD)
        mount = neko::vfs_mount(NEKO_PACK_GAMEDATA, "gamedir/../");
        mount = neko::vfs_mount(NEKO_PACK_LUACODE, "gamedir/../");
#else
        mount = neko::vfs_mount(NEKO_PACK_GAMEDATA, "gamedir/../gamedata.zip");
        mount = neko::vfs_mount(NEKO_PACK_LUACODE, "gamedir/../luacode.zip");
#endif

        neko_module_interface_init(ENGINE_INTERFACE());

        neko_instance()->L = neko_scripting_init();

        // neko_lua_safe_dofile(neko_instance()->L, "startup");

        {
            neko::lua_bind::bind("__neko_file_path", +[](const_str path) -> std::string { return path; });
            lua_newtable(neko_instance()->L);
            for (int n = 0; n < argc; ++n) {
                lua_pushstring(neko_instance()->L, argv[n]);
                lua_rawseti(neko_instance()->L, -2, n);
            }
            lua_setglobal(neko_instance()->L, "arg");
        }

        auto& module_list = ENGINE_INTERFACE()->modules;

        Neko_Module m = {};
        s32 sss;
        sss = Neko_OnModuleLoad_Sound(&m, ENGINE_INTERFACE());
        neko_dyn_array_push(module_list, m);
        sss = Neko_OnModuleLoad_Physics(&m, ENGINE_INTERFACE());
        neko_dyn_array_push(module_list, m);

        for (u32 i = 0; i < neko_dyn_array_size(module_list); ++i) {
            Neko_Module& module = module_list[i];
            module.func.OnInit(neko_instance()->L);
        }

        {
            u32 w, h;
            u32 display_w, display_h;

            neko_pf_window_size(neko_pf_main_window(), &w, &h);
            neko_pf_framebuffer_size(neko_pf_main_window(), &display_w, &display_h);

            neko_game().DisplaySize = neko_v2((float)w, (float)h);
            neko_game().DisplayFramebufferScale = neko_v2((float)display_w / w, (float)display_h / h);
        }

#if 0
        {

            ENGINE_INTERFACE()->ecs = ecs_init(neko_instance()->L);

            auto registry = ENGINE_INTERFACE()->ecs;
            const ecs_entity_t pos_component = ECS_COMPONENT(registry, position_t);
            const ecs_entity_t vel_component = ECS_COMPONENT(registry, velocity_t);
            const ecs_entity_t bou_component = ECS_COMPONENT(registry, bounds_t);
            const ecs_entity_t col_component = ECS_COMPONENT(registry, color_t);
            const ecs_entity_t obj_component = ECS_COMPONENT(registry, CGameObjectTest);

            for (int i = 0; i < 5; i++) {

                neko_vec2_t bounds = neko_v2((f32)random_val(10, 100), (f32)random_val(10, 100));

                position_t p = {(f32)random_val(0, (int32_t)neko_game().DisplaySize.x - (int32_t)bounds.x), (f32)random_val(0, (int32_t)neko_game().DisplaySize.y - (int32_t)bounds.y)};
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

            ECS_SYSTEM(registry, neko::ecs_component::movement_system, 4, pos_component, vel_component, bou_component, col_component);
            ECS_SYSTEM(registry, neko::ecs_component::render_system, 4, pos_component, vel_component, bou_component, col_component);

            // const ecs_entity_t test_component = ECS_COMPONENT(registry, uptr);
            // for (int i = 0; i < 1024; i++) {
            //     ecs_entity_t e = ecs_entity(registry);
            //     ecs_attach(registry, e, test_component);
            // }
        }
#endif

        neko_app(neko_instance()->L);

        // neko_lua_safe_dofile(neko_instance()->L, "boot");

        std::string boot_code = R"lua(
startup = require "startup"

-- local w = neko.ecs_f()
-- local e = w:create_ent()
-- w:attach(e, "neko_client_userdata_t")
-- set_client_init(e)
-- print(e)

local boot = require "boot"

local w = boot.fetch_world("sandbox")

function boot_init()

    local game_userdata = sandbox_init()

    w:register("neko_client_userdata_t", {ud})
    local eid1 = w:new{
        neko_client_userdata_t = {
            ud = game_userdata
        }
    }
end

function boot_update()
    for cl in w:match("all", "neko_client_userdata_t") do
        sandbox_update(cl.ud)
    end
    w:update()
end

function boot_fini()
    for cl in w:match("all", "neko_client_userdata_t") do
        sandbox_fini(cl.ud)
    end
end
)lua";

        neko_lua_run_string(neko_instance()->L, boot_code.c_str());

        neko_instance()->init();
        neko_instance()->game.is_running = true;

        // 设置按下主窗口关闭按钮时的默认回调
        neko_pf_set_window_close_callback(neko_pf_main_window(), &neko_default_main_window_close_callback);
    }

    return neko_instance();
}

// 主框架函数
NEKO_API_DECL void neko_frame() {

    {
        //        uintptr_t profile_id_engine;
        //        profile_id_engine = neko_profiler_begin_scope(__FILE__, __LINE__, "engine");

        neko_pf_t* platform = neko_subsystem(platform);

        neko_pf_window_t* win = (neko_slot_array_getp(platform->windows, neko_pf_main_window()));

        // 帧开始时的缓存时间
        platform->time.elapsed = (f32)neko_pf_elapsed_time();
        platform->time.update = platform->time.elapsed - platform->time.previous;
        platform->time.previous = platform->time.elapsed;

        // 更新平台和流程输入
        neko_pf_update(platform);

        if (win->focus /*|| neko_instance()->game.window.running_background*/) {

            neko_instance()->update();

            // neko_idraw_defaults(&ENGINE_INTERFACE()->idraw);
            // neko_idraw_camera2d(&ENGINE_INTERFACE()->idraw, neko_game().DisplaySize.x, neko_game().DisplaySize.y);
            // ecs_step(ENGINE_INTERFACE()->ecs);

            neko_instance()->post_update();
            {
                neko_render_command_buffer_submit(&ENGINE_INTERFACE()->cb);
                neko_check_gl_error();
            }

            auto& module_list = ENGINE_INTERFACE()->modules;
            for (u32 i = 0; i < neko_dyn_array_size(module_list); ++i) {
                Neko_Module& module = module_list[i];
                module.func.OnPostUpdate(neko_instance()->L);
            }
        }

        // 清除所有平台事件
        neko_dyn_array_clear(platform->events);

        {
            for (neko_slot_array_iter it = 0; neko_slot_array_iter_valid(platform->windows, it); neko_slot_array_iter_advance(platform->windows, it)) {
                neko_pf_window_swap_buffer(it);
            }
            // neko_pf_window_swap_buffer(neko_pf_main_window());
        }

        // 帧锁定
        platform->time.elapsed = (f32)neko_pf_elapsed_time();
        platform->time.render = platform->time.elapsed - platform->time.previous;
        platform->time.previous = platform->time.elapsed;
        platform->time.frame = platform->time.update + platform->time.render;  // 总帧时间
        platform->time.delta = platform->time.frame / 1000.f;

        f32 target = (1000.f / platform->time.max_fps);

        if (platform->time.frame < target) {
            neko_pf_sleep((f32)(target - platform->time.frame));
            platform->time.elapsed = (f32)neko_pf_elapsed_time();
            double wait_time = platform->time.elapsed - platform->time.previous;
            platform->time.previous = platform->time.elapsed;
            platform->time.frame += wait_time;
            platform->time.delta = platform->time.frame / 1000.f;
        }

        //        neko_profiler_end_scope(profile_id_engine);
    }

    if (!neko_instance()->game.is_running) {
        neko_fini();
        return;
    }
}

void neko_fini() {
    // Shutdown application
    neko_instance()->shutdown();
    neko_instance()->game.is_running = false;

    auto& module_list = ENGINE_INTERFACE()->modules;
    for (u32 i = 0; i < neko_dyn_array_size(module_list); ++i) {
        Neko_Module& module = module_list[i];
        module.func.OnFini(neko_instance()->L);
    }

    neko_module_interface_fini(ENGINE_INTERFACE());

    neko_scripting_end(neko_instance()->L);

    neko::vfs_fini({});

    neko_render_shutdown(neko_subsystem(render));
    neko_render_destroy(neko_subsystem(render));

    neko_pf_shutdown(neko_subsystem(platform));
    neko_pf_destroy(neko_subsystem(platform));

    __neko_config_free();

    // 在 app 结束后进行内存检查
    __neko_mem_fini();
}

NEKO_API_DECL void neko_default_main_window_close_callback(void* window) { neko_instance()->game.is_running = false; }

void neko_quit() {
#ifndef NEKO_PF_WEB
    neko_instance()->game.is_running = false;
#endif
}

void neko_module_interface_init(Neko_ModuleInterface* module_interface) {

    module_interface->common.__neko_mem_safe_alloc = &__neko_mem_safe_alloc;
    module_interface->common.__neko_mem_safe_calloc = &__neko_mem_safe_calloc;
    module_interface->common.__neko_mem_safe_realloc = &__neko_mem_safe_realloc;
    module_interface->common.__neko_mem_safe_free = &__neko_mem_safe_free;
    module_interface->common.capi_vfs_read_file = &neko_capi_vfs_read_file;

    module_interface->cb = neko_command_buffer_new();
    module_interface->idraw = neko_immediate_draw_new();
    module_interface->am = neko_asset_manager_new();
}

void neko_module_interface_fini(Neko_ModuleInterface* module_interface) {

    neko_immediate_draw_free(&module_interface->idraw);
    neko_command_buffer_free(&module_interface->cb);
    neko_asset_manager_free(&module_interface->am);
}

//=============================
// MAIN
//=============================

s32 main(s32 argv, char** argc) {
    neko_t* inst = neko_create(argv, argc);
    while (neko_instance()->game.is_running) {
        neko_frame();
    }
    return 0;
}