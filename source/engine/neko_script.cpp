#include "engine/neko_script.h"

#include <stdlib.h>
#include <string.h>

#include "cgame_ffi.h"
#include "console.h"
#include "engine/neko_game.h"
#include "engine/neko_api.hpp"
#include "engine/neko_game.h"
#include "engine/neko_lua.h"
#include "engine/neko_lua_wrap.h"
#include "engine/neko_base.h"
#include "engine/neko_game.h"
#include "engine/neko_input.h"

static lua_State *L;

#define errcheck(...)                                         \
    do                                                        \
        if (__VA_ARGS__) {                                    \
            console_printf("lua: %s\n", lua_tostring(L, -1)); \
            lua_pop(L, 1);                                    \
        }                                                     \
    while (0)

static int _traceback(lua_State *L) {
    if (!lua_isstring(L, 1)) {
        if (lua_isnoneornil(L, 1) || !luaL_callmeta(L, 1, "__tostring") || !lua_isstring(L, -1)) return 1;
        lua_remove(L, 1);
    }
    luaL_traceback(L, L, lua_tostring(L, 1), 1);
    return 1;
}

static int _pcall(lua_State *L, int nargs, int nresults) {
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
    luaL_loadstring(L, s);
    errcheck(_pcall(L, 0, LUA_MULTRET));
}
void script_run_file(const char *filename) {
    luaL_loadfile(L, filename);
    errcheck(_pcall(L, 0, LUA_MULTRET));
}
void script_error(const char *s) { luaL_error(L, s); }

// 将对象推送为 cdata, t 必须是字符串形式的 FFI 类型说明符
// 如推入一个 CVec2 应该为 _push_cdata("CVec2 *", &v)
// 结果是堆栈上的 CVec2 cdata (不是指针)
static void _push_cdata(const char *t, void *p) {
    // just call __deref_cdata(t, p)
    lua_getglobal(L, "ng");
    lua_getfield(L, -1, "__deref_cdata");
    lua_remove(L, -2);
    lua_pushstring(L, t);
    lua_pushlightuserdata(L, p);
    errcheck(_pcall(L, 2, 1));
}

static void _push_event(const char *event) {
    // call nekogame.__fire_event(event, ...)
    lua_getglobal(L, "ng");
    lua_getfield(L, -1, "__fire_event");
    lua_remove(L, -2);
    lua_pushstring(L, event);
}

// 将命令行参数转发为 nekogame_args[0], nekogame_args[1], ...
static void _forward_args() {
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
    lua_pushstring(L, "./source/game/");
    lua_setglobal(L, "nekogame_script_path");
    lua_pushstring(L, data_path(""));
    lua_setglobal(L, "nekogame_data_path");
    lua_pushstring(L, usr_path(""));
    lua_setglobal(L, "nekogame_usr_path");
}

// LuaJIT FFI parser doesn't like 'NEKO_EXPORT' -- make it whitespace
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
    unsigned int i;
    char *fixed;
    luaL_Buffer buf;  // 将累积 nekogame_ffi cdefs 到这里

    // get ffi.cdef
    lua_getglobal(L, "require");
    lua_pushstring(L, "ffi");
    errcheck(_pcall(L, 1, 1));
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

    errcheck(_pcall(L, 1, 0));
}

// #define NEKO_CFFI

#ifdef NEKO_CFFI

extern "C" {
int luaopen_cffi(lua_State *L);
int luaopen_bit(lua_State *L);
}

inline void luax_package_preload(lua_State *L, const char *name, lua_CFunction function) {
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_pushcfunction(L, function);
    lua_setfield(L, -2, name);
    lua_pop(L, 2);
}

#endif

void script_init() {
    // L = luaL_newstate();
    // luaL_openlibs(L);

    L = neko::neko_lua_create();

    ENGINE_LUA() = L;

    open_neko_api(L);

#ifdef NEKO_CFFI
    luax_package_preload(L, "ffi", luaopen_cffi);
    luax_package_preload(L, "bit", luaopen_bit);

    lua_atpanic(
            L, +[](lua_State *L) {
                auto msg = lua_tostring(L, -1);
                printf("[lua] neko_panic error: %s", msg);
                return 0;
            });
#endif

    _load_nekogame_ffi();
    _forward_args();
    _set_paths();

    // run main.lua
    errcheck(luaL_loadfile(L, "./source/game/script/main.lua"));
    errcheck(_pcall(L, 0, 0));

    // fire init event
    _push_event("init");
    errcheck(_pcall(L, 1, 0));
}

void script_deinit() {
    _push_event("deinit");
    errcheck(_pcall(L, 1, 0));

    lua_close(L);
}

void script_update_all() {
    _push_event("update_all");
    errcheck(_pcall(L, 1, 0));
}

void script_post_update_all() {
    _push_event("post_update_all");
    errcheck(_pcall(L, 1, 0));
}

void script_draw_all() {
    _push_event("draw_all");
    errcheck(_pcall(L, 1, 0));
}

void script_key_down(KeyCode key) {
    _push_event("key_down");
    _push_cdata("KeyCode *", &key);
    errcheck(_pcall(L, 2, 0));
}
void script_key_up(KeyCode key) {
    _push_event("key_up");
    _push_cdata("KeyCode *", &key);
    errcheck(_pcall(L, 2, 0));
}

void script_mouse_down(MouseCode mouse) {
    _push_event("mouse_down");
    _push_cdata("MouseCode *", &mouse);
    errcheck(_pcall(L, 2, 0));
}
void script_mouse_up(MouseCode mouse) {
    _push_event("mouse_up");
    _push_cdata("MouseCode *", &mouse);
    errcheck(_pcall(L, 2, 0));
}

void script_mouse_move(CVec2 pos) {
    _push_event("mouse_move");
    _push_cdata("CVec2 *", &pos);
    errcheck(_pcall(L, 2, 0));
}

void script_scroll(CVec2 scroll) {
    _push_event("scroll");
    _push_cdata("CVec2 *", &scroll);
    errcheck(_pcall(L, 2, 0));
}

void script_save_all(Store *s) {
    Store *t;
    const char *str;

    if (store_child_save(&t, "script", s)) {
        // get string from Lua
        lua_getglobal(L, "ng");
        lua_getfield(L, -1, "__save_all");
        lua_remove(L, -2);
        errcheck(_pcall(L, 0, 1));
        str = lua_tostring(L, -1);

        // save it
        string_save(&str, "str", t);

        // release
        lua_pop(L, 1);
    }
}

void script_load_all(Store *s) {
    Store *t;
    char *str;

    if (store_child_load(&t, "script", s))
        if (string_load(&str, "str", NULL, t)) {
            // send it to Lua
            lua_getglobal(L, "ng");
            lua_getfield(L, -1, "__load_all");
            lua_remove(L, -2);
            lua_pushstring(L, str);
            errcheck(_pcall(L, 1, 0));

            // release
            mem_free(str);
        }
}
