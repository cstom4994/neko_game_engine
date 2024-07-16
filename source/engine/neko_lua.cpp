#include "neko_lua.h"

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <algorithm>
#include <iostream>

#include "neko_api.hpp"
#include "neko_app.h"
#include "neko_asset.h"
#include "neko_base.h"
#include "neko_lua_wrap.h"
#include "neko_os.h"
#include "neko_prelude.h"

void __neko_luabind_init(lua_State *L) {

    lua_pushinteger(L, 0);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_index");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_ids");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_names");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_sizes");

    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "stack_push");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "stack_to");

    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs_offset");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_sizes");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_values");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "functions");

    lua_newuserdata(L, LUAA_RETURN_STACK_SIZE);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "call_ret_stk");
    lua_newuserdata(L, LUAA_ARGUMENT_STACK_SIZE);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "call_arg_stk");
    lua_pushinteger(L, 0);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "call_ret_ptr");
    lua_pushinteger(L, 0);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "call_arg_ptr");

    // compiler does weird macro expansion with "bool" so no magic macro for you
    neko_luabind_conversion_type(L, neko_luabind_type_add(L, "bool", sizeof(bool)), neko_luabind_push_bool, neko_luabind_to_bool);
    neko_luabind_conversion_type(L, neko_luabind_type_add(L, "_Bool", sizeof(bool)), neko_luabind_push_bool, neko_luabind_to_bool);
    neko_luabind_conversion(L, char, neko_luabind_push_char, neko_luabind_to_char);
    neko_luabind_conversion(L, signed char, neko_luabind_push_signed_char, neko_luabind_to_signed_char);
    neko_luabind_conversion(L, unsigned char, neko_luabind_push_unsigned_char, neko_luabind_to_unsigned_char);
    neko_luabind_conversion(L, short, neko_luabind_push_short, neko_luabind_to_short);
    neko_luabind_conversion(L, unsigned short, neko_luabind_push_unsigned_short, neko_luabind_to_unsigned_short);
    neko_luabind_conversion(L, int, neko_luabind_push_int, neko_luabind_to_int);
    neko_luabind_conversion(L, unsigned int, neko_luabind_push_unsigned_int, neko_luabind_to_unsigned_int);
    neko_luabind_conversion(L, long, neko_luabind_push_long, neko_luabind_to_long);
    neko_luabind_conversion(L, i32, neko_luabind_push_long, neko_luabind_to_long);
    neko_luabind_conversion(L, unsigned long, neko_luabind_push_unsigned_long, neko_luabind_to_unsigned_long);
    neko_luabind_conversion(L, u32, neko_luabind_push_unsigned_long, neko_luabind_to_unsigned_long);
    neko_luabind_conversion(L, long long, neko_luabind_push_long_long, neko_luabind_to_long_long);
    neko_luabind_conversion(L, i64, neko_luabind_push_long_long, neko_luabind_to_long_long);
    neko_luabind_conversion(L, unsigned long long, neko_luabind_push_unsigned_long_long, neko_luabind_to_unsigned_long_long);
    neko_luabind_conversion(L, u64, neko_luabind_push_unsigned_long_long, neko_luabind_to_unsigned_long_long);
    neko_luabind_conversion(L, float, neko_luabind_push_float, neko_luabind_to_float);
    neko_luabind_conversion(L, f32, neko_luabind_push_float, neko_luabind_to_float);
    neko_luabind_conversion(L, double, neko_luabind_push_double, neko_luabind_to_double);
    neko_luabind_conversion(L, f64, neko_luabind_push_double, neko_luabind_to_double);
    neko_luabind_conversion(L, long double, neko_luabind_push_long_double, neko_luabind_to_long_double);

    neko_luabind_conversion_push_type(L, neko_luabind_type_add(L, "const bool", sizeof(bool)), neko_luabind_push_bool);
    neko_luabind_conversion_push_type(L, neko_luabind_type_add(L, "const _Bool", sizeof(bool)), neko_luabind_push_bool);
    neko_luabind_conversion_push(L, const char, neko_luabind_push_char);
    neko_luabind_conversion_push(L, const signed char, neko_luabind_push_signed_char);
    neko_luabind_conversion_push(L, const unsigned char, neko_luabind_push_unsigned_char);
    neko_luabind_conversion_push(L, const short, neko_luabind_push_short);
    neko_luabind_conversion_push(L, const unsigned short, neko_luabind_push_unsigned_short);
    neko_luabind_conversion_push(L, const int, neko_luabind_push_int);
    neko_luabind_conversion_push(L, const unsigned int, neko_luabind_push_unsigned_int);
    neko_luabind_conversion_push(L, const long, neko_luabind_push_long);
    neko_luabind_conversion_push(L, const unsigned long, neko_luabind_push_unsigned_long);
    neko_luabind_conversion_push(L, const long long, neko_luabind_push_long_long);
    neko_luabind_conversion_push(L, const unsigned long long, neko_luabind_push_unsigned_long_long);
    neko_luabind_conversion_push(L, const float, neko_luabind_push_float);
    neko_luabind_conversion_push(L, const double, neko_luabind_push_double);
    neko_luabind_conversion_push(L, const long double, neko_luabind_push_long_double);

    neko_luabind_conversion(L, char *, neko_luabind_push_char_ptr, neko_luabind_to_char_ptr);
    neko_luabind_conversion(L, const char *, neko_luabind_push_const_char_ptr, neko_luabind_to_const_char_ptr);
    neko_luabind_conversion(L, void *, neko_luabind_push_void_ptr, neko_luabind_to_void_ptr);

    neko_luabind_conversion_push_type(L, neko_luabind_type_add(L, "void", 1), neko_luabind_push_void);  // sizeof(void) is 1 on gcc
}

void __neko_luabind_fini(lua_State *L) {

    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_index");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_ids");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_names");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_sizes");

    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "stack_push");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "stack_to");

    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs_offset");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_sizes");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_values");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "functions");

    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "call_ret_stk");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "call_arg_stk");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "call_ret_ptr");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "call_arg_ptr");
}

neko_luabind_Type neko_luabind_type_add(lua_State *L, const char *type, size_t size) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_ids");
    lua_getfield(L, -1, type);

    if (lua_isnumber(L, -1)) {

        neko_luabind_Type id = lua_tointeger(L, -1);
        lua_pop(L, 2);
        return id;

    } else {

        lua_pop(L, 2);

        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_index");

        neko_luabind_Type id = lua_tointeger(L, -1);
        lua_pop(L, 1);
        id++;

        lua_pushinteger(L, id);
        lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_index");

        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_ids");
        lua_pushinteger(L, id);
        lua_setfield(L, -2, type);
        lua_pop(L, 1);

        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_names");
        lua_pushinteger(L, id);
        lua_pushstring(L, type);
        lua_settable(L, -3);
        lua_pop(L, 1);

        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_sizes");
        lua_pushinteger(L, id);
        lua_pushinteger(L, size);
        lua_settable(L, -3);
        lua_pop(L, 1);

        return id;
    }
}

neko_luabind_Type neko_luabind_type_find(lua_State *L, const char *type) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_ids");
    lua_getfield(L, -1, type);

    neko_luabind_Type id = lua_isnil(L, -1) ? LUAA_INVALID_TYPE : lua_tointeger(L, -1);
    lua_pop(L, 2);

    return id;
}

const char *neko_luabind_typename(lua_State *L, neko_luabind_Type id) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_names");
    lua_pushinteger(L, id);
    lua_gettable(L, -2);

    const char *type = lua_isnil(L, -1) ? "LUAA_INVALID_TYPE" : lua_tostring(L, -1);
    lua_pop(L, 2);

    return type;
}

size_t neko_luabind_typesize(lua_State *L, neko_luabind_Type id) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_sizes");
    lua_pushinteger(L, id);
    lua_gettable(L, -2);

    size_t size = lua_isnil(L, -1) ? -1 : lua_tointeger(L, -1);
    lua_pop(L, 2);

    return size;
}

/*
** Stack
*/

int neko_luabind_push_type(lua_State *L, neko_luabind_Type type_id, const void *c_in) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "stack_push");
    lua_pushinteger(L, type_id);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {
        neko_luabind_Pushfunc func = (neko_luabind_Pushfunc)lua_touserdata(L, -1);
        lua_pop(L, 2);
        return func(L, type_id, c_in);
    }

    lua_pop(L, 2);

    if (neko_luabind_struct_registered_type(L, type_id)) {
        return neko_luabind_struct_push_type(L, type_id, c_in);
    }

    if (neko_luabind_enum_registered_type(L, type_id)) {
        return neko_luabind_enum_push_type(L, type_id, c_in);
    }

    lua_pushfstring(L, "neko_luabind_push: conversion to Lua object from type '%s' not registered!", neko_luabind_typename(L, type_id));
    lua_error(L);
    return 0;
}

void neko_luabind_to_type(lua_State *L, neko_luabind_Type type_id, void *c_out, int index) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "stack_to");
    lua_pushinteger(L, type_id);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {
        neko_luabind_Tofunc func = (neko_luabind_Tofunc)lua_touserdata(L, -1);
        lua_pop(L, 2);
        func(L, type_id, c_out, index);
        return;
    }

    lua_pop(L, 2);

    if (neko_luabind_struct_registered_type(L, type_id)) {
        neko_luabind_struct_to_type(L, type_id, c_out, index);
        return;
    }

    if (neko_luabind_enum_registered_type(L, type_id)) {
        neko_luabind_enum_to_type(L, type_id, c_out, index);
        return;
    }

    lua_pushfstring(L, "neko_luabind_to: conversion from Lua object to type '%s' not registered!", neko_luabind_typename(L, type_id));
    lua_error(L);
}

void neko_luabind_conversion_type(lua_State *L, neko_luabind_Type type_id, neko_luabind_Pushfunc push_func, neko_luabind_Tofunc to_func) {
    neko_luabind_conversion_push_type(L, type_id, push_func);
    neko_luabind_conversion_to_type(L, type_id, to_func);
}

void neko_luabind_conversion_push_type(lua_State *L, neko_luabind_Type type_id, neko_luabind_Pushfunc func) {
    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "stack_push");
    lua_pushinteger(L, type_id);
    lua_pushlightuserdata(L, (void *)func);
    lua_settable(L, -3);
    lua_pop(L, 1);
}

void neko_luabind_conversion_to_type(lua_State *L, neko_luabind_Type type_id, neko_luabind_Tofunc func) {
    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "stack_to");
    lua_pushinteger(L, type_id);
    lua_pushlightuserdata(L, (void *)func);
    lua_settable(L, -3);
    lua_pop(L, 1);
}

int neko_luabind_push_bool(lua_State *L, neko_luabind_Type type_id, const void *c_in) {
    lua_pushboolean(L, *(bool *)c_in);
    return 1;
}

void neko_luabind_to_bool(lua_State *L, neko_luabind_Type type_id, void *c_out, int index) { *(bool *)c_out = lua_toboolean(L, index); }

int neko_luabind_push_char(lua_State *L, neko_luabind_Type type_id, const void *c_in) {
    lua_pushinteger(L, *(char *)c_in);
    return 1;
}

void neko_luabind_to_char(lua_State *L, neko_luabind_Type type_id, void *c_out, int index) { *(char *)c_out = lua_tointeger(L, index); }

int neko_luabind_push_signed_char(lua_State *L, neko_luabind_Type type_id, const void *c_in) {
    lua_pushinteger(L, *(signed char *)c_in);
    return 1;
}

