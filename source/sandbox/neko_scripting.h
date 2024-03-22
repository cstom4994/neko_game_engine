// Copyright(c) 2022-2023, KaoruXun All rights reserved.

#ifndef NEKO_SCRIPTING_H
#define NEKO_SCRIPTING_H

#include <cstring>
#include <filesystem>
#include <format>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "engine/neko.h"
#include "engine/neko.hpp"
#include "engine/neko_engine.h"

// lua
#include "engine/util/neko_lua.hpp"

struct lua_State;

template <typename T>
neko_static_inline void struct_as(std::string &s, const char *table, const char *key, const T &value) {
    s += std::format("{0}.{1} = {2}\n", table, key, value);
}

template <>
neko_static_inline void struct_as(std::string &s, const char *table, const char *key, const std::string &value) {
    s += std::format("{0}.{1} = \"{2}\"\n", table, key, value);
}

using ppair = std::pair<const char *, const void *>;

struct test_visitor {
    std::vector<ppair> result;

    template <typename T>
    void operator()(const char *name, const T &t) {
        result.emplace_back(ppair{name, static_cast<const void *>(&t)});
    }
};

// template <typename T>
// void SaveLuaConfig(const T &_struct, const char *table_name, std::string &out) {
//     ME::meta::dostruct::for_each(_struct, [&](const char *name, const auto &value) {
//         // METADOT_INFO("{} == {} ({})", name, value, typeid(value).name());
//         struct_as(out, table_name, name, value);
//     });
// }

// #define LoadLuaConfig(_struct, _luat, _c) _struct->_c = _luat[#_c].get<decltype(_struct->_c)>()

// template<typename T>
// void LoadLuaConfig(const T &_struct, lua_wrapper::LuaTable *luat) {
//     int idx = 0;
//     test_visitor vis;
//     ME::meta::dostruct::apply_visitor(vis, _struct);
//     ME::meta::dostruct::for_each(_struct, [&](const char *name, const auto &value) {
//         // (*ME::meta::dostruct::get_pointer<idx>()) =
//         //         (*luat)[name].get<decltype(ME::meta::dostruct::get<idx>(_struct))>();
//         // (*vis1.result[idx].first) = (*luat)[name].get<>();
//     });
// }

void print_error(lua_State *state, int result = 0);

neko_inline int add_package_path(lua_State *L, const std::string &str_) {
    std::string new_path = "package.path = package.path .. \"";
    if (str_.empty()) {
        return -1;
    }

    if (str_[0] != ';') {
        new_path += ";";
    }

    new_path += str_;

    if (str_[str_.length() - 1] != '/') {
        new_path += "/";
    }

    new_path += "?.lua\" ";

    neko_lua_wrap_run_string(L, new_path);
    return 0;
}

#define FUTIL_ASSERT_EXIST(x)

std::string game_assets(const std::string &path);

void print_error(lua_State *state, int result) {
    const char *message = lua_tostring(state, -1);
    neko_log_error("LuaScript ERROR:\n  %s", (message ? message : "no message"));

    if (result != 0) {
        switch (result) {
            case LUA_ERRRUN:
                neko_log_error("%s", "Lua Runtime error");
                break;
            case LUA_ERRSYNTAX:
                neko_log_error("%s", "Lua syntax error");
                break;
            case LUA_ERRMEM:
                neko_log_error("%s", "Lua was unable to allocate the required memory");
                break;
            case LUA_ERRFILE:
                neko_log_error("%s", "Lua was unable to find boot file");
                break;
            default:
                neko_log_error("Unknown lua error: %d", result);
        }
    }

    lua_pop(state, 1);
}

extern "C" int luaopen_neko_gui(lua_State *L);
extern "C" int luaopen_cffi(lua_State *L);

#define PRELOAD(name, function)     \
    lua_getglobal(L, "package");    \
    lua_getfield(L, -1, "preload"); \
    lua_pushcfunction(L, function); \
    lua_setfield(L, -2, name);      \
    lua_pop(L, 2)

void neko_register(lua_State *L);

static void lua_reg(lua_State *L) {

    luaopen_cstruct_core(L);
    luaopen_cstruct_test(L);

    neko_register(L);

    PRELOAD("neko_lua_ds.core", luaopen_ds_core);
    PRELOAD("neko_lua_datalist.core", luaopen_datalist);
    PRELOAD("neko_gui", luaopen_neko_gui);
    PRELOAD("cffi", luaopen_cffi);
}

static int __neko_lua_catch_panic(lua_State *L) {
    auto msg = neko_lua_to<const_str>(L, -1);
    neko_log_error("[lua] panic error: %s", msg);
    return 0;
}

neko_inline lua_State *neko_scripting_init() {
    neko_timer timer;
    timer.start();

    lua_State *L = neko_lua_wrap_create();

    try {

        lua_atpanic(L, __neko_lua_catch_panic);

        lua_reg(L);

        add_package_path(L, "./");

        neko_lua_wrap_run_string(L, std::format("package.path = "
                                                "'{1}/?.lua;{0}/?.lua;{0}/libs/?.lua;{0}/libs/?/init.lua;{0}/libs/"
                                                "?/?.lua;' .. package.path",
                                                game_assets("lua_scripts"), neko_fs_normalize_path(std::filesystem::current_path().string()).c_str()));

        neko_lua_wrap_run_string(L, std::format("package.cpath = "
                                                "'{1}/?.{2};{0}/?.{2};{0}/libs/?.{2};{0}/libs/?/init.{2};{0}/libs/"
                                                "?/?.{2};' .. package.cpath",
                                                game_assets("lua_scripts"), neko_fs_normalize_path(std::filesystem::current_path().string()).c_str(), "dll"));

        neko_lua_wrap_safe_dofile(L, "init");

    } catch (std::exception &ex) {
        neko_log_error("%s", ex.what());
    }

    timer.stop();
    neko_log_info(std::format("lua init done in {0:.3f} ms", timer.get()).c_str());

    return L;
}

neko_inline void neko_scripting_end(lua_State *L) { neko_lua_wrap_destory(L); }

#endif
