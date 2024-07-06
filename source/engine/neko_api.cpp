
#include "neko_api.hpp"

#include <filesystem>
#include <string>

#include "engine/neko.h"
#include "engine/neko_asset.h"
#include "engine/neko_common.h"
#include "engine/neko_engine.h"
#include "engine/neko_lua.hpp"
#include "engine/neko_luabind.hpp"

// game
#include "sandbox/game_main.h"

void __neko_lua_print_error(lua_State* state, int result) {
    const_str message = lua_tostring(state, -1);
    NEKO_ERROR("LuaScript ERROR:\n  %s", (message ? message : "no message"));

    if (result != 0) {
        switch (result) {
            case LUA_ERRRUN:
                NEKO_ERROR("%s", "Lua runtime error");
                break;
            case LUA_ERRSYNTAX:
                NEKO_ERROR("%s", "Lua syntax error");
                break;
            case LUA_ERRMEM:
                NEKO_ERROR("%s", "Lua was unable to allocate the required memory");
                break;
            case LUA_ERRFILE:
                NEKO_ERROR("%s", "Lua was unable to find boot file");
                break;
            default:
                NEKO_ERROR("Unknown lua error: %d", result);
        }
    }

    lua_pop(state, 1);
}

lua_Number luax_number_field(lua_State* L, s32 arg, const char* key) {
    lua_getfield(L, arg, key);
    lua_Number num = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return num;
}

lua_Number luax_opt_number_field(lua_State* L, s32 arg, const char* key, lua_Number fallback) {
    s32 type = lua_getfield(L, arg, key);

    lua_Number num = fallback;
    if (type != LUA_TNIL) {
        num = luaL_optnumber(L, -1, fallback);
    }

    lua_pop(L, 1);
    return num;
}

bool luax_boolean_field(lua_State* L, s32 arg, const char* key, bool fallback) {
    s32 type = lua_getfield(L, arg, key);

    bool b = fallback;
    if (type != LUA_TNIL) {
        b = lua_toboolean(L, -1);
    }

    lua_pop(L, 1);
    return b;
}

neko::string luax_check_string(lua_State* L, s32 arg) {
    size_t len = 0;
    char* str = (char*)luaL_checklstring(L, arg, &len);
    return {str, len};
}