void neko_luabind_to_signed_char(lua_State *L, neko_luabind_Type type_id, void *c_out, int index) { *(signed char *)c_out = lua_tointeger(L, index); }

int neko_luabind_push_unsigned_char(lua_State *L, neko_luabind_Type type_id, const void *c_in) {
    lua_pushinteger(L, *(unsigned char *)c_in);
    return 1;
}

void neko_luabind_to_unsigned_char(lua_State *L, neko_luabind_Type type_id, void *c_out, int index) { *(unsigned char *)c_out = lua_tointeger(L, index); }

int neko_luabind_push_short(lua_State *L, neko_luabind_Type type_id, const void *c_in) {
    lua_pushinteger(L, *(short *)c_in);
    return 1;
}

void neko_luabind_to_short(lua_State *L, neko_luabind_Type type_id, void *c_out, int index) { *(short *)c_out = lua_tointeger(L, index); }

int neko_luabind_push_unsigned_short(lua_State *L, neko_luabind_Type type_id, const void *c_in) {
    lua_pushinteger(L, *(unsigned short *)c_in);
    return 1;
}

void neko_luabind_to_unsigned_short(lua_State *L, neko_luabind_Type type_id, void *c_out, int index) { *(unsigned short *)c_out = lua_tointeger(L, index); }

int neko_luabind_push_int(lua_State *L, neko_luabind_Type type_id, const void *c_in) {
    lua_pushinteger(L, *(int *)c_in);
    return 1;
}

void neko_luabind_to_int(lua_State *L, neko_luabind_Type type_id, void *c_out, int index) { *(int *)c_out = lua_tointeger(L, index); }

int neko_luabind_push_unsigned_int(lua_State *L, neko_luabind_Type type_id, const void *c_in) {
    lua_pushinteger(L, *(unsigned int *)c_in);
    return 1;
}

void neko_luabind_to_unsigned_int(lua_State *L, neko_luabind_Type type_id, void *c_out, int index) { *(unsigned int *)c_out = lua_tointeger(L, index); }

int neko_luabind_push_long(lua_State *L, neko_luabind_Type type_id, const void *c_in) {
    lua_pushinteger(L, *(long *)c_in);
    return 1;
}

void neko_luabind_to_long(lua_State *L, neko_luabind_Type type_id, void *c_out, int index) { *(long *)c_out = lua_tointeger(L, index); }

int neko_luabind_push_unsigned_long(lua_State *L, neko_luabind_Type type_id, const void *c_in) {
    lua_pushinteger(L, *(unsigned long *)c_in);
    return 1;
}

void neko_luabind_to_unsigned_long(lua_State *L, neko_luabind_Type type_id, void *c_out, int index) { *(unsigned long *)c_out = lua_tointeger(L, index); }

int neko_luabind_push_long_long(lua_State *L, neko_luabind_Type type_id, const void *c_in) {
    lua_pushinteger(L, *(long long *)c_in);
    return 1;
}

void neko_luabind_to_long_long(lua_State *L, neko_luabind_Type type_id, void *c_out, int index) { *(long long *)c_out = lua_tointeger(L, index); }

int neko_luabind_push_unsigned_long_long(lua_State *L, neko_luabind_Type type_id, const void *c_in) {
    lua_pushinteger(L, *(unsigned long long *)c_in);
    return 1;
}

void neko_luabind_to_unsigned_long_long(lua_State *L, neko_luabind_Type type_id, void *c_out, int index) { *(unsigned long long *)c_out = lua_tointeger(L, index); }

int neko_luabind_push_float(lua_State *L, neko_luabind_Type type_id, const void *c_in) {
    lua_pushnumber(L, *(float *)c_in);
    return 1;
}

void neko_luabind_to_float(lua_State *L, neko_luabind_Type type_id, void *c_out, int index) { *(float *)c_out = lua_tonumber(L, index); }

int neko_luabind_push_double(lua_State *L, neko_luabind_Type type_id, const void *c_in) {
    lua_pushnumber(L, *(double *)c_in);
    return 1;
}

void neko_luabind_to_double(lua_State *L, neko_luabind_Type type_id, void *c_out, int index) { *(double *)c_out = lua_tonumber(L, index); }

int neko_luabind_push_long_double(lua_State *L, neko_luabind_Type type_id, const void *c_in) {
    lua_pushnumber(L, *(long double *)c_in);
    return 1;
}

void neko_luabind_to_long_double(lua_State *L, neko_luabind_Type type_id, void *c_out, int index) { *(long double *)c_out = lua_tonumber(L, index); }

int neko_luabind_push_char_ptr(lua_State *L, neko_luabind_Type type_id, const void *c_in) {
    lua_pushstring(L, *(char **)c_in);
    return 1;
}

void neko_luabind_to_char_ptr(lua_State *L, neko_luabind_Type type_id, void *c_out, int index) { *(char **)c_out = (char *)lua_tostring(L, index); }

int neko_luabind_push_const_char_ptr(lua_State *L, neko_luabind_Type type_id, const void *c_in) {
    lua_pushstring(L, *(const char **)c_in);
    return 1;
}

void neko_luabind_to_const_char_ptr(lua_State *L, neko_luabind_Type type_id, void *c_out, int index) { *(const char **)c_out = lua_tostring(L, index); }

int neko_luabind_push_void_ptr(lua_State *L, neko_luabind_Type type_id, const void *c_in) {
    lua_pushlightuserdata(L, *(void **)c_in);
    return 1;
}

void neko_luabind_to_void_ptr(lua_State *L, neko_luabind_Type type_id, void *c_out, int index) { *(void **)c_out = (void *)lua_touserdata(L, index); }

int neko_luabind_push_void(lua_State *L, neko_luabind_Type type_id, const void *c_in) {
    lua_pushnil(L);
    return 1;
}

bool neko_luabind_conversion_registered_type(lua_State *L, neko_luabind_Type type_id) {
    return (neko_luabind_conversion_push_registered_type(L, type_id) && neko_luabind_conversion_to_registered_type(L, type_id));
}

bool neko_luabind_conversion_push_registered_type(lua_State *L, neko_luabind_Type type_id) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "stack_push");
    lua_pushinteger(L, type_id);
    lua_gettable(L, -2);

    bool reg = !lua_isnil(L, -1);
    lua_pop(L, 2);

    return reg;
}

bool neko_luabind_conversion_to_registered_type(lua_State *L, neko_luabind_Type type_id) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "stack_to");
    lua_pushinteger(L, type_id);
    lua_gettable(L, -2);

    bool reg = !lua_isnil(L, -1);
    lua_pop(L, 2);

    return reg;
}

/*
** Structs
*/

int neko_luabind_struct_push_member_offset_type(lua_State *L, neko_luabind_Type type, size_t offset, const void *c_in) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs_offset");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_pushinteger(L, offset);
        lua_gettable(L, -2);

        if (!lua_isnil(L, -1)) {
            lua_getfield(L, -1, "type");
            neko_luabind_Type stype = lua_tointeger(L, -1);
            lua_pop(L, 4);
            return neko_luabind_push_type(L, stype, (char *)c_in + offset);
        }

        lua_pop(L, 3);
        lua_pushfstring(L, "neko_luabind_struct_push_member: Member offset '%d' not registered for struct '%s'!", offset, neko_luabind_typename(L, type));
        lua_error(L);
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_luabind_struct_push_member: Struct '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
    return 0;
}

int neko_luabind_struct_push_member_name_type(lua_State *L, neko_luabind_Type type, const char *member, const void *c_in) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_getfield(L, -1, member);

        if (!lua_isnil(L, -1)) {
            lua_getfield(L, -1, "type");
            neko_luabind_Type stype = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, -1, "offset");
            size_t offset = lua_tointeger(L, -1);
            lua_pop(L, 4);
            return neko_luabind_push_type(L, stype, (char *)c_in + offset);
        }

        lua_pop(L, 3);
        lua_pushfstring(L, "neko_luabind_struct_push_member: Member name '%s' not registered for struct '%s'!", member, neko_luabind_typename(L, type));
        lua_error(L);
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_luabind_struct_push_member: Struct '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
    return 0;
}

void neko_luabind_struct_to_member_offset_type(lua_State *L, neko_luabind_Type type, size_t offset, void *c_out, int index) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs_offset");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_pushinteger(L, offset);
        lua_gettable(L, -2);

        if (!lua_isnil(L, -1)) {
            lua_getfield(L, -1, "type");
            neko_luabind_Type stype = lua_tointeger(L, -1);
            lua_pop(L, 4);
            neko_luabind_to_type(L, stype, (char *)c_out + offset, index);
            return;
        }

        lua_pop(L, 3);
        lua_pushfstring(L, "neko_luabind_struct_to_member: Member offset '%d' not registered for struct '%s'!", offset, neko_luabind_typename(L, type));
        lua_error(L);
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_luabind_struct_to_member: Struct '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
}

void neko_luabind_struct_to_member_name_type(lua_State *L, neko_luabind_Type type, const char *member, void *c_out, int index) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_pushstring(L, member);
        lua_gettable(L, -2);

        if (!lua_isnil(L, -1)) {
            lua_getfield(L, -1, "type");
            neko_luabind_Type stype = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, -1, "offset");
            size_t offset = lua_tointeger(L, -1);
            lua_pop(L, 4);
            neko_luabind_to_type(L, stype, (char *)c_out + offset, index);
            return;
        }

        lua_pop(L, 3);
        lua_pushfstring(L, "neko_luabind_struct_to_member: Member name '%s' not registered for struct '%s'!", member, neko_luabind_typename(L, type));
        lua_error(L);
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_luabind_struct_to_member: Struct '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
}

bool neko_luabind_struct_has_member_offset_type(lua_State *L, neko_luabind_Type type, size_t offset) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs_offset");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_pushinteger(L, offset);
        lua_gettable(L, -2);

        if (!lua_isnil(L, -1)) {
            lua_pop(L, 3);
            return true;
        }

        lua_pop(L, 3);
        return false;
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_luabind_struct_has_member: Struct '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
    return false;
}

bool neko_luabind_struct_has_member_name_type(lua_State *L, neko_luabind_Type type, const char *member) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_pushstring(L, member);
        lua_gettable(L, -2);

        if (!lua_isnil(L, -1)) {
            lua_pop(L, 3);
            return true;
        }

        lua_pop(L, 3);
        return false;
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_luabind_struct_has_member: Struct '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
    return false;
}

neko_luabind_Type neko_luabind_struct_typeof_member_offset_type(lua_State *L, neko_luabind_Type type, size_t offset) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs_offset");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_pushinteger(L, offset);
        lua_gettable(L, -2);

        if (!lua_isnil(L, -1)) {
            lua_getfield(L, -1, "type");
            neko_luabind_Type stype = lua_tointeger(L, -1);
            lua_pop(L, 4);
            return stype;
        }

        lua_pop(L, 3);
        lua_pushfstring(L, "neko_luabind_struct_typeof_member: Member offset '%d' not registered for struct '%s'!", offset, neko_luabind_typename(L, type));
        lua_error(L);
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_luabind_struct_typeof_member: Struct '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
    return 0;
}

neko_luabind_Type neko_luabind_struct_typeof_member_name_type(lua_State *L, neko_luabind_Type type, const char *member) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_pushstring(L, member);
        lua_gettable(L, -2);

        if (!lua_isnil(L, -1)) {
            lua_getfield(L, -1, "type");
            neko_luabind_Type type = lua_tointeger(L, -1);
            lua_pop(L, 4);
            return type;
        }

        lua_pop(L, 3);
        lua_pushfstring(L, "neko_luabind_struct_typeof_member: Member name '%s' not registered for struct '%s'!", member, neko_luabind_typename(L, type));
        lua_error(L);
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_luabind_struct_typeof_member: Struct '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
    return 0;
}

