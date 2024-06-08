
#include "neko.h"
#include "neko_engine.h"

//=============================
// NEKO_ENGINE
//=============================

NEKO_API_DECL void neko_default_main_window_close_callback(void* window);

NEKO_API_DECL neko_t* neko_instance() {
    NEKO_STATIC neko_t g_neko_instance = NEKO_DEFAULT_VAL();
    return &g_neko_instance;
}

NEKO_API_DECL neko_t* neko_create(int argc, char** argv) {
    if (neko_instance() != NULL) {

        __neko_mem_init(argc, argv);
        
        // 设置函数指针
        neko_instance()->fini = &neko_fini;

        neko_app();

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
        neko_subsystem(platform) = neko_platform_create();

        // 此处平台的默认初始化
        neko_platform_init(neko_subsystem(platform));

        // 设置应用程序的帧速率
        neko_subsystem(platform)->time.max_fps = neko_cvar("settings.window.frame_rate")->value.f;

        neko_platform_running_desc_t window = {.title = "Neko Engine", .width = 1280, .height = 720, .vsync = false, .frame_rate = 60.f, .hdpi = false, .center = true, .running_background = true};

        // 构建主窗口
        neko_platform_window_create(&window);

        // 设置视频垂直同步
        neko_platform_enable_vsync(neko_cvar("settings.window.vsync")->value.i);

        // 构建图形API
        neko_subsystem(render) = neko_render_create();

        // 初始化图形
        neko_render_init(neko_subsystem(render));

        // // 构建音频API
        // neko_subsystem(audio) = __neko_audio_construct();

        // // 初始化音频
        // neko_subsystem(audio)->init(neko_subsystem(audio));

        // 初始化应用程序并设置为运行
        neko_instance()->init();
        neko_instance()->game.is_running = true;

        // 设置按下主窗口关闭按钮时的默认回调
        neko_platform_set_window_close_callback(neko_platform_main_window(), &neko_default_main_window_close_callback);
    }

    return neko_instance();
}

// 主框架函数
NEKO_API_DECL void neko_frame() {
    // Remove these...
    NEKO_PRIVATE(u32) curr_ticks = 0;
    NEKO_PRIVATE(u32) prev_ticks = 0;

    {
        //        uintptr_t profile_id_engine;
        //        profile_id_engine = neko_profiler_begin_scope(__FILE__, __LINE__, "engine");

        // Cache platform pointer
        neko_platform_t* platform = neko_subsystem(platform);
        // neko_audio_t* audio = neko_subsystem(audio);

        neko_platform_window_t* win = (neko_slot_array_getp(platform->windows, neko_platform_main_window()));

        // Cache times at start of frame
        platform->time.elapsed = (f32)neko_platform_elapsed_time();
        platform->time.update = platform->time.elapsed - platform->time.previous;
        platform->time.previous = platform->time.elapsed;

        // Update platform and process input
        neko_platform_update(platform);

        if (win->focus /*|| neko_instance()->game.window.running_background*/) {

            // Process application context
            neko_instance()->update();

            {
                // TODO:: 这里设置清理 garbage_sounds
                // Audio update and commit
                // if (audio) {
                //     if (audio->update) {
                //         audio->update(audio);
                //     }
                //     if (audio->commit) {
                //         audio->commit(audio);
                //     }
                // }
            }
        }

        // Clear all platform events
        neko_dyn_array_clear(platform->events);

        {
            for (neko_slot_array_iter it = 0; neko_slot_array_iter_valid(platform->windows, it); neko_slot_array_iter_advance(platform->windows, it)) {
                neko_platform_window_swap_buffer(it);
            }
            // neko_platform_window_swap_buffer(neko_platform_main_window());
        }

        // Frame locking (not sure if this should be done here, but it is what it is)
        platform->time.elapsed = (f32)neko_platform_elapsed_time();
        platform->time.render = platform->time.elapsed - platform->time.previous;
        platform->time.previous = platform->time.elapsed;
        platform->time.frame = platform->time.update + platform->time.render;  // Total frame time
        platform->time.delta = platform->time.frame / 1000.f;

        f32 target = (1000.f / platform->time.max_fps);

        if (platform->time.frame < target) {
            neko_platform_sleep((f32)(target - platform->time.frame));
            platform->time.elapsed = (f32)neko_platform_elapsed_time();
            double wait_time = platform->time.elapsed - platform->time.previous;
            platform->time.previous = platform->time.elapsed;
            platform->time.frame += wait_time;
            platform->time.delta = platform->time.frame / 1000.f;
        }

        //        neko_profiler_end_scope(profile_id_engine);
    }

    if (!neko_instance()->game.is_running) {
        neko_instance()->fini();
        return;
    }
}

void neko_fini() {
    // Shutdown application
    neko_instance()->shutdown();
    neko_instance()->game.is_running = false;

    neko_render_shutdown(neko_subsystem(render));
    neko_render_destroy(neko_subsystem(render));

    // neko_audio_shutdown(neko_subsystem(audio));
    // neko_audio_destroy(neko_subsystem(audio));

    neko_platform_shutdown(neko_subsystem(platform));
    neko_platform_destroy(neko_subsystem(platform));

    __neko_config_free();

    // 在 app 结束后进行内存检查
    __neko_mem_fini();
}

NEKO_API_DECL void neko_default_main_window_close_callback(void* window) { neko_instance()->game.is_running = false; }

void neko_quit() {
#ifndef NEKO_PLATFORM_WEB
    neko_instance()->game.is_running = false;
#endif
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