#include "neko_lua.h"

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <iostream>

#include "engine/neko.h"
#include "engine/neko.hpp"
#include "engine/neko_engine.h"
#include "engine/neko_math.h"

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
    neko_luabind_conversion(L, s32, neko_luabind_push_long, neko_luabind_to_long);
    neko_luabind_conversion(L, unsigned long, neko_luabind_push_unsigned_long, neko_luabind_to_unsigned_long);
    neko_luabind_conversion(L, u32, neko_luabind_push_unsigned_long, neko_luabind_to_unsigned_long);
    neko_luabind_conversion(L, long long, neko_luabind_push_long_long, neko_luabind_to_long_long);
    neko_luabind_conversion(L, s64, neko_luabind_push_long_long, neko_luabind_to_long_long);
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

NEKO_API_DECL bool neko_lua_equal(lua_State *state, int index1, int index2) {
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

LUASTRUCT_BEGIN(neko_vec3)
LUASTRUCT_FIELD(x, float)
LUASTRUCT_FIELD(y, float)
LUASTRUCT_FIELD(z, float)
LUASTRUCT_END

NEKO_API_DECL void createStructTables(lua_State *L) {
    neko_vec3_create(L, "VEC3");

    ARRAY_uchar8_create(L);
}

// neko_tolua based on tolua by Waldemar Celes (www.tecgraf.puc-rio.br/~celes/tolua)
// it's licensed under the terms of the MIT license

#if defined(NEKO_TOLUA)

/* Store at ubox
 * It stores, creating the corresponding table if needed,
 * the pair key/value in the corresponding ubox table
 */
static void storeatubox(lua_State *L, int lo) {
#if LUA_VERSION_NUM == 501
    lua_getfenv(L, lo);
    if (lua_rawequal(L, -1, TOLUA_NOPEER)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setfenv(L, lo); /* stack: k,v,table  */
    };
    lua_insert(L, -3);
    lua_settable(L, -3); /* on lua 5.1, we trade the "neko_tolua_peers" lookup for a settable call */
    lua_pop(L, 1);
#elif LUA_VERSION_NUM > 501
    lua_getuservalue(L, lo);
    if (lua_rawequal(L, -1, TOLUA_NOPEER)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setuservalue(L, lo); /* stack: k,v,table  */
    };
    lua_insert(L, -3);
    lua_settable(L, -3); /* on lua 5.1, we trade the "neko_tolua_peers" lookup for a settable call */
    lua_pop(L, 1);
#else
    /* stack: key value (to be stored) */
    lua_pushstring(L, "neko_tolua_peers");
    lua_rawget(L, LUA_REGISTRYINDEX); /* stack: k v ubox */
    lua_pushvalue(L, lo);
    lua_rawget(L, -2); /* stack: k v ubox ubox[u] */
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);   /* stack: k v ubox */
        lua_newtable(L); /* stack: k v ubox table */
        lua_pushvalue(L, 1);
        lua_pushvalue(L, -2); /* stack: k v ubox table u table */
        lua_rawset(L, -4);    /* stack: k v ubox ubox[u]=table */
    }
    lua_insert(L, -4); /* put table before k */
    lua_pop(L, 1);     /* pop ubox */
    lua_rawset(L, -3); /* store at table */
    lua_pop(L, 1);     /* pop ubox[u] */
#endif
}

/* Module index function
 */