void neko_luabind_struct_type(lua_State *L, neko_luabind_Type type) {
    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs");
    lua_pushinteger(L, type);
    lua_newtable(L);
    lua_settable(L, -3);
    lua_pop(L, 1);

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs_offset");
    lua_pushinteger(L, type);
    lua_newtable(L);
    lua_settable(L, -3);
    lua_pop(L, 1);
}

void neko_luabind_struct_member_type(lua_State *L, neko_luabind_Type type, const char *member, neko_luabind_Type mtype, size_t offset) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_newtable(L);

        lua_pushinteger(L, mtype);
        lua_setfield(L, -2, "type");
        lua_pushinteger(L, offset);
        lua_setfield(L, -2, "offset");
        lua_pushstring(L, member);
        lua_setfield(L, -2, "name");

        lua_setfield(L, -2, member);

        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs_offset");
        lua_pushinteger(L, type);
        lua_gettable(L, -2);

        lua_pushinteger(L, offset);
        lua_getfield(L, -4, member);
        lua_settable(L, -3);
        lua_pop(L, 4);
        return;
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_luabind_struct_member: Struct '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
}

bool neko_luabind_struct_registered_type(lua_State *L, neko_luabind_Type type) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    bool reg = !lua_isnil(L, -1);
    lua_pop(L, 2);

    return reg;
}

int neko_luabind_struct_push_type(lua_State *L, neko_luabind_Type type, const void *c_in) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {
        lua_remove(L, -2);
        lua_newtable(L);

        lua_pushnil(L);
        while (lua_next(L, -3)) {

            if (lua_type(L, -2) == LUA_TSTRING) {
                lua_getfield(L, -1, "name");
                const char *name = lua_tostring(L, -1);
                lua_pop(L, 1);
                int num = neko_luabind_struct_push_member_name_type(L, type, name, c_in);
                if (num > 1) {
                    lua_pop(L, 5);
                    lua_pushfstring(L,
                                    "neko_luabind_struct_push: Conversion pushed %d values to stack,"
                                    " don't know how to include in struct!",
                                    num);
                    lua_error(L);
                }
                lua_remove(L, -2);
                lua_pushvalue(L, -2);
                lua_insert(L, -2);
                lua_settable(L, -4);
            } else {
                lua_pop(L, 1);
            }
        }

        lua_remove(L, -2);
        return 1;
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "lua_struct_push: Struct '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
    return 0;
}

void neko_luabind_struct_to_type(lua_State *L, neko_luabind_Type type, void *c_out, int index) {

    lua_pushnil(L);
    while (lua_next(L, index - 1)) {

        if (lua_type(L, -2) == LUA_TSTRING) {
            neko_luabind_struct_to_member_name_type(L, type, lua_tostring(L, -2), c_out, -1);
        }

        lua_pop(L, 1);
    }
}

const char *neko_luabind_struct_next_member_name_type(lua_State *L, neko_luabind_Type type, const char *member) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        if (!member) {
            lua_pushnil(L);
        } else {
            lua_pushstring(L, member);
        }
        if (!lua_next(L, -2)) {
            lua_pop(L, 2);
            return LUAA_INVALID_MEMBER_NAME;
        }
        const char *result = lua_tostring(L, -2);
        lua_pop(L, 4);
        return result;
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_luabind_struct_next_member: Struct '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
    return NULL;
}

/*
** Enums
*/

int neko_luabind_enum_push_type(lua_State *L, neko_luabind_Type type, const void *value) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_values");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_sizes");
        lua_pushinteger(L, type);
        lua_gettable(L, -2);
        size_t size = lua_tointeger(L, -1);
        lua_pop(L, 2);

        lua_Integer lvalue = 0;
        memcpy(&lvalue, value, size);

        lua_pushinteger(L, lvalue);
        lua_gettable(L, -2);

        if (!lua_isnil(L, -1)) {
            lua_getfield(L, -1, "name");
            lua_remove(L, -2);
            lua_remove(L, -2);
            lua_remove(L, -2);
            return 1;
        }

        lua_pop(L, 3);
        lua_pushfstring(L, "neko_luabind_enum_push: Enum '%s' value %d not registered!", neko_luabind_typename(L, type), lvalue);
        lua_error(L);
        return 0;
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_luabind_enum_push: Enum '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
    return 0;
}

void neko_luabind_enum_to_type(lua_State *L, neko_luabind_Type type, void *c_out, int index) {

    const char *name = lua_tostring(L, index);

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_sizes");
        lua_pushinteger(L, type);
        lua_gettable(L, -2);
        size_t size = lua_tointeger(L, -1);
        lua_pop(L, 2);

        lua_pushstring(L, name);
        lua_gettable(L, -2);

        if (!lua_isnil(L, -1)) {
            lua_getfield(L, -1, "value");
            lua_Integer value = lua_tointeger(L, -1);
            lua_pop(L, 4);
            memcpy(c_out, &value, size);
            return;
        }

        lua_pop(L, 3);
        lua_pushfstring(L, "neko_luabind_enum_to: Enum '%s' field '%s' not registered!", neko_luabind_typename(L, type), name);
        lua_error(L);
        return;
    }

    lua_pop(L, 3);
    lua_pushfstring(L, "neko_luabind_enum_to: Enum '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
    return;
}

bool neko_luabind_enum_has_value_type(lua_State *L, neko_luabind_Type type, const void *value) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_values");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_sizes");
        lua_pushinteger(L, type);
        lua_gettable(L, -2);
        size_t size = lua_tointeger(L, -1);
        lua_pop(L, 2);

        lua_Integer lvalue = 0;
        memcpy(&lvalue, value, size);

        lua_pushinteger(L, lvalue);
        lua_gettable(L, -2);

        if (lua_isnil(L, -1)) {
            lua_pop(L, 3);
            return false;
        } else {
            lua_pop(L, 3);
            return true;
        }
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_luabind_enum_has_value: Enum '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
    return false;
}

bool neko_luabind_enum_has_name_type(lua_State *L, neko_luabind_Type type, const char *name) {
    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_getfield(L, -1, name);

        if (lua_isnil(L, -1)) {
            lua_pop(L, 3);
            return false;
        } else {
            lua_pop(L, 3);
            return true;
        }
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_luabind_enum_has_name: Enum '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
    return false;
}

void neko_luabind_enum_type(lua_State *L, neko_luabind_Type type, size_t size) {
    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums");
    lua_pushinteger(L, type);
    lua_newtable(L);
    lua_settable(L, -3);
    lua_pop(L, 1);

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_values");
    lua_pushinteger(L, type);
    lua_newtable(L);
    lua_settable(L, -3);
    lua_pop(L, 1);

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_sizes");
    lua_pushinteger(L, type);
    lua_pushinteger(L, size);
    lua_settable(L, -3);
    lua_pop(L, 1);
}

void neko_luabind_enum_value_type(lua_State *L, neko_luabind_Type type, const void *value, const char *name) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_sizes");
        lua_pushinteger(L, type);
        lua_gettable(L, -2);
        size_t size = lua_tointeger(L, -1);
        lua_pop(L, 2);

        lua_newtable(L);

        lua_Integer lvalue = 0;
        memcpy(&lvalue, value, size);

        lua_pushinteger(L, lvalue);
        lua_setfield(L, -2, "value");

        lua_pushstring(L, name);
        lua_setfield(L, -2, "name");

        lua_setfield(L, -2, name);

        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_values");
        lua_pushinteger(L, type);
        lua_gettable(L, -2);
        lua_pushinteger(L, lvalue);
        lua_getfield(L, -4, name);
        lua_settable(L, -3);

        lua_pop(L, 4);
        return;
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_luabind_enum_value: Enum '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
}

bool neko_luabind_enum_registered_type(lua_State *L, neko_luabind_Type type) {
    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);
    bool reg = !lua_isnil(L, -1);
    lua_pop(L, 2);
    return reg;
}

const char *neko_luabind_enum_next_value_name_type(lua_State *L, neko_luabind_Type type, const char *member) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        if (!member) {
            lua_pushnil(L);
        } else {
            lua_pushstring(L, member);
        }
        if (!lua_next(L, -2)) {
            lua_pop(L, 2);
            return LUAA_INVALID_MEMBER_NAME;
        }
        const char *result = lua_tostring(L, -2);
        lua_pop(L, 4);
        return result;
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_luabind_enum_next_enum_name_type: Enum '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
    return NULL;
}

/*
** Functions
*/

static int neko_luabind_call_entry(lua_State *L) {

    /* Get return size */

    lua_getfield(L, -1, "ret_type");
    neko_luabind_Type ret_type = lua_tointeger(L, -1);
    lua_pop(L, 1);

    size_t ret_size = neko_luabind_typesize(L, ret_type);

    /* Get total arguments sizes */

    lua_getfield(L, -1, "arg_types");

    size_t arg_size = 0;
    size_t arg_num = lua_rawlen(L, -1);

    if (lua_gettop(L) < arg_num + 2) {
        lua_pop(L, 1);
        lua_pushfstring(L, "neko_luabind_call: Too few arguments to function!");
        lua_error(L);
        return 0;
    }

    for (int i = 0; i < arg_num; i++) {
        lua_pushinteger(L, i + 1);
        lua_gettable(L, -2);
        neko_luabind_Type arg_type = lua_tointeger(L, -1);
        lua_pop(L, 1);
        arg_size += neko_luabind_typesize(L, arg_type);
    }

    lua_pop(L, 1);

    /* Test to see if using heap */

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "call_ret_stk");
    void *ret_stack = lua_touserdata(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "call_arg_stk");
    void *arg_stack = lua_touserdata(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "call_ret_ptr");
    lua_Integer ret_ptr = lua_tointeger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "call_arg_ptr");
    lua_Integer arg_ptr = lua_tointeger(L, -1);
    lua_pop(L, 1);

    void *ret_data = (char *)ret_stack + ret_ptr;
    void *arg_data = (char *)arg_stack + arg_ptr;

    /* If fixed allocation exhausted use heap instead */

    bool ret_heap = false;
    bool arg_heap = false;

    if (ret_ptr + ret_size > LUAA_RETURN_STACK_SIZE) {
        ret_heap = true;
        ret_data = mem_alloc(ret_size);
        if (ret_data == NULL) {
            lua_pushfstring(L, "neko_luabind_call: Out of memory!");
            lua_error(L);
            return 0;
        }
    }

    if (arg_ptr + arg_size > LUAA_ARGUMENT_STACK_SIZE) {
        arg_heap = true;
        arg_data = mem_alloc(arg_size);
        if (arg_data == NULL) {
            if (ret_heap) {
                mem_free(ret_data);
            }
            lua_pushfstring(L, "neko_luabind_call: Out of memory!");
            lua_error(L);
            return 0;
        }
    }

    /* If not using heap update stack pointers */

    if (!ret_heap) {
        lua_pushinteger(L, ret_ptr + ret_size);
        lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "call_ret_ptr");
    }

    if (!arg_heap) {
        lua_pushinteger(L, arg_ptr + arg_size);
        lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "call_arg_ptr");
    }

    /* Pop args and place in memory */

    lua_getfield(L, -1, "arg_types");

    void *arg_pos = arg_data;
    for (int i = 0; i < arg_num; i++) {
        lua_pushinteger(L, i + 1);
        lua_gettable(L, -2);
        neko_luabind_Type arg_type = lua_tointeger(L, -1);
        lua_pop(L, 1);
        neko_luabind_to_type(L, arg_type, arg_pos, arg_num + i - 2);
        arg_pos = (char *)arg_pos + neko_luabind_typesize(L, arg_type);
    }

    lua_pop(L, 1);

    /* Pop arguments from stack */

    for (int i = 0; i < arg_num; i++) {
        lua_remove(L, -2);
    }

    /* Get Function Pointer and Call */

    lua_getfield(L, -1, "auto_func");
    neko_luabind_Func auto_func = (neko_luabind_Func)lua_touserdata(L, -1);
    lua_pop(L, 2);

    auto_func(ret_data, arg_data);

    int count = neko_luabind_push_type(L, ret_type, ret_data);

    /* Either free heap data or reduce stack pointers */

    if (!ret_heap) {
        lua_pushinteger(L, ret_ptr);
        lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "call_ret_ptr");
    } else {
        mem_free(ret_data);
    }

    if (!arg_heap) {
        lua_pushinteger(L, arg_ptr);
        lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "argument_ptr");
    } else {
        mem_free(arg_data);
    }

    return count;
}

