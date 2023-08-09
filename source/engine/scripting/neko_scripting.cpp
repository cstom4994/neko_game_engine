// Copyright(c) 2022-2023, KaoruXun All rights reserved.

#include "neko_scripting.h"

#include <cstring>
#include <filesystem>
#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "engine/base/neko_ecs.h"
#include "engine/base/neko_engine.h"
#include "engine/common/neko_str.h"
#include "engine/platform/neko_platform.h"
#include "engine/scripting/neko_binding_engine.h"

#define FUTIL_ASSERT_EXIST(x)

namespace neko {

auto f = [](auto &...args) { (..., As(args)); };

static void bindBasic() {
    // MyStruct myStruct;
    // meta::struct_apply(myStruct, f);
}

static void __neko_lua_bug(const_str message) { neko_debug("[LUA] ", message); }
static void __neko_lua_info(const_str message) { neko_info("[LUA] ", message); }
static void __neko_lua_trace(const_str message) { neko_trace("[LUA] ", message); }
static void __neko_lua_error(const_str message) { neko_error("[LUA] ", message); }
static void __neko_lua_warn(const_str message) { neko_warn("[LUA] ", message); }

static int __neko_lua_catch_panic(lua_State *L) {
    const char *msg = lua_tostring(L, -1);
    neko_error("[LUA] PANIC ERROR: ", msg);
    return 0;
}

static void __neko_lua_exit(lua_State *L) {
    // s_lua_layer->closeConsole();
}

// returns table with pairs of path and isDirectory
static int ls(lua_State *L) {
    if (!lua_isstring(L, 1)) {
        neko_warn("invalid lua argument");
        return 0;
    }
    auto string = lua_tostring(L, 1);
    if (!std::filesystem::is_directory(string)) {
        neko_warn(std::format("{0} is not directory", string));
        return 0;
    }

    lua_newtable(L);
    int i = 0;
    for (auto &p : std::filesystem::directory_iterator(string)) {
        lua_pushnumber(L, i + 1);  // parent table index
        lua_newtable(L);
        lua_pushstring(L, "path");
        lua_pushstring(L, p.path().generic_string().c_str());
        lua_settable(L, -3);
        lua_pushstring(L, "isDirectory");
        lua_pushboolean(L, p.is_directory());
        lua_settable(L, -3);
        lua_settable(L, -3);
        i++;
    }
    return 1;
}

static void __neko_add_packagepath(const char *p) { neko_sc()->neko_lua.add_package_path(p); }

void print_error(lua_State *state, int result) {
    const char *message = lua_tostring(state, -1);
    neko_error("LuaScript ERROR:\n  ", (message ? message : "no message"));

    if (result != 0) {
        switch (result) {
            case LUA_ERRRUN:
                neko_error("Lua Runtime error");
                break;
            case LUA_ERRSYNTAX:
                neko_error("Lua syntax error");
                break;
            case LUA_ERRMEM:
                neko_error("Lua was unable to allocate the required memory");
                break;
            case LUA_ERRFILE:
                neko_error("Lua was unable to find boot file");
                break;
            default:
                neko_error("Unknown lua error: %d", result);
        }
    }

    lua_pop(state, 1);
}

#if 0
static void InitLua(neko_scripting *lc) {

    lc->L = lc->s_lua.state();

    luaopen_base(lc->L);
    luaL_openlibs(lc->L);

    lua_register(lc->L, "METADOT_TRACE", __neko_lua_trace);
    lua_register(lc->L, "METADOT_INFO", __neko_lua_info);
    lua_register(lc->L, "neko_warn", __neko_lua_warn);
    lua_register(lc->L, "METADOT_BUG", __neko_lua_bug);
    lua_register(lc->L, "neko_error", __neko_lua_error);
    lua_register(lc->L, "autoload", __neko_lua_autoload);
    lua_register(lc->L, "exit", __neko_lua_exit);
    lua_register(lc->L, "ls", ls);




    // __neko_lua_bind_image(lc->L);
    // __neko_lua_bind_gpu(lc->L);
    // __neko_lua_bind_fs(lc->L);
    // __neko_lua_bind_lz4(lc->L);
    // __neko_lua_bind_cstructcore(lc->L);
    // __neko_lua_bind_cstructtest(lc->L);

    // neko_lua_preload_auto(lc->L, luaopen_surface, "_neko_lua_surface");
    // neko_lua_preload_auto(lc->L, luaopen_surface_color, "_neko_lua_surface_color");

    // neko_lua_preload_auto(lc->L, ffi_module_open, "ffi");
    // neko_lua_preload_auto(lc->L, luaopen_lbind, "lbind");

    // lc->s_lua["METADOT_RESLOC"] = lua_wrapper::function([](const char *a) { return METADOT_RESLOC(a); });
    // lc->s_lua["GetSurfaceFromTexture"] = lua_wrapper::function([](TextureRef tex) { return tex->surface(); });
    // lc->s_lua["GetWindowH"] = lua_wrapper::function([]() { return the<engine>().eng()->windowHeight; });
    // lc->s_lua["GetWindowW"] = lua_wrapper::function([]() { return the<engine>().eng()->windowWidth; });

    // lc->s_lua["SDL_FreeSurface"] = lua_wrapper::function(SDL_FreeSurface);
    // lc->s_lua["R_SetImageFilter"] = lua_wrapper::function(R_SetImageFilter);
    // lc->s_lua["R_CopyImageFromSurface"] = lua_wrapper::function(R_CopyImageFromSurface);
    // lc->s_lua["R_GetTextureHandle"] = lua_wrapper::function(R_GetTextureHandle);
    // lc->s_lua["R_GetTextureAttr"] = lua_wrapper::function(R_GetTextureAttr);
    // lc->s_lua["LoadTexture"] = lua_wrapper::function(LoadTexture);
    //// lc->s_lua["DestroyTexture"] = lua_wrapper::function(DestroyTexture);
    //// lc->s_lua["CreateTexture"] = lua_wrapper::function(CreateTexture);
    // lc->s_lua["__neko_lua_buildnum"] = lua_wrapper::function(neko_lua_buildnum);
    // lc->s_lua["__neko_lua_metadata"] = lua_wrapper::function(neko_lua_metadata);
    // lc->s_lua["add_packagepath"] = lua_wrapper::function(add_packagepath);

    script_runfile("data/scripts/init.lua");
}

#endif

// void run_script_in_console(scripting *_struct, const char *c) {
//     luaL_loadstring(_struct->L, c);
//     auto result = neko_lua_debug_pcall(_struct->L, 0, LUA_MULTRET, 0);
//     if (result != LUA_OK) {
//         print_error(_struct->L);
//         return;
//     }
// }
//
// void script_runfile(const char *filePath) {
//     FUTIL_ASSERT_EXIST(filePath);
//
//     int result = luaL_loadfile(the<scripting>().L, neko_file_path(filePath));
//     if (result != LUA_OK) {
//         print_error(the<scripting>().L);
//         return;
//     }
//     result = neko_lua_debug_pcall(the<scripting>().L, 0, LUA_MULTRET, 0);
//
//     if (result != LUA_OK) {
//         print_error(the<scripting>().L);
//     }
// }

static void lua_reg_ecs(lua_State *ls);

static void lua_reg(lua_State *ls) {
    neko_lua_register_t<>(ls)
            .def(&__neko_lua_trace, "log_trace")  // logger
            .def(&__neko_lua_bug, "log_debug")
            .def(&__neko_lua_error, "log_error")
            .def(&__neko_lua_warn, "log_warn")
            .def(&__neko_lua_info, "log_info")

            .def(&__neko_add_packagepath, "add_packagepath");

    lua_reg_ecs(ls);

    neko_lua_debug_setup(ls, "debugger", "dbg", NULL, NULL);

    luaopen_cstruct_core(ls);
    luaopen_cstruct_test(ls);
    luaopen_datalist(ls);

    neko_register(ls);
}

void neko_scripting::__init() {
    neko_timer timer;
    timer.start();

    try {

        neko_lua.setModFuncFlag(true);

        lua_atpanic(neko_lua.get_lua_state(), __neko_lua_catch_panic);

        neko_lua.reg(lua_reg);

        neko_lua.add_package_path("./");

        neko_lua.run_string(
                std::format("package.path = "
                            "'{1}/?.lua;{0}/?.lua;{0}/libs/?.lua;{0}/libs/?/init.lua;{0}/libs/"
                            "?/?.lua;' .. package.path",
                            neko_file_path("data/scripts"), neko_fs_normalize_path(std::filesystem::current_path().string()).c_str()));

        neko_lua.run_string(
                std::format("package.cpath = "
                            "'{1}/?.{2};{0}/?.{2};{0}/libs/?.{2};{0}/libs/?/init.{2};{0}/libs/"
                            "?/?.{2};' .. package.cpath",
                            neko_file_path("data/scripts"), neko_fs_normalize_path(std::filesystem::current_path().string()).c_str(), "dll"));

        neko_lua.load_file(neko_file_path("data/scripts/init.lua"));

    } catch (std::exception &ex) {
        neko_error(ex.what());
    }

    timer.stop();
    neko_info(std::format("lua loading done in {0:.4f} ms", timer.get()));
}

void neko_scripting::__end() {}

void neko_scripting::update() {
    // luaL_loadstring(_struct->L, s_couroutineFileSrc.c_str());
    // if (__neko_lua_debug_pcall(_struct->L, 0, LUA_MULTRET, 0) != LUA_OK) {
    //     print_error(_struct->L);
    //     return;
    // }
    // auto &luawrap = this->s_lua;
    // auto OnUpdate = luawrap["OnUpdate"];
    // OnUpdate();
}

void neko_scripting::update_render() {}

void neko_scripting::update_tick() {}

static void lua_reg_ecs(lua_State *ls) {
    neko_lua_register_t<>(ls)
            .def(&neko_ecs_make, "neko_ecs_make")  // logger
            .def(&neko_ecs_destroy, "neko_ecs_destroy")
            //.def(&neko_ecs_register_component, "neko_ecs_register_component")
            //.def(&neko_ecs_register_system, "neko_ecs_register_system")
            //.def(&neko_ecs_run_systems, "neko_ecs_run_systems")
            //.def(&neko_ecs_run_system, "neko_ecs_run_system")
            .def(&neko_ecs_for_count, "neko_ecs_for_count")
            .def(&neko_ecs_get_ent, "neko_ecs_get_ent")
            .def(&neko_ecs_ent_make, "neko_ecs_ent_make")
            .def(&neko_ecs_ent_destroy, "neko_ecs_ent_destroy")
            .def(&neko_ecs_ent_add_component, "neko_ecs_ent_add_component")
            .def(&neko_ecs_ent_remove_component, "neko_ecs_ent_remove_component")
            .def(&neko_ecs_ent_get_component, "neko_ecs_ent_get_component")
            .def(&neko_ecs_ent_has_component, "neko_ecs_ent_has_component")
            .def(&neko_ecs_ent_has_mask, "neko_ecs_ent_has_mask")
            .def(&neko_ecs_ent_is_valid, "neko_ecs_ent_is_valid")
            .def(&neko_ecs_ent_get_version, "neko_ecs_ent_get_version")
            .def(&neko_ecs_ent_print, "neko_ecs_ent_print");
}

}  // namespace neko