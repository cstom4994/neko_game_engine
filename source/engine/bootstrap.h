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

#define DATA_DIR "../gamedir/assets/data/"
#define USR_DIR "../gamedir/usr/"

#define data_path(path) (DATA_DIR path)
#define usr_path(path) (USR_DIR path)

#define default_font_size 22.f

using namespace Neko;
using namespace Neko::ecs;

extern CBase gBase;

namespace Neko {
struct LuaInspector;
}

struct AppTime {
    u64 startup;
    u64 last;
    u64 accumulator;
    u64 target_ticks;
    f64 delta;

    f32 dt;
    f32 true_dt;  // 实际增量时间 不受 scale/pause 影响
};

void timing_set_scale(f32 s);
f32 timing_get_scale();
f32 timing_get_elapsed();
void timing_set_paused(bool p);
bool timing_get_paused();

AppTime get_timing_instance();

struct lua_State;

// ECS_COMPONENT_EXTERN(pos_t);
// ECS_COMPONENT_EXTERN(vel_t);
// ECS_COMPONENT_EXTERN(rect_t);

class CL : public Neko::SingletonClass<CL> {
public:
    bool g_quit = false;  // 如果为 true 则退出主循环
    Mutex g_init_mtx;
    AppTime timing_instance{};
    lua_State *L{};
    EcsWorld *ECS{};
    batch_renderer *batch{};
    ui_context_t *ui{};
    bool win_console{};
    engine_cfg_t cfg{};
    LuaInspector *inspector{};
    ImGuiID devui_vp{};
    FontFamily *default_font{};
    Window *window{};

    f32 scale = 1.0f;
    bool paused = false;

#if NEKO_AUDIO == 1
    void *miniaudio_vfs;
    ma_engine audio_engine;
    Array<Sound *> garbage_sounds;
#endif

public:
    CL();

public:
    lua_State *&get_lua() { return this->L; }
    EcsWorld *&get_ecs() { return this->ECS; }

    void init();
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
    void SplashScreen();
    int timing_update(Event evt);
    void system_init();
    void system_fini();
};

inline lua_State *&ENGINE_LUA() { return the<CL>().get_lua(); }
inline EcsWorld *&ENGINE_ECS() { return the<CL>().get_ecs(); }

extern Int32 Main(int argc, const char *argv[]);

#endif