int neko_luabind_call(lua_State *L, void *func_ptr) {
    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "functions");
    lua_pushlightuserdata(L, func_ptr);
    lua_gettable(L, -2);
    lua_remove(L, -2);

    if (!lua_isnil(L, -1)) {
        return neko_luabind_call_entry(L);
    }

    lua_pop(L, 1);
    lua_pushfstring(L, "neko_luabind_call: Function with address '%p' is not registered!", func_ptr);
    lua_error(L);
    return 0;
}

int neko_luabind_call_name(lua_State *L, const char *func_name) {
    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "functions");
    lua_pushstring(L, func_name);
    lua_gettable(L, -2);
    lua_remove(L, -2);

    if (!lua_isnil(L, -1)) {
        return neko_luabind_call_entry(L);
    }

    lua_pop(L, 1);
    lua_pushfstring(L, "neko_luabind_call_name: Function '%s' is not registered!", func_name);
    lua_error(L);
    return 0;
}

void neko_luabind_function_register_type(lua_State *L, void *src_func, neko_luabind_Func auto_func, const char *name, neko_luabind_Type ret_t, int num_args, ...) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "functions");
    lua_pushstring(L, name);

    lua_newtable(L);

    lua_pushlightuserdata(L, src_func);
    lua_setfield(L, -2, "src_func");
    lua_pushlightuserdata(L, (void *)auto_func);
    lua_setfield(L, -2, "auto_func");

    lua_pushinteger(L, ret_t);
    lua_setfield(L, -2, "ret_type");

    lua_pushstring(L, "arg_types");
    lua_newtable(L);

    va_list va;
    va_start(va, num_args);
    for (int i = 0; i < num_args; i++) {
        lua_pushinteger(L, i + 1);
        lua_pushinteger(L, va_arg(va, neko_luabind_Type));
        lua_settable(L, -3);
    }
    va_end(va);

    lua_settable(L, -3);
    lua_settable(L, -3);
    lua_pop(L, 1);

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "functions");
    lua_pushlightuserdata(L, src_func);

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "functions");
    lua_getfield(L, -1, name);
    lua_remove(L, -2);

    lua_settable(L, -3);
    lua_pop(L, 1);
}

bool neko_lua_equal(lua_State *state, int index1, int index2) {
#if LUA_VERSION_NUM <= 501
    return lua_equal(state, index1, index2) == 1;
#else
    return lua_compare(state, index1, index2, LUA_OPEQ) == 1;
#endif
}

int neko_lua_preload(lua_State *L, lua_CFunction f, const char *name) {
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_pushcfunction(L, f);
    lua_setfield(L, -2, name);
    lua_pop(L, 2);
    return 0;
}

int neko_lua_preload_auto(lua_State *L, lua_CFunction f, const char *name) {
    neko_lua_preload(L, f, name);
    lua_getglobal(L, "require");
    lua_pushstring(L, name);
    lua_call(L, 1, 0);
    return 0;
}

void neko_lua_load(lua_State *L, const luaL_Reg *l, const char *name) {
    lua_getglobal(L, name);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
    }
    luaL_setfuncs(L, l, 0);
    lua_setglobal(L, name);
}

void neko_lua_loadover(lua_State *L, const luaL_Reg *l, const char *name) {
    lua_newtable(L);
    luaL_setfuncs(L, l, 0);
    lua_setglobal(L, name);
}

int neko_lua_get_table_pairs_count(lua_State *L, int index) {
    int count = 0;
    index = lua_absindex(L, index);
    lua_pushnil(L);
    while (lua_next(L, index) != 0) {
        // 现在栈顶是value，下一个栈元素是key
        count++;
        lua_pop(L, 1);  // 移除value，保留key作为下一次迭代的key
    }
    return count;
}

static int g_reference_table = LUA_NOREF;

// 从堆栈顶部弹出一个结构体实例
// 将结构体实例的引用表压入堆栈
static void affirmReferenceTable(lua_State *L) {
    int instanceIndex = lua_gettop(L);

    if (g_reference_table == LUA_NOREF) {
        // 创建全局参考表
        lua_newtable(L);

        // 创建元表
        lua_newtable(L);

        // 使用弱键 以便实例引用表自动清理
        lua_pushstring(L, "k");
        lua_setfield(L, -2, "__mode");

        // 在全局引用表上设置元表
        lua_setmetatable(L, -2);

        // 存储对全局引用表的引用
        g_reference_table = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    // 获取全局引用表
    lua_rawgeti(L, LUA_REGISTRYINDEX, g_reference_table);

    int globalIndex = lua_gettop(L);

    // 获取实例引用表
    lua_pushvalue(L, instanceIndex);
    lua_gettable(L, globalIndex);

    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);

        // 创建实例引用表
        lua_newtable(L);

        // 添加到全局参考表
        lua_pushvalue(L, instanceIndex);
        lua_pushvalue(L, -2);
        lua_settable(L, globalIndex);
    }

    // 将实例引用表移动到位并整理
    lua_replace(L, instanceIndex);
    lua_settop(L, instanceIndex);
}

static void LUASTRUCT_setmetatable(lua_State *L, const char *metatable, int index) {
    luaL_getmetatable(L, metatable);

    if (lua_isnoneornil(L, -1)) {
        luaL_error(L, "The metatable for %s has not been defined", metatable);
    }

    lua_setmetatable(L, index - 1);
}

int LUASTRUCT_new(lua_State *L, const char *metatable, size_t size) {
    int *reference = (int *)lua_newuserdata(L, sizeof(int) + size);

    *reference = LUA_NOREF;

    void *data = (void *)(reference + 1);

    memset(data, 0, size);

    LUASTRUCT_setmetatable(L, metatable, -1);

    return 1;
}

// ParentIndex 是包含对象的堆栈索引 或者 0 表示不包含对象
int LUASTRUCT_newref(lua_State *L, const char *metatable, int parentIndex, const void *data) {
    int *reference = (int *)lua_newuserdata(L, sizeof(int) + sizeof(data));

    if (parentIndex != 0) {
        // 存储对包含对象的引用
        lua_pushvalue(L, parentIndex);
        *reference = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
        *reference = LUA_REFNIL;
    }

    *((const void **)(reference + 1)) = data;

    LUASTRUCT_setmetatable(L, metatable, -1);

    return 1;
}

static int LUASTRUCT_gc(lua_State *L, const char *metatable) {
    int *reference = (int *)luaL_checkudata(L, 1, metatable);
    luaL_unref(L, LUA_REGISTRYINDEX, *reference);

    return 0;
}

int LUASTRUCT_is(lua_State *L, const char *metatable, int index) {
    if (lua_type(L, index) != LUA_TUSERDATA) {
        return 0;
    }
    lua_getmetatable(L, index);
    luaL_getmetatable(L, metatable);

    int metatablesMatch = lua_rawequal(L, -1, -2);

    lua_pop(L, 2);

    return metatablesMatch;
}

void *LUASTRUCT_todata(lua_State *L, const char *metatable, int index, int required) {
    if (required == LUASTRUCT_OPTIONAL && lua_isnoneornil(L, index)) {
        return NULL;
    }

    int *reference = (int *)luaL_checkudata(L, index, metatable);

    if (*reference == LUA_NOREF) {
        return reference + 1;
    } else {
        return *((void **)(reference + 1));
    }
}

static void LUASTRUCT_create(lua_State *L, const char *fieldName, const char *metatable, lua_CFunction _new, lua_CFunction gc, lua_CFunction index, lua_CFunction newindex) {
    if (fieldName) {
        lua_createtable(L, 0, 0);

        lua_pushcfunction(L, _new);
        lua_setfield(L, -2, "new");

        lua_setfield(L, -2, fieldName);
    }

    // 创建实例元表
    luaL_newmetatable(L, metatable);

    lua_pushcfunction(L, gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, index);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, newindex);
    lua_setfield(L, -2, "__newindex");

    lua_pop(L, 1);
}

static const char *LUASTRUCT_fieldname(lua_State *L, int index, size_t *length) {
    luaL_argcheck(L, lua_type(L, index) == LUA_TSTRING, index, "Field name must be a string");

    return lua_tolstring(L, index, length);
}

static int LUASTRUCT_access_float(lua_State *L, const char *fieldName, float *data, int parentIndex, int set, int valueIndex) {
    if (set) {
        *data = (float)luaL_checknumber(L, valueIndex);
        return 0;
    } else {
        lua_pushnumber(L, *data);
        return 1;
    }
}

static int LUASTRUCT_access_int(lua_State *L, const char *fieldName, int *data, int parentIndex, int set, int valueIndex) {
    if (set) {
        *data = luaL_checkinteger(L, valueIndex);
        return 0;
    } else {
        lua_pushinteger(L, *data);
        return 1;
    }
}

static int LUASTRUCT_access_uint(lua_State *L, const char *fieldName, unsigned int *data, int parentIndex, int set, int valueIndex) {
    if (set) {
        *data = luaL_checkinteger(L, valueIndex);
        return 0;
    } else {
        lua_pushinteger(L, *data);
        return 1;
    }
}

static int LUASTRUCT_access_ushort(lua_State *L, const char *fieldName, unsigned short *data, int parentIndex, int set, int valueIndex) {
    if (set) {
        *data = luaL_checkinteger(L, valueIndex);
        return 0;
    } else {
        lua_pushinteger(L, *data);
        return 1;
    }
}

static int LUASTRUCT_access_uchar(lua_State *L, const char *fieldName, unsigned char *data, int parentIndex, int set, int valueIndex) {
    if (set) {
        *data = luaL_checkinteger(L, valueIndex);
        return 0;
    } else {
        lua_pushinteger(L, *data);
        return 1;
    }
}

static int LUASTRUCT_access_boolean(lua_State *L, const char *fieldName, bool *data, int parentIndex, int set, int valueIndex) {
    if (set) {
        *data = lua_toboolean(L, valueIndex);
        return 0;
    } else {
        lua_pushboolean(L, *data);
        return 1;
    }
}

