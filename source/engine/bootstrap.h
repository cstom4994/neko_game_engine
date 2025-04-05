#ifndef GAME_H
#define GAME_H

#include <atomic>

#include "engine/asset.h"
#include "engine/base.hpp"
#include "base/common/singleton.hpp"
#include "base/cbase.hpp"
#include "base/common/array.hpp"
#include "base/common/string.hpp"
#include "engine/component.h"
#include "engine/draw.h"
#include "engine/ecs/entity.h"
#include "engine/ecs/lua_ecs.hpp"
#include "engine/graphics.h"
#include "engine/sound.h"
#include "engine/test.h"
#include "engine/ui.h"
#include "engine/imgui.hpp"
#include "base/common/logger.hpp"
#include "engine/window.h"

// deps
#include "extern/luaalloc.h"

using namespace Neko;
using namespace Neko::ecs;

extern CBase gBase;

struct lua_State;

namespace Neko {
struct LuaInspector;
}

class CL : public Neko::SingletonClass<CL> {
public:
    bool g_quit = false;  // 如果为 true 则退出主循环
    Mutex g_init_mtx;
    struct TimeInfo {
        u64 startup;
        u64 last;
        u64 accumulator;
        u64 target_ticks;
        f64 delta;

        f32 dt;
        f32 true_dt;  // 实际增量时间 不受 scale/pause 影响
    } time{};
    lua_State *L{};
    EcsWorld *ECS{};
    ui_context_t *ui{};
    bool win_console{};
    LuaInspector *inspector{};
    ImGuiID devui_vp{};
    FontFamily *default_font{};
    Window *window{};

    f32 scale = 1.0f;
    bool paused = false;

    struct State {
        bool show_editor;
        bool show_demo_window;
        bool show_gui;
        bool shader_inspect;
        bool hello_ai_shit;
        bool vsync;
        bool is_hotfix;
        String title;
        String game_proxy;
        String default_font;
        bool hot_reload;
        bool startup_load_scripts;
        bool fullscreen;
        bool debug_on;
        f32 reload_interval;
        f32 swap_interval;
        f32 target_fps;
        i32 batch_vertex_capacity;
        bool dump_allocs_detailed;
        f32 width;
        f32 height;
        f32 bg[3];  //
        f32 posteffect_intensity = 2.0f;
    } state{};

    void *miniaudio_vfs;
    ma_engine audio_engine;
    Array<Sound *> garbage_sounds;

public:
    CL();

public:
    lua_State *&get_lua() { return this->L; }
    EcsWorld *&get_ecs() { return this->ECS; }

    void init();
    void fini();

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
    void game_draw();
    void SplashScreen();

    inline CL::TimeInfo GetTimeInfo() const { return the<CL>().time; }
    inline f32 timing_get_elapsed() { return glfwGetTime() * 1000.0f; }
    inline void timing_set_scale(f32 s) { the<CL>().scale = s; }
    inline f32 timing_get_scale() { return the<CL>().scale; }
    inline void timing_set_paused(bool p) { the<CL>().paused = p; }  // 暂停将刻度设置为 0 并在恢复时恢复它
    inline bool timing_get_paused() { return the<CL>().paused; }

private:
    int update_time(Event evt);
};

inline lua_State *&ENGINE_LUA() { return the<CL>().get_lua(); }
inline EcsWorld *&ENGINE_ECS() { return the<CL>().get_ecs(); }

extern Int32 Main(int argc, const char *argv[]);

#endif
