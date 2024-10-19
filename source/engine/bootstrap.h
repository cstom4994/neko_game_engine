#ifndef GAME_H
#define GAME_H

#include <atomic>

#include "engine/asset.h"
#include "engine/base.hpp"
#include "engine/base/singleton.hpp"
#include "engine/component.h"
#include "engine/draw.h"
#include "engine/ecs/entity.h"
#include "engine/ecs/lua_ecs.hpp"
#include "engine/graphics.h"
#include "engine/sound.h"
#include "engine/test.h"
#include "engine/ui.h"

// deps
#include "vendor/luaalloc.h"

#define NEKO_C_EXTERN extern "C"

#define DATA_DIR "../gamedir/assets/data/"
#define USR_DIR "../gamedir/usr/"

#define data_path(path) (DATA_DIR path)
#define usr_path(path) (USR_DIR path)

#define default_font_size 22.f

using namespace neko::ecs;

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

        NEKO_EXPORT AppTime * get_timing_instance();

        NEKO_EXPORT void timing_set_scale(f32 s);

        NEKO_EXPORT f32 timing_get_scale();

        NEKO_EXPORT f32 timing_get_elapsed();

        NEKO_EXPORT void timing_set_paused(bool p);  // 暂停将刻度设置为 0 并在恢复时恢复它

        NEKO_EXPORT bool timing_get_paused();

)

struct lua_State;
struct App {
    Mutex gpu_mtx;

    bool g_quit = false;  // 如果为 true 则退出主循环
    Mutex g_init_mtx;

    gfx_texture_t test_ase;

    AppTime timing_instance;

    lua_State *L;
    EcsWorld *ECS;
    lua_State *LiteLua;

    batch_renderer *batch;

    ui_context_t *ui;

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
    std::atomic<f32> reload_interval;

    engine_cfg_t cfg;

    bool mouse_state[3];
    bool prev_mouse_state[3];
    float prev_mouse_x;
    float prev_mouse_y;
    float mouse_x;
    float mouse_y;
    float scroll_x;
    float scroll_y;

    FontFamily *default_font;

    f32 scale = 1.0f;
    bool paused = false;

    int g_lua_callbacks_table_ref;  // LUA_NOREF

#if NEKO_AUDIO == 1
    void *miniaudio_vfs;
    ma_engine audio_engine;
    Array<Sound *> garbage_sounds;
#endif

    GLFWwindow *game_window;
};

extern App *g_app;

void fatal_error(String str);

inline lua_State *&ENGINE_LUA() { return g_app->L; }
inline EcsWorld *&ENGINE_ECS() { return g_app->ECS; }

i32 neko_buildnum(void);

// ECS_COMPONENT_EXTERN(pos_t);
// ECS_COMPONENT_EXTERN(vel_t);
// ECS_COMPONENT_EXTERN(rect_t);

// 入口点
void GameMain(int argc, char **argv);

class Game : public neko::SingletonClass<Game> {
    int argc;
    char **argv;

public:
    Game(int argc, char **argv);

public:
    void init() override;
    void fini() override;
    void update() override;

    // 获取 argc argv 传递给 game_run(...)
    int game_get_argc() const;
    char **game_get_argv() const;

    void game_set_bg_color(Color c);

    // 屏幕空间坐标系:
    // unit: (0, 0) 中间, (1, 1) 右上
    // pixels: (0, 0) 左上方, game_get_window_size() 右下角
    void set_window_size(vec2 s);  // width, height in pixels

    vec2 get_window_size();

    int set_window_title(const char *title);

    vec2 unit_to_pixels(vec2 p);

    vec2 pixels_to_unit(vec2 p);

    void quit();

    void WindowSwapBuffer();
    void SplashScreen();
};

NEKO_SCRIPT(game,

            NEKO_EXPORT const char *window_clipboard();

            NEKO_EXPORT int window_prompt(const char *msg, const char *title);

            NEKO_EXPORT void window_setclipboard(const char *text);

            NEKO_EXPORT void window_focus();

            NEKO_EXPORT int window_has_focus();

            NEKO_EXPORT double window_scale();

)

int timing_update(App *app, event_t evt);
void timing_save_all(Store *s);
void timing_load_all(Store *s);

#endif
