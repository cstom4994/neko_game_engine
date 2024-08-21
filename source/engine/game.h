#ifndef GAME_H
#define GAME_H

#include <atomic>

#include "engine/asset.h"
#include "engine/base.h"
#include "engine/base.hpp"
#include "engine/component.h"
#include "engine/draw.h"
#include "engine/entity.h"
#include "engine/gfx.h"
#include "engine/math.h"
#include "engine/physics.h"
#include "engine/prelude.h"
#include "engine/ui.h"
#include "glew_glfw.h"

// deps
#include "vendor/luaalloc.h"

#define NEKO_C_EXTERN extern "C"

#define DATA_DIR "./gamedir/assets/data/"
#define USR_DIR "./gamedir/usr/"

#define data_path(path) (DATA_DIR path)
#define usr_path(path) (USR_DIR path)

#define default_font_size 22.f

struct NEKO_PACKS {
    static constexpr const_str GAMEDATA = "default_pack";
    static constexpr const_str LUACODE = "luacode";
    static constexpr const_str DEFAULT_FONT = "assets/fonts/Monocraft.ttf";
};

typedef struct Store Store;

NEKO_SCRIPT(
        timing,

        typedef struct AppTime {
            u64 startup;
            u64 last;
            u64 accumulator;
            u64 target_ticks;
            f64 delta;

            f32 dt;
            f32 true_dt;  // 实际增量时间 不受 scale/pause 影响
        } AppTime;

        NEKO_EXPORT AppTime timing_instance;

        NEKO_EXPORT void timing_set_scale(f32 s);

        NEKO_EXPORT f32 timing_get_scale();

        NEKO_EXPORT f32 timing_get_elapsed();

        NEKO_EXPORT void timing_set_paused(bool p);  // 暂停将刻度设置为 0 并在恢复时恢复它

        NEKO_EXPORT bool timing_get_paused();

)

struct lua_State;
struct App {
    Mutex gpu_mtx;

    // LuaAlloc *LA;
    lua_State *L;
    lua_State *lite_L;  // lua state for lite editor
    ecs_t *ECS;

    command_buffer_t cb;
    // ui_context_t ui;
    // idraw_t idraw;

    ui_context_t *ui;

    f64 width;
    f64 height;

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

    bool debug_on;

    String lite_init_path;

    FontFamily *default_font;

    int g_lua_callbacks_table_ref;  // LUA_NOREF

#if NEKO_AUDIO == 1
    void *miniaudio_vfs;
    ma_engine audio_engine;
    Array<Sound *> garbage_sounds;
#endif

    GLFWwindow *game_window;

    neko_dyn_array(shader_pair) shader_array;
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

inline lua_State *&ENGINE_LUA() { return g_app->L; }
inline ecs_t *&ENGINE_ECS() { return g_app->ECS; }

i32 neko_buildnum(void);

// ECS_COMPONENT_EXTERN(pos_t);
// ECS_COMPONENT_EXTERN(vel_t);
// ECS_COMPONENT_EXTERN(rect_t);

void scratch_run();
void scratch_update();

// 入口点
void game_run(int argc, char **argv);

// 获取 argc argv 传递给 game_run(...)
int game_get_argc();
char **game_get_argv();

NEKO_SCRIPT(game,

            NEKO_EXPORT void game_set_bg_color(Color c);

            // 屏幕空间坐标系:
            // unit: (0, 0) 中间, (1, 1) 右上
            // pixels: (0, 0) 左上方, game_get_window_size() 右下角
            NEKO_EXPORT void game_set_window_size(LuaVec2 s);  // width, height in pixels

            NEKO_EXPORT LuaVec2 game_get_window_size();

            NEKO_EXPORT LuaVec2 game_unit_to_pixels(LuaVec2 p);

            NEKO_EXPORT LuaVec2 game_pixels_to_unit(LuaVec2 p);

            NEKO_EXPORT void game_quit();

            NEKO_EXPORT const char *window_clipboard();

            NEKO_EXPORT void window_setclipboard(const char *text);

            NEKO_EXPORT void window_focus();

            NEKO_EXPORT int window_has_focus();

            NEKO_EXPORT double window_scale();

)

void timing_update();
void timing_save_all(Store *s);
void timing_load_all(Store *s);

#endif