// 从堆栈顶部弹出 struct userdata
// 将根结构体 userdata 压入堆栈
static int getRootStruct(lua_State *L) {
    int *reference = (int *)lua_touserdata(L, -1);

    if (!reference) {
        return luaL_error(L, "expected struct userdata at the top of the stack");
    }

    if (*reference == LUA_NOREF || *reference == LUA_REFNIL) {
        return 1;
    } else {
        lua_pop(L, 1);
        lua_rawgeti(L, LUA_REGISTRYINDEX, *reference);
        return getRootStruct(L);
    }
}

static int LUASTRUCT_access_cstring(lua_State *L, const char *fieldName, const char **data, int parentIndex, int set, int valueIndex) {
    if (set) {
        lua_pushvalue(L, valueIndex);
        *data = lua_tostring(L, -1);

        int copyIndex = lua_gettop(L);

        // 保留对 Lua 字符串的引用以防止垃圾回收
        lua_pushvalue(L, parentIndex);
        getRootStruct(L);
        affirmReferenceTable(L);

        if (*data) {
            lua_pushvalue(L, copyIndex);
        } else {
            lua_pushnil(L);
        }

        lua_setfield(L, -2, fieldName);

        return 0;
    } else {
        lua_pushstring(L, *data);
        return 1;
    }
}

int createHandle(lua_State *L, const char *metatable, void *value) {
    *((void **)lua_newuserdata(L, sizeof(value))) = value;
    luaL_getmetatable(L, metatable);
    lua_setmetatable(L, -2);

    return 0;
}

static int LUASTRUCT_access_handle(lua_State *L, void **data, const char *metatable, int parentIndex, int set, int valueIndex) {
    if (set) {
        *data = *((void **)luaL_checkudata(L, valueIndex, metatable));
        return 0;
    } else {
        createHandle(L, metatable, *data);
        return 1;
    }
}

typedef unsigned char(uchar8)[8];

static int ARRAY_uchar8_newref(lua_State *L, int parentIndex, uchar8 *data) { return LUASTRUCT_newref(L, "ARRAY_uchar8", parentIndex, data); }

static int ARRAY_uchar8_gc(lua_State *L) { return LUASTRUCT_gc(L, "ARRAY_uchar8"); }

static uchar8 *ARRAY_uchar8_todata(lua_State *L, int index, int required) { return (uchar8 *)LUASTRUCT_todata(L, "ARRAY_uchar8", index, required); }

static int ARRAY_uchar8_elementaccess(lua_State *L, int index, int set);

static int ARRAY_uchar8_index(lua_State *L) { return ARRAY_uchar8_elementaccess(L, 1, 0); }

static int ARRAY_uchar8_newindex(lua_State *L) { return ARRAY_uchar8_elementaccess(L, 1, 1); }

static void ARRAY_uchar8_create(lua_State *L) { LUASTRUCT_create(L, NULL, "ARRAY_uchar8", NULL, ARRAY_uchar8_gc, ARRAY_uchar8_index, ARRAY_uchar8_newindex); }

static int ARRAY_uchar8_elementaccess(lua_State *L, int index, int set) {
    uchar8 *data = ARRAY_uchar8_todata(L, index, LUASTRUCT_REQUIRED);

    int elementIndex = luaL_checkinteger(L, index + 1);

    if (0 <= elementIndex && elementIndex < 8) {
        return LUASTRUCT_access_uchar(L, NULL, (*data) + elementIndex, index, set, index + 2);
    }

    return luaL_error(L, "Invalid index %d", elementIndex);
}

static int LUASTRUCT_access_ARRAY_uchar8(lua_State *L, const char *fieldName, uchar8 *data, int parentIndex, int set, int valueIndex) {
    if (set) {
        return luaL_error(L, "Attempt to set a read-only field");
    } else {
        return ARRAY_uchar8_newref(L, parentIndex, data);
    }
}

#define LUASTRUCT_BEGIN(type)                                        \
    LUASTRUCT_NEW(type)                                              \
    LUASTRUCT_NEWREF(type)                                           \
    LUASTRUCT_GC(type)                                               \
    static int type##_fieldaccess(lua_State *L, int index, int set); \
    LUASTRUCT_INDEX(type)                                            \
    LUASTRUCT_NEWINDEX(type)                                         \
    LUASTRUCT_ACCESS(type)                                           \
    LUASTRUCT_CREATE(type)                                           \
    LUASTRUCT_FIELDACCESS_BEGIN(type)

#define LUASTRUCT_END LUASTRUCT_FIELDACCESS_END

#define LUASTRUCT_NEW(type) \
    int type##_new(lua_State *L) { return LUASTRUCT_new(L, #type, sizeof(type)); }

#define LUASTRUCT_NEWREF(type) \
    int type##_newref(lua_State *L, int parentIndex, type *data) { return LUASTRUCT_newref(L, #type, parentIndex, data); }

#define LUASTRUCT_GC(type) \
    static int type##_gc(lua_State *L) { return LUASTRUCT_gc(L, #type); }

#define LUASTRUCT_INDEX(type) \
    static int type##_index(lua_State *L) { return type##_fieldaccess(L, 1, 0); }

#define LUASTRUCT_NEWINDEX(type) \
    static int type##_newindex(lua_State *L) { return type##_fieldaccess(L, 1, 1); }

#define LUASTRUCT_ACCESS(type)                                                                                                      \
    static int LUASTRUCT_access_##type(lua_State *L, const char *fieldName, type *data, int parentIndex, int set, int valueIndex) { \
        if (set) {                                                                                                                  \
            *data = *CHECK_STRUCT(L, valueIndex, type);                                                                             \
            return 0;                                                                                                               \
        } else {                                                                                                                    \
            return type##_newref(L, parentIndex, data);                                                                             \
        }                                                                                                                           \
    }

#define LUASTRUCT_CREATE(type) \
    static void type##_create(lua_State *L, const char *fieldName) { LUASTRUCT_create(L, fieldName, #type, type##_new, type##_gc, type##_index, type##_newindex); }

#define LUASTRUCT_FIELDACCESS_BEGIN(type)                             \
    static int type##_fieldaccess(lua_State *L, int index, int set) { \
        static const char *typeName = #type;                          \
        type *data = CHECK_STRUCT(L, index, type);                    \
        size_t length = 0;                                            \
        const char *field = LUASTRUCT_fieldname(L, index + 1, &length);

