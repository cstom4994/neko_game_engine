#ifndef NEKO_BINDING_ENGINE_H
#define NEKO_BINDING_ENGINE_H

#include "engine/neko.hpp"
#include "engine/neko_asset.h"
#include "engine/neko_engine.h"
#include "engine/neko_lua.h"
#include "engine/neko_math.h"

typedef struct {
    const char *name;
    lua_CFunction func;
} neko_luaL_reg;

void neko_register(lua_State *L);

lua_Number luax_number_field(lua_State *L, s32 arg, const char *key);
lua_Number luax_opt_number_field(lua_State *L, s32 arg, const char *key, lua_Number fallback);

bool luax_boolean_field(lua_State *L, s32 arg, const char *key, bool fallback = false);

void luax_new_class(lua_State *L, const char *mt_name, const luaL_Reg *l);

int luax_msgh(lua_State *L);

enum {
    LUAX_UD_TNAME = 1,
    LUAX_UD_PTR_SIZE = 2,
};

template <typename T>
void luax_new_userdata(lua_State *L, T data, const char *tname) {
    void *new_udata = lua_newuserdatauv(L, sizeof(T), 2);

    lua_pushstring(L, tname);
    lua_setiuservalue(L, -2, LUAX_UD_TNAME);

    lua_pushnumber(L, sizeof(T));
    lua_setiuservalue(L, -2, LUAX_UD_PTR_SIZE);

    memcpy(new_udata, &data, sizeof(T));
    luaL_setmetatable(L, tname);
}

#define luax_ptr_userdata luax_new_userdata

namespace neko::lua {
void package_preload(lua_State *L);
}

#endif