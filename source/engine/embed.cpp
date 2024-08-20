
#include <string>

#include "engine/lua_util.h"
#include "engine/luabind.hpp"
#include "engine/luax.h"

int load_embed_lua(lua_State* L, const u8 B[], const_str name) {
    std::string contents = (const_str)B;
    if (luaL_loadbuffer(L, contents.c_str(), contents.size(), name) != LUA_OK) {
        luaL_error(L, "%s", lua_tostring(L, -1));
        return 0;
    }
    if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
        luaL_error(L, "%s", lua_tostring(L, -1));
        return 0;
    }
    return 1;
}

#define LUAOPEN_EMBED_DATA(func, name, compressed_data) \
    static int func(lua_State* L) {                     \
        i32 top = lua_gettop(L);                        \
        load_embed_lua(L, compressed_data, name);       \
        return lua_gettop(L) - top;                     \
    }

static const u8 g_lua_common_data[] = {
#include "common.lua.h"
};
static const u8 g_lua_bootstrap_data[] = {
#include "bootstrap.lua.h"
};
static const u8 g_lua_nekogame_data[] = {
#include "nekogame.lua.h"
};

LUAOPEN_EMBED_DATA(open_embed_common, "common.lua", g_lua_common_data);
LUAOPEN_EMBED_DATA(open_embed_bootstrap, "bootstrap.lua", g_lua_bootstrap_data);
// LUAOPEN_EMBED_DATA(open_embed_nekogame, "nekogame.lua", g_lua_nekogame_data);

extern "C" {
int luaopen_socket_core(lua_State* L);
int luaopen_mime_core(lua_State* L);
int luaopen_http(lua_State* L);
}

namespace neko::lua {
void package_preload_embed(lua_State* L) {

    luaL_Reg preloads[] = {
            {"common", open_embed_common},
            {"http", luaopen_http},
    };

    for (auto m : preloads) {
        luax_package_preload(L, m.name, m.func);
    }
}
void luax_run_bootstrap(lua_State* L) {
    std::string contents = (const_str)g_lua_bootstrap_data;
    if (luaL_loadbuffer(L, contents.c_str(), contents.size(), "bootstrap.lua") != LUA_OK) {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        neko_panic("failed to load bootstrap");
    }
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char* errorMsg = lua_tostring(L, -1);
        fprintf(stderr, "bootstrap error: %s\n", errorMsg);
        lua_pop(L, 1);
        neko_panic("failed to run bootstrap");
    }
    console_log("loaded bootstrap");
}
void luax_run_nekogame(lua_State* L) {
    std::string contents = (const_str)g_lua_nekogame_data;
    if (luaL_loadbuffer(L, contents.c_str(), contents.size(), "nekogame.lua") != LUA_OK) {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        neko_panic("failed to load nekogame");
    }
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char* errorMsg = lua_tostring(L, -1);
        fprintf(stderr, "nekogame error: %s\n", errorMsg);
        lua_pop(L, 1);
        neko_panic("failed to run nekogame");
    }
    console_log("loaded nekogame");
}
}  // namespace neko::lua