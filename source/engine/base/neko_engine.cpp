#include "engine/base/neko_engine.h"

#include <functional>

#include "engine/audio/neko_audio.h"
#include "engine/common/neko_mem.h"
#include "engine/common/neko_str.h"
#include "engine/common/neko_util.h"
#include "engine/editor/neko_dbgui.hpp"
#include "engine/graphics/neko_graphics.h"
#include "engine/gui/neko_text_renderer.hpp"
#include "engine/math/neko_math.h"
#include "engine/platform/neko_platform.h"
#include "engine/scripting/neko_scripting.h"
#include "engine/utility/enum.hpp"
#include "engine/utility/logger.hpp"
#include "engine/utility/module.hpp"
#include "engine/utility/name.hpp"

using namespace neko;

neko_define_logger_impl();

// Global instance of neko engine (...THERE CAN ONLY BE ONE)
neko_global neko_engine_t *neko_engine_instance_g = {0};

// Function forward declarations
neko_result neko_engine_run();
neko_result neko_engine_shutdown();
neko_result __neko_default_app_init();
neko_result __neko_default_app_update();
neko_result __neko_default_app_shutdown();
void __neko_default_main_window_close_callback(void *window);

neko_resource_handle window;

ENUM_HPP_CLASS_DECL(neko_modules_op, u32, (init = 1)(shutdown = 2));
ENUM_HPP_REGISTER_TRAITS(neko_modules_op);

#define __neko_module_types() text_renderer, dbgui

// 递归展开类型列表，并调用函数模板
template <typename T, typename... Rest>
void __neko_module_command(cpp::TypeList<T, Rest...>, neko_modules_op op) {

    switch (op) {
        case neko_modules_op::init:
            neko_info(std::format("initialize {0}", std::string(cpp::type_name<T>().View())));

            modules::initialize<T>();
            break;
        case neko_modules_op::shutdown:
            neko_info(std::format("shutdown {0}", std::string(cpp::type_name<T>().View())));

            modules::shutdown<T>();
            break;
        default:
            break;
    }

    // 继续递归调用 __neko_module_command，处理 Rest...
    __neko_module_command(cpp::TypeList<Rest...>(), op);
}

// 终止递归的特化版本，处理空的 TypeList
void __neko_module_command(cpp::TypeList<>, neko_modules_op op) {
    // 结束操作
}

void __neko_engine_construct_high() {
    // auto f = [&]<typename T>(void) { modules::initialize<decltype(std::declval<T>)>(); };
    // using module_types_t = std::tuple<__neko_module_types()>;
    // std::apply([&](auto &&...args) { (f<decltype(args)>(), ...); }, module_types_t{});

    using __module_init_type_list = cpp::TypeList<__neko_module_types()>;
    __neko_module_command(__module_init_type_list{}, neko_modules_op::init);
}

void __neko_engine_shutdown_high() {
    // 逆排序 TypeList<__neko_module_types()>
    // 最先初始化的模块最后关闭
    using __module_shutdown_type_list = cpp::Reverse_t<cpp::TypeList<__neko_module_types()>>;
    __neko_module_command(__module_shutdown_type_list{}, neko_modules_op::shutdown);
}

