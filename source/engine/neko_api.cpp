
#include "neko_api.hpp"

#include <filesystem>
#include <string>

#include "engine/neko.h"
#include "engine/neko_asset.h"
#include "engine/neko_common.h"
#include "engine/neko_ecs.h"
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
                NEKO_ERROR("%s", "Lua Runtime error");
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

    // if (LockGuard lock{&g_app->error_mtx}) {
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

void addMethods(lua_State* L, const_str name, const luaL_Reg* methods) {
    luaL_newmetatable(L, name);

    lua_pushstring(L, "__index");
    lua_newtable(L);

    for (const luaL_Reg* method = methods; method->name != NULL; ++method) {
        lua_pushcfunction(L, method->func);
        lua_setfield(L, -2, method->name);
    }

    lua_settable(L, -3);

    lua_setmetatable(L, -2);
}

void setFieldInt(lua_State* L, const_str key, float data) {
    lua_pushstring(L, key);
    lua_pushinteger(L, data);
    lua_settable(L, -3);
}

void setFieldFloat(lua_State* L, const_str key, float data) {
    lua_pushstring(L, key);
    lua_pushnumber(L, data);
    lua_settable(L, -3);
}

int tostring(lua_State* L) {
    lua_getfield(L, 1, "x");
    lua_getfield(L, 1, "y");
    lua_getfield(L, 1, "z");

    const_str x = lua_tostring(L, -3);
    const_str y = lua_tostring(L, -2);
    const_str z = lua_tostring(L, -1);

    neko_snprintfc(temp, 64, "%f, %f, %f", x, y, z);

    lua_pop(L, 3);

    lua_pushstring(L, temp);

    return 1;
}

static const luaL_Reg tableMethods[] = {{"tostring", tostring}, {NULL, NULL}};

int Vector(lua_State* L) {
    float x = luaL_optnumber(L, 1, 0.0f);
    float y = luaL_optnumber(L, 2, 0.0f);
    float z = luaL_optnumber(L, 3, 0.0f);

    lua_newtable(L);
    setFieldFloat(L, "x", x);
    setFieldFloat(L, "y", y);
    setFieldFloat(L, "z", z);

    addMethods(L, "table", tableMethods);

    return 1;
}

int Angle(lua_State* L) {
    float roll = luaL_optnumber(L, 1, 0.0f);
    float pitch = luaL_optnumber(L, 2, 0.0f);
    float yaw = luaL_optnumber(L, 3, 0.0f);

    lua_newtable(L);
    setFieldFloat(L, "roll", roll);
    setFieldFloat(L, "pitch", pitch);
    setFieldFloat(L, "yaw", yaw);

    addMethods(L, "table", tableMethods);

    return 1;
}

int Color(lua_State* L) {
    float r = luaL_optnumber(L, 1, 255.0f);
    float g = luaL_optnumber(L, 2, 255.0f);
    float b = luaL_optnumber(L, 3, 255.0f);
    float a = luaL_optnumber(L, 4, 255.0f);

    lua_newtable(L);
    setFieldInt(L, "r", r);
    setFieldInt(L, "g", g);
    setFieldInt(L, "b", b);
    setFieldInt(L, "a", a);

    // addMethods(L, "color", colorMethods);

    return 1;
}

void registerGlobals(lua_State* L, const neko_luaL_reg* funcs) {
    for (; funcs->name != NULL; ++funcs) {
        lua_pushstring(L, funcs->name);
        funcs->func(L);
        lua_setglobal(L, funcs->name);
    }
}

static int __neko_loader(lua_State* L) {
    const_str name = luaL_checkstring(L, 1);
    std::string path = name;
    std::replace(path.begin(), path.end(), '.', '/');
    path += ".lua";

    // neko_println("fuck:%s", path.c_str());

    u8* data;
    u32 data_size;

    int result = neko_pack_item_data(&CL_GAME_INTERFACE()->pack, path.c_str(), (const u8**)&data, &data_size);
    if (result == 0) {
        if (luaL_loadbuffer(L, (char*)data, data_size, name) != LUA_OK) {
            lua_pop(L, 1);
        } else {
            neko_println("loaded:%s", path.c_str());
        }

        neko_pack_item_free(&CL_GAME_INTERFACE()->pack, data);
    }

    return 1;
}

#if 1
#include "engine/luabind/core.hpp"
#include "engine/luabind/debug.hpp"
#include "engine/luabind/ffi.hpp"
#include "engine/luabind/filewatch.hpp"
#include "engine/luabind/imgui.hpp"
#include "engine/luabind/prefab.hpp"
#include "engine/luabind/struct.hpp"
#endif

#define PRELOAD(name, function)     \
    lua_getglobal(L, "package");    \
    lua_getfield(L, -1, "preload"); \
    lua_pushcfunction(L, function); \
    lua_setfield(L, -2, name);      \
    lua_pop(L, 2)

void neko_register(lua_State* L) {

    NEKO_ASSERT(L != NULL, "?");

    neko_register_common(L);
    neko_register_platform(L);
    neko_register_test(L);

    neko::lua::preload_module(L);   // 新的模块系统
    neko::lua::package_preload(L);  // 新的模块系统

    // 自定义加载器
    lua_register(L, "__neko_loader", __neko_loader);
    const_str str = "table.insert(package.searchers, 2, __neko_loader) \n";
    luaL_dostring(L, str);

    neko::neko_lua_add_package_path(L, "./");
}