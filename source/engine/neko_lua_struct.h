
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

// luabind

#define neko_luabind_type(L, type) neko_luabind_type_add(L, #type, sizeof(type))

enum { LUAA_INVALID_TYPE = -1 };

typedef lua_Integer neko_luabind_Type;
typedef int (*neko_luabind_Pushfunc)(lua_State *, neko_luabind_Type, const void *);
typedef void (*neko_luabind_Tofunc)(lua_State *, neko_luabind_Type, void *, int);

neko_luabind_Type neko_luabind_type_add(lua_State *L, const char *type, size_t size);
neko_luabind_Type neko_luabind_type_find(lua_State *L, const char *type);

const char *neko_luabind_typename(lua_State *L, neko_luabind_Type id);
size_t neko_luabind_typesize(lua_State *L, neko_luabind_Type id);

#define neko_luabind_push(L, type, c_in) neko_luabind_push_type(L, neko_luabind_type(L, type), c_in)
#define neko_luabind_to(L, type, c_out, index) neko_luabind_to_type(L, neko_luabind_type(L, type), c_out, index)

int neko_luabind_push_type(lua_State *L, neko_luabind_Type type, const void *c_in);
void neko_luabind_to_type(lua_State *L, neko_luabind_Type type, void *c_out, int index);

#define LUAA_INVALID_MEMBER_NAME NULL

#define neko_luabind_enum(L, type) neko_luabind_enum_type(L, neko_luabind_type(L, type), sizeof(type))

#define neko_luabind_enum_value(L, type, value)                    \
    const type __neko_luabind_enum_value_temp_##value[] = {value}; \
    neko_luabind_enum_value_type(L, neko_luabind_type(L, type), __neko_luabind_enum_value_temp_##value, #value)

#define neko_luabind_enum_value_name(L, type, value, name)         \
    const type __neko_luabind_enum_value_temp_##value[] = {value}; \
    neko_luabind_enum_value_type(L, neko_luabind_type(L, type), __neko_luabind_enum_value_temp_##value, name)

#define neko_luabind_enum_push(L, type, c_in) neko_luabind_enum_push_type(L, neko_luabind_type(L, type), c_in)
#define neko_luabind_enum_to(L, type, c_out, index) neko_luabind_enum_to_type(L, neko_luabind_type(L, type), c_out, index)

#define neko_luabind_enum_has_value(L, type, value)                \
    const type __neko_luabind_enum_value_temp_##value[] = {value}; \
    neko_luabind_enum_has_value_type(L, neko_luabind_type(L, type), __neko_luabind_enum_value_temp_##value)

#define neko_luabind_enum_has_name(L, type, name) neko_luabind_enum_has_name_type(L, neko_luabind_type(L, type), name)

#define neko_luabind_enum_registered(L, type) neko_luabind_enum_registered_type(L, neko_luabind_type(L, type))
#define neko_luabind_enum_next_value_name(L, type, member) neko_luabind_enum_next_value_name_type(L, neko_luabind_type(L, type), member)

void neko_luabind_enum_type(lua_State *L, neko_luabind_Type type, size_t size);
void neko_luabind_enum_value_type(lua_State *L, neko_luabind_Type type, const void *value, const char *name);

int neko_luabind_enum_push_type(lua_State *L, neko_luabind_Type type, const void *c_in);
void neko_luabind_enum_to_type(lua_State *L, neko_luabind_Type type, void *c_out, int index);

bool neko_luabind_enum_has_value_type(lua_State *L, neko_luabind_Type type, const void *value);
bool neko_luabind_enum_has_name_type(lua_State *L, neko_luabind_Type type, const char *name);

bool neko_luabind_enum_registered_type(lua_State *L, neko_luabind_Type type);
const char *neko_luabind_enum_next_value_name_type(lua_State *L, neko_luabind_Type type, const char *member);

#endif