static int module_index_event(lua_State *L) {
    lua_pushstring(L, ".get");
    lua_rawget(L, -3);
    if (lua_istable(L, -1)) {
        lua_pushvalue(L, 2); /* key */
        lua_rawget(L, -2);
        if (lua_iscfunction(L, -1)) {
            lua_call(L, 0, 1);
            return 1;
        } else if (lua_istable(L, -1))
            return 1;
    }
    /* call old index meta event */
    if (lua_getmetatable(L, 1)) {
        lua_pushstring(L, "__index");
        lua_rawget(L, -2);
        lua_pushvalue(L, 1);
        lua_pushvalue(L, 2);
        if (lua_isfunction(L, -1)) {
            lua_call(L, 2, 1);
            return 1;
        } else if (lua_istable(L, -1)) {
            lua_gettable(L, -3);
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

/* Module newindex function
 */
static int module_newindex_event(lua_State *L) {
    lua_pushstring(L, ".set");
    lua_rawget(L, -4);
    if (lua_istable(L, -1)) {
        lua_pushvalue(L, 2); /* key */
        lua_rawget(L, -2);
        if (lua_iscfunction(L, -1)) {
            lua_pushvalue(L, 1); /* only to be compatible with non-static vars */
            lua_pushvalue(L, 3); /* value */
            lua_call(L, 2, 0);
            return 0;
        }
    }
    /* call old newindex meta event */
    if (lua_getmetatable(L, 1) && lua_getmetatable(L, -1)) {
        lua_pushstring(L, "__newindex");
        lua_rawget(L, -2);
        if (lua_isfunction(L, -1)) {
            lua_pushvalue(L, 1);
            lua_pushvalue(L, 2);
            lua_pushvalue(L, 3);
            lua_call(L, 3, 0);
        }
    }
    lua_settop(L, 3);
    lua_rawset(L, -3);
    return 0;
}

/* Class index function
 * If the object is a userdata (ie, an object), it searches the field in
 * the alternative table stored in the corresponding "ubox" table.
 */
static int class_index_event(lua_State *L) {
    int t = lua_type(L, 1);
    if (t == LUA_TUSERDATA) {
/* Access alternative table */
#if LUA_VERSION_NUM == 501
        lua_getfenv(L, 1);
        if (!lua_rawequal(L, -1, TOLUA_NOPEER)) {
            lua_pushvalue(L, 2); /* key */
            lua_gettable(L, -2); /* on lua 5.1, we trade the "neko_tolua_peers" lookup for a gettable call */
            if (!lua_isnil(L, -1)) return 1;
        };
#elif LUA_VERSION_NUM > 501
        lua_getuservalue(L, 1);
        if (!lua_rawequal(L, -1, TOLUA_NOPEER)) {
            lua_pushvalue(L, 2); /* key */
            lua_gettable(L, -2); /* on lua 5.1, we trade the "neko_tolua_peers" lookup for a gettable call */
            if (!lua_isnil(L, -1)) return 1;
        };
#else
        lua_pushstring(L, "neko_tolua_peers");
        lua_rawget(L, LUA_REGISTRYINDEX); /* stack: obj key ubox */
        lua_pushvalue(L, 1);
        lua_rawget(L, -2); /* stack: obj key ubox ubox[u] */
        if (lua_istable(L, -1)) {
            lua_pushvalue(L, 2); /* key */
            lua_rawget(L, -2);   /* stack: obj key ubox ubox[u] value */
            if (!lua_isnil(L, -1)) return 1;
        }
#endif
        lua_settop(L, 2); /* stack: obj key */
        /* Try metatables */
        lua_pushvalue(L, 1);              /* stack: obj key obj */
        while (lua_getmetatable(L, -1)) { /* stack: obj key obj mt */
            lua_remove(L, -2);            /* stack: obj key mt */
            if (lua_isnumber(L, 2))       /* check if key is a numeric value */
            {
                /* try operator[] */
                lua_pushstring(L, ".geti");
                lua_rawget(L, -2); /* stack: obj key mt func */
                if (lua_isfunction(L, -1)) {
                    lua_pushvalue(L, 1);
                    lua_pushvalue(L, 2);
                    lua_call(L, 2, 1);
                    return 1;
                }
            } else {
                lua_pushvalue(L, 2); /* stack: obj key mt key */
                lua_rawget(L, -2);   /* stack: obj key mt value */
                if (!lua_isnil(L, -1))
                    return 1;
                else
                    lua_pop(L, 1);
                /* try C/C++ variable */
                lua_pushstring(L, ".get");
                lua_rawget(L, -2); /* stack: obj key mt tget */
                if (lua_istable(L, -1)) {
                    lua_pushvalue(L, 2);
                    lua_rawget(L, -2); /* stack: obj key mt value */
                    if (lua_iscfunction(L, -1)) {
                        lua_pushvalue(L, 1);
                        lua_pushvalue(L, 2);
                        lua_call(L, 2, 1);
                        return 1;
                    } else if (lua_istable(L, -1)) {
                        /* deal with array: create table to be returned and cache it in ubox */
                        void *u = *((void **)lua_touserdata(L, 1));
                        lua_newtable(L); /* stack: obj key mt value table */
                        lua_pushstring(L, ".self");
                        lua_pushlightuserdata(L, u);
                        lua_rawset(L, -3);       /* store usertype in ".self" */
                        lua_insert(L, -2);       /* stack: obj key mt table value */
                        lua_setmetatable(L, -2); /* set stored value as metatable */
                        lua_pushvalue(L, -1);    /* stack: obj key met table table */
                        lua_pushvalue(L, 2);     /* stack: obj key mt table table key */
                        lua_insert(L, -2);       /*  stack: obj key mt table key table */
                        storeatubox(L, 1);       /* stack: obj key mt table */
                        return 1;
                    }
                }
            }
            lua_settop(L, 3);
        }
        lua_pushnil(L);
        return 1;
    } else if (t == LUA_TTABLE) {
        module_index_event(L);
        return 1;
    }
    lua_pushnil(L);
    return 1;
}

/* Newindex function
 * It first searches for a C/C++ varaible to be set.
 * Then, it either stores it in the alternative ubox table (in the case it is
 * an object) or in the own table (that represents the class or module).
 */
static int class_newindex_event(lua_State *L) {
    int t = lua_type(L, 1);
    if (t == LUA_TUSERDATA) {
        /* Try accessing a C/C++ variable to be set */
        lua_getmetatable(L, 1);
        while (lua_istable(L, -1)) /* stack: t k v mt */
        {
            if (lua_isnumber(L, 2)) /* check if key is a numeric value */
            {
                /* try operator[] */
                lua_pushstring(L, ".seti");
                lua_rawget(L, -2); /* stack: obj key mt func */
                if (lua_isfunction(L, -1)) {
                    lua_pushvalue(L, 1);
                    lua_pushvalue(L, 2);
                    lua_pushvalue(L, 3);
                    lua_call(L, 3, 0);
                    return 0;
                }
            } else {
                lua_pushstring(L, ".set");
                lua_rawget(L, -2); /* stack: t k v mt tset */
                if (lua_istable(L, -1)) {
                    lua_pushvalue(L, 2);
                    lua_rawget(L, -2); /* stack: t k v mt tset func */
                    if (lua_iscfunction(L, -1)) {
                        lua_pushvalue(L, 1);
                        lua_pushvalue(L, 3);
                        lua_call(L, 2, 0);
                        return 0;
                    }
                    lua_pop(L, 1); /* stack: t k v mt tset */
                }
                lua_pop(L, 1);                /* stack: t k v mt */
                if (!lua_getmetatable(L, -1)) /* stack: t k v mt mt */
                    lua_pushnil(L);
                lua_remove(L, -2); /* stack: t k v mt */
            }
        }
        lua_settop(L, 3); /* stack: t k v */

        /* then, store as a new field */
        storeatubox(L, 1);
    } else if (t == LUA_TTABLE) {
        module_newindex_event(L);
    }
    return 0;
}

static int class_call_event(lua_State *L) {

    if (lua_istable(L, 1)) {
        lua_pushstring(L, ".call");
        lua_rawget(L, 1);
        if (lua_isfunction(L, -1)) {

            lua_insert(L, 1);
            lua_call(L, lua_gettop(L) - 1, 1);

            return 1;
        };
    };
    neko_tolua_error(L, "Attempt to call a non-callable object.", NULL);
    return 0;
};

static int do_operator(lua_State *L, const char *op) {
    if (lua_isuserdata(L, 1)) {
        /* Try metatables */
        lua_pushvalue(L, 1);              /* stack: op1 op2 */
        while (lua_getmetatable(L, -1)) { /* stack: op1 op2 op1 mt */
            lua_remove(L, -2);            /* stack: op1 op2 mt */
            lua_pushstring(L, op);        /* stack: op1 op2 mt key */
            lua_rawget(L, -2);            /* stack: obj key mt func */
            if (lua_isfunction(L, -1)) {
                lua_pushvalue(L, 1);
                lua_pushvalue(L, 2);
                lua_call(L, 2, 1);
                return 1;
            }
            lua_settop(L, 3);
        }
    }
    neko_tolua_error(L, "Attempt to perform operation on an invalid operand", NULL);
    return 0;
}

static int class_add_event(lua_State *L) { return do_operator(L, ".add"); }

static int class_sub_event(lua_State *L) { return do_operator(L, ".sub"); }

static int class_mul_event(lua_State *L) { return do_operator(L, ".mul"); }

static int class_div_event(lua_State *L) { return do_operator(L, ".div"); }

static int class_lt_event(lua_State *L) { return do_operator(L, ".lt"); }

static int class_le_event(lua_State *L) { return do_operator(L, ".le"); }

static int class_eq_event(lua_State *L) {
    /* copying code from do_operator here to return false when no operator is found */
    if (lua_isuserdata(L, 1)) {
        /* Try metatables */
        lua_pushvalue(L, 1);              /* stack: op1 op2 */
        while (lua_getmetatable(L, -1)) { /* stack: op1 op2 op1 mt */
            lua_remove(L, -2);            /* stack: op1 op2 mt */
            lua_pushstring(L, ".eq");     /* stack: op1 op2 mt key */
            lua_rawget(L, -2);            /* stack: obj key mt func */
            if (lua_isfunction(L, -1)) {
                lua_pushvalue(L, 1);
                lua_pushvalue(L, 2);
                lua_call(L, 2, 1);
                return 1;
            }
            lua_settop(L, 3);
        }
    }

    lua_settop(L, 3);
    lua_pushboolean(L, 0);
    return 1;
}

/*
static int class_gc_event (lua_State* L)
{
    void* u = *((void**)lua_touserdata(L,1));
    fprintf(stderr, "collecting: looking at %p\n", u);
    lua_pushstring(L,"neko_tolua_gc");
    lua_rawget(L,LUA_REGISTRYINDEX);
    lua_pushlightuserdata(L,u);
    lua_rawget(L,-2);
    if (lua_isfunction(L,-1))
    {
        lua_pushvalue(L,1);
        lua_call(L,1,0);
        lua_pushlightuserdata(L,u);
        lua_pushnil(L);
        lua_rawset(L,-3);
    }
    lua_pop(L,2);
    return 0;
}
*/
NEKO_API_DECL int class_gc_event(lua_State *L) {
    if (lua_istable(L, 1)) return 0;
    void *u = *((void **)lua_touserdata(L, 1));
    int top;
    /*fprintf(stderr, "collecting: looking at %p\n", u);*/
    /*
    lua_pushstring(L,"neko_tolua_gc");
    lua_rawget(L,LUA_REGISTRYINDEX);
    */
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_pushlightuserdata(L, u);
    lua_rawget(L, -2);      /* stack: gc umt    */
    lua_getmetatable(L, 1); /* stack: gc umt mt */
    /*fprintf(stderr, "checking type\n");*/
    top = lua_gettop(L);
    if (neko_tolua_fast_isa(L, top, top - 1, lua_upvalueindex(2))) /* make sure we collect correct type */
    {
        /*fprintf(stderr, "Found type!\n");*/
        /* get gc function */
        lua_pushliteral(L, ".collector");
        lua_rawget(L, -2); /* stack: gc umt mt collector */
        if (lua_isfunction(L, -1)) {
            /*fprintf(stderr, "Found .collector!\n");*/
        } else {
            lua_pop(L, 1);
            /*fprintf(stderr, "Using default cleanup\n");*/
            lua_pushcfunction(L, neko_tolua_default_collect);
        }

        lua_pushvalue(L, 1); /* stack: gc umt mt collector u */
        lua_call(L, 1, 0);

        lua_pushlightuserdata(L, u); /* stack: gc umt mt u */
        lua_pushnil(L);              /* stack: gc umt mt u nil */
        lua_rawset(L, -5);           /* stack: gc umt mt */
    }
    lua_pop(L, 3);
    return 0;
}

/* Register module events
 * It expects the metatable on the top of the stack
 */
NEKO_API_DECL void neko_tolua_moduleevents(lua_State *L) {
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, module_index_event);
    lua_rawset(L, -3);
    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, module_newindex_event);
    lua_rawset(L, -3);
}

/* Check if the object on the top has a module metatable
 */
NEKO_API_DECL int neko_tolua_ismodulemetatable(lua_State *L) {
    int r = 0;
    if (lua_getmetatable(L, -1)) {
        lua_pushstring(L, "__index");
        lua_rawget(L, -2);
        r = (lua_tocfunction(L, -1) == module_index_event);
        lua_pop(L, 2);
    }
    return r;
}

/* Register class events
 * It expects the metatable on the top of the stack
 */
NEKO_API_DECL void neko_tolua_classevents(lua_State *L) {
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, class_index_event);
    lua_rawset(L, -3);
    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, class_newindex_event);
    lua_rawset(L, -3);

    lua_pushstring(L, "__add");
    lua_pushcfunction(L, class_add_event);
    lua_rawset(L, -3);
    lua_pushstring(L, "__sub");
    lua_pushcfunction(L, class_sub_event);
    lua_rawset(L, -3);
    lua_pushstring(L, "__mul");
    lua_pushcfunction(L, class_mul_event);
    lua_rawset(L, -3);
    lua_pushstring(L, "__div");
    lua_pushcfunction(L, class_div_event);
    lua_rawset(L, -3);

    lua_pushstring(L, "__lt");
    lua_pushcfunction(L, class_lt_event);
    lua_rawset(L, -3);
    lua_pushstring(L, "__le");
    lua_pushcfunction(L, class_le_event);
    lua_rawset(L, -3);
    lua_pushstring(L, "__eq");
    lua_pushcfunction(L, class_eq_event);
    lua_rawset(L, -3);

    lua_pushstring(L, "__call");
    lua_pushcfunction(L, class_call_event);
    lua_rawset(L, -3);

    lua_pushstring(L, "__gc");
    lua_pushstring(L, "neko_tolua_gc_event");
    lua_rawget(L, LUA_REGISTRYINDEX);
    /*lua_pushcfunction(L,class_gc_event);*/
    lua_rawset(L, -3);
}