neko_engine_t *neko_engine_construct() {
    if (neko_engine_instance_g == NULL) {

        neko_mem_init();

        // Construct instance
        neko_engine_instance_g = neko_malloc_init(neko_engine_t);

        // Set application description for engine
        // neko_engine_instance_g->ctx.app = app_desc;

        // Initialize the meta class registry
        // Want a way to add user meta class information as well as the engine's (haven't considered how that looks yet)
        // neko_meta_class_registry_init(&neko_engine_instance_g->ctx.registry);

        // neko_assert(neko_engine_instance_g->ctx.registry.classes != NULL);

        scripting script;

        script.__init();

        auto L = script.neko_lua.get_lua_state();

        neko_lua_auto_struct(L, neko_application_desc_t);
        neko_lua_auto_struct_member(L, neko_application_desc_t, window_title, const char *);
        neko_lua_auto_struct_member(L, neko_application_desc_t, window_width, unsigned int);
        neko_lua_auto_struct_member(L, neko_application_desc_t, window_height, unsigned int);

        try {
            script.neko_lua.load_file("data/scripts/main.lua");

            neko_application_desc_t t = {"Test", 1440, 840, neko_window_flags::resizable, 60.0f};
            // neko_lua_auto_push(L, neko_application_desc_t, &t);

            if (lua_istable(L, -1)) {

                if (lua_getfield(L, -1, "window_title") != LUA_TNIL) t.window_title = lua_tostring(L, -1);
                lua_pop(L, 1);

                if (lua_getfield(L, -1, "window_width") != LUA_TNIL) t.window_width = lua_tointeger(L, -1);
                lua_pop(L, 1);

                if (lua_getfield(L, -1, "window_height") != LUA_TNIL) t.window_height = lua_tointeger(L, -1);
                lua_pop(L, 1);

                if (lua_getfield(L, -1, "window_flags") != LUA_TNIL) t.window_flags = (neko_window_flags)lua_tointeger(L, -1);
                lua_pop(L, 1);

                if (lua_getfield(L, -1, "frame_rate") != LUA_TNIL) t.frame_rate = lua_tonumber(L, -1);
                lua_pop(L, 1);

                lua_pop(L, 1);
            }

            neko_engine_instance_g->ctx.app = t;

        } catch (std::exception &ex) {
            neko_error(ex.what());
        }

        // Set up function pointers
        neko_engine_instance_g->run = &neko_engine_run;
        neko_engine_instance_g->shutdown = &neko_engine_shutdown;

        // Need to have video settings passed down from user
        neko_engine_instance_g->ctx.platform = neko_platform_construct();

        // Default initialization for platform here
        __neko_default_init_platform(neko_engine_instance_g->ctx.platform);

        // Set frame rate for application
        if (neko_engine_instance_g->ctx.app.frame_rate > 0.f) {
            neko_engine_instance_g->ctx.platform->time.max_fps = neko_engine_instance_g->ctx.app.frame_rate;
        }

        // Set vsync for video
        neko_engine_instance_g->ctx.platform->enable_vsync(false);

        // Construct window
        neko_engine_instance()->ctx.platform->create_window(neko_engine_instance_g->ctx.app.window_title, neko_engine_instance_g->ctx.app.window_width, neko_engine_instance_g->ctx.app.window_height);

        // Construct graphics api
        neko_engine_instance_g->ctx.graphics = __neko_graphics_construct();

        // Initialize graphics here
        neko_engine_instance_g->ctx.graphics->init(neko_engine_instance_g->ctx.graphics);

        // Construct audio api
        neko_engine_instance_g->ctx.audio = __neko_audio_construct();

        // Initialize audio
        neko_engine_instance_g->ctx.audio->init(neko_engine_instance_g->ctx.audio);

        // Default application context
        neko_engine_instance_g->ctx.init = &__neko_default_app_init;
        neko_engine_instance_g->ctx.update = &__neko_default_app_update;
        neko_engine_instance_g->ctx.shutdown = &__neko_default_app_shutdown;

        __neko_engine_construct_high();

        if (neko_engine_instance_g->ctx.init) {
            if (neko_engine_instance_g->ctx.init() != neko_result_success) {
                // Show error before aborting
                neko_error("application failed to initialize.");
                neko_assert(false);
            }
        }

        neko_engine_instance_g->ctx.is_running = true;

        // Set default callback for when main window close button is pressed
        neko_platform_i *platform = neko_engine_instance_g->ctx.platform;
        platform->set_window_close_callback(platform->main_window(), &__neko_default_main_window_close_callback);
    }

    return neko_engine_instance_g;
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
            return (neko_engine_instance()->shutdown());
        }

        // Process application context
        if (neko_engine_instance()->ctx.update() != neko_result_in_progress || !neko_engine_instance()->ctx.is_running) {
            // Shutdown engine and return
            return (neko_engine_instance()->shutdown());
        }

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

    __neko_engine_shutdown_high();

    neko_safe_free(neko_engine_instance()->ctx.platform->ctx.gamepath);

    // 在 app 结束后进行内存检查
    neko_mem_end();

    return app_result;
}

neko_engine_t *neko_engine_instance() { return neko_engine_instance_g; }

neko_result __neko_default_app_init() {
    extern neko_result app_init();
    return app_init();
}

neko_result __neko_default_app_update() {
    extern neko_result app_update();
    return app_update();
}

neko_result __neko_default_app_shutdown() {
    extern neko_result app_shutdown();
    return app_shutdown();
}

void __neko_default_main_window_close_callback(void *window) { neko_engine_instance()->ctx.is_running = false; }

/*========================
// Hash Table
========================*/

void __neko_hash_table_init_impl(void **ht, size_t sz) { *ht = neko_safe_malloc(sz); }