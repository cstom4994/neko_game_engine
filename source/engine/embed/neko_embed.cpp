
#include <string>

#include "engine/neko_lua.h"
#include "engine/neko_lua_wrap.h"
#include "engine/neko_luabind.hpp"
#include "engine/neko_tolua.h"

int load_embed_lua(lua_State* L, const u8 B[], const_str name) {
    std::string contents = (const_str)B;
    if (luaL_loadbuffer(L, contents.c_str(), contents.size(), name) != LUA_OK) {
        luaL_error(L, "%s", lua_tostring(L, -1));
        return 0;
    }
    if (lua_pcall(L, 0, LUA_MULTRET, 1) != LUA_OK) {
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
static const u8 g_lua_cstruct_data[] = {
#include "cstruct.lua.h"
};
static const u8 g_lua_prefabs_data[] = {
#include "prefabs.lua.h"
};
static const u8 g_lua_bootstrap_data[] = {
#include "bootstrap.lua.h"
};

LUAOPEN_EMBED_DATA(open_embed_common, "common.lua", g_lua_common_data);
LUAOPEN_EMBED_DATA(open_embed_cstruct, "cstruct.lua", g_lua_cstruct_data);
LUAOPEN_EMBED_DATA(open_embed_prefabs, "prefabs.lua", g_lua_prefabs_data);
LUAOPEN_EMBED_DATA(open_embed_bootstrap, "bootstrap.lua", g_lua_bootstrap_data);

static void package_preload(lua_State* L, const_str name, lua_CFunction function) {
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_pushcfunction(L, function);
    lua_setfield(L, -2, name);
    lua_pop(L, 2);
    NEKO_INFO("[luabind] loaded embed %s", name);
}

namespace neko::lua {
void package_preload(lua_State* L) {
    package_preload(L, "common", open_embed_common);
    package_preload(L, "cstruct", open_embed_cstruct);
    package_preload(L, "prefabs", open_embed_prefabs);
}
void luax_run_bootstrap(lua_State* L) {
    std::string contents = (const_str)g_lua_bootstrap_data;
    if (luaL_loadbuffer(L, contents.c_str(), contents.size(), "bootstrap.lua") != LUA_OK) {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        panic("failed to load bootstrap");
    }
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        panic("failed to run bootstrap");
    }
    NEKO_INFO("loaded bootstrap");
}
}  // namespace neko::lua

int neko_tolua_boot_open(lua_State* L);

int neko_tolua_boot_open(lua_State* L) {
    neko_tolua_open(L);
    neko_tolua_module(L, NULL, 0);
    neko_tolua_beginmodule(L, NULL);

    {
        int top = lua_gettop(L);
        static const u8 B[] = {
#include "tolua.lua.h"
        };
        load_embed_lua(L, B, "tolua embedded: lua/tolua.lua");
        lua_settop(L, top);
    }

    {
        int top = lua_gettop(L);
        static const unsigned char B[] = R"lua(
local err, msg = xpcall(doit, debug.traceback)
if not err then
    print(msg)
    local _,_,label,msg = strfind(msg,"(.-:.-:%s*)(.*)")
    neko_tolua_error(msg,label)
else
    print("neko tolua generate ok")
end
        )lua";
        load_embed_lua(L, B, "tolua: embedded boot code");
        lua_settop(L, top);
    }

    neko_tolua_endmodule(L);
    return 1;
}