/* a fast check if a is b, without parameter validation
 i.e. if b is equal to a or a superclass of a. */
NEKO_API_DECL int neko_tolua_fast_isa(lua_State *L, int mt_indexa, int mt_indexb, int super_index) {
    int result;
    if (lua_rawequal(L, mt_indexa, mt_indexb))
        result = 1;
    else {
        if (super_index) {
            lua_pushvalue(L, super_index);
        } else {
            lua_pushliteral(L, "neko_tolua_super");
            lua_rawget(L, LUA_REGISTRYINDEX); /* stack: super */
        };
        lua_pushvalue(L, mt_indexa);      /* stack: super mta */
        lua_rawget(L, -2);                /* stack: super super[mta] */
        lua_pushvalue(L, mt_indexb);      /* stack: super super[mta] mtb */
        lua_rawget(L, LUA_REGISTRYINDEX); /* stack: super super[mta] typenameB */
        lua_rawget(L, -2);                /* stack: super super[mta] bool */
        result = lua_toboolean(L, -1);
        lua_pop(L, 3);
    }
    return result;
}

/* Push and returns the corresponding object typename */
NEKO_API_DECL const char *neko_tolua_typename(lua_State *L, int lo) {
    int tag = lua_type(L, lo);
    if (tag == LUA_TNONE)
        lua_pushstring(L, "[no object]");
    else if (tag != LUA_TUSERDATA && tag != LUA_TTABLE)
        lua_pushstring(L, lua_typename(L, tag));
    else if (tag == LUA_TUSERDATA) {
        if (!lua_getmetatable(L, lo))
            lua_pushstring(L, lua_typename(L, tag));
        else {
            lua_rawget(L, LUA_REGISTRYINDEX);
            if (!lua_isstring(L, -1)) {
                lua_pop(L, 1);
                lua_pushstring(L, "[undefined]");
            }
        }
    } else /* is table */
    {
        lua_pushvalue(L, lo);
        lua_rawget(L, LUA_REGISTRYINDEX);
        if (!lua_isstring(L, -1)) {
            lua_pop(L, 1);
            lua_pushstring(L, "table");
        } else {
            lua_pushstring(L, "class ");
            lua_insert(L, -2);
            lua_concat(L, 2);
        }
    }
    return lua_tostring(L, -1);
}

NEKO_API_DECL void neko_tolua_error(lua_State *L, const char *msg, neko_tolua_Error *err) {
    if (msg[0] == '#') {
        const char *expected = err->type;
        const char *provided = neko_tolua_typename(L, err->index);
        if (msg[1] == 'f') {
            int narg = err->index;
            if (err->array)
                luaL_error(L, "%s\n     argument #%d is array of '%s'; array of '%s' expected.\n", msg + 2, narg, provided, expected);
            else
                luaL_error(L, "%s\n     argument #%d is '%s'; '%s' expected.\n", msg + 2, narg, provided, expected);
        } else if (msg[1] == 'v') {
            if (err->array)
                luaL_error(L, "%s\n     value is array of '%s'; array of '%s' expected.\n", msg + 2, provided, expected);
            else
                luaL_error(L, "%s\n     value is '%s'; '%s' expected.\n", msg + 2, provided, expected);
        }
    } else
        luaL_error(L, msg);
}

/* the equivalent of lua_is* for usertable */
static int lua_isusertable(lua_State *L, int lo, const_str type) {
    int r = 0;
    if (lo < 0) lo = lua_gettop(L) + lo + 1;
    lua_pushvalue(L, lo);
    lua_rawget(L, LUA_REGISTRYINDEX); /* get registry[t] */
    if (lua_isstring(L, -1)) {
        r = strcmp(lua_tostring(L, -1), type) == 0;
        if (!r) {
            /* try const */
            lua_pushstring(L, "const ");
            lua_insert(L, -2);
            lua_concat(L, 2);
            r = lua_isstring(L, -1) && strcmp(lua_tostring(L, -1), type) == 0;
        }
    }
    lua_pop(L, 1);
    return r;
}

int push_table_instance(lua_State *L, int lo) {

    if (lua_istable(L, lo)) {

        lua_pushstring(L, ".c_instance");
        lua_gettable(L, lo);
        if (lua_isuserdata(L, -1)) {

            lua_replace(L, lo);
            return 1;
        } else {

            lua_pop(L, 1);
            return 0;
        };
    } else {
        return 0;
    };

    return 0;
};

/* the equivalent of lua_is* for usertype */
static int lua_isusertype(lua_State *L, int lo, const char *type) {
    if (!lua_isuserdata(L, lo)) {
        if (!push_table_instance(L, lo)) {
            return 0;
        };
    };
    {
        /* check if it is of the same type */
        int r;
        const char *tn;
        if (lua_getmetatable(L, lo)) /* if metatable? */
        {
            lua_rawget(L, LUA_REGISTRYINDEX); /* get registry[mt] */
            tn = lua_tostring(L, -1);
            r = tn && (strcmp(tn, type) == 0);
            lua_pop(L, 1);
            if (r)
                return 1;
            else {
                /* check if it is a specialized class */
                lua_pushstring(L, "neko_tolua_super");
                lua_rawget(L, LUA_REGISTRYINDEX); /* get super */
                lua_getmetatable(L, lo);
                lua_rawget(L, -2); /* get super[mt] */
                if (lua_istable(L, -1)) {
                    int b;
                    lua_pushstring(L, type);
                    lua_rawget(L, -2); /* get super[mt][type] */
                    b = lua_toboolean(L, -1);
                    lua_pop(L, 3);
                    if (b) return 1;
                }
            }
        }
    }
    return 0;
}

NEKO_API_DECL int neko_tolua_isnoobj(lua_State *L, int lo, neko_tolua_Error *err) {
    if (lua_gettop(L) < abs(lo)) return 1;
    err->index = lo;
    err->array = 0;
    err->type = "[no object]";
    return 0;
}

NEKO_API_DECL int neko_tolua_isboolean(lua_State *L, int lo, int def, neko_tolua_Error *err) {
    if (def && lua_gettop(L) < abs(lo)) return 1;
    if (lua_isnil(L, lo) || lua_isboolean(L, lo)) return 1;
    err->index = lo;
    err->array = 0;
    err->type = "boolean";
    return 0;
}

NEKO_API_DECL int neko_tolua_isnumber(lua_State *L, int lo, int def, neko_tolua_Error *err) {
    if (def && lua_gettop(L) < abs(lo)) return 1;
    if (lua_isnumber(L, lo)) return 1;
    err->index = lo;
    err->array = 0;
    err->type = "number";
    return 0;
}

NEKO_API_DECL int neko_tolua_isinteger(lua_State *L, int lo, int def, neko_tolua_Error *err) {
    if (def && lua_gettop(L) < abs(lo)) return 1;
    if (lua_isinteger(L, lo)) return 1;
    err->index = lo;
    err->array = 0;
    err->type = "integer";
    return 0;
}

