#include "engine/script.h"

#include <stdlib.h>
#include <string.h>

#include "console.h"
#include "edit.h"
#include "engine/api.hpp"
#include "engine/asset.h"
#include "engine/base.h"
#include "engine/camera.h"
#include "engine/game.h"
#include "engine/input.h"
#include "engine/lua_util.h"
#include "engine/luax.h"
#include "engine/prefab.h"
#include "engine/sprite.h"
#include "engine/system.h"
#include "engine/transform.h"
#include "engine/ui.h"

static const char **nekogame_ffi[] = {
        &nekogame_ffi_scalar,
        &nekogame_ffi_saveload,
        &nekogame_ffi_vec2,
        &nekogame_ffi_mat3,
        &nekogame_ffi_bbox,
        &nekogame_ffi_color,
        &nekogame_ffi_fs,
        &nekogame_ffi_game,
        &nekogame_ffi_system,
        &nekogame_ffi_input,
        &nekogame_ffi_entity,
        &nekogame_ffi_prefab,
        &nekogame_ffi_timing,
        &nekogame_ffi_transform,
        &nekogame_ffi_camera,
        &nekogame_ffi_sprite,
        &nekogame_ffi_gui,
        &nekogame_ffi_console,
        // &nekogame_ffi_sound,
        // &nekogame_ffi_physics,
        &nekogame_ffi_edit,

        &nekogame_ffi_keyboard_controlled,
};

static const unsigned int n_nekogame_ffi = sizeof(nekogame_ffi) / sizeof(nekogame_ffi[0]);

static int _traceback(lua_State *L) {
    if (!lua_isstring(L, 1)) {
        if (lua_isnoneornil(L, 1) || !luaL_callmeta(L, 1, "__tostring") || !lua_isstring(L, -1)) return 1;
        lua_remove(L, 1);
    }
    luaL_traceback(L, L, lua_tostring(L, 1), 1);
    return 1;
}

int luax_pcall_nothrow(lua_State *L, int nargs, int nresults) {
    int r, errfunc;
    // 将 _traceback 放在 function 和 args 下
    errfunc = lua_gettop(L) - nargs;
    lua_pushcfunction(L, _traceback);
    lua_insert(L, errfunc);
    // call, remove _traceback
    r = lua_pcall(L, nargs, nresults, errfunc);
    lua_remove(L, errfunc);
    return r;
}

void script_run_string(const char *s) {
    lua_State *L = ENGINE_LUA();
    luaL_loadstring(L, s);
    errcheck(luax_pcall_nothrow(L, 0, LUA_MULTRET));
}
void script_run_file(const char *filename) {
    lua_State *L = ENGINE_LUA();
    luaL_loadfile(L, filename);
    errcheck(luax_pcall_nothrow(L, 0, LUA_MULTRET));
}
void script_error(const char *s) {
    lua_State *L = ENGINE_LUA();
    luaL_error(L, s);
}

// 将对象推送为 cdata, t 必须是字符串形式的 FFI 类型说明符
// 如推入一个 CVec2 应该为 _push_cdata("CVec2 *", &v)
// 结果是堆栈上的 CVec2 cdata (不是指针)
static void _push_cdata(const char *t, void *p) {
    // just call __deref_cdata(t, p)
    lua_State *L = ENGINE_LUA();
    lua_getglobal(L, "ng");
    lua_getfield(L, -1, "__deref_cdata");
    lua_remove(L, -2);
    lua_pushstring(L, t);
    lua_pushlightuserdata(L, p);
    errcheck(luax_pcall_nothrow(L, 2, 1));
}

void script_push_event(const char *event) {
    // call nekogame.__fire_event(event, ...)
    lua_State *L = ENGINE_LUA();
    lua_getglobal(L, "ng");
    lua_getfield(L, -1, "__fire_event");
    lua_remove(L, -2);
    lua_pushstring(L, event);
}

// 将命令行参数转发为 nekogame_args[0], nekogame_args[1], ...
static void _forward_args() {
    lua_State *L = ENGINE_LUA();

    int i, argc;
    char **argv;

    argc = game_get_argc();
    argv = game_get_argv();

    lua_createtable(L, argc, 0);
    for (i = 0; i < argc; ++i) {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i);
    }
    lua_setglobal(L, "nekogame_args");
}

static void _set_paths() {
    lua_State *L = ENGINE_LUA();
    lua_pushstring(L, data_path(""));
    lua_setglobal(L, "nekogame_data_path");
    lua_pushstring(L, usr_path(""));
    lua_setglobal(L, "nekogame_usr_path");
}

// LuaJIT FFI 解析器无法解析 'NEKO_EXPORT' -- 使其成为空白
static void _fix_exports(char *s) {
    static const char keyword[] = "NEKO_EXPORT";
    unsigned int i;

    while ((s = strstr(s, keyword)))
        for (i = 0; i < sizeof(keyword) - 1; ++i) *s++ = ' ';
}

