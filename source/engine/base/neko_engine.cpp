#include "engine/base/neko_engine.h"

#include <functional>

#include "engine/audio/neko_audio.h"
#include "engine/base/neko_component.h"
#include "engine/base/neko_meta.hpp"
#include "engine/common/neko_mem.h"
#include "engine/common/neko_str.h"
#include "engine/common/neko_util.h"
#include "engine/editor/neko_dbgui.hpp"
#include "engine/graphics/neko_graphics.h"
#include "engine/math/neko_math.h"
#include "engine/platform/neko_platform.h"
#include "engine/utility/enum.hpp"
#include "engine/utility/logger.hpp"
#include "engine/utility/module.hpp"
#include "engine/utility/name.hpp"

using namespace neko;

using namespace std::literals;

neko_define_logger_impl();

// Global instance of neko engine
neko_global neko::scope<neko_engine> g_neko;

// Function forward declarations
neko_result neko_engine_run();
neko_result neko_engine_shutdown();
neko_result __neko_default_game_init();
neko_result __neko_default_game_update();
neko_result __neko_default_game_shutdown();
void __neko_default_main_window_close_callback(void *window);

neko_resource_handle window;

ENUM_HPP_CLASS_DECL(neko_modules_op, u32, (init = 1)(shutdown = 2)(update = 3));
ENUM_HPP_REGISTER_TRAITS(neko_modules_op);

#define __neko_module_types() dbgui

// 递归展开类型列表，并调用函数模板
template <typename T, typename... Rest>
void __neko_module_command(cpp::TypeList<T, Rest...>, neko_modules_op op) {
    try {
        switch (op) {
            case neko_modules_op::init:
                neko_info(std::format("initialize {0}", std::string(cpp::type_name<T>().View())));
                modules::initialize<T>();
                break;
            case neko_modules_op::shutdown:
                neko_info(std::format("shutdown {0}", std::string(cpp::type_name<T>().View())));
                modules::shutdown<T>();
                break;
            case neko_modules_op::update:
                the<T>().__update();
                break;
            default:
                break;
        }

        // 继续递归调用 __neko_module_command，处理 Rest...
        __neko_module_command(cpp::TypeList<Rest...>(), op);
    } catch (std::exception &ex) {
        neko_error("__neko_module_command failed ", ex.what());
    }
}

// 终止递归的特化版本，处理空的 TypeList
void __neko_module_command(cpp::TypeList<>, neko_modules_op op) {
    // 结束操作
}

void __neko_engine_construct_high() {
    using __module_init_type_list = cpp::TypeList<__neko_module_types()>;
    __neko_module_command(__module_init_type_list{}, neko_modules_op::init);
}

void __neko_engine_shutdown_high() {
    // 逆排序 TypeList<__neko_module_types()>
    // 最先初始化的模块最后关闭
    using __module_shutdown_type_list = cpp::Reverse_t<cpp::TypeList<__neko_module_types()>>;
    __neko_module_command(__module_shutdown_type_list{}, neko_modules_op::shutdown);
}

void __neko_engine_update_high() {
    using __module_init_type_list = cpp::TypeList<__neko_module_types()>;
    __neko_module_command(__module_init_type_list{}, neko_modules_op::update);
}

neko_engine *neko_engine_construct(int argc, char **argv) {

    if (g_neko != NULL) {
        neko_assert(!g_neko, "repeated initialization");
        return g_neko.get();
    }

    __neko_mem_init(argc, argv);

    // Construct instance
    g_neko = create_scope<neko_engine>();

    // Set application description for engine
    // g_neko->ctx.app = app_desc;

    // Initialize the meta class registry
    // Want a way to add user meta class information as well as the engine's (haven't considered how that looks yet)
    // neko_meta_class_registry_init(&g_neko->ctx.registry);

    // neko_assert(g_neko->ctx.registry.classes != NULL);

    // TODO: 初始化反射

    // 设置引擎主函数指针
    g_neko->engine_run = &neko_engine_run;
    g_neko->engine_shutdown = &neko_engine_shutdown;

    // 初始化 cvars
    __neko_config_init();

    // 初始化Platform子系统
    g_neko->ctx.platform = neko_platform_construct();    // 需要从用户那里传递视频设置
    __neko_default_init_platform(g_neko->ctx.platform);  // 此处平台的默认初始化

    // 初始化脚本系统
    neko_sc()->__init();

    auto L = neko_sc()->neko_lua.get_lua_state();

    try {

        lua_newtable(L);
        for (int n = 0; n < argc; ++n) {
            lua_pushstring(L, argv[n]);
            lua_rawseti(L, -2, n);
        }
        lua_setglobal(L, "arg");

        neko_sc()->neko_lua.load_file(neko_file_path("data/scripts/main.lua"));

        neko_application_desc_t t = {"Test", 1440, 840, neko_window_flags::resizable, 60.0f};

        // neko_lua_auto_push(L, neko_application_dedsc_t, &t);

        if (lua_getglobal(L, "neko_app") == LUA_TNIL) throw std::exception("no app");

        if (lua_istable(L, -1)) {
            neko::meta::static_refl::TypeInfo<neko_application_desc_t>::ForEachVarOf(t, [&L](auto field, auto &&value) {
                static_assert(std::is_lvalue_reference_v<decltype(value)>);
                if (lua_getfield(L, -1, std::string(field.name).c_str()) != LUA_TNIL) value = neko::neko_lua_to<std::remove_reference_t<decltype(value)>>(L, -1);
                lua_pop(L, 1);
            });
        } else {
            throw std::exception("no app table");
        }

        lua_pop(L, 1);

        g_neko->ctx.app = t;

        // Set frame rate for application
        if (g_neko->ctx.app.frame_rate > 0.f) g_neko->ctx.platform->time.max_fps = g_neko->ctx.app.frame_rate;

        // Set vsync for video
        g_neko->ctx.platform->enable_vsync(false);

        // Construct window
        neko_engine_instance()->ctx.platform->create_window(g_neko->ctx.app.window_title, g_neko->ctx.app.window_width, g_neko->ctx.app.window_height);

        // Construct graphics api
        g_neko->ctx.graphics = __neko_graphics_construct();

        // Initialize graphics here
        g_neko->ctx.graphics->init(g_neko->ctx.graphics);

        // Construct audio api
        g_neko->ctx.audio = __neko_audio_construct();

        // Initialize audio
        g_neko->ctx.audio->init(g_neko->ctx.audio);

        // Default application context
        g_neko->ctx.update = &__neko_default_game_update;
        g_neko->ctx.shutdown = &__neko_default_game_shutdown;

        __neko_engine_construct_high();

        if (__neko_default_game_init() != neko_result_success) {
            // Show error before aborting
            neko_error("application failed to initialize.");
            neko_assert(false);
        }

        g_neko->ctx.is_running = true;

        // Set default callback for when main window close button is pressed
        neko_platform_i *platform = g_neko->ctx.platform;
        platform->set_window_close_callback(platform->main_window(), &__neko_default_main_window_close_callback);

        return g_neko.get();

    } catch (std::exception &ex) {
        neko_error(ex.what());

        return nullptr;
    }
}