NEKO_API_DECL int neko_tolua_isstring(lua_State *L, int lo, int def, neko_tolua_Error *err) {
    if (def && lua_gettop(L) < abs(lo)) return 1;
    if (lua_isnil(L, lo) || lua_isstring(L, lo)) return 1;
    err->index = lo;
    err->array = 0;
    err->type = "string";
    return 0;
}

NEKO_API_DECL int neko_tolua_istable(lua_State *L, int lo, int def, neko_tolua_Error *err) {
    if (def && lua_gettop(L) < abs(lo)) return 1;
    if (lua_istable(L, lo)) return 1;
    err->index = lo;
    err->array = 0;
    err->type = "table";
    return 0;
}

NEKO_API_DECL int neko_tolua_isusertable(lua_State *L, int lo, const char *type, int def, neko_tolua_Error *err) {
    if (def && lua_gettop(L) < abs(lo)) return 1;
    if (lua_isusertable(L, lo, type)) return 1;
    err->index = lo;
    err->array = 0;
    err->type = type;
    return 0;
}

NEKO_API_DECL int neko_tolua_isuserdata(lua_State *L, int lo, int def, neko_tolua_Error *err) {
    if (def && lua_gettop(L) < abs(lo)) return 1;
    if (lua_isnil(L, lo) || lua_isuserdata(L, lo)) return 1;
    err->index = lo;
    err->array = 0;
    err->type = "userdata";
    return 0;
}

NEKO_API_DECL int neko_tolua_isvaluenil(lua_State *L, int lo, neko_tolua_Error *err) {

    if (lua_gettop(L) < abs(lo)) return 0; /* somebody else should chack this */
    if (!lua_isnil(L, lo)) return 0;

    err->index = lo;
    err->array = 0;
    err->type = "value";
    return 1;
};

NEKO_API_DECL int neko_tolua_isvalue(lua_State *L, int lo, int def, neko_tolua_Error *err) {
    if (def || abs(lo) <= lua_gettop(L)) /* any valid index */
        return 1;
    err->index = lo;
    err->array = 0;
    err->type = "value";
    return 0;
}

NEKO_API_DECL int neko_tolua_isusertype(lua_State *L, int lo, const char *type, int def, neko_tolua_Error *err) {
    if (def && lua_gettop(L) < abs(lo)) return 1;
    if (lua_isnil(L, lo) || lua_isusertype(L, lo, type)) return 1;
    err->index = lo;
    err->array = 0;
    err->type = type;
    return 0;
}

NEKO_API_DECL int neko_tolua_isvaluearray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err) {
    if (!neko_tolua_istable(L, lo, def, err))
        return 0;
    else
        return 1;
}

NEKO_API_DECL int neko_tolua_isbooleanarray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err) {
    if (!neko_tolua_istable(L, lo, def, err))
        return 0;
    else {
        int i;
        for (i = 1; i <= dim; ++i) {
            lua_pushinteger(L, i);
            lua_gettable(L, lo);
            if (!(lua_isnil(L, -1) || lua_isboolean(L, -1)) && !(def && lua_isnil(L, -1))) {
                err->index = lo;
                err->array = 1;
                err->type = "boolean";
                return 0;
            }
            lua_pop(L, 1);
        }
    }
    return 1;
}

NEKO_API_DECL int neko_tolua_isnumberarray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err) {
    if (!neko_tolua_istable(L, lo, def, err))
        return 0;
    else {
        int i;
        for (i = 1; i <= dim; ++i) {
            lua_pushinteger(L, i);
            lua_gettable(L, lo);
            if (!lua_isnumber(L, -1) && !(def && lua_isnil(L, -1))) {
                err->index = lo;
                err->array = 1;
                err->type = "number";
                return 0;
            }
            lua_pop(L, 1);
        }
    }
    return 1;
}

NEKO_API_DECL int neko_tolua_isintegerarray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err) {
    if (!neko_tolua_istable(L, lo, def, err))
        return 0;
    else {
        int i;
        for (i = 1; i <= dim; ++i) {
            lua_pushinteger(L, i);
            lua_gettable(L, lo);
            if (!lua_isinteger(L, -1) && !(def && lua_isnil(L, -1))) {
                err->index = lo;
                err->array = 1;
                err->type = "integer";
                return 0;
            }
            lua_pop(L, 1);
        }
    }
    return 1;
}

NEKO_API_DECL int neko_tolua_isstringarray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err) {
    if (!neko_tolua_istable(L, lo, def, err))
        return 0;
    else {
        int i;
        for (i = 1; i <= dim; ++i) {
            lua_pushinteger(L, i);
            lua_gettable(L, lo);
            if (!(lua_isnil(L, -1) || lua_isstring(L, -1)) && !(def && lua_isnil(L, -1))) {
                err->index = lo;
                err->array = 1;
                err->type = "string";
                return 0;
            }
            lua_pop(L, 1);
        }
    }
    return 1;
}

NEKO_API_DECL int neko_tolua_istablearray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err) {
    if (!neko_tolua_istable(L, lo, def, err))
        return 0;
    else {
        int i;
        for (i = 1; i <= dim; ++i) {
            lua_pushinteger(L, i);
            lua_gettable(L, lo);
            if (!lua_istable(L, -1) && !(def && lua_isnil(L, -1))) {
                err->index = lo;
                err->array = 1;
                err->type = "table";
                return 0;
            }
            lua_pop(L, 1);
        }
    }
    return 1;
}

NEKO_API_DECL int neko_tolua_isuserdataarray(lua_State *L, int lo, int dim, int def, neko_tolua_Error *err) {
    if (!neko_tolua_istable(L, lo, def, err))
        return 0;
    else {
        int i;
        for (i = 1; i <= dim; ++i) {
            lua_pushinteger(L, i);
            lua_gettable(L, lo);
            if (!(lua_isnil(L, -1) || lua_isuserdata(L, -1)) && !(def && lua_isnil(L, -1))) {
                err->index = lo;
                err->array = 1;
                err->type = "userdata";
                return 0;
            }
            lua_pop(L, 1);
        }
    }
    return 1;
}

NEKO_API_DECL int neko_tolua_isusertypearray(lua_State *L, int lo, const char *type, int dim, int def, neko_tolua_Error *err) {
    if (!neko_tolua_istable(L, lo, def, err))
        return 0;
    else {
        int i;
        for (i = 1; i <= dim; ++i) {
            lua_pushinteger(L, i);
            lua_gettable(L, lo);
            if (!(lua_isnil(L, -1) || lua_isuserdata(L, -1)) && !(def && lua_isnil(L, -1))) {
                err->index = lo;
                err->type = type;
                err->array = 1;
                return 0;
            }
            lua_pop(L, 1);
        }
    }
    return 1;
}

#if 0
int neko_tolua_isbooleanfield
 (lua_State* L, int lo, int i, int def, neko_tolua_Error* err)
{
	lua_pushnumber(L,i);
	lua_gettable(L,lo);
	if (!(lua_isnil(L,-1) || lua_isboolean(L,-1)) &&
			  !(def && lua_isnil(L,-1))
				)
	{
		err->index = lo;
		err->array = 1;
		err->type = "boolean";
		return 0;
	}
	lua_pop(L,1);
 return 1;
}

int neko_tolua_isnumberfield
 (lua_State* L, int lo, int i, int def, neko_tolua_Error* err)
{
	lua_pushnumber(L,i);
	lua_gettable(L,lo);
	if (!lua_isnumber(L,-1) &&
			  !(def && lua_isnil(L,-1))
				)
	{
		err->index = lo;
		err->array = 1;
		err->type = "number";
		return 0;
	}
	lua_pop(L,1);
 return 1;
}

int neko_tolua_isstringfield
 (lua_State* L, int lo, int i, int def, neko_tolua_Error* err)
{
	lua_pushnumber(L,i);
	lua_gettable(L,lo);
 if (!(lua_isnil(L,-1) || lua_isstring(L,-1)) &&
	    !(def && lua_isnil(L,-1))
				)
	{
		err->index = lo;
		err->array = 1;
		err->type = "string";
		return 0;
	}
	lua_pop(L,1);
 return 1;
}