// 相当于:
// ffi = require 'ffi'
// ffi.cdef(nekogame_ffi[0] .. nekogame_ffi[1] .. ...)
// with 'NEKO_EXPORT's fixed -- after this nekogame.lua can bind the FFI
static void _load_nekogame_ffi() {
    lua_State *L = ENGINE_LUA();

    unsigned int i;
    char *fixed;
    luaL_Buffer buf;  // 将累积 nekogame_ffi cdefs 到这里

    // get ffi.cdef
    lua_getglobal(L, "require");
    lua_pushstring(L, "ffi");
    errcheck(luax_pcall_nothrow(L, 1, 1));
    lua_getfield(L, lua_gettop(L), "cdef");

    // accumulate nekogame_ffi cdefs
    luaL_buffinit(L, &buf);
    for (i = 0; i < n_nekogame_ffi; ++i) {
        fixed = (char *)mem_alloc(strlen(*nekogame_ffi[i]) + 1);
        strcpy(fixed, *nekogame_ffi[i]);
        _fix_exports(fixed);
        luaL_addstring(&buf, fixed);
        mem_free(fixed);
    }
    luaL_pushresult(&buf);

    errcheck(luax_pcall_nothrow(L, 1, 0));
}

// #define NEKO_CFFI

#ifdef NEKO_CFFI

extern "C" {
int luaopen_ffi(lua_State *L);
int luaopen_bit(lua_State *L);
}

#endif

void script_init() {
    PROFILE_FUNC();

    neko::timer timer;
    timer.start();

    lua_State *L = neko::neko_lua_create();

    ENGINE_LUA() = L;

    open_neko_api(L);

#ifdef NEKO_CFFI
    luax_package_preload(L, "ffi", luaopen_ffi);
    luax_package_preload(L, "bit", luaopen_bit);
#endif

    lua_atpanic(
            L, +[](lua_State *L) {
                auto msg = lua_tostring(L, -1);
                printf("[lua] neko_panic error: %s", msg);
                return 0;
            });

    _load_nekogame_ffi();
    _forward_args();
    _set_paths();

    // ENGINE_ECS() = ecs_init(ENGINE_LUA());

    // ECS_COMPONENT_DEFINE(pos_t, NULL, NULL);
    // ECS_COMPONENT_DEFINE(vel_t, NULL, NULL);
    // ECS_COMPONENT_DEFINE(rect_t, NULL, NULL);

    g_app->g_lua_callbacks_table_ref = LUA_NOREF;

    lua_channels_setup();

    neko::lua::luax_run_bootstrap(L);

    // lua_pushcfunction(L, luax_msgh);  // 添加错误消息处理程序 始终位于堆栈底部

    timer.stop();
    console_log(std::format("lua init done in {0:.3f} ms", timer.get()).c_str());
}

void script_fini() {
    lua_State *L = ENGINE_LUA();

    script_push_event("fini");
    errcheck(luax_pcall_nothrow(L, 1, 0));

    lua_pop(L, 1);  // FFI

    neko::neko_lua_fini(L);
}

void script_update_all() {
    lua_State *L = ENGINE_LUA();

    script_push_event("update_all");
    errcheck(luax_pcall_nothrow(L, 1, 0));
}

void script_post_update_all() {
    lua_State *L = ENGINE_LUA();

    script_push_event("post_update_all");
    errcheck(luax_pcall_nothrow(L, 1, 0));
}

void script_draw_all() {
    lua_State *L = ENGINE_LUA();

    script_push_event("draw_all");
    errcheck(luax_pcall_nothrow(L, 1, 0));
}

void script_key_down(KeyCode key) {
    lua_State *L = ENGINE_LUA();

    script_push_event("key_down");
    _push_cdata("KeyCode *", &key);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}
void script_key_up(KeyCode key) {
    lua_State *L = ENGINE_LUA();

    script_push_event("key_up");
    _push_cdata("KeyCode *", &key);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}

void script_mouse_down(MouseCode mouse) {
    lua_State *L = ENGINE_LUA();

    script_push_event("mouse_down");
    _push_cdata("MouseCode *", &mouse);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}
void script_mouse_up(MouseCode mouse) {
    lua_State *L = ENGINE_LUA();

    script_push_event("mouse_up");
    _push_cdata("MouseCode *", &mouse);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}

void script_mouse_move(CVec2 pos) {
    lua_State *L = ENGINE_LUA();

    script_push_event("mouse_move");
    _push_cdata("CVec2 *", &pos);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}

void script_scroll(CVec2 scroll) {
    lua_State *L = ENGINE_LUA();

    script_push_event("scroll");
    _push_cdata("CVec2 *", &scroll);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}

void script_save_all(Store *s) {
    lua_State *L = ENGINE_LUA();

    Store *t;
    const char *str;

    if (store_child_save(&t, "script", s)) {
        // get string from Lua
        lua_getglobal(L, "ng");
        lua_getfield(L, -1, "__save_all");
        lua_remove(L, -2);
        errcheck(luax_pcall_nothrow(L, 0, 1));
        str = lua_tostring(L, -1);

        // save it
        string_save(&str, "str", t);

        // release
        lua_pop(L, 1);
    }
}

void script_load_all(Store *s) {
    lua_State *L = ENGINE_LUA();

    Store *t;
    char *str;

    if (store_child_load(&t, "script", s))
        if (string_load(&str, "str", NULL, t)) {
            // send it to Lua
            lua_getglobal(L, "ng");
            lua_getfield(L, -1, "__load_all");
            lua_remove(L, -2);
            lua_pushstring(L, str);
            errcheck(luax_pcall_nothrow(L, 1, 0));

            // release
            mem_free(str);
        }
}