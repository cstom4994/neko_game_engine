#pragma once

#include <sokol_gfx.h>
#include <util/sokol_gl.h>

#include <atomic>

#include "neko_api_core.h"
#include "neko_asset.h"
#include "neko_base.h"
#include "neko_os.h"
#include "neko_sound.h"

// deps
#include "vendor/luaalloc.h"

struct NEKO_PACKS {
    static constexpr const_str GAMEDATA = "default_pack";
    static constexpr const_str LUACODE = "luacode";
    static constexpr const_str DEFAULT_FONT = "assets/fonts/Monocraft.ttf";
};

struct AppTime {
    u64 startup;
    u64 last;
    u64 accumulator;
    u64 target_ticks;
    double delta;
};

struct lua_State;
struct App {
    Mutex gpu_mtx;

    // LuaAlloc *LA;
    lua_State *L;
    // ecs_world_t *ECS;

    AppTime time;

    bool win_console;
    Slice<String> args;

    std::atomic<u64> main_thread_id;
    std::atomic<bool> error_mode;
    std::atomic<bool> is_fused;

    Mutex log_mtx;

    Mutex error_mtx;
    String fatal_error;
    String traceback;

    std::atomic<bool> hot_reload_enabled;
    std::atomic<u32> reload_interval;

    bool key_state[349];
    bool prev_key_state[349];

    bool mouse_state[3];
    bool prev_mouse_state[3];
    float prev_mouse_x;
    float prev_mouse_y;
    float mouse_x;
    float mouse_y;
    float scroll_x;
    float scroll_y;

    FontFamily *default_font;

    neko_api_BlendMode blendmode;

    float current_color[4];

    ShaderItem *shader;  // ShaderItem (internal)

    void *miniaudio_vfs;
    ma_engine audio_engine;
    Array<Sound *> garbage_sounds;
};

extern App *g_app;

inline void fatal_error(String str) {
    if (!g_app->error_mode.load()) {
        LockGuard lock{&g_app->error_mtx};

        g_app->fatal_error = to_cstr(str);
        fprintf(stderr, "%s\n", g_app->fatal_error.data);
        g_app->error_mode.store(true);
    }
}

// inline ecs_world_t *&ENGINE_ECS() { return g_app->ECS; }
inline lua_State *&ENGINE_LUA() { return g_app->L; }

i32 neko_buildnum(void);