#define LUASTRUCT_FIELD(name, type)                                                   \
    if (strncmp(#name, field, length) == 0) {                                         \
        return LUASTRUCT_access_##type(L, #name, &data->name, index, set, index + 2); \
    }

#define LUASTRUCT_FIELD_HANDLE(name, type)                                            \
    if (strncmp(#name, field, length) == 0) {                                         \
        return LUASTRUCT_access_handle(L, &data->name, #type, index, set, index + 2); \
    }

#define LUASTRUCT_FIELD_CONSTANT(name, type)                           \
    if (strncmp(#name, field, length) == 0) {                          \
        CONSTANT_ACCESS_DECLARE(type);                                 \
        return CONSTANT_access_##type(L, &data->name, set, index + 2); \
    }

#define LUASTRUCT_FIELD_CUSTOM(name, ...)     \
    if (strncmp(#name, field, length) == 0) { \
        __VA_ARGS__                           \
    }

#define LUASTRUCT_FIELDACCESS_END                                 \
    return luaL_error(L, "Invalid field %s.%s", typeName, field); \
    }

// 这里定义 LUASTRUCT 结构

LUASTRUCT_BEGIN(Vector4)
LUASTRUCT_FIELD(x, float)
LUASTRUCT_FIELD(y, float)
LUASTRUCT_FIELD(z, float)
LUASTRUCT_FIELD(w, float)
LUASTRUCT_END

void createStructTables(lua_State *L) {
    Vector4_create(L, "Vector4");

    ARRAY_uchar8_create(L);
}

namespace neko {

static size_t lua_mem_usage;

size_t neko_lua_mem_usage() { return lua_mem_usage; }

void *Allocf(void *ud, void *ptr, size_t osize, size_t nsize) {
    if (!ptr) osize = 0;
    if (!nsize) {
        lua_mem_usage -= osize;
        mem_free(ptr);
        return NULL;
    }
    lua_mem_usage += (nsize - osize);
    return mem_realloc(ptr, nsize);
}

void neko_lua_run_string(lua_State *m_ls, const_str str_) {
    if (luaL_dostring(m_ls, str_)) {
        std::string err = neko_lua_tool_t::dump_error(m_ls, "run_string ::lua_pcall_wrap failed str<%s>", str_);
        ::lua_pop(m_ls, 1);
        // NEKO_ERROR("%s", err.c_str());
    }
}

namespace {
std::string default_str("");
}

ScriptReference::ScriptReference() : script_ref(0) {}

ScriptReference::~ScriptReference() { NEKO_ASSERT(script_ref == 0, "Warning, you have deleted an instance without unregistering it"); }

lua_table::lua_table() : __lua(NULL), script_ref(0) {}

lua_table::lua_table(lua_State *L, int script_ref_) : __lua(L), script_ref(script_ref_) {}

lua_table::lua_table(const lua_table &other) : __lua(other.__lua), script_ref(0) {
    if (other.script_ref != 0) {
        lua_rawgeti(__lua, LUA_REGISTRYINDEX, other.script_ref);
        script_ref = luaL_ref(__lua, LUA_REGISTRYINDEX);
    }
}

lua_table::~lua_table() {
    if (script_ref != 0) {
        luaL_unref(__lua, LUA_REGISTRYINDEX, script_ref);
        script_ref = 0;
    }
}

std::string lua_table::getString(const_str key) const {
    if (script_ref == 0 || key == NULL) return default_str;

    lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
    lua_pushstring(__lua, key);
    lua_gettable(__lua, -2);

    if (lua_isstring(__lua, -1)) {
        const_str str = lua_tostring(__lua, -1);
        std::string result(str);
        lua_pop(__lua, 2);
        return result;
    }

    lua_pop(__lua, 2);
    return default_str;
}

double lua_table::getDouble(const_str key) const {
    if (script_ref == 0 || key == NULL) return 0.0;

    lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
    lua_pushstring(__lua, key);
    lua_gettable(__lua, -2);

    double result = 0.0;

    if (lua_isnumber(__lua, -1)) result = lua_tonumber(__lua, -1);

    lua_pop(__lua, 2);
    return result;
}

float lua_table::getFloat(const_str key) const {
    if (script_ref == 0 || key == NULL) return 0.0f;

    lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
    lua_pushstring(__lua, key);
    lua_gettable(__lua, -2);

    float result = 0.0f;

    if (lua_isnumber(__lua, -1)) result = (float)lua_tonumber(__lua, -1);

    lua_pop(__lua, 2);
    return result;
}

bool lua_table::getBool(const_str key) const {
    if (script_ref == 0 || key == NULL) return 0.0f;

    lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
    lua_pushstring(__lua, key);
    lua_gettable(__lua, -2);

    bool result = false;

    if (lua_isboolean(__lua, -1)) result = lua_toboolean(__lua, -1) != 0;

    lua_pop(__lua, 2);
    return result;
}

int lua_table::getInt(const_str key) const {
    if (script_ref == 0 || key == NULL) return 0;

    lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
    lua_pushstring(__lua, key);
    lua_gettable(__lua, -2);

    int result = 0;

    if (lua_isnumber(__lua, -1)) result = lua_tointeger(__lua, -1);

    lua_pop(__lua, 2);
    return result;
}

ScriptObject *lua_table::getPointer(const_str key) const {
    if (script_ref == 0 || key == NULL) return NULL;

    lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
    lua_pushstring(__lua, key);
    lua_gettable(__lua, -2);

    ScriptObject *result = NULL;
    lua_value<ScriptObject *>::pop(__lua, result);
    lua_pop(__lua, 1);
    return result;
}

lua_table lua_table::get_table(const_str key) const {
    if (script_ref == 0 || key == NULL) return lua_table();

    lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
    lua_pushstring(__lua, key);
    lua_gettable(__lua, -2);

    if (lua_istable(__lua, -1)) {
        lua_table result(__lua, luaL_ref(__lua, LUA_REGISTRYINDEX));
        lua_pop(__lua, 1);
        return result;
    }

    lua_pop(__lua, 2);
    return lua_table(__lua, 0);
}

lua_table &lua_table::operator=(const lua_table &other) {
    if (other.script_ref != 0) {
        __lua = other.__lua;
        lua_rawgeti(__lua, LUA_REGISTRYINDEX, other.script_ref);
        script_ref = luaL_ref(__lua, LUA_REGISTRYINDEX);
    }

    return *this;
}

lua_table_iter lua_table::getIterator() const { return lua_table_iter(__lua, script_ref); }

lua_table_iter::lua_table_iter() : script_ref(0), __lua(NULL), mNumPopsRequired(0) {}

lua_table_iter::lua_table_iter(lua_State *L, int script_ref_) : script_ref(script_ref_), __lua(L), mNumPopsRequired(0) {
    if (script_ref > 0) {
        lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
        lua_pushnil(__lua);
        mNumPopsRequired = 2;
    }
}

lua_table_iter::lua_table_iter(const lua_table_iter &it) : script_ref(it.script_ref), __lua(it.__lua), mNumPopsRequired(0) {
    if (script_ref > 0) {
        lua_rawgeti(__lua, LUA_REGISTRYINDEX, script_ref);
        lua_pushnil(__lua);
        mNumPopsRequired = 2;
    }
}

lua_table_iter::~lua_table_iter() {
    if (script_ref > 0) {
        lua_pop(__lua, mNumPopsRequired);
        script_ref = 0;
    }
}

bool lua_table_iter::hasNext() {
    if (script_ref <= 0) return false;

    if (mNumPopsRequired > 2) lua_pop(__lua, 1);

    bool next = lua_next(__lua, -2) != 0;
    if (next) mNumPopsRequired++;

    return next;
}

std::string lua_table_iter::getKey() const {
    NEKO_ASSERT(script_ref > 0, "This iterator instance is corrupter. Are you sure that hasNext() returned true?");
    std::string key = lua_tostring(__lua, -2);
    return key;
}

std::string lua_table_iter::getString() {
    NEKO_ASSERT(script_ref > 0, "This iterator instance is corrupter. Are you sure that hasNext() returned true?");
    std::string val = lua_tostring(__lua, -1);
    lua_pop(__lua, 1);
    mNumPopsRequired--;
    return val;
}

double lua_table_iter::getDouble() {
    NEKO_ASSERT(script_ref > 0, "This iterator instance is corrupter. Are you sure that hasNext() returned true?");
    double val = lua_tonumber(__lua, -1);
    lua_pop(__lua, 1);
    mNumPopsRequired--;
    return val;
}

float lua_table_iter::getFloat() {
    NEKO_ASSERT(script_ref > 0, "This iterator instance is corrupter. Are you sure that hasNext() returned true?");
    float val = (float)lua_tonumber(__lua, -1);
    lua_pop(__lua, 1);
    mNumPopsRequired--;
    return val;
}

int lua_table_iter::getInt() {
    NEKO_ASSERT(script_ref > 0, "This iterator instance is corrupter. Are you sure that hasNext() returned true?");
    int val = lua_tointeger(__lua, -1);
    lua_pop(__lua, 1);
    mNumPopsRequired--;
    return val;
}

bool lua_table_iter::getBool() {
    NEKO_ASSERT(script_ref > 0, "This iterator instance is corrupter. Are you sure that hasNext() returned true?");
    bool val = lua_toboolean(__lua, -1) != 0;
    lua_pop(__lua, 1);
    mNumPopsRequired--;
    return val;
}

ScriptObject *lua_table_iter::getPointer() {
    NEKO_ASSERT(script_ref > 0, "This iterator instance is corrupter. Are you sure that hasNext() returned true?");
    ScriptObject *ptr = NULL;
    lua_value<ScriptObject *>::pop(__lua, ptr);
    mNumPopsRequired--;
    return ptr;
}

lua_table lua_table_iter::get_table() {
    NEKO_ASSERT(script_ref > 0, "This iterator instance is corrupter. Are you sure that hasNext() returned true?");
    lua_table dict;
    lua_value<lua_table>::pop(__lua, dict);
    mNumPopsRequired--;
    return dict;
}

lua_State *ScriptReference::__lua() const {
    lua_State *L = ENGINE_LUA();
    NEKO_ASSERT(L);
    return L;
}

ScriptInvoker::ScriptInvoker() : ScriptReference() {}

ScriptInvoker::~ScriptInvoker() {}

void ScriptInvoker::invoke(const_str method) {
    lua_State *L = ENGINE_LUA();
    NEKO_ASSERT(L);

#ifdef _DEBUG
    int top1 = lua_gettop(L);
#endif
    if (!findAndPushMethod(method)) return;

    lua_rawgeti(L, LUA_REGISTRYINDEX, script_ref);
    if (lua_pcall(L, 1, 0, NULL) != 0) {
        NEKO_WARN("[lua] err: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }

#ifdef _DEBUG
    int currentStack = lua_gettop(L);
    NEKO_ASSERT(top1 == currentStack, "The stack after the method call is corrupt");
#endif
}

bool ScriptInvoker::findAndPushMethod(const_str method_name) {
    lua_State *L = ENGINE_LUA();
    NEKO_ASSERT(L);

    if (method_name == NULL) return false;

    NEKO_ASSERT(script_ref != 0, "You must register this instance using 'registerObject' before you can invoke any script methods on it");

    lua_rawgeti(L, LUA_REGISTRYINDEX, script_ref);
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, method_name);
        if (lua_isfunction(L, -1)) {
            lua_remove(L, -2);  // Only the reference to the method exists on the stack after this
            return true;
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    NEKO_WARN("[lua] not find method \"%s\" with script_ref_%d", method_name, script_ref);

    return false;
}

bool ScriptInvoker::isMethodDefined(const_str method_name) const {
    lua_State *L = ENGINE_LUA();
    NEKO_ASSERT(L);

    if (method_name == NULL) return false;

    NEKO_ASSERT(script_ref != 0, "You must register this instance before you can invoke any script methods on it");

    lua_rawgeti(L, LUA_REGISTRYINDEX, script_ref);
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, method_name);
        if (lua_isfunction(L, -1)) {
            lua_pop(L, 2);
            return true;
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    return false;
}

////////////////////////////////////////////////////////////////

lua_class_define_impl<ScriptObject> ScriptObject::lua_class_defs("ScriptObject", NULL);

lua_class_define *ScriptObject::getClassDef() const { return &lua_class_defs; }

lua_class_define_impl<ScriptObject> *ScriptObject::getStaticClassDef() { return &ScriptObject::lua_class_defs; }

////////////////////////////////////////////////////////////////

ScriptObject::ScriptObject() : ScriptInvoker(), obj_last_entry(NULL) {}

ScriptObject::~ScriptObject() {}

bool ScriptObject::registerObject() {
    lua_State *L = ENGINE_LUA();
    NEKO_ASSERT(L);

    NEKO_ASSERT(script_ref == 0, "You are trying to register the same object twice");

    // Get the global lua state
    script_ref = getClassDef()->instantiate(L, this);

    if (!onAdd()) {
        unregisterObject();
        return false;
    }

    return true;
}

bool ScriptObject::registerObject(int refId) {
    lua_State *L = ENGINE_LUA();
    NEKO_ASSERT(L);

    NEKO_ASSERT(script_ref == 0, "You are trying to register the same object twice");

    script_ref = refId;

    lua_rawgeti(L, LUA_REGISTRYINDEX, script_ref);
    lua_pushstring(L, "_instance");
    lua_pushlightuserdata(L, this);
    lua_settable(L, -3);

    lua_pop(L, 1);

    if (!onAdd()) {
        unregisterObject();
        return false;
    }

    return true;
}

void ScriptObject::unregisterObject() {
    lua_State *L = ENGINE_LUA();
    NEKO_ASSERT(L);

    NEKO_ASSERT(script_ref != 0, "You are trying to unregister the same object twice");

    onRemove();

    // Set _instance to nil
    lua_rawgeti(L, LUA_REGISTRYINDEX, script_ref);
    lua_pushstring(L, "_instance");
    lua_pushnil(L);
    lua_rawset(L, -3);
    lua_pop(L, 1);

    luaL_unref(L, LUA_REGISTRYINDEX, script_ref);

    script_ref = 0;

    release_pointers();
}

bool ScriptObject::onAdd() {
    invoke("onAdd");
    return true;
}

void ScriptObject::onRemove() { invoke("onRemove", 10); }

void ScriptObject::detach_pointer(ScriptObjectEntry *entry) {
    ScriptObjectEntry *prev = entry->prev;
    ScriptObjectEntry *next = entry->next;

    if (prev != NULL) {
        prev->next = next;
        if (next != NULL) {
            next->prev = prev;
        }

        if (entry == obj_last_entry) {
            obj_last_entry = entry->prev;
        }
    } else {
        if (next != NULL) {
            next->prev = NULL;
        }
    }

    mem_free(entry);
}

ScriptObjectEntry *ScriptObject::attach_pointer(ScriptObject **ptr) {
    if (obj_last_entry == NULL) {
        obj_last_entry = reinterpret_cast<ScriptObjectEntry *>(mem_alloc(sizeof(ScriptObjectEntry)));
        memset(obj_last_entry, 0, sizeof(ScriptObjectEntry));
    } else {
        ScriptObjectEntry *next = reinterpret_cast<ScriptObjectEntry *>(mem_alloc(sizeof(ScriptObjectEntry)));
        memset(next, 0, sizeof(ScriptObjectEntry));
        obj_last_entry->next = next;
        next->prev = obj_last_entry;
        obj_last_entry = next;
    }

    obj_last_entry->ptr = ptr;
    return obj_last_entry;
}

void ScriptObject::release_pointers() {
    if (obj_last_entry == NULL) return;

    ScriptObjectEntry *entry = obj_last_entry;
    while (entry != NULL) {
        ScriptObjectEntry *prev = entry->prev;

        ScriptObject **ptr = entry->ptr;
        *ptr = NULL;
        mem_free(entry);

        entry = prev;
    }

    obj_last_entry = NULL;
}

int lua_delete(lua_State *L) {
    int numobjects = lua_gettop(L);
    for (int i = 0; i < numobjects; ++i) {
        ScriptObject *self;
        if (lua_value<ScriptObject *>::pop(L, self)) {
            self->unregisterObject();
            delete self;
        } else {
            NEKO_WARN("[lua] cannot delete an instance that isn't a ScriptObject*");
        }
    }

    return 0;
}

lua_State *lua_bind::L;

void lua_bind::initialize(lua_State *_L) {

    if (L != NULL) {
        NEKO_WARN("[lua] lua_bind already initialized");
        return;
    }

    L = _L;
    // 确保lua中有delete功能
    bind("delete", lua_delete);
    // 注册脚本对象
    bind<ScriptObject>();
}

lua_State *lua_bind::getLuaState() { return L; }

void lua_bind::evaluatef(const_str fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 1024, fmt, args);
    va_end(args);

    if (luaL_loadstring(L, buffer) != 0) {
        std::string err = lua_tostring(L, -1);
        NEKO_WARN("[lua] could not evaluate string \"%s\":\n%s", buffer, err.c_str());
        lua_pop(L, 1);
    }
}

void lua_bind::loadFile(const_str pathToFile) {
    int res = luaL_loadfile(L, pathToFile);
    if (res != 0) {
        std::string err = lua_tostring(L, -1);
        NEKO_WARN("[lua] could not load file: %s", err.c_str());
        lua_pop(L, 1);
    } else {
        res = lua_pcall(L, 0, 0, NULL);
        if (res != 0) {
            std::string err = lua_tostring(L, -1);
            NEKO_WARN("[lua] could not compile supplied script file: %s", err.c_str());
            lua_pop(L, 1);
        }
    }
}

namespace luavalue {
void set(lua_State *L, int idx, value &v) {
    switch (lua_type(L, idx)) {
        case LUA_TNIL:
            v.emplace<std::monostate>();
            break;
        case LUA_TBOOLEAN:
            v.emplace<bool>(!!lua_toboolean(L, idx));
            break;
        case LUA_TLIGHTUSERDATA:
            v.emplace<void *>(lua_touserdata(L, idx));
            break;
        case LUA_TNUMBER:
            if (lua_isinteger(L, idx)) {
                v.emplace<lua_Integer>(lua_tointeger(L, idx));
            } else {
                v.emplace<lua_Number>(lua_tonumber(L, idx));
            }
            break;
        case LUA_TSTRING: {
            size_t sz = 0;
            const char *str = lua_tolstring(L, idx, &sz);
            v.emplace<std::string>(str, sz);
            break;
        }
        case LUA_TFUNCTION: {
            lua_CFunction func = lua_tocfunction(L, idx);
            if (func == NULL || lua_getupvalue(L, idx, 1) != NULL) {
                luaL_error(L, "Only light C function can be serialized");
                return;
            }
            v.emplace<lua_CFunction>(func);
            break;
        }
        default:
            luaL_error(L, "Unsupport type %s to serialize", lua_typename(L, idx));
    }
}

void set(lua_State *L, int idx, table &t) {
    luaL_checktype(L, idx, LUA_TTABLE);
    lua_pushnil(L);
    while (lua_next(L, idx)) {
        size_t sz = 0;
        const char *str = luaL_checklstring(L, -2, &sz);
        std::pair<std::string, value> pair;
        pair.first.assign(str, sz);
        set(L, -1, pair.second);
        t.emplace(pair);
        lua_pop(L, 1);
    }
}

void get(lua_State *L, const value &v) {
    std::visit(
            [=](auto &&arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::monostate>) {
                    lua_pushnil(L);
                } else if constexpr (std::is_same_v<T, bool>) {
                    lua_pushboolean(L, arg);
                } else if constexpr (std::is_same_v<T, void *>) {
                    lua_pushlightuserdata(L, arg);
                } else if constexpr (std::is_same_v<T, lua_Integer>) {
                    lua_pushinteger(L, arg);
                } else if constexpr (std::is_same_v<T, lua_Number>) {
                    lua_pushnumber(L, arg);
                } else if constexpr (std::is_same_v<T, std::string>) {
                    lua_pushlstring(L, arg.data(), arg.size());
                } else if constexpr (std::is_same_v<T, lua_CFunction>) {
                    lua_pushcfunction(L, arg);
                } else {
                    static_assert(always_false_v<T>, "non-exhaustive visitor!");
                }
            },
            v);
}

void get(lua_State *L, const table &t) {
    lua_createtable(L, 0, static_cast<int>(t.size()));
    for (const auto &[k, v] : t) {
        lua_pushlstring(L, k.data(), k.size());
        get(L, v);
        lua_rawset(L, -3);
    }
}
}  // namespace luavalue

int vfs_lua_loader(lua_State *L) {
    const_str name = luaL_checkstring(L, 1);
    std::string path = name;
    std::replace(path.begin(), path.end(), '.', '/');

    // neko_println("fuck:%s", path.c_str());

    bool ok = false;
    auto load_list = {"source/lua/game/", "source/lua/libs/"};
    for (auto p : load_list) {
        std::string load_path = p + path + ".lua";
        String contents = {};
        ok = vfs_read_entire_file(NEKO_PACKS::LUACODE, &contents, load_path.c_str());
        if (ok) {
            neko_defer(mem_free(contents.data));
            if (luaL_loadbuffer(L, contents.data, contents.len, name) != LUA_OK) {
                lua_pushfstring(L, "[lua] error loading module \"%s\"", name);
                lua_pop(L, 1);
            } else {
                NEKO_TRACE("[lua] loaded : \"%s\"", path.c_str());
            }
            return 1;
        }
    }

#if 0
    u8* data;
    u32 data_size;

    int result = neko_pak_item_data(&ENGINE_INTERFACE()->pack, path.c_str(), (const u8**)&data, &data_size);
    if (result == 0) {
        if (luaL_loadbuffer(L, (char*)data, data_size, name) != LUA_OK) {
            lua_pop(L, 1);
        } else {
            neko_println("loaded:%s", path.c_str());
        }

        neko_pak_item_free(&ENGINE_INTERFACE()->pack, data);
    }
#endif

    lua_pushfstring(L, "[lua] module \"%s\" not found", name);
    return 1;
}

}  // namespace neko

i32 luax_require_script(lua_State *L, String filepath) {
    PROFILE_FUNC();

    if (g_app->error_mode.load()) {
        return LUA_REFNIL;
    }

    String path = to_cstr(filepath);
    neko_defer(mem_free(path.data));

    String contents;
    bool ok = vfs_read_entire_file(NEKO_PACKS::GAMEDATA, &contents, filepath);
    if (!ok) {
        StringBuilder sb = {};
        neko_defer(sb.trash());
        fatal_error(String(sb << "failed to read file: " << filepath));
        return LUA_REFNIL;
    }
    neko_defer(mem_free(contents.data));

    lua_newtable(L);
    i32 module_table = lua_gettop(L);

    {
        PROFILE_BLOCK("load lua script");

        if (luaL_loadbuffer(L, contents.data, contents.len, path.data) != LUA_OK) {
            fatal_error(luax_check_string(L, -1));
            return LUA_REFNIL;
        }
    }

    // run script
    if (lua_pcall(L, 0, LUA_MULTRET, 1) != LUA_OK) {
        lua_pop(L, 2);  // also pop module table
        return LUA_REFNIL;
    }

    // copy return results to module table
    i32 top = lua_gettop(L);
    for (i32 i = 1; i <= top - module_table; i++) {
        lua_seti(L, module_table, i);
    }

    return luaL_ref(L, LUA_REGISTRYINDEX);
}

void luax_stack_dump(lua_State *L) {
    i32 top = lua_gettop(L);
    printf("  --- lua stack (%d) ---\n", top);
    for (i32 i = 1; i <= top; i++) {
        printf("  [%d] (%s): ", i, luaL_typename(L, i));

        switch (lua_type(L, i)) {
            case LUA_TNUMBER:
                printf("%f\n", lua_tonumber(L, i));
                break;
            case LUA_TSTRING:
                printf("%s\n", lua_tostring(L, i));
                break;
            case LUA_TBOOLEAN:
                printf("%d\n", lua_toboolean(L, i));
                break;
            case LUA_TNIL:
                printf("nil\n");
                break;
            default:
                printf("%p\n", lua_topointer(L, i));
                break;
        }
    }
}

void luax_pcall(lua_State *L, i32 args, i32 results) {
    if (lua_pcall(L, args, results, 1) != LUA_OK) {
        lua_pop(L, 1);
    }
}

void luax_neko_get(lua_State *L, const char *field) {
    lua_getglobal(L, "neko");
    lua_getfield(L, -1, field);
    lua_remove(L, -2);
}

int luax_msgh(lua_State *L) {
    if (g_app->error_mode.load()) {
        return 0;
    }

    String err = luax_check_string(L, -1);

    // traceback = debug.traceback(nil, 2)
    lua_getglobal(L, "debug");
    lua_getfield(L, -1, "traceback");
    lua_remove(L, -2);
    lua_pushnil(L);
    lua_pushinteger(L, 2);
    lua_call(L, 2, 1);
    String traceback = luax_check_string(L, -1);

    if (LockGuard lock{&g_app->error_mtx}) {
        g_app->fatal_error = to_cstr(err);
        g_app->traceback = to_cstr(traceback);

        fprintf(stderr, "%s\n", g_app->fatal_error.data);
        fprintf(stderr, "%s\n", g_app->traceback.data);

        for (u64 i = 0; i < g_app->traceback.len; i++) {
            if (g_app->traceback.data[i] == '\t') {
                g_app->traceback.data[i] = ' ';
            }
        }

        g_app->error_mode.store(true);
    }

    lua_pop(L, 2);  // traceback and error
    return 0;
}

lua_Integer luax_len(lua_State *L, i32 arg) {
    lua_len(L, arg);
    lua_Integer len = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return len;
}

void luax_geti(lua_State *L, i32 arg, lua_Integer n) {
    lua_pushinteger(L, n);
    lua_gettable(L, arg);
}

void luax_set_number_field(lua_State *L, const char *key, lua_Number n) {
    lua_pushnumber(L, n);
    lua_setfield(L, -2, key);
}

void luax_set_int_field(lua_State *L, const char *key, lua_Integer n) {
    lua_pushinteger(L, n);
    lua_setfield(L, -2, key);
}

void luax_set_string_field(lua_State *L, const char *key, const char *str) {
    lua_pushstring(L, str);
    lua_setfield(L, -2, key);
}

lua_Number luax_number_field(lua_State *L, i32 arg, const char *key) {
    lua_getfield(L, arg, key);
    lua_Number num = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return num;
}

lua_Number luax_opt_number_field(lua_State *L, i32 arg, const char *key, lua_Number fallback) {
    i32 type = lua_getfield(L, arg, key);

    lua_Number num = fallback;
    if (type != LUA_TNIL) {
        num = luaL_optnumber(L, -1, fallback);
    }

    lua_pop(L, 1);
    return num;
}

lua_Integer luax_int_field(lua_State *L, i32 arg, const char *key) {
    lua_getfield(L, arg, key);
    lua_Integer num = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return num;
}

lua_Number luax_opt_int_field(lua_State *L, i32 arg, const char *key, lua_Number fallback) {
    i32 type = lua_getfield(L, arg, key);

    lua_Number num = fallback;
    if (type != LUA_TNIL) {
        num = luaL_optinteger(L, -1, fallback);
    }

    lua_pop(L, 1);
    return num;
}

String luax_string_field(lua_State *L, i32 arg, const char *key) {
    lua_getfield(L, arg, key);
    size_t len = 0;
    char *str = (char *)luaL_checklstring(L, -1, &len);
    lua_pop(L, 1);
    return {str, len};
}

String luax_opt_string_field(lua_State *L, i32 arg, const char *key, const char *fallback) {
    lua_getfield(L, arg, key);
    size_t len = 0;
    char *str = (char *)luaL_optlstring(L, -1, fallback, &len);
    lua_pop(L, 1);
    return {str, len};
}

bool luax_boolean_field(lua_State *L, i32 arg, const char *key, bool fallback) {
    i32 type = lua_getfield(L, arg, key);

    bool b = fallback;
    if (type != LUA_TNIL) {
        b = lua_toboolean(L, -1);
    }

    lua_pop(L, 1);
    return b;
}

String luax_check_string(lua_State *L, i32 arg) {
    size_t len = 0;
    char *str = (char *)luaL_checklstring(L, arg, &len);
    return {str, len};
}

String luax_opt_string(lua_State *L, i32 arg, String def) { return lua_isstring(L, arg) ? luax_check_string(L, arg) : def; }

int luax_string_oneof(lua_State *L, std::initializer_list<String> haystack, String needle) {
    StringBuilder sb = {};
    neko_defer(sb.trash());

    sb << "expected one of: {";
    for (String s : haystack) {
        sb << "\"" << s << "\", ";
    }
    if (haystack.size() != 0) {
        sb.len -= 2;
    }
    sb << "} got: \"" << needle << "\".";

    return luaL_error(L, "%s", sb.data);
}

void luax_new_class(lua_State *L, const char *mt_name, const luaL_Reg *l) {
    luaL_newmetatable(L, mt_name);
    luaL_setfuncs(L, l, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

static void lua_thread_proc(void *udata) {
    PROFILE_FUNC();

    LuaThread *lt = (LuaThread *)udata;

    // lua_State *L = lua_newstate(luaalloc, LA);
    // neko_defer(lua_close(L));

    lua_State *L = neko::neko_lua_create();
    neko_defer(neko::neko_lua_fini(L));

    {
        PROFILE_BLOCK("open libs");
        luaL_openlibs(L);
    }

    {
        PROFILE_BLOCK("open api");
        open_neko_api(L);
    }

    {
        PROFILE_BLOCK("open luasocket");
        open_luasocket(L);
    }

    {
        PROFILE_BLOCK("run bootstrap");
        neko::lua::luax_run_bootstrap(L);
    }

    String contents = lt->contents;

    {
        PROFILE_BLOCK("load chunk");
        if (luaL_loadbuffer(L, contents.data, contents.len, lt->name.data) != LUA_OK) {
            String err = luax_check_string(L, -1);
            fprintf(stderr, "%s\n", err.data);

            mem_free(contents.data);
            mem_free(lt->name.data);
            return;
        }
    }

    mem_free(contents.data);
    mem_free(lt->name.data);

    {
        PROFILE_BLOCK("run chunk");
        if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
            String err = luax_check_string(L, -1);
            fprintf(stderr, "%s\n", err.data);
        }
    }
}

void LuaThread::make(String code, String thread_name) {
    mtx.make();
    contents = to_cstr(code);
    name = to_cstr(thread_name);

    LockGuard lock{&mtx};
    thread.make(lua_thread_proc, this);
}

void LuaThread::join() {
    if (LockGuard lock{&mtx}) {
        thread.join();
    }

    mtx.trash();
}

//

void LuaVariant::make(lua_State *L, i32 arg) {
    type = lua_type(L, arg);

    switch (type) {
        case LUA_TBOOLEAN:
            boolean = lua_toboolean(L, arg);
            break;
        case LUA_TNUMBER:
            number = luaL_checknumber(L, arg);
            break;
        case LUA_TSTRING: {
            String s = luax_check_string(L, arg);
            string = to_cstr(s);
            break;
        }
        case LUA_TTABLE: {
            Array<LuaTableEntry> entries = {};
            entries.resize(luax_len(L, arg));

            lua_pushvalue(L, arg);
            for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
                LuaVariant key = {};
                key.make(L, -2);

                LuaVariant value = {};
                value.make(L, -1);

                entries.push({key, value});
            }
            lua_pop(L, 1);

            table = Slice(entries);
            break;
        }
        case LUA_TUSERDATA: {
            i32 kind = lua_getiuservalue(L, arg, LUAX_UD_TNAME);
            neko_defer(lua_pop(L, 1));
            if (kind != LUA_TSTRING) {
                return;
            }

            kind = lua_getiuservalue(L, arg, LUAX_UD_PTR_SIZE);
            neko_defer(lua_pop(L, 1));
            if (kind != LUA_TNUMBER) {
                return;
            }

            String tname = luax_check_string(L, -2);
            u64 size = luaL_checkinteger(L, -1);

            if (size != sizeof(void *)) {
                return;
            }

            udata.ptr = *(void **)lua_touserdata(L, arg);
            udata.tname = to_cstr(tname);

            break;
        }
        default:
            break;
    }
}

void LuaVariant::trash() {
    switch (type) {
        case LUA_TSTRING: {
            mem_free(string.data);
            break;
        }
        case LUA_TTABLE: {
            for (LuaTableEntry e : table) {
                e.key.trash();
                e.value.trash();
            }
            mem_free(table.data);
        }
        case LUA_TUSERDATA: {
            mem_free(udata.tname.data);
        }
        default:
            break;
    }
}

void LuaVariant::push(lua_State *L) {
    switch (type) {
        case LUA_TBOOLEAN:
            lua_pushboolean(L, boolean);
            break;
        case LUA_TNUMBER:
            lua_pushnumber(L, number);
            break;
        case LUA_TSTRING:
            lua_pushlstring(L, string.data, string.len);
            break;
        case LUA_TTABLE: {
            lua_newtable(L);
            for (LuaTableEntry e : table) {
                e.key.push(L);
                e.value.push(L);
                lua_rawset(L, -3);
            }
            break;
        }
        case LUA_TUSERDATA: {
            luax_ptr_userdata(L, udata.ptr, udata.tname.data);
            break;
        }
        default:
            break;
    }
}

//

struct LuaChannels {
    Mutex mtx;
    Cond select;
    HashMap<LuaChannel *> by_name;
};

static LuaChannels g_channels = {};

void LuaChannel::make(String n, u64 buf) {
    mtx.make();
    sent.make();
    received.make();
    items.data = (LuaVariant *)mem_alloc(sizeof(LuaVariant) * (buf + 1));
    items.len = (buf + 1);
    front = 0;
    back = 0;
    len = 0;

    name.store(to_cstr(n).data);
}

void LuaChannel::trash() {
    for (i32 i = 0; i < len; i++) {
        items[front].trash();
        front = (front + 1) % items.len;
    }

    mem_free(items.data);
    mem_free(name.exchange(nullptr));
    mtx.trash();
    sent.trash();
    received.trash();
}

void LuaChannel::send(LuaVariant item) {
    LockGuard lock{&mtx};

    while (len == items.len) {
        received.wait(&mtx);
    }

    items[back] = item;
    back = (back + 1) % items.len;
    len++;

    g_channels.select.broadcast();
    sent.signal();
    sent_total++;

    while (sent_total >= received_total + items.len) {
        received.wait(&mtx);
    }
}

static LuaVariant lua_channel_dequeue(LuaChannel *ch) {
    LuaVariant item = ch->items[ch->front];
    ch->front = (ch->front + 1) % ch->items.len;
    ch->len--;

    ch->received.broadcast();
    ch->received_total++;

    return item;
}

LuaVariant LuaChannel::recv() {
    LockGuard lock{&mtx};

    while (len == 0) {
        sent.wait(&mtx);
    }

    return lua_channel_dequeue(this);
}

bool LuaChannel::try_recv(LuaVariant *v) {
    LockGuard lock{&mtx};

    if (len == 0) {
        return false;
    }

    *v = lua_channel_dequeue(this);
    return true;
}

LuaChannel *lua_channel_make(String name, u64 buf) {
    LuaChannel *chan = (LuaChannel *)mem_alloc(sizeof(LuaChannel));
    new (&chan->name) std::atomic<char *>();
    chan->make(name, buf);

    LockGuard lock{&g_channels.mtx};
    g_channels.by_name[fnv1a(name)] = chan;

    return chan;
}

LuaChannel *lua_channel_get(String name) {
    LockGuard lock{&g_channels.mtx};

    LuaChannel **chan = g_channels.by_name.get(fnv1a(name));
    if (chan == nullptr) {
        return nullptr;
    }

    return *chan;
}

LuaChannel *lua_channels_select(lua_State *L, LuaVariant *v) {
    i32 len = lua_gettop(L);
    if (len == 0) {
        return nullptr;
    }

    LuaChannel *buf[16] = {};
    for (i32 i = 0; i < len; i++) {
        buf[i] = *(LuaChannel **)luaL_checkudata(L, i + 1, "mt_channel");
    }

    Mutex mtx = {};
    mtx.make();
    LockGuard lock{&mtx};

    while (true) {
        for (i32 i = 0; i < len; i++) {
            LockGuard lock{&buf[i]->mtx};
            if (buf[i]->len > 0) {
                *v = lua_channel_dequeue(buf[i]);
                return buf[i];
            }
        }

        g_channels.select.wait(&mtx);
    }
}

void lua_channels_setup() {
    g_channels.select.make();
    g_channels.mtx.make();
}

void lua_channels_shutdown() {
    for (auto [k, v] : g_channels.by_name) {
        LuaChannel *chan = *v;
        chan->trash();
        mem_free(chan);
    }
    g_channels.by_name.trash();
    g_channels.select.trash();
    g_channels.mtx.trash();
}

#define FREELIST 1

void neko_luaref::make(lua_State *L) {
    this->refL = lua_newthread(L);
    lua_rawsetp(L, LUA_REGISTRYINDEX, refL);
    lua_newtable(refL);
}

void neko_luaref::fini() {
    lua_State *L = (lua_State *)refL;
    lua_pushnil(L);
    lua_rawsetp(L, LUA_REGISTRYINDEX, refL);
}

bool neko_luaref::isvalid(int ref) {
    if (ref <= FREELIST || ref > lua_gettop(refL)) {
        return false;
    }
    lua_pushnil(refL);
    while (lua_next(refL, FREELIST)) {
        lua_pop(refL, 1);
        if (lua_tointeger(refL, -1) == ref) {
            lua_pop(refL, 1);
            return false;
        }
    }
    return true;
}

int neko_luaref::ref(lua_State *L) {
    if (!lua_checkstack(refL, 3)) {
        return LUA_NOREF;
    }
    lua_xmove(L, refL, 1);
    lua_pushnil(refL);
    if (!lua_next(refL, FREELIST)) {
        return lua_gettop(refL);
    }
    int r = (int)lua_tointeger(refL, -2);
    lua_pop(refL, 1);
    lua_pushnil(refL);
    lua_rawset(refL, FREELIST);
    lua_replace(refL, r);
    return r;
}

void neko_luaref::unref(int ref) {
    if (ref <= FREELIST) {
        return;
    }
    int top = lua_gettop(refL);
    if (ref > top) {
        return;
    }
    if (ref < top) {
        lua_pushboolean(refL, 1);
        lua_rawseti(refL, FREELIST, ref);
        lua_pushnil(refL);
        lua_replace(refL, ref);
        return;
    }
    for (--top; top > FREELIST; --top) {
        if (LUA_TNIL == lua_rawgeti(refL, FREELIST, top)) {
            lua_pop(refL, 1);
            break;
        }
        lua_pop(refL, 1);
        lua_pushnil(refL);
        lua_rawseti(refL, FREELIST, top);
    }
    lua_settop(refL, top);
}

void neko_luaref::get(lua_State *L, int ref) {
    assert(isvalid(ref));
    lua_pushvalue(refL, ref);
    lua_xmove(refL, L, 1);
}

void neko_luaref::set(lua_State *L, int ref) {
    assert(isvalid(ref));
    lua_xmove(L, refL, 1);
    lua_replace(refL, ref);
}