int neko_tolua_istablefield
 (lua_State* L, int lo, int i, int def, neko_tolua_Error* err)
{
	lua_pushnumber(L,i+1);
	lua_gettable(L,lo);
	if (! lua_istable(L,-1) &&
	    !(def && lua_isnil(L,-1))
				)
	{
		err->index = lo;
		err->array = 1;
		err->type = "table";
		return 0;
	}
	lua_pop(L,1);
}

int neko_tolua_isusertablefield
 (lua_State* L, int lo, const char* type, int i, int def, neko_tolua_Error* err)
{
	lua_pushnumber(L,i);
	lua_gettable(L,lo);
	if (! lua_isusertable(L,-1,type) &&
	    !(def && lua_isnil(L,-1))
				)
	{
		err->index = lo;
		err->array = 1;
		err->type = type;
		return 0;
	}
	lua_pop(L,1);
 return 1;
}

int neko_tolua_isuserdatafield
 (lua_State* L, int lo, int i, int def, neko_tolua_Error* err)
{
	lua_pushnumber(L,i);
	lua_gettable(L,lo);
	if (!(lua_isnil(L,-1) || lua_isuserdata(L,-1)) &&
	    !(def && lua_isnil(L,-1))
				)
	{
		err->index = lo;
		err->array = 1;
		err->type = "userdata";
		return 0;
	}
	lua_pop(L,1);
 return 1;
}

int neko_tolua_isusertypefield
 (lua_State* L, int lo, const char* type, int i, int def, neko_tolua_Error* err)
{
	lua_pushnumber(L,i);
	lua_gettable(L,lo);
	if (!(lua_isnil(L,-1) || lua_isusertype(L,-1,type)) &&
	    !(def && lua_isnil(L,-1))
				)
	{
		err->index = lo;
		err->type = type;
		err->array = 1;
		return 0;
	}
	lua_pop(L,1);
 return 1;
}

#endif

/* Create metatable
 * Create and register new metatable
 */
static int neko_tolua_newmetatable(lua_State *L, const_str name) {
    int r = luaL_newmetatable(L, name);

#ifdef LUA_VERSION_NUM /* only lua 5.1 */
    if (r) {
        lua_pushvalue(L, -1);
        lua_pushstring(L, name);
        lua_settable(L, LUA_REGISTRYINDEX); /* reg[mt] = type_name */
    };
#endif

    if (r) neko_tolua_classevents(L); /* set meta events */
    lua_pop(L, 1);
    return r;
}

/* Map super classes
 * It sets 'name' as being also a 'base', mapping all super classes of 'base' in 'name'
 */
static void mapsuper(lua_State *L, const char *name, const char *base) {
    /* push registry.super */
    lua_pushstring(L, "neko_tolua_super");
    lua_rawget(L, LUA_REGISTRYINDEX); /* stack: super */
    luaL_getmetatable(L, name);       /* stack: super mt */
    lua_rawget(L, -2);                /* stack: super table */
    if (lua_isnil(L, -1)) {
        /* create table */
        lua_pop(L, 1);
        lua_newtable(L);            /* stack: super table */
        luaL_getmetatable(L, name); /* stack: super table mt */
        lua_pushvalue(L, -2);       /* stack: super table mt table */
        lua_rawset(L, -4);          /* stack: super table */
    }

    /* set base as super class */
    lua_pushstring(L, base);
    lua_pushboolean(L, 1);
    lua_rawset(L, -3); /* stack: super table */

    /* set all super class of base as super class of name */
    luaL_getmetatable(L, base); /* stack: super table base_mt */
    lua_rawget(L, -3);          /* stack: super table base_table */
    if (lua_istable(L, -1)) {
        /* traverse base table */
        lua_pushnil(L); /* first key */
        while (lua_next(L, -2) != 0) {
            /* stack: ... base_table key value */
            lua_pushvalue(L, -2); /* stack: ... base_table key value key */
            lua_insert(L, -2);    /* stack: ... base_table key key value */
            lua_rawset(L, -5);    /* stack: ... base_table key */
        }
    }
    lua_pop(L, 3); /* stack: <empty> */
}

/* creates a 'neko_tolua_ubox' table for base clases, and
// expects the metatable and base metatable on the stack */
static void set_ubox(lua_State *L) {

    /* mt basemt */
    if (!lua_isnil(L, -1)) {
        lua_pushstring(L, "neko_tolua_ubox");
        lua_rawget(L, -2);
    } else {
        lua_pushnil(L);
    };
    /* mt basemt base_ubox */
    if (!lua_isnil(L, -1)) {
        lua_pushstring(L, "neko_tolua_ubox");
        lua_insert(L, -2);
        /* mt basemt key ubox */
        lua_rawset(L, -4);
        /* (mt with ubox) basemt */
    } else {
        /* mt basemt nil */
        lua_pop(L, 1);
        lua_pushstring(L, "neko_tolua_ubox");
        lua_newtable(L);
        /* make weak value metatable for ubox table to allow userdata to be
        garbage-collected */
        lua_newtable(L);
        lua_pushliteral(L, "__mode");
        lua_pushliteral(L, "v");
        lua_rawset(L, -3);       /* stack: string ubox mt */
        lua_setmetatable(L, -2); /* stack:mt basemt string ubox */
        lua_rawset(L, -4);
    };
};

/* Map inheritance
 * It sets 'name' as derived from 'base' by setting 'base' as metatable of 'name'
 */
static void mapinheritance(lua_State *L, const char *name, const char *base) {
    /* set metatable inheritance */
    luaL_getmetatable(L, name);

    if (base && *base)
        luaL_getmetatable(L, base);
    else {

        if (lua_getmetatable(L, -1)) { /* already has a mt, we don't overwrite it */
            lua_pop(L, 2);
            return;
        };
        luaL_getmetatable(L, "neko_tolua_commonclass");
    };

    set_ubox(L);

    lua_setmetatable(L, -2);
    lua_pop(L, 1);
}

/* Object type
 */
static int neko_tolua_bnd_type(lua_State *L) {
    neko_tolua_typename(L, lua_gettop(L));
    return 1;
}

/* Take ownership
 */
static int neko_tolua_bnd_takeownership(lua_State *L) {
    int success = 0;
    if (lua_isuserdata(L, 1)) {
        if (lua_getmetatable(L, 1)) /* if metatable? */
        {
            lua_pop(L, 1); /* clear metatable off stack */
/* force garbage collection to avoid C to reuse a to-be-collected address */
#ifdef LUA_VERSION_NUM
            lua_gc(L, LUA_GCCOLLECT, 0);
#else
            lua_setgcthreshold(L, 0);
#endif

            success = neko_tolua_register_gc(L, 1);
        }
    }
    lua_pushboolean(L, success != 0);
    return 1;
}

/* Release ownership
 */
static int neko_tolua_bnd_releaseownership(lua_State *L) {
    int done = 0;
    if (lua_isuserdata(L, 1)) {
        void *u = *((void **)lua_touserdata(L, 1));
/* force garbage collection to avoid releasing a to-be-collected address */
#ifdef LUA_VERSION_NUM
        lua_gc(L, LUA_GCCOLLECT, 0);
#else
        lua_setgcthreshold(L, 0);
#endif
        lua_pushstring(L, "neko_tolua_gc");
        lua_rawget(L, LUA_REGISTRYINDEX);
        lua_pushlightuserdata(L, u);
        lua_rawget(L, -2);
        lua_getmetatable(L, 1);
        if (lua_rawequal(L, -1, -2)) /* check that we are releasing the correct type */
        {
            lua_pushlightuserdata(L, u);
            lua_pushnil(L);
            lua_rawset(L, -5);
            done = 1;
        }
    }
    lua_pushboolean(L, done != 0);
    return 1;
}

/* Type casting
 */
static int neko_tolua_bnd_cast(lua_State *L) {

    /* // old code
            void* v = neko_tolua_tousertype(L,1,NULL);
            const char* s = neko_tolua_tostring(L,2,NULL);
            if (v && s)
             neko_tolua_pushusertype(L,v,s);
            else
             lua_pushnil(L);
            return 1;
    */

    void *v;
    const char *s;
    if (lua_islightuserdata(L, 1)) {
        v = neko_tolua_touserdata(L, 1, NULL);
    } else {
        v = neko_tolua_tousertype(L, 1, 0);
    };

    s = neko_tolua_tostring(L, 2, NULL);
    if (v && s)
        neko_tolua_pushusertype(L, v, s);
    else
        lua_pushnil(L);
    return 1;
}

/* Inheritance
 */