neko_result neko_engine_run() {
    // Main engine loop
    while (true) {
        static u32 curr_ticks = 0;
        static u32 prev_ticks = 0;

        // Cache platform pointer
        neko_platform_i *platform = neko_engine_instance()->ctx.platform;
        neko_graphics_i *gfx = neko_engine_instance()->ctx.graphics;
        neko_audio_i *audio = neko_engine_instance()->ctx.audio;

        // Cache times at start of frame
        platform->time.current = platform->elapsed_time();
        platform->time.update = platform->time.current - platform->time.previous;
        platform->time.previous = platform->time.current;

        // Update platform input from previous frame
        platform->update_input(&platform->input);

        // Process input for this frame
        if (platform->process_input(NULL) != neko_result_in_progress) {
            return (neko_engine_instance()->engine_shutdown());
        }

        neko_imgui_new_frame();

        // Process application context
        if (neko_engine_instance()->ctx.update() != neko_result_in_progress || !neko_engine_instance()->ctx.is_running) {
            // Shutdown engine and return
            return (neko_engine_instance()->engine_shutdown());
        }

        try {
            neko_sc()->neko_lua.call("test_update");
        } catch (std::exception &ex) {
            neko_error(ex.what());
        }

        __neko_engine_update_high();

        gfx->fontcache_draw();

        // 渲染 imgui 内容
        neko_imgui_render();

        // Audio update and commit
        if (audio) {
            if (audio->update) {
                audio->update(audio);
            }
            if (audio->commit) {
                audio->commit(audio);
            }
        }

        // Graphics update and commit
        if (gfx && gfx->update) {
            gfx->update(gfx);
        }

        // Swap all platform window buffers? Sure...
        neko_for_range_i(neko_dyn_array_size(platform->active_window_handles)) { platform->window_swap_buffer(platform->active_window_handles[i]); }

        // Frame locking
        platform->time.current = platform->elapsed_time();
        platform->time.render = platform->time.current - platform->time.previous;
        platform->time.previous = platform->time.current;
        platform->time.frame = platform->time.update + platform->time.render;  // Total frame time
        platform->time.delta = platform->time.frame / 1000.f;

        f32 target = (1000.f / platform->time.max_fps);

        if (platform->time.frame < target) {
            platform->sleep((f32)(target - platform->time.frame));

            platform->time.current = platform->elapsed_time();
            f64 wait_time = platform->time.current - platform->time.previous;
            platform->time.previous = platform->time.current;
            platform->time.frame += wait_time;
            platform->time.delta = platform->time.frame / 1000.f;
        }
    }

    // Shouldn't hit here
    neko_assert(false);
    return neko_result_failure;
}

neko_result neko_engine_shutdown() {
    // Shutdown application
    neko_result app_result = (neko_engine_instance()->ctx.shutdown());

    neko_imgui_destroy();

    neko_ecs_destroy(neko_engine_subsystem(ecs));

    __neko_engine_shutdown_high();

    neko_safe_free(neko_engine_instance()->ctx.platform->ctx.gamepath);

    neko_sc()->__end();

    __neko_config_free();

    // 在 app 结束后进行内存检查
    __neko_mem_end();

    return app_result;
}

neko_engine *neko_engine_instance() { return g_neko.get(); }

neko_result __neko_default_game_init() {
    // 初始化 ecs
    neko_engine_subsystem(ecs) = neko_ecs_make(1024, COMPONENT_COUNT, 2);

    // 启用 imgui
    neko_imgui_init();

    try {
        neko_sc()->neko_lua.call("game_init");
    } catch (std::exception &ex) {
        neko_error(ex.what());
    }

    extern neko_result app_init();
    return app_init();
}

neko_result __neko_default_game_update() {
    extern neko_result app_update();
    return app_update();
}

neko_result __neko_default_game_shutdown() {

    try {
        neko_sc()->neko_lua.call("game_shutdown");
    } catch (std::exception &ex) {
        neko_error(ex.what());
    }

    extern neko_result app_shutdown();
    return app_shutdown();
}

void __neko_default_main_window_close_callback(void *window) { neko_engine_instance()->ctx.is_running = false; }

/*========================
// Hash Table
========================*/

void __neko_hash_table_init_impl(void **ht, size_t sz) { *ht = neko_safe_malloc(sz); }