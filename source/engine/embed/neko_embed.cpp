
#include "engine/neko_luabind.hpp"

#define LUAOPEN_EMBED_DATA(func, name, compressed_data)                              \
    static int func(lua_State* L) {                                                  \
        s32 top = lua_gettop(L);                                                     \
                                                                                     \
        std::string contents = (const_str)compressed_data;                           \
                                                                                     \
        if (luaL_loadbuffer(L, contents.c_str(), contents.size(), name) != LUA_OK) { \
            luaL_error(L, "%s", lua_tostring(L, -1));                                \
            return 0;                                                                \
        }                                                                            \
                                                                                     \
        if (lua_pcall(L, 0, LUA_MULTRET, 1) != LUA_OK) {                             \
            luaL_error(L, "%s", lua_tostring(L, -1));                                \
            return 0;                                                                \
        }                                                                            \
                                                                                     \
        return lua_gettop(L) - top;                                                  \
    }

static const u8 g_lua_behavior_data[] = {
#include "behavior.lua.h"
};
static const u8 g_lua_common_data[] = {
#include "common.lua.h"
};
static const u8 g_lua_cstruct_data[] = {
#include "cstruct.lua.h"
};
static const u8 g_lua_ecs_data[] = {
#include "ecs.lua.h"
};

LUAOPEN_EMBED_DATA(open_embed_behavior, "behavior.lua", g_lua_behavior_data);
LUAOPEN_EMBED_DATA(open_embed_common, "common.lua", g_lua_common_data);
LUAOPEN_EMBED_DATA(open_embed_cstruct, "cstruct.lua", g_lua_cstruct_data);
LUAOPEN_EMBED_DATA(open_embed_ecs, "ecs.lua", g_lua_ecs_data);

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
    package_preload(L, "behavior", open_embed_behavior);
    package_preload(L, "common", open_embed_common);
    package_preload(L, "cstruct", open_embed_cstruct);
    package_preload(L, "ecs", open_embed_ecs);
}
}  // namespace neko::lua