static int neko_tolua_bnd_inherit(lua_State *L) {

    /* stack: lua object, c object */
    lua_pushstring(L, ".c_instance");
    lua_pushvalue(L, -2);
    lua_rawset(L, -4);
    /* l_obj[".c_instance"] = c_obj */

    return 0;
};

#ifdef LUA_VERSION_NUM /* lua 5.1 */
static int neko_tolua_bnd_setpeer(lua_State *L) {

    /* stack: userdata, table */
    if (!lua_isuserdata(L, -2)) {
        lua_pushstring(L, "Invalid argument #1 to setpeer: userdata expected.");
        lua_error(L);
    };

    if (lua_isnil(L, -1)) {

        lua_pop(L, 1);
        lua_pushvalue(L, TOLUA_NOPEER);
    };
#if LUA_VERSION_NUM > 501
    lua_setuservalue(L, -1);
#else
    lua_setfenv(L, -2);
#endif

    return 0;
};

static int neko_tolua_bnd_getpeer(lua_State *L) {

    /* stack: userdata */
#if LUA_VERSION_NUM > 501
    lua_getuservalue(L, -1);
#else
    lua_getfenv(L, -1);
#endif
    if (lua_rawequal(L, -1, TOLUA_NOPEER)) {
        lua_pop(L, 1);
        lua_pushnil(L);
    };
    return 1;
};
#endif

/* static int class_gc_event (lua_State* L); */

NEKO_API_DECL void neko_tolua_open(lua_State *L) {
    int top = lua_gettop(L);
    lua_pushstring(L, "neko_tolua_opened");
    lua_rawget(L, LUA_REGISTRYINDEX);
    if (!lua_isboolean(L, -1)) {
        lua_pushstring(L, "neko_tolua_opened");
        lua_pushboolean(L, 1);
        lua_rawset(L, LUA_REGISTRYINDEX);

#ifndef LUA_VERSION_NUM /* only prior to lua 5.1 */
        /* create peer object table */
        lua_pushstring(L, "neko_tolua_peers");
        lua_newtable(L);
        /* make weak key metatable for peers indexed by userdata object */
        lua_newtable(L);
        lua_pushliteral(L, "__mode");
        lua_pushliteral(L, "k");
        lua_rawset(L, -3);       /* stack: string peers mt */
        lua_setmetatable(L, -2); /* stack: string peers */
        lua_rawset(L, LUA_REGISTRYINDEX);
#endif

        /* create object ptr -> udata mapping table */
        lua_pushstring(L, "neko_tolua_ubox");
        lua_newtable(L);
        /* make weak value metatable for ubox table to allow userdata to be
           garbage-collected */
        lua_newtable(L);
        lua_pushliteral(L, "__mode");
        lua_pushliteral(L, "v");
        lua_rawset(L, -3);       /* stack: string ubox mt */
        lua_setmetatable(L, -2); /* stack: string ubox */
        lua_rawset(L, LUA_REGISTRYINDEX);

        lua_pushstring(L, "neko_tolua_super");
        lua_newtable(L);
        lua_rawset(L, LUA_REGISTRYINDEX);
        lua_pushstring(L, "neko_tolua_gc");
        lua_newtable(L);
        lua_rawset(L, LUA_REGISTRYINDEX);

        /* create gc_event closure */
        lua_pushstring(L, "neko_tolua_gc_event");
        lua_pushstring(L, "neko_tolua_gc");
        lua_rawget(L, LUA_REGISTRYINDEX);
        lua_pushstring(L, "neko_tolua_super");
        lua_rawget(L, LUA_REGISTRYINDEX);
        lua_pushcclosure(L, class_gc_event, 2);
        lua_rawset(L, LUA_REGISTRYINDEX);

        neko_tolua_newmetatable(L, "neko_tolua_commonclass");

        neko_tolua_module(L, NULL, 0);
        neko_tolua_beginmodule(L, NULL);
        neko_tolua_module(L, "tolua", 0);
        neko_tolua_beginmodule(L, "tolua");
        neko_tolua_function(L, "type", neko_tolua_bnd_type);
        neko_tolua_function(L, "takeownership", neko_tolua_bnd_takeownership);
        neko_tolua_function(L, "releaseownership", neko_tolua_bnd_releaseownership);
        neko_tolua_function(L, "cast", neko_tolua_bnd_cast);
        neko_tolua_function(L, "inherit", neko_tolua_bnd_inherit);
#ifdef LUA_VERSION_NUM /* lua 5.1 */
        neko_tolua_function(L, "setpeer", neko_tolua_bnd_setpeer);
        neko_tolua_function(L, "getpeer", neko_tolua_bnd_getpeer);
#endif

        neko_tolua_endmodule(L);
        neko_tolua_endmodule(L);
    }
    lua_settop(L, top);
}

/* Copy a C object
 */
NEKO_API_DECL void *neko_tolua_copy(lua_State *L, void *value, unsigned int size) {
    void *clone = (void *)malloc(size);
    if (clone)
        memcpy(clone, value, size);
    else
        neko_tolua_error(L, "insuficient memory", NULL);
    return clone;
}

/* Default collect function
 */
NEKO_API_DECL int neko_tolua_default_collect(lua_State *L) {
    void *self = neko_tolua_tousertype(L, 1, 0);
    free(self);
    return 0;
}

/* Do clone
 */
NEKO_API_DECL int neko_tolua_register_gc(lua_State *L, int lo) {
    int success = 1;
    void *value = *(void **)lua_touserdata(L, lo);
    lua_pushstring(L, "neko_tolua_gc");
    lua_rawget(L, LUA_REGISTRYINDEX);
    lua_pushlightuserdata(L, value);
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1)) /* make sure that object is not already owned */
        success = 0;
    else {
        lua_pushlightuserdata(L, value);
        lua_getmetatable(L, lo);
        lua_rawset(L, -4);
    }
    lua_pop(L, 2);
    return success;
}

/* Register a usertype
 * It creates the correspoding metatable in the registry, for both 'type' and 'const type'.
 * It maps 'const type' as being also a 'type'
 */
NEKO_API_DECL void neko_tolua_usertype(lua_State *L, const char *type) {
    char ctype[128] = "const ";
    strncat(ctype, type, 120);

    /* create both metatables */
    if (neko_tolua_newmetatable(L, ctype) && neko_tolua_newmetatable(L, type)) mapsuper(L, type, ctype); /* 'type' is also a 'const type' */
}

/* Begin module
 * It pushes the module (or class) table on the stack
 */
NEKO_API_DECL void neko_tolua_beginmodule(lua_State *L, const char *name) {
    if (name) {
        lua_pushstring(L, name);
        lua_rawget(L, -2);
    } else
        /*	 lua_pushvalue(L,LUA_GLOBALSINDEX);*/
        lua_pushglobaltable(L);
}

/* End module
 * It pops the module (or class) from the stack
 */
NEKO_API_DECL void neko_tolua_endmodule(lua_State *L) { lua_pop(L, 1); }

/* Map module
 * It creates a new module
 */
#if 1
NEKO_API_DECL void neko_tolua_module(lua_State *L, const char *name, int hasvar) {
    if (name) {
        /* tolua module */
        lua_pushstring(L, name);
        lua_rawget(L, -2);
        if (!lua_istable(L, -1)) /* check if module already exists */
        {
            lua_pop(L, 1);
            lua_newtable(L);
            lua_pushstring(L, name);
            lua_pushvalue(L, -2);
            lua_rawset(L, -4); /* assing module into module */
        }
    } else {
        /* global table */
        /* lua_pushvalue(L,LUA_GLOBALSINDEX); */
        lua_pushglobaltable(L);
    }
    if (hasvar) {
        if (!neko_tolua_ismodulemetatable(L)) /* check if it already has a module metatable */
        {
            /* create metatable to get/set C/C++ variable */
            lua_newtable(L);
            neko_tolua_moduleevents(L);
            if (lua_getmetatable(L, -2)) lua_setmetatable(L, -2); /* set old metatable as metatable of metatable */
            lua_setmetatable(L, -2);
        }
    }
    lua_pop(L, 1); /* pop module */
}
#else
NEKO_API_DECL void neko_tolua_module(lua_State *L, const char *name, int hasvar) {
    if (name) {
        /* tolua module */
        lua_pushstring(L, name);
        lua_newtable(L);
    } else {
        /* global table */
        lua_pushvalue(L, LUA_GLOBALSINDEX);
    }
    if (hasvar) {
        /* create metatable to get/set C/C++ variable */
        lua_newtable(L);
        neko_tolua_moduleevents(L);
        if (lua_getmetatable(L, -2)) lua_setmetatable(L, -2); /* set old metatable as metatable of metatable */
        lua_setmetatable(L, -2);
    }
    if (name)
        lua_rawset(L, -3); /* assing module into module */
    else
        lua_pop(L, 1); /* pop global table */
}
#endif