void luax_new_class(lua_State* L, const char* mt_name, const luaL_Reg* l) {
    luaL_newmetatable(L, mt_name);
    luaL_setfuncs(L, l, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

int luax_msgh(lua_State* L) {
    // if (g_app->error_mode.load()) {
    //     return 0;
    // }

    neko::string err = luax_check_string(L, -1);

    // traceback = debug.traceback(nil, 2)
    lua_getglobal(L, "debug");
    lua_getfield(L, -1, "traceback");
    lua_remove(L, -2);
    lua_pushnil(L);
    lua_pushinteger(L, 2);
    lua_call(L, 2, 1);
    neko::string traceback = luax_check_string(L, -1);

    // if (lock_guard lock{&g_app->error_mtx}) {
    //     g_app->fatal_error = to_cstr(err);
    //     g_app->traceback = to_cstr(traceback);

    //     fprintf(stderr, "%s\n", g_app->fatal_error.data);
    //     fprintf(stderr, "%s\n", g_app->traceback.data);

    //     for (u64 i = 0; i < g_app->traceback.len; i++) {
    //         if (g_app->traceback.data[i] == '\t') {
    //             g_app->traceback.data[i] = ' ';
    //         }
    //     }

    //     g_app->error_mode.store(true);
    // }

    lua_pop(L, 2);  // traceback and error
    return 0;
}

namespace lua2struct {
template <>
neko_vec2 unpack<neko_vec2>(lua_State* L, int idx) {
    luaL_checktype(L, idx, LUA_TTABLE);
    lua_getfield(L, idx, "x");
    float x = lua2struct::unpack<float>(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, idx, "y");
    float y = lua2struct::unpack<float>(L, -1);
    lua_pop(L, 1);
    return {x, y};
}

template <>
neko_color_t unpack<neko_color_t>(lua_State* L, int idx) {
    luaL_checktype(L, idx, LUA_TTABLE);
    lua_getfield(L, idx, "r");
    u8 r = lua2struct::unpack<u8>(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, idx, "g");
    u8 g = lua2struct::unpack<u8>(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, idx, "b");
    u8 b = lua2struct::unpack<u8>(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, idx, "a");
    u8 a = lua2struct::unpack<u8>(L, -1);
    lua_pop(L, 1);
    return neko_color(r, g, b, a);
}
}  // namespace lua2struct

static void neko_tolua_setfield(lua_State* L, int table, const_str f, const_str v) {
    lua_pushstring(L, f);
    lua_pushstring(L, v);
    lua_settable(L, table);
}

static void neko_tolua_add_extra(lua_State* L, const_str value) {
    int len;
    lua_getglobal(L, "_extra_parameters");
#if LUA_VERSION_NUM > 501
    len = lua_rawlen(L, -1);
#else
    len = luaL_getn(L, -1);
#endif
    lua_pushstring(L, value);
    lua_rawseti(L, -2, len + 1);
    lua_pop(L, 1);
};

NEKO_API_DECL void neko_tolua_boot(int argc, char** argv) {

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    lua_pushstring(L, NEKO_TOLUA_VERSION);
    lua_setglobal(L, "NEKO_TOLUA_VERSION");
    lua_pushstring(L, LUA_VERSION);
    lua_setglobal(L, "TOLUA_LUA_VERSION");

    {
        int i, t;
        lua_newtable(L);
        lua_setglobal(L, "_extra_parameters");
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "neko_tolua_flags");
        t = lua_gettop(L);
        for (i = 1; i < argc; ++i) {
            if (*argv[i] == '-') {
                switch (argv[i][1]) {
                    case 'o':
                        neko_tolua_setfield(L, t, "o", argv[++i]);
                        break;
                    // disable automatic exporting of destructors for classes
                    // that have constructors (for compatibility with tolua5)
                    case 'D':
                        neko_tolua_setfield(L, t, "D", "");
                        break;
                    // add extra values to the luastate
                    case 'E':
                        neko_tolua_add_extra(L, argv[++i]);
                        break;
                    default:
                        break;
                }
            } else {
                neko_tolua_setfield(L, t, "f", argv[i]);
                break;
            }
        }
        lua_pop(L, 1);
    }

    int neko_tolua_boot_open(lua_State * L);
    neko_tolua_boot_open(L);
}

#if 1
#include "engine/luabind/asset.hpp"
#include "engine/luabind/core.hpp"
#include "engine/luabind/debug.hpp"
#include "engine/luabind/ffi.hpp"
#include "engine/luabind/filewatch.hpp"
#include "engine/luabind/imgui.hpp"
#include "engine/luabind/lowlua.hpp"
#include "engine/luabind/pack.hpp"
#include "engine/luabind/prefab.hpp"
#include "engine/luabind/struct.hpp"
#endif

#define PRELOAD(name, function)     \
    lua_getglobal(L, "package");    \
    lua_getfield(L, -1, "preload"); \
    lua_pushcfunction(L, function); \
    lua_setfield(L, -2, name);      \
    lua_pop(L, 2)

NEKO_API_DECL int luaopen_enet(lua_State* L);

void neko_register(lua_State* L) {

    NEKO_ASSERT(L != NULL, "?");

    neko_register_common(L);
    neko_register_platform(L);
    neko_register_test(L);

    neko::lua::preload_module(L);   // 新的模块系统
    neko::lua::package_preload(L);  // 新的模块系统

    PRELOAD("enet", luaopen_enet);  // test

    // 自定义加载器
    lua_register(L, "__neko_loader", neko::vfs_lua_loader);
    const_str str = "table.insert(package.searchers, 2, __neko_loader) \n";
    luaL_dostring(L, str);

    neko::neko_lua_add_package_path(L, "./");
}