
#ifndef NEKO_LUA_STRUCT_H
#define NEKO_LUA_STRUCT_H

#include "neko_lua.h"

#define LUASTRUCT_REQUIRED 1
#define LUASTRUCT_OPTIONAL 0

#define IS_STRUCT(L, index, type) LUASTRUCT_is(L, #type, index)
#define CHECK_STRUCT(L, index, type) ((type *)LUASTRUCT_todata(L, #type, index, LUASTRUCT_REQUIRED))
#define OPTIONAL_STRUCT(L, index, type) ((type *)LUASTRUCT_todata(L, #type, index, LUASTRUCT_OPTIONAL))

#define PUSH_STRUCT(L, type, value)            \
    do {                                       \
        LUASTRUCT_new(L, #type, sizeof(type)); \
        *CHECK_STRUCT(L, -1, type) = (value);  \
    } while (0)

int LUASTRUCT_new(lua_State *L, const char *metatable, size_t size);
int LUASTRUCT_newref(lua_State *L, const char *metatable, int parentIndex, const void *data);
int LUASTRUCT_is(lua_State *L, const char *metatable, int index);
void *LUASTRUCT_todata(lua_State *L, const char *metatable, int index, int required);

#endif