static void push_collector(lua_State *L, const char *type, lua_CFunction col) {

    /* push collector function, but only if it's not NULL, or if there's no
       collector already */
    if (!col) return;
    luaL_getmetatable(L, type);
    lua_pushstring(L, ".collector");
    /*
    if (!col) {
        lua_pushvalue(L, -1);
        lua_rawget(L, -3);
        if (!lua_isnil(L, -1)) {
            lua_pop(L, 3);
            return;
        };
        lua_pop(L, 1);
    };
    //	*/
    lua_pushcfunction(L, col);

    lua_rawset(L, -3);
    lua_pop(L, 1);
};

/* Map C class
 * It maps a C class, setting the appropriate inheritance and super classes.
 */
NEKO_API_DECL void neko_tolua_cclass(lua_State *L, const char *lname, const char *name, const char *base, lua_CFunction col) {
    char cname[128] = "const ";
    char cbase[128] = "const ";
    strncat(cname, name, 120);
    strncat(cbase, base, 120);

    mapinheritance(L, name, base);
    mapinheritance(L, cname, name);

    mapsuper(L, cname, cbase);
    mapsuper(L, name, base);

    lua_pushstring(L, lname);

    push_collector(L, name, col);
    /*
    luaL_getmetatable(L,name);
    lua_pushstring(L,".collector");
    lua_pushcfunction(L,col);

    lua_rawset(L,-3);
    */

    luaL_getmetatable(L, name);
    lua_rawset(L, -3); /* assign class metatable to module */

    /* now we also need to store the collector table for the const
       instances of the class */
    push_collector(L, cname, col);
    /*
    luaL_getmetatable(L,cname);
    lua_pushstring(L,".collector");
    lua_pushcfunction(L,col);
    lua_rawset(L,-3);
    lua_pop(L,1);
    */
}

/* Add base
    * It adds additional base classes to a class (for multiple inheritance)
    * (not for now)
NEKO_API_DECL void neko_tolua_addbase(lua_State* L, char* name, char* base) {

    char cname[128] = "const ";
    char cbase[128] = "const ";
    strncat(cname,name,120);
    strncat(cbase,base,120);

    mapsuper(L,cname,cbase);
    mapsuper(L,name,base);
};
*/

/* Map function
 * It assigns a function into the current module (or class)
 */
NEKO_API_DECL void neko_tolua_function(lua_State *L, const char *name, lua_CFunction func) {
    lua_pushstring(L, name);
    lua_pushcfunction(L, func);
    lua_rawset(L, -3);
}

/* sets the __call event for the class (expects the class' main table on top) */
/*	never really worked :(
NEKO_API_DECL void neko_tolua_set_call_event(lua_State* L, lua_CFunction func, char* type) {

    lua_getmetatable(L, -1);
    //luaL_getmetatable(L, type);
    lua_pushstring(L,"__call");
    lua_pushcfunction(L,func);
    lua_rawset(L,-3);
    lua_pop(L, 1);
};
*/

/* Map constant number
 * It assigns a constant number into the current module (or class)
 */
NEKO_API_DECL void neko_tolua_constant(lua_State *L, const char *name, lua_Number value) {
    lua_pushstring(L, name);
    if ((long long)value == value)
        neko_tolua_pushinteger(L, value);
    else
        neko_tolua_pushnumber(L, value);
    lua_rawset(L, -3);
}

/* Map variable
 * It assigns a variable into the current module (or class)
 */
NEKO_API_DECL void neko_tolua_variable(lua_State *L, const char *name, lua_CFunction get, lua_CFunction set) {
    /* get func */
    lua_pushstring(L, ".get");
    lua_rawget(L, -2);
    if (!lua_istable(L, -1)) {
        /* create .get table, leaving it at the top */
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushstring(L, ".get");
        lua_pushvalue(L, -2);
        lua_rawset(L, -4);
    }
    lua_pushstring(L, name);
    lua_pushcfunction(L, get);
    lua_rawset(L, -3); /* store variable */
    lua_pop(L, 1);     /* pop .get table */

    /* set func */
    if (set) {
        lua_pushstring(L, ".set");
        lua_rawget(L, -2);
        if (!lua_istable(L, -1)) {
            /* create .set table, leaving it at the top */
            lua_pop(L, 1);
            lua_newtable(L);
            lua_pushstring(L, ".set");
            lua_pushvalue(L, -2);
            lua_rawset(L, -4);
        }
        lua_pushstring(L, name);
        lua_pushcfunction(L, set);
        lua_rawset(L, -3); /* store variable */
        lua_pop(L, 1);     /* pop .set table */
    }
}

/* Access const array
 * It reports an error when trying to write into a const array
 */
static int const_array(lua_State *L) {
    luaL_error(L, "value of const array cannot be changed");
    return 0;
}

/* Map an array
 * It assigns an array into the current module (or class)
 */
NEKO_API_DECL void neko_tolua_array(lua_State *L, const char *name, lua_CFunction get, lua_CFunction set) {
    lua_pushstring(L, ".get");
    lua_rawget(L, -2);
    if (!lua_istable(L, -1)) {
        /* create .get table, leaving it at the top */
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushstring(L, ".get");
        lua_pushvalue(L, -2);
        lua_rawset(L, -4);
    }
    lua_pushstring(L, name);

    lua_newtable(L); /* create array metatable */
    lua_pushvalue(L, -1);
    lua_setmetatable(L, -2); /* set the own table as metatable (for modules) */
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, get);
    lua_rawset(L, -3);
    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, set ? set : const_array);
    lua_rawset(L, -3);

    lua_rawset(L, -3); /* store variable */
    lua_pop(L, 1);     /* pop .get table */
}

NEKO_API_DECL void neko_tolua_dobuffer(lua_State *L, char *B, unsigned int size, const char *name) { luaL_loadbuffer(L, B, size, name) || lua_pcall(L, 0, 0, 0); };

NEKO_API_DECL void neko_tolua_pushvalue(lua_State *L, int lo) { lua_pushvalue(L, lo); }

NEKO_API_DECL void neko_tolua_pushboolean(lua_State *L, int value) { lua_pushboolean(L, value); }

NEKO_API_DECL void neko_tolua_pushnumber(lua_State *L, lua_Number value) { lua_pushnumber(L, value); }

NEKO_API_DECL void neko_tolua_pushinteger(lua_State *L, lua_Integer value) { lua_pushinteger(L, value); }

NEKO_API_DECL void neko_tolua_pushstring(lua_State *L, const char *value) {
    if (value == NULL)
        lua_pushnil(L);
    else
        lua_pushstring(L, value);
}

NEKO_API_DECL void neko_tolua_pushuserdata(lua_State *L, void *value) {
    if (value == NULL)
        lua_pushnil(L);
    else
        lua_pushlightuserdata(L, value);
}

