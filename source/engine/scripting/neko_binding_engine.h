
#ifndef NEKO_BINDING_ENGINE_H
#define NEKO_BINDING_ENGINE_H

#include "engine/base/neko_engine.h"
#include "engine/scripting/neko_lua_base.h"

int __neko_app_int(lua_State* L) {}
int __neko_app_update(lua_State* L) {}
int __neko_app_shutdown(lua_State* L) {}

neko_static_inline int luaA_push_app_decl(lua_State* L, neko_lua_auto_Type t, const void* c_in) {
    neko_application_desc_t* p = (neko_application_desc_t*)c_in;
    lua_pushcfunction(L, __neko_app_int);
    lua_pushcfunction(L, __neko_app_update);
    lua_pushcfunction(L, __neko_app_shutdown);
    lua_pushstring(L, p->window_title);
    lua_pushinteger(L, p->window_width);
    lua_pushinteger(L, p->window_height);
    lua_pushinteger(L, (s64)p->window_flags);
    lua_pushnumber(L, p->frame_rate);
    lua_pushboolean(L, p->enable_vsync);
    lua_pushboolean(L, p->is_running);
    return 10;
}

neko_static_inline void luaA_to_app_decl(lua_State* L, neko_lua_auto_Type t, void* c_out, int index) {
    neko_application_desc_t* p = (neko_application_desc_t*)c_out;
    // p->init = lua_tocfunction(L, index);
    // p->update = lua_tocfunction(L, index - 1);
    // p->shutdown = lua_tocfunction(L, index - 2);
    p->window_title = lua_tostring(L, index - 3);
    p->window_width = lua_tointeger(L, index - 4);
    p->window_height = lua_tointeger(L, index - 5);
    p->window_flags = (neko_window_flags)lua_tointeger(L, index - 6);
    p->frame_rate = lua_tonumber(L, index - 7);
    p->enable_vsync = lua_toboolean(L, index - 8);
    p->is_running = lua_toboolean(L, index - 9);
}

#endif  // !NEKO_BINDING_ENGINE_H