NEKO_API_DECL void neko_tolua_pushusertype(lua_State *L, void *value, const char *type) {
    if (value == NULL)
        lua_pushnil(L);
    else {
        luaL_getmetatable(L, type);
        lua_pushstring(L, "neko_tolua_ubox");
        lua_rawget(L, -2); /* stack: mt ubox */
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            lua_pushstring(L, "neko_tolua_ubox");
            lua_rawget(L, LUA_REGISTRYINDEX);
        };
        lua_pushlightuserdata(L, value);
        lua_rawget(L, -2); /* stack: mt ubox ubox[u] */
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1); /* stack: mt ubox */
            lua_pushlightuserdata(L, value);
            *(void **)lua_newuserdata(L, sizeof(void *)) = value; /* stack: mt ubox u newud */
            lua_pushvalue(L, -1);                                 /* stack: mt ubox u newud newud */
            lua_insert(L, -4);                                    /* stack: mt newud ubox u newud */
            lua_rawset(L, -3);                                    /* stack: mt newud ubox */
            lua_pop(L, 1);                                        /* stack: mt newud */
                                                                  /*luaL_getmetatable(L,type);*/
            lua_pushvalue(L, -2);                                 /* stack: mt newud mt */
            lua_setmetatable(L, -2);                              /* stack: mt newud */
        } else {
            /* check the need of updating the metatable to a more specialized class */
            lua_insert(L, -2); /* stack: mt ubox[u] ubox */
            lua_pop(L, 1);     /* stack: mt ubox[u] */
            lua_pushstring(L, "neko_tolua_super");
            lua_rawget(L, LUA_REGISTRYINDEX); /* stack: mt ubox[u] super */
            lua_getmetatable(L, -2);          /* stack: mt ubox[u] super mt */
            lua_rawget(L, -2);                /* stack: mt ubox[u] super super[mt] */
            if (lua_istable(L, -1)) {
                lua_pushstring(L, type);       /* stack: mt ubox[u] super super[mt] type */
                lua_rawget(L, -2);             /* stack: mt ubox[u] super super[mt] flag */
                if (lua_toboolean(L, -1) == 1) /* if true */
                {
                    lua_pop(L, 3); /* mt ubox[u]*/
                } else {
                    /* type represents a more specilized type */
                    /*luaL_getmetatable(L,type);             // stack: mt ubox[u] super super[mt] flag mt */
                    lua_pushvalue(L, -5);    /* stack: mt ubox[u] super super[mt] flag mt */
                    lua_setmetatable(L, -5); /* stack: mt ubox[u] super super[mt] flag */
                    lua_pop(L, 3);           /* stack: mt ubox[u] */
                }
            } else {
                lua_pop(L, 2); /* stack: mt ubox[u] */
            }
        }
#if LUA_VERSION_NUM == 501
        lua_pushvalue(L, TOLUA_NOPEER);
        lua_setfenv(L, -2);
#elif LUA_VERSION_NUM > 501
        lua_pushvalue(L, TOLUA_NOPEER);
        lua_setuservalue(L, -2);
#endif

        lua_remove(L, -2); /* stack: ubox[u]*/
    }
}

NEKO_API_DECL void neko_tolua_pushusertype_and_takeownership(lua_State *L, void *value, const char *type) {
    neko_tolua_pushusertype(L, value, type);
    neko_tolua_register_gc(L, lua_gettop(L));
}

NEKO_API_DECL void neko_tolua_pushfieldvalue(lua_State *L, int lo, int index, int v) {
    lua_pushnumber(L, index);
    lua_pushvalue(L, v);
    lua_settable(L, lo);
}

NEKO_API_DECL void neko_tolua_pushfieldboolean(lua_State *L, int lo, int index, int v) {
    lua_pushnumber(L, index);
    lua_pushboolean(L, v);
    lua_settable(L, lo);
}

NEKO_API_DECL void neko_tolua_pushfieldnumber(lua_State *L, int lo, int index, lua_Number v) {
    lua_pushnumber(L, index);
    neko_tolua_pushnumber(L, v);
    lua_settable(L, lo);
}

NEKO_API_DECL void neko_tolua_pushfieldinteger(lua_State *L, int lo, int index, lua_Integer v) {
    lua_pushinteger(L, index);
    neko_tolua_pushinteger(L, v);
    lua_settable(L, lo);
}

NEKO_API_DECL void neko_tolua_pushfieldstring(lua_State *L, int lo, int index, const char *v) {
    lua_pushnumber(L, index);
    neko_tolua_pushstring(L, v);
    lua_settable(L, lo);
}

NEKO_API_DECL void neko_tolua_pushfielduserdata(lua_State *L, int lo, int index, void *v) {
    lua_pushnumber(L, index);
    neko_tolua_pushuserdata(L, v);
    lua_settable(L, lo);
}

NEKO_API_DECL void neko_tolua_pushfieldusertype(lua_State *L, int lo, int index, void *v, const char *type) {
    lua_pushnumber(L, index);
    neko_tolua_pushusertype(L, v, type);
    lua_settable(L, lo);
}

NEKO_API_DECL void neko_tolua_pushfieldusertype_and_takeownership(lua_State *L, int lo, int index, void *v, const char *type) {
    lua_pushnumber(L, index);
    neko_tolua_pushusertype(L, v, type);
    neko_tolua_register_gc(L, lua_gettop(L));
    lua_settable(L, lo);
}

NEKO_API_DECL lua_Number neko_tolua_tonumber(lua_State *L, int narg, lua_Number def) { return lua_gettop(L) < abs(narg) ? def : lua_tonumber(L, narg); }

NEKO_API_DECL lua_Integer neko_tolua_tointeger(lua_State *L, int narg, lua_Integer def) { return lua_gettop(L) < abs(narg) ? def : lua_tointeger(L, narg); }

NEKO_API_DECL const char *neko_tolua_tostring(lua_State *L, int narg, const char *def) { return lua_gettop(L) < abs(narg) ? def : lua_tostring(L, narg); }

NEKO_API_DECL void *neko_tolua_touserdata(lua_State *L, int narg, void *def) {

    /* return lua_gettop(L)<abs(narg) ? def : lua_touserdata(L,narg); */

    if (lua_gettop(L) < abs(narg)) {
        return def;
    };

    if (lua_islightuserdata(L, narg)) {

        return lua_touserdata(L, narg);
    };

    return neko_tolua_tousertype(L, narg, def);
}

extern int push_table_instance(lua_State *L, int lo);

NEKO_API_DECL void *neko_tolua_tousertype(lua_State *L, int narg, void *def) {
    if (lua_gettop(L) < abs(narg))
        return def;
    else {
        void *u;
        if (!lua_isuserdata(L, narg)) {
            if (!push_table_instance(L, narg)) return NULL;
        };
        u = lua_touserdata(L, narg);
        return (u == NULL) ? NULL : *((void **)u); /* nil represents NULL */
    }
}

NEKO_API_DECL int neko_tolua_tovalue(lua_State *L, int narg, int def) { return lua_gettop(L) < abs(narg) ? def : narg; }

NEKO_API_DECL int neko_tolua_toboolean(lua_State *L, int narg, int def) { return lua_gettop(L) < abs(narg) ? def : lua_toboolean(L, narg); }

NEKO_API_DECL lua_Number neko_tolua_tofieldnumber(lua_State *L, int lo, int index, lua_Number def) {
    double v;
    lua_pushnumber(L, index);
    lua_gettable(L, lo);
    v = lua_isnil(L, -1) ? def : lua_tonumber(L, -1);
    lua_pop(L, 1);
    return v;
}

NEKO_API_DECL lua_Integer neko_tolua_tofieldinteger(lua_State *L, int lo, int index, lua_Integer def) {
    lua_Integer v;
    lua_pushinteger(L, index);
    lua_gettable(L, lo);
    v = lua_isnil(L, -1) ? def : lua_tointeger(L, -1);
    lua_pop(L, 1);
    return v;
}

NEKO_API_DECL const char *neko_tolua_tofieldstring(lua_State *L, int lo, int index, const char *def) {
    const char *v;
    lua_pushnumber(L, index);
    lua_gettable(L, lo);
    v = lua_isnil(L, -1) ? def : lua_tostring(L, -1);
    lua_pop(L, 1);
    return v;
}

NEKO_API_DECL void *neko_tolua_tofielduserdata(lua_State *L, int lo, int index, void *def) {
    void *v;
    lua_pushnumber(L, index);
    lua_gettable(L, lo);
    v = lua_isnil(L, -1) ? def : lua_touserdata(L, -1);
    lua_pop(L, 1);
    return v;
}

NEKO_API_DECL void *neko_tolua_tofieldusertype(lua_State *L, int lo, int index, void *def) {
    void *v;
    lua_pushnumber(L, index);
    lua_gettable(L, lo);
    v = lua_isnil(L, -1) ? def : (*(void **)(lua_touserdata(L, -1))); /* lua_unboxpointer(L,-1); */
    lua_pop(L, 1);
    return v;
}

NEKO_API_DECL int neko_tolua_tofieldvalue(lua_State *L, int lo, int index, int def) {
    int v;
    lua_pushnumber(L, index);
    lua_gettable(L, lo);
    v = lua_isnil(L, -1) ? def : lo;
    lua_pop(L, 1);
    return v;
}

NEKO_API_DECL int neko_tolua_getfieldboolean(lua_State *L, int lo, int index, int def) {
    int v;
    lua_pushnumber(L, index);
    lua_gettable(L, lo);
    v = lua_isnil(L, -1) ? 0 : lua_toboolean(L, -1);
    lua_pop(L, 1);
    return v;
}

#endif

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
        NEKO_ERROR("%s", err.c_str());
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
