
#include "neko_lua.h"

#include <setjmp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "engine/neko.h"

#pragma region LuaA

void __neko_lua_auto_init(lua_State *L) {

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
    neko_lua_auto_conversion_type(L, neko_lua_auto_type_add(L, "bool", sizeof(bool)), neko_lua_auto_push_bool, neko_lua_auto_to_bool);
    neko_lua_auto_conversion_type(L, neko_lua_auto_type_add(L, "_Bool", sizeof(bool)), neko_lua_auto_push_bool, neko_lua_auto_to_bool);
    neko_lua_auto_conversion(L, char, neko_lua_auto_push_char, neko_lua_auto_to_char);
    neko_lua_auto_conversion(L, signed char, neko_lua_auto_push_signed_char, neko_lua_auto_to_signed_char);
    neko_lua_auto_conversion(L, unsigned char, neko_lua_auto_push_unsigned_char, neko_lua_auto_to_unsigned_char);
    neko_lua_auto_conversion(L, short, neko_lua_auto_push_short, neko_lua_auto_to_short);
    neko_lua_auto_conversion(L, unsigned short, neko_lua_auto_push_unsigned_short, neko_lua_auto_to_unsigned_short);
    neko_lua_auto_conversion(L, int, neko_lua_auto_push_int, neko_lua_auto_to_int);
    neko_lua_auto_conversion(L, unsigned int, neko_lua_auto_push_unsigned_int, neko_lua_auto_to_unsigned_int);
    neko_lua_auto_conversion(L, long, neko_lua_auto_push_long, neko_lua_auto_to_long);
    neko_lua_auto_conversion(L, s32, neko_lua_auto_push_long, neko_lua_auto_to_long);
    neko_lua_auto_conversion(L, unsigned long, neko_lua_auto_push_unsigned_long, neko_lua_auto_to_unsigned_long);
    neko_lua_auto_conversion(L, u32, neko_lua_auto_push_unsigned_long, neko_lua_auto_to_unsigned_long);
    neko_lua_auto_conversion(L, long long, neko_lua_auto_push_long_long, neko_lua_auto_to_long_long);
    neko_lua_auto_conversion(L, s64, neko_lua_auto_push_long_long, neko_lua_auto_to_long_long);
    neko_lua_auto_conversion(L, unsigned long long, neko_lua_auto_push_unsigned_long_long, neko_lua_auto_to_unsigned_long_long);
    neko_lua_auto_conversion(L, u64, neko_lua_auto_push_unsigned_long_long, neko_lua_auto_to_unsigned_long_long);
    neko_lua_auto_conversion(L, float, neko_lua_auto_push_float, neko_lua_auto_to_float);
    neko_lua_auto_conversion(L, f32, neko_lua_auto_push_float, neko_lua_auto_to_float);
    neko_lua_auto_conversion(L, double, neko_lua_auto_push_double, neko_lua_auto_to_double);
    neko_lua_auto_conversion(L, f64, neko_lua_auto_push_double, neko_lua_auto_to_double);
    neko_lua_auto_conversion(L, long double, neko_lua_auto_push_long_double, neko_lua_auto_to_long_double);

    neko_lua_auto_conversion_push_type(L, neko_lua_auto_type_add(L, "const bool", sizeof(bool)), neko_lua_auto_push_bool);
    neko_lua_auto_conversion_push_type(L, neko_lua_auto_type_add(L, "const _Bool", sizeof(bool)), neko_lua_auto_push_bool);
    neko_lua_auto_conversion_push(L, const char, neko_lua_auto_push_char);
    neko_lua_auto_conversion_push(L, const signed char, neko_lua_auto_push_signed_char);
    neko_lua_auto_conversion_push(L, const unsigned char, neko_lua_auto_push_unsigned_char);
    neko_lua_auto_conversion_push(L, const short, neko_lua_auto_push_short);
    neko_lua_auto_conversion_push(L, const unsigned short, neko_lua_auto_push_unsigned_short);
    neko_lua_auto_conversion_push(L, const int, neko_lua_auto_push_int);
    neko_lua_auto_conversion_push(L, const unsigned int, neko_lua_auto_push_unsigned_int);
    neko_lua_auto_conversion_push(L, const long, neko_lua_auto_push_long);
    neko_lua_auto_conversion_push(L, const unsigned long, neko_lua_auto_push_unsigned_long);
    neko_lua_auto_conversion_push(L, const long long, neko_lua_auto_push_long_long);
    neko_lua_auto_conversion_push(L, const unsigned long long, neko_lua_auto_push_unsigned_long_long);
    neko_lua_auto_conversion_push(L, const float, neko_lua_auto_push_float);
    neko_lua_auto_conversion_push(L, const double, neko_lua_auto_push_double);
    neko_lua_auto_conversion_push(L, const long double, neko_lua_auto_push_long_double);

    neko_lua_auto_conversion(L, char *, neko_lua_auto_push_char_ptr, neko_lua_auto_to_char_ptr);
    neko_lua_auto_conversion(L, const char *, neko_lua_auto_push_const_char_ptr, neko_lua_auto_to_const_char_ptr);
    neko_lua_auto_conversion(L, void *, neko_lua_auto_push_void_ptr, neko_lua_auto_to_void_ptr);

    neko_lua_auto_conversion_push_type(L, neko_lua_auto_type_add(L, "void", 1), neko_lua_auto_push_void);  // sizeof(void) is 1 on gcc
}

void __neko_lua_auto_fini(lua_State *L) {

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

neko_lua_auto_Type neko_lua_auto_type_add(lua_State *L, const char *type, size_t size) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_ids");
    lua_getfield(L, -1, type);

    if (lua_isnumber(L, -1)) {

        neko_lua_auto_Type id = lua_tointeger(L, -1);
        lua_pop(L, 2);
        return id;

    } else {

        lua_pop(L, 2);

        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_index");

        neko_lua_auto_Type id = lua_tointeger(L, -1);
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

neko_lua_auto_Type neko_lua_auto_type_find(lua_State *L, const char *type) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_ids");
    lua_getfield(L, -1, type);

    neko_lua_auto_Type id = lua_isnil(L, -1) ? LUAA_INVALID_TYPE : lua_tointeger(L, -1);
    lua_pop(L, 2);

    return id;
}

const char *neko_lua_auto_typename(lua_State *L, neko_lua_auto_Type id) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "type_names");
    lua_pushinteger(L, id);
    lua_gettable(L, -2);

    const char *type = lua_isnil(L, -1) ? "LUAA_INVALID_TYPE" : lua_tostring(L, -1);
    lua_pop(L, 2);

    return type;
}

size_t neko_lua_auto_typesize(lua_State *L, neko_lua_auto_Type id) {

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

int neko_lua_auto_push_type(lua_State *L, neko_lua_auto_Type type_id, const void *c_in) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "stack_push");
    lua_pushinteger(L, type_id);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {
        neko_lua_auto_Pushfunc func = (neko_lua_auto_Pushfunc)lua_touserdata(L, -1);
        lua_pop(L, 2);
        return func(L, type_id, c_in);
    }

    lua_pop(L, 2);

    if (neko_lua_auto_struct_registered_type(L, type_id)) {
        return neko_lua_auto_struct_push_type(L, type_id, c_in);
    }

    if (neko_lua_auto_enum_registered_type(L, type_id)) {
        return neko_lua_auto_enum_push_type(L, type_id, c_in);
    }

    lua_pushfstring(L, "neko_lua_auto_push: conversion to Lua object from type '%s' not registered!", neko_lua_auto_typename(L, type_id));
    lua_error(L);
    return 0;
}

void neko_lua_auto_to_type(lua_State *L, neko_lua_auto_Type type_id, void *c_out, int index) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "stack_to");
    lua_pushinteger(L, type_id);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {
        neko_lua_auto_Tofunc func = (neko_lua_auto_Tofunc)lua_touserdata(L, -1);
        lua_pop(L, 2);
        func(L, type_id, c_out, index);
        return;
    }

    lua_pop(L, 2);

    if (neko_lua_auto_struct_registered_type(L, type_id)) {
        neko_lua_auto_struct_to_type(L, type_id, c_out, index);
        return;
    }

    if (neko_lua_auto_enum_registered_type(L, type_id)) {
        neko_lua_auto_enum_to_type(L, type_id, c_out, index);
        return;
    }

    lua_pushfstring(L, "neko_lua_auto_to: conversion from Lua object to type '%s' not registered!", neko_lua_auto_typename(L, type_id));
    lua_error(L);
}

void neko_lua_auto_conversion_type(lua_State *L, neko_lua_auto_Type type_id, neko_lua_auto_Pushfunc push_func, neko_lua_auto_Tofunc to_func) {
    neko_lua_auto_conversion_push_type(L, type_id, push_func);
    neko_lua_auto_conversion_to_type(L, type_id, to_func);
}

void neko_lua_auto_conversion_push_type(lua_State *L, neko_lua_auto_Type type_id, neko_lua_auto_Pushfunc func) {
    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "stack_push");
    lua_pushinteger(L, type_id);
    lua_pushlightuserdata(L, func);
    lua_settable(L, -3);
    lua_pop(L, 1);
}

void neko_lua_auto_conversion_to_type(lua_State *L, neko_lua_auto_Type type_id, neko_lua_auto_Tofunc func) {
    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "stack_to");
    lua_pushinteger(L, type_id);
    lua_pushlightuserdata(L, func);
    lua_settable(L, -3);
    lua_pop(L, 1);
}

int neko_lua_auto_push_bool(lua_State *L, neko_lua_auto_Type type_id, const void *c_in) {
    lua_pushboolean(L, *(bool *)c_in);
    return 1;
}

void neko_lua_auto_to_bool(lua_State *L, neko_lua_auto_Type type_id, void *c_out, int index) { *(bool *)c_out = lua_toboolean(L, index); }

int neko_lua_auto_push_char(lua_State *L, neko_lua_auto_Type type_id, const void *c_in) {
    lua_pushinteger(L, *(char *)c_in);
    return 1;
}

void neko_lua_auto_to_char(lua_State *L, neko_lua_auto_Type type_id, void *c_out, int index) { *(char *)c_out = lua_tointeger(L, index); }

int neko_lua_auto_push_signed_char(lua_State *L, neko_lua_auto_Type type_id, const void *c_in) {
    lua_pushinteger(L, *(signed char *)c_in);
    return 1;
}

void neko_lua_auto_to_signed_char(lua_State *L, neko_lua_auto_Type type_id, void *c_out, int index) { *(signed char *)c_out = lua_tointeger(L, index); }

int neko_lua_auto_push_unsigned_char(lua_State *L, neko_lua_auto_Type type_id, const void *c_in) {
    lua_pushinteger(L, *(unsigned char *)c_in);
    return 1;
}

void neko_lua_auto_to_unsigned_char(lua_State *L, neko_lua_auto_Type type_id, void *c_out, int index) { *(unsigned char *)c_out = lua_tointeger(L, index); }

int neko_lua_auto_push_short(lua_State *L, neko_lua_auto_Type type_id, const void *c_in) {
    lua_pushinteger(L, *(short *)c_in);
    return 1;
}

void neko_lua_auto_to_short(lua_State *L, neko_lua_auto_Type type_id, void *c_out, int index) { *(short *)c_out = lua_tointeger(L, index); }

int neko_lua_auto_push_unsigned_short(lua_State *L, neko_lua_auto_Type type_id, const void *c_in) {
    lua_pushinteger(L, *(unsigned short *)c_in);
    return 1;
}

void neko_lua_auto_to_unsigned_short(lua_State *L, neko_lua_auto_Type type_id, void *c_out, int index) { *(unsigned short *)c_out = lua_tointeger(L, index); }

int neko_lua_auto_push_int(lua_State *L, neko_lua_auto_Type type_id, const void *c_in) {
    lua_pushinteger(L, *(int *)c_in);
    return 1;
}

void neko_lua_auto_to_int(lua_State *L, neko_lua_auto_Type type_id, void *c_out, int index) { *(int *)c_out = lua_tointeger(L, index); }

int neko_lua_auto_push_unsigned_int(lua_State *L, neko_lua_auto_Type type_id, const void *c_in) {
    lua_pushinteger(L, *(unsigned int *)c_in);
    return 1;
}

void neko_lua_auto_to_unsigned_int(lua_State *L, neko_lua_auto_Type type_id, void *c_out, int index) { *(unsigned int *)c_out = lua_tointeger(L, index); }

int neko_lua_auto_push_long(lua_State *L, neko_lua_auto_Type type_id, const void *c_in) {
    lua_pushinteger(L, *(long *)c_in);
    return 1;
}

void neko_lua_auto_to_long(lua_State *L, neko_lua_auto_Type type_id, void *c_out, int index) { *(long *)c_out = lua_tointeger(L, index); }

int neko_lua_auto_push_unsigned_long(lua_State *L, neko_lua_auto_Type type_id, const void *c_in) {
    lua_pushinteger(L, *(unsigned long *)c_in);
    return 1;
}

void neko_lua_auto_to_unsigned_long(lua_State *L, neko_lua_auto_Type type_id, void *c_out, int index) { *(unsigned long *)c_out = lua_tointeger(L, index); }

int neko_lua_auto_push_long_long(lua_State *L, neko_lua_auto_Type type_id, const void *c_in) {
    lua_pushinteger(L, *(long long *)c_in);
    return 1;
}

void neko_lua_auto_to_long_long(lua_State *L, neko_lua_auto_Type type_id, void *c_out, int index) { *(long long *)c_out = lua_tointeger(L, index); }

int neko_lua_auto_push_unsigned_long_long(lua_State *L, neko_lua_auto_Type type_id, const void *c_in) {
    lua_pushinteger(L, *(unsigned long long *)c_in);
    return 1;
}

void neko_lua_auto_to_unsigned_long_long(lua_State *L, neko_lua_auto_Type type_id, void *c_out, int index) { *(unsigned long long *)c_out = lua_tointeger(L, index); }

int neko_lua_auto_push_float(lua_State *L, neko_lua_auto_Type type_id, const void *c_in) {
    lua_pushnumber(L, *(float *)c_in);
    return 1;
}

void neko_lua_auto_to_float(lua_State *L, neko_lua_auto_Type type_id, void *c_out, int index) { *(float *)c_out = lua_tonumber(L, index); }

int neko_lua_auto_push_double(lua_State *L, neko_lua_auto_Type type_id, const void *c_in) {
    lua_pushnumber(L, *(double *)c_in);
    return 1;
}

void neko_lua_auto_to_double(lua_State *L, neko_lua_auto_Type type_id, void *c_out, int index) { *(double *)c_out = lua_tonumber(L, index); }

int neko_lua_auto_push_long_double(lua_State *L, neko_lua_auto_Type type_id, const void *c_in) {
    lua_pushnumber(L, *(long double *)c_in);
    return 1;
}

void neko_lua_auto_to_long_double(lua_State *L, neko_lua_auto_Type type_id, void *c_out, int index) { *(long double *)c_out = lua_tonumber(L, index); }

int neko_lua_auto_push_char_ptr(lua_State *L, neko_lua_auto_Type type_id, const void *c_in) {
    lua_pushstring(L, *(char **)c_in);
    return 1;
}

void neko_lua_auto_to_char_ptr(lua_State *L, neko_lua_auto_Type type_id, void *c_out, int index) { *(char **)c_out = (char *)lua_tostring(L, index); }

int neko_lua_auto_push_const_char_ptr(lua_State *L, neko_lua_auto_Type type_id, const void *c_in) {
    lua_pushstring(L, *(const char **)c_in);
    return 1;
}

void neko_lua_auto_to_const_char_ptr(lua_State *L, neko_lua_auto_Type type_id, void *c_out, int index) { *(const char **)c_out = lua_tostring(L, index); }

int neko_lua_auto_push_void_ptr(lua_State *L, neko_lua_auto_Type type_id, const void *c_in) {
    lua_pushlightuserdata(L, *(void **)c_in);
    return 1;
}

void neko_lua_auto_to_void_ptr(lua_State *L, neko_lua_auto_Type type_id, void *c_out, int index) { *(void **)c_out = (void *)lua_touserdata(L, index); }

int neko_lua_auto_push_void(lua_State *L, neko_lua_auto_Type type_id, const void *c_in) {
    lua_pushnil(L);
    return 1;
}

bool neko_lua_auto_conversion_registered_type(lua_State *L, neko_lua_auto_Type type_id) {
    return (neko_lua_auto_conversion_push_registered_type(L, type_id) && neko_lua_auto_conversion_to_registered_type(L, type_id));
}

bool neko_lua_auto_conversion_push_registered_type(lua_State *L, neko_lua_auto_Type type_id) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "stack_push");
    lua_pushinteger(L, type_id);
    lua_gettable(L, -2);

    bool reg = !lua_isnil(L, -1);
    lua_pop(L, 2);

    return reg;
}

bool neko_lua_auto_conversion_to_registered_type(lua_State *L, neko_lua_auto_Type type_id) {

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

int neko_lua_auto_struct_push_member_offset_type(lua_State *L, neko_lua_auto_Type type, size_t offset, const void *c_in) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs_offset");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_pushinteger(L, offset);
        lua_gettable(L, -2);

        if (!lua_isnil(L, -1)) {
            lua_getfield(L, -1, "type");
            neko_lua_auto_Type stype = lua_tointeger(L, -1);
            lua_pop(L, 4);
            return neko_lua_auto_push_type(L, stype, (char *)c_in + offset);
        }

        lua_pop(L, 3);
        lua_pushfstring(L, "neko_lua_auto_struct_push_member: Member offset '%d' not registered for struct '%s'!", offset, neko_lua_auto_typename(L, type));
        lua_error(L);
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_lua_auto_struct_push_member: Struct '%s' not registered!", neko_lua_auto_typename(L, type));
    lua_error(L);
    return 0;
}

int neko_lua_auto_struct_push_member_name_type(lua_State *L, neko_lua_auto_Type type, const char *member, const void *c_in) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_getfield(L, -1, member);

        if (!lua_isnil(L, -1)) {
            lua_getfield(L, -1, "type");
            neko_lua_auto_Type stype = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, -1, "offset");
            size_t offset = lua_tointeger(L, -1);
            lua_pop(L, 4);
            return neko_lua_auto_push_type(L, stype, (char *)c_in + offset);
        }

        lua_pop(L, 3);
        lua_pushfstring(L, "neko_lua_auto_struct_push_member: Member name '%s' not registered for struct '%s'!", member, neko_lua_auto_typename(L, type));
        lua_error(L);
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_lua_auto_struct_push_member: Struct '%s' not registered!", neko_lua_auto_typename(L, type));
    lua_error(L);
    return 0;
}

void neko_lua_auto_struct_to_member_offset_type(lua_State *L, neko_lua_auto_Type type, size_t offset, void *c_out, int index) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs_offset");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_pushinteger(L, offset);
        lua_gettable(L, -2);

        if (!lua_isnil(L, -1)) {
            lua_getfield(L, -1, "type");
            neko_lua_auto_Type stype = lua_tointeger(L, -1);
            lua_pop(L, 4);
            neko_lua_auto_to_type(L, stype, (char *)c_out + offset, index);
            return;
        }

        lua_pop(L, 3);
        lua_pushfstring(L, "neko_lua_auto_struct_to_member: Member offset '%d' not registered for struct '%s'!", offset, neko_lua_auto_typename(L, type));
        lua_error(L);
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_lua_auto_struct_to_member: Struct '%s' not registered!", neko_lua_auto_typename(L, type));
    lua_error(L);
}

void neko_lua_auto_struct_to_member_name_type(lua_State *L, neko_lua_auto_Type type, const char *member, void *c_out, int index) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_pushstring(L, member);
        lua_gettable(L, -2);

        if (!lua_isnil(L, -1)) {
            lua_getfield(L, -1, "type");
            neko_lua_auto_Type stype = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, -1, "offset");
            size_t offset = lua_tointeger(L, -1);
            lua_pop(L, 4);
            neko_lua_auto_to_type(L, stype, (char *)c_out + offset, index);
            return;
        }

        lua_pop(L, 3);
        lua_pushfstring(L, "neko_lua_auto_struct_to_member: Member name '%s' not registered for struct '%s'!", member, neko_lua_auto_typename(L, type));
        lua_error(L);
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_lua_auto_struct_to_member: Struct '%s' not registered!", neko_lua_auto_typename(L, type));
    lua_error(L);
}

bool neko_lua_auto_struct_has_member_offset_type(lua_State *L, neko_lua_auto_Type type, size_t offset) {

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
    lua_pushfstring(L, "neko_lua_auto_struct_has_member: Struct '%s' not registered!", neko_lua_auto_typename(L, type));
    lua_error(L);
    return false;
}

bool neko_lua_auto_struct_has_member_name_type(lua_State *L, neko_lua_auto_Type type, const char *member) {

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
    lua_pushfstring(L, "neko_lua_auto_struct_has_member: Struct '%s' not registered!", neko_lua_auto_typename(L, type));
    lua_error(L);
    return false;
}

neko_lua_auto_Type neko_lua_auto_struct_typeof_member_offset_type(lua_State *L, neko_lua_auto_Type type, size_t offset) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs_offset");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_pushinteger(L, offset);
        lua_gettable(L, -2);

        if (!lua_isnil(L, -1)) {
            lua_getfield(L, -1, "type");
            neko_lua_auto_Type stype = lua_tointeger(L, -1);
            lua_pop(L, 4);
            return stype;
        }

        lua_pop(L, 3);
        lua_pushfstring(L, "neko_lua_auto_struct_typeof_member: Member offset '%d' not registered for struct '%s'!", offset, neko_lua_auto_typename(L, type));
        lua_error(L);
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_lua_auto_struct_typeof_member: Struct '%s' not registered!", neko_lua_auto_typename(L, type));
    lua_error(L);
    return 0;
}

neko_lua_auto_Type neko_lua_auto_struct_typeof_member_name_type(lua_State *L, neko_lua_auto_Type type, const char *member) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_pushstring(L, member);
        lua_gettable(L, -2);

        if (!lua_isnil(L, -1)) {
            lua_getfield(L, -1, "type");
            neko_lua_auto_Type type = lua_tointeger(L, -1);
            lua_pop(L, 4);
            return type;
        }

        lua_pop(L, 3);
        lua_pushfstring(L, "neko_lua_auto_struct_typeof_member: Member name '%s' not registered for struct '%s'!", member, neko_lua_auto_typename(L, type));
        lua_error(L);
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_lua_auto_struct_typeof_member: Struct '%s' not registered!", neko_lua_auto_typename(L, type));
    lua_error(L);
    return 0;
}

void neko_lua_auto_struct_type(lua_State *L, neko_lua_auto_Type type) {
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

void neko_lua_auto_struct_member_type(lua_State *L, neko_lua_auto_Type type, const char *member, neko_lua_auto_Type mtype, size_t offset) {

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
    lua_pushfstring(L, "neko_lua_auto_struct_member: Struct '%s' not registered!", neko_lua_auto_typename(L, type));
    lua_error(L);
}

bool neko_lua_auto_struct_registered_type(lua_State *L, neko_lua_auto_Type type) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "structs");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    bool reg = !lua_isnil(L, -1);
    lua_pop(L, 2);

    return reg;
}

int neko_lua_auto_struct_push_type(lua_State *L, neko_lua_auto_Type type, const void *c_in) {

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
                int num = neko_lua_auto_struct_push_member_name_type(L, type, name, c_in);
                if (num > 1) {
                    lua_pop(L, 5);
                    lua_pushfstring(L,
                                    "neko_lua_auto_struct_push: Conversion pushed %d values to stack,"
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
    lua_pushfstring(L, "lua_struct_push: Struct '%s' not registered!", neko_lua_auto_typename(L, type));
    lua_error(L);
    return 0;
}

void neko_lua_auto_struct_to_type(lua_State *L, neko_lua_auto_Type type, void *c_out, int index) {

    lua_pushnil(L);
    while (lua_next(L, index - 1)) {

        if (lua_type(L, -2) == LUA_TSTRING) {
            neko_lua_auto_struct_to_member_name_type(L, type, lua_tostring(L, -2), c_out, -1);
        }

        lua_pop(L, 1);
    }
}

const char *neko_lua_auto_struct_next_member_name_type(lua_State *L, neko_lua_auto_Type type, const char *member) {

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
    lua_pushfstring(L, "neko_lua_auto_struct_next_member: Struct '%s' not registered!", neko_lua_auto_typename(L, type));
    lua_error(L);
    return NULL;
}

/*
** Enums
*/

int neko_lua_auto_enum_push_type(lua_State *L, neko_lua_auto_Type type, const void *value) {

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
        lua_pushfstring(L, "neko_lua_auto_enum_push: Enum '%s' value %d not registered!", neko_lua_auto_typename(L, type), lvalue);
        lua_error(L);
        return 0;
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_lua_auto_enum_push: Enum '%s' not registered!", neko_lua_auto_typename(L, type));
    lua_error(L);
    return 0;
}

void neko_lua_auto_enum_to_type(lua_State *L, neko_lua_auto_Type type, void *c_out, int index) {

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
        lua_pushfstring(L, "neko_lua_auto_enum_to: Enum '%s' field '%s' not registered!", neko_lua_auto_typename(L, type), name);
        lua_error(L);
        return;
    }

    lua_pop(L, 3);
    lua_pushfstring(L, "neko_lua_auto_enum_to: Enum '%s' not registered!", neko_lua_auto_typename(L, type));
    lua_error(L);
    return;
}

bool neko_lua_auto_enum_has_value_type(lua_State *L, neko_lua_auto_Type type, const void *value) {

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
    lua_pushfstring(L, "neko_lua_auto_enum_has_value: Enum '%s' not registered!", neko_lua_auto_typename(L, type));
    lua_error(L);
    return false;
}

bool neko_lua_auto_enum_has_name_type(lua_State *L, neko_lua_auto_Type type, const char *name) {
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
    lua_pushfstring(L, "neko_lua_auto_enum_has_name: Enum '%s' not registered!", neko_lua_auto_typename(L, type));
    lua_error(L);
    return false;
}

void neko_lua_auto_enum_type(lua_State *L, neko_lua_auto_Type type, size_t size) {
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

void neko_lua_auto_enum_value_type(lua_State *L, neko_lua_auto_Type type, const void *value, const char *name) {

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
    lua_pushfstring(L, "neko_lua_auto_enum_value: Enum '%s' not registered!", neko_lua_auto_typename(L, type));
    lua_error(L);
}

bool neko_lua_auto_enum_registered_type(lua_State *L, neko_lua_auto_Type type) {
    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);
    bool reg = !lua_isnil(L, -1);
    lua_pop(L, 2);
    return reg;
}

const char *neko_lua_auto_enum_next_value_name_type(lua_State *L, neko_lua_auto_Type type, const char *member) {

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
    lua_pushfstring(L, "neko_lua_auto_enum_next_enum_name_type: Enum '%s' not registered!", neko_lua_auto_typename(L, type));
    lua_error(L);
    return NULL;
}

/*
** Functions
*/

static int neko_lua_auto_call_entry(lua_State *L) {

    /* Get return size */

    lua_getfield(L, -1, "ret_type");
    neko_lua_auto_Type ret_type = lua_tointeger(L, -1);
    lua_pop(L, 1);

    size_t ret_size = neko_lua_auto_typesize(L, ret_type);

    /* Get total arguments sizes */

    lua_getfield(L, -1, "arg_types");

    size_t arg_size = 0;
    size_t arg_num = lua_rawlen(L, -1);

    if (lua_gettop(L) < arg_num + 2) {
        lua_pop(L, 1);
        lua_pushfstring(L, "neko_lua_auto_call: Too few arguments to function!");
        lua_error(L);
        return 0;
    }

    for (int i = 0; i < arg_num; i++) {
        lua_pushinteger(L, i + 1);
        lua_gettable(L, -2);
        neko_lua_auto_Type arg_type = lua_tointeger(L, -1);
        lua_pop(L, 1);
        arg_size += neko_lua_auto_typesize(L, arg_type);
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
        ret_data = malloc(ret_size);
        if (ret_data == NULL) {
            lua_pushfstring(L, "neko_lua_auto_call: Out of memory!");
            lua_error(L);
            return 0;
        }
    }

    if (arg_ptr + arg_size > LUAA_ARGUMENT_STACK_SIZE) {
        arg_heap = true;
        arg_data = malloc(arg_size);
        if (arg_data == NULL) {
            if (ret_heap) {
                free(ret_data);
            }
            lua_pushfstring(L, "neko_lua_auto_call: Out of memory!");
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
        neko_lua_auto_Type arg_type = lua_tointeger(L, -1);
        lua_pop(L, 1);
        neko_lua_auto_to_type(L, arg_type, arg_pos, arg_num + i - 2);
        arg_pos = (char *)arg_pos + neko_lua_auto_typesize(L, arg_type);
    }

    lua_pop(L, 1);

    /* Pop arguments from stack */

    for (int i = 0; i < arg_num; i++) {
        lua_remove(L, -2);
    }

    /* Get Function Pointer and Call */

    lua_getfield(L, -1, "auto_func");
    neko_lua_auto_Func auto_func = (neko_lua_auto_Func)lua_touserdata(L, -1);
    lua_pop(L, 2);

    auto_func(ret_data, arg_data);

    int count = neko_lua_auto_push_type(L, ret_type, ret_data);

    /* Either free heap data or reduce stack pointers */

    if (!ret_heap) {
        lua_pushinteger(L, ret_ptr);
        lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "call_ret_ptr");
    } else {
        free(ret_data);
    }

    if (!arg_heap) {
        lua_pushinteger(L, arg_ptr);
        lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "argument_ptr");
    } else {
        free(arg_data);
    }

    return count;
}

int neko_lua_auto_call(lua_State *L, void *func_ptr) {
    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "functions");
    lua_pushlightuserdata(L, func_ptr);
    lua_gettable(L, -2);
    lua_remove(L, -2);

    if (!lua_isnil(L, -1)) {
        return neko_lua_auto_call_entry(L);
    }

    lua_pop(L, 1);
    lua_pushfstring(L, "neko_lua_auto_call: Function with address '%p' is not registered!", func_ptr);
    lua_error(L);
    return 0;
}

int neko_lua_auto_call_name(lua_State *L, const char *func_name) {
    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "functions");
    lua_pushstring(L, func_name);
    lua_gettable(L, -2);
    lua_remove(L, -2);

    if (!lua_isnil(L, -1)) {
        return neko_lua_auto_call_entry(L);
    }

    lua_pop(L, 1);
    lua_pushfstring(L, "neko_lua_auto_call_name: Function '%s' is not registered!", func_name);
    lua_error(L);
    return 0;
}

void neko_lua_auto_function_register_type(lua_State *L, void *src_func, neko_lua_auto_Func auto_func, const char *name, neko_lua_auto_Type ret_t, int num_args, ...) {

    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "functions");
    lua_pushstring(L, name);

    lua_newtable(L);

    lua_pushlightuserdata(L, src_func);
    lua_setfield(L, -2, "src_func");
    lua_pushlightuserdata(L, auto_func);
    lua_setfield(L, -2, "auto_func");

    lua_pushinteger(L, ret_t);
    lua_setfield(L, -2, "ret_type");

    lua_pushstring(L, "arg_types");
    lua_newtable(L);

    va_list va;
    va_start(va, num_args);
    for (int i = 0; i < num_args; i++) {
        lua_pushinteger(L, i + 1);
        lua_pushinteger(L, va_arg(va, neko_lua_auto_Type));
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

#pragma endregion LuaA

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

#pragma region lua_struct

#define TYPEID_int8 0
#define TYPEID_int16 1
#define TYPEID_int32 2
#define TYPEID_int64 3
#define TYPEID_uint8 4
#define TYPEID_uint16 5
#define TYPEID_uint32 6
#define TYPEID_uint64 7
#define TYPEID_bool 8
#define TYPEID_ptr 9
#define TYPEID_float 10
#define TYPEID_double 11
#define TYPEID_COUNT 12

#define TYPEID_(type) (TYPEID_##type)

static inline void set_int8(void *p, lua_Integer v) { *(int8_t *)p = (int8_t)v; }

static inline void set_int16(void *p, lua_Integer v) { *(int16_t *)p = (int16_t)v; }

static inline void set_int32(void *p, lua_Integer v) { *(int32_t *)p = (int32_t)v; }

static inline void set_int64(void *p, lua_Integer v) { *(int64_t *)p = (int64_t)v; }

static inline void set_uint8(void *p, lua_Integer v) { *(int8_t *)p = (uint8_t)v; }

static inline void set_uint16(void *p, lua_Integer v) { *(int16_t *)p = (uint16_t)v; }

static inline void set_uint32(void *p, lua_Integer v) { *(int32_t *)p = (uint32_t)v; }

static inline void set_uint64(void *p, lua_Integer v) { *(int64_t *)p = (uint64_t)v; }

static inline void set_float(void *p, lua_Number v) { *(float *)p = (float)v; }

static inline void set_bool(void *p, int v) { *(int8_t *)p = v; }

static inline void set_ptr(void *p, void *v) { *(void **)p = v; }

static inline void set_double(void *p, lua_Number v) { *(float *)p = (double)v; }

static inline int8_t get_int8(void *p) { return *(int8_t *)p; }

static inline int16_t get_int16(void *p) { return *(int16_t *)p; }

static inline int32_t get_int32(void *p) { return *(int32_t *)p; }

static inline int64_t get_int64(void *p) { return *(int64_t *)p; }

static inline uint8_t get_uint8(void *p) { return *(uint8_t *)p; }

static inline uint16_t get_uint16(void *p) { return *(uint16_t *)p; }

static inline uint32_t get_uint32(void *p) { return *(uint32_t *)p; }

static inline uint64_t get_uint64(void *p) { return *(uint64_t *)p; }

static inline void *get_ptr(void *p) { return *(void **)p; }

static inline int get_bool(void *p) { return *(int8_t *)p; }

static inline float get_float(void *p) { return *(float *)p; }

static inline double get_double(void *p) { return *(double *)p; }

static inline int get_stride(int type) {
    switch (type) {
        case TYPEID_int8:
            return sizeof(int8_t);
        case TYPEID_int16:
            return sizeof(int16_t);
        case TYPEID_int32:
            return sizeof(int32_t);
        case TYPEID_int64:
            return sizeof(int64_t);
        case TYPEID_uint8:
            return sizeof(uint8_t);
        case TYPEID_uint16:
            return sizeof(uint16_t);
        case TYPEID_uint32:
            return sizeof(uint32_t);
        case TYPEID_uint64:
            return sizeof(uint64_t);
        case TYPEID_ptr:
            return sizeof(void *);
        case TYPEID_bool:
            return 1;
        case TYPEID_float:
            return sizeof(float);
        case TYPEID_double:
            return sizeof(double);
        default:
            return 0;
    }
}

static int setter(lua_State *L, void *p, int type, int offset) {
    p = (char *)p + offset * get_stride(type);
    switch (type) {
        case TYPEID_(int8):
            set_int8(p, luaL_checkinteger(L, 2));
            break;
        case TYPEID_(int16):
            set_int16(p, luaL_checkinteger(L, 2));
            break;
        case TYPEID_(int32):
            set_int32(p, luaL_checkinteger(L, 2));
            break;
        case TYPEID_(int64):
            set_int64(p, luaL_checkinteger(L, 2));
            break;
        case TYPEID_(uint8):
            set_uint8(p, luaL_checkinteger(L, 2));
            break;
        case TYPEID_(uint16):
            set_uint16(p, luaL_checkinteger(L, 2));
            break;
        case TYPEID_(uint32):
            set_uint32(p, luaL_checkinteger(L, 2));
            break;
        case TYPEID_(uint64):
            set_uint64(p, luaL_checkinteger(L, 2));
            break;
        case TYPEID_(bool):
            set_bool(p, lua_toboolean(L, 2));
            break;
        case TYPEID_(ptr):
            set_ptr(p, lua_touserdata(L, 2));
            break;
        case TYPEID_(float):
            set_float(p, luaL_checknumber(L, 2));
            break;
        case TYPEID_(double):
            set_double(p, luaL_checknumber(L, 2));
            break;
    }
    return 0;
}

static inline int getter(lua_State *L, void *p, int type, int offset) {
    p = (char *)p + offset * get_stride(type);
    switch (type) {
        case TYPEID_(int8):
            lua_pushinteger(L, get_int8(p));
            break;
        case TYPEID_(int16):
            lua_pushinteger(L, get_int16(p));
            break;
        case TYPEID_(int32):
            lua_pushinteger(L, get_int32(p));
            break;
        case TYPEID_(int64):
            lua_pushinteger(L, (lua_Integer)get_int64(p));
            break;
        case TYPEID_(uint8):
            lua_pushinteger(L, get_int8(p));
            break;
        case TYPEID_(uint16):
            lua_pushinteger(L, get_int16(p));
            break;
        case TYPEID_(uint32):
            lua_pushinteger(L, get_int32(p));
            break;
        case TYPEID_(uint64):
            lua_pushinteger(L, (lua_Integer)get_int64(p));
            break;
        case TYPEID_(bool):
            lua_pushboolean(L, get_bool(p));
            break;
        case TYPEID_(ptr):
            lua_pushlightuserdata(L, get_ptr(p));
            break;
        case TYPEID_(float):
            lua_pushnumber(L, get_float(p));
            break;
        case TYPEID_(double):
            lua_pushnumber(L, get_double(p));
            break;
    }
    return 1;
}

#define ACCESSOR_FUNC(TYPE, OFF)                                                                                \
    static int get_##TYPE##_##OFF(lua_State *L) { return getter(L, lua_touserdata(L, 1), TYPEID_##TYPE, OFF); } \
    static int set_##TYPE##_##OFF(lua_State *L) { return setter(L, lua_touserdata(L, 1), TYPEID_##TYPE, OFF); }

#define ACCESSOR_OFFSET(TYPE)                                                                                                                      \
    static int get_##TYPE##_offset(lua_State *L) { return getter(L, lua_touserdata(L, 1), TYPEID_##TYPE, lua_tointeger(L, lua_upvalueindex(1))); } \
    static int set_##TYPE##_offset(lua_State *L) { return setter(L, lua_touserdata(L, 1), TYPEID_##TYPE, lua_tointeger(L, lua_upvalueindex(1))); }

#define ACCESSOR(GS, TYPE)                                     \
    static int GS##ter_func_##TYPE(lua_State *L, int offset) { \
        switch (offset) {                                      \
            case 0:                                            \
                lua_pushcfunction(L, GS##_##TYPE##_0);         \
                break;                                         \
            case 1:                                            \
                lua_pushcfunction(L, GS##_##TYPE##_1);         \
                break;                                         \
            case 2:                                            \
                lua_pushcfunction(L, GS##_##TYPE##_2);         \
                break;                                         \
            case 3:                                            \
                lua_pushcfunction(L, GS##_##TYPE##_3);         \
                break;                                         \
            case 4:                                            \
                lua_pushcfunction(L, GS##_##TYPE##_4);         \
                break;                                         \
            case 5:                                            \
                lua_pushcfunction(L, GS##_##TYPE##_5);         \
                break;                                         \
            case 6:                                            \
                lua_pushcfunction(L, GS##_##TYPE##_6);         \
                break;                                         \
            case 7:                                            \
                lua_pushcfunction(L, GS##_##TYPE##_7);         \
                break;                                         \
            default:                                           \
                lua_pushinteger(L, offset);                    \
                lua_pushcclosure(L, GS##_##TYPE##_offset, 1);  \
                break;                                         \
        }                                                      \
        return 1;                                              \
    }

ACCESSOR_FUNC(float, 0)
ACCESSOR_FUNC(float, 1)
ACCESSOR_FUNC(float, 2)
ACCESSOR_FUNC(float, 3)
ACCESSOR_FUNC(float, 4)
ACCESSOR_FUNC(float, 5)
ACCESSOR_FUNC(float, 6)
ACCESSOR_FUNC(float, 7)
ACCESSOR_OFFSET(float)
ACCESSOR(get, float)
ACCESSOR(set, float)

ACCESSOR_FUNC(double, 0)
ACCESSOR_FUNC(double, 1)
ACCESSOR_FUNC(double, 2)
ACCESSOR_FUNC(double, 3)
ACCESSOR_FUNC(double, 4)
ACCESSOR_FUNC(double, 5)
ACCESSOR_FUNC(double, 6)
ACCESSOR_FUNC(double, 7)
ACCESSOR_OFFSET(double)
ACCESSOR(get, double)
ACCESSOR(set, double)

ACCESSOR_FUNC(int8, 0)
ACCESSOR_FUNC(int8, 1)
ACCESSOR_FUNC(int8, 2)
ACCESSOR_FUNC(int8, 3)
ACCESSOR_FUNC(int8, 4)
ACCESSOR_FUNC(int8, 5)
ACCESSOR_FUNC(int8, 6)
ACCESSOR_FUNC(int8, 7)
ACCESSOR_OFFSET(int8)
ACCESSOR(get, int8)
ACCESSOR(set, int8)

ACCESSOR_FUNC(int16, 0)
ACCESSOR_FUNC(int16, 1)
ACCESSOR_FUNC(int16, 2)
ACCESSOR_FUNC(int16, 3)
ACCESSOR_FUNC(int16, 4)
ACCESSOR_FUNC(int16, 5)
ACCESSOR_FUNC(int16, 6)
ACCESSOR_FUNC(int16, 7)
ACCESSOR_OFFSET(int16)
ACCESSOR(get, int16)
ACCESSOR(set, int16)

ACCESSOR_FUNC(int32, 0)
ACCESSOR_FUNC(int32, 1)
ACCESSOR_FUNC(int32, 2)
ACCESSOR_FUNC(int32, 3)
ACCESSOR_FUNC(int32, 4)
ACCESSOR_FUNC(int32, 5)
ACCESSOR_FUNC(int32, 6)
ACCESSOR_FUNC(int32, 7)
ACCESSOR_OFFSET(int32)
ACCESSOR(get, int32)
ACCESSOR(set, int32)

ACCESSOR_FUNC(int64, 0)
ACCESSOR_FUNC(int64, 1)
ACCESSOR_FUNC(int64, 2)
ACCESSOR_FUNC(int64, 3)
ACCESSOR_FUNC(int64, 4)
ACCESSOR_FUNC(int64, 5)
ACCESSOR_FUNC(int64, 6)
ACCESSOR_FUNC(int64, 7)
ACCESSOR_OFFSET(int64)
ACCESSOR(get, int64)
ACCESSOR(set, int64)

ACCESSOR_FUNC(uint8, 0)
ACCESSOR_FUNC(uint8, 1)
ACCESSOR_FUNC(uint8, 2)
ACCESSOR_FUNC(uint8, 3)
ACCESSOR_FUNC(uint8, 4)
ACCESSOR_FUNC(uint8, 5)
ACCESSOR_FUNC(uint8, 6)
ACCESSOR_FUNC(uint8, 7)
ACCESSOR_OFFSET(uint8)
ACCESSOR(get, uint8)
ACCESSOR(set, uint8)

ACCESSOR_FUNC(uint16, 0)
ACCESSOR_FUNC(uint16, 1)
ACCESSOR_FUNC(uint16, 2)
ACCESSOR_FUNC(uint16, 3)
ACCESSOR_FUNC(uint16, 4)
ACCESSOR_FUNC(uint16, 5)
ACCESSOR_FUNC(uint16, 6)
ACCESSOR_FUNC(uint16, 7)
ACCESSOR_OFFSET(uint16)
ACCESSOR(get, uint16)
ACCESSOR(set, uint16)

ACCESSOR_FUNC(uint32, 0)
ACCESSOR_FUNC(uint32, 1)
ACCESSOR_FUNC(uint32, 2)
ACCESSOR_FUNC(uint32, 3)
ACCESSOR_FUNC(uint32, 4)
ACCESSOR_FUNC(uint32, 5)
ACCESSOR_FUNC(uint32, 6)
ACCESSOR_FUNC(uint32, 7)
ACCESSOR_OFFSET(uint32)
ACCESSOR(get, uint32)
ACCESSOR(set, uint32)

ACCESSOR_FUNC(uint64, 0)
ACCESSOR_FUNC(uint64, 1)
ACCESSOR_FUNC(uint64, 2)
ACCESSOR_FUNC(uint64, 3)
ACCESSOR_FUNC(uint64, 4)
ACCESSOR_FUNC(uint64, 5)
ACCESSOR_FUNC(uint64, 6)
ACCESSOR_FUNC(uint64, 7)
ACCESSOR_OFFSET(uint64)
ACCESSOR(get, uint64)
ACCESSOR(set, uint64)

ACCESSOR_FUNC(bool, 0)
ACCESSOR_FUNC(bool, 1)
ACCESSOR_FUNC(bool, 2)
ACCESSOR_FUNC(bool, 3)
ACCESSOR_FUNC(bool, 4)
ACCESSOR_FUNC(bool, 5)
ACCESSOR_FUNC(bool, 6)
ACCESSOR_FUNC(bool, 7)
ACCESSOR_OFFSET(bool)
ACCESSOR(get, bool)
ACCESSOR(set, bool)

ACCESSOR_FUNC(ptr, 0)
ACCESSOR_FUNC(ptr, 1)
ACCESSOR_FUNC(ptr, 2)
ACCESSOR_FUNC(ptr, 3)
ACCESSOR_FUNC(ptr, 4)
ACCESSOR_FUNC(ptr, 5)
ACCESSOR_FUNC(ptr, 6)
ACCESSOR_FUNC(ptr, 7)
ACCESSOR_OFFSET(ptr)
ACCESSOR(get, ptr)
ACCESSOR(set, ptr)

static inline int get_value(lua_State *L, int type, int offset) {
    switch (type) {
        case TYPEID_int8:
            getter_func_int8(L, offset);
            break;
        case TYPEID_int16:
            getter_func_int16(L, offset);
            break;
        case TYPEID_int32:
            getter_func_int32(L, offset);
            break;
        case TYPEID_int64:
            getter_func_int64(L, offset);
            break;
        case TYPEID_uint8:
            getter_func_uint8(L, offset);
            break;
        case TYPEID_uint16:
            getter_func_uint16(L, offset);
            break;
        case TYPEID_uint32:
            getter_func_uint32(L, offset);
            break;
        case TYPEID_uint64:
            getter_func_uint64(L, offset);
            break;
        case TYPEID_bool:
            getter_func_bool(L, offset);
            break;
        case TYPEID_ptr:
            getter_func_ptr(L, offset);
            break;
        case TYPEID_float:
            getter_func_float(L, offset);
            break;
        case TYPEID_double:
            getter_func_double(L, offset);
            break;
        default:
            return luaL_error(L, "Invalid type %d\n", type);
    }
    return 1;
}

static int getter_direct(lua_State *L) {
    int type = luaL_checkinteger(L, 1);
    if (type < 0 || type >= TYPEID_COUNT) return luaL_error(L, "Invalid type %d", type);
    int offset = luaL_checkinteger(L, 2);
    int stride = get_stride(type);
    if (offset % stride != 0) {
        return luaL_error(L, "Invalid offset %d for type %d", offset, type);
    }
    offset /= stride;
    return get_value(L, type, offset);
}

static inline int set_value(lua_State *L, int type, int offset) {
    switch (type) {
        case TYPEID_int8:
            setter_func_int8(L, offset);
            break;
        case TYPEID_int16:
            setter_func_int16(L, offset);
            break;
        case TYPEID_int32:
            setter_func_int32(L, offset);
            break;
        case TYPEID_int64:
            setter_func_int64(L, offset);
            break;
        case TYPEID_uint8:
            setter_func_uint8(L, offset);
            break;
        case TYPEID_uint16:
            setter_func_uint16(L, offset);
            break;
        case TYPEID_uint32:
            setter_func_uint32(L, offset);
            break;
        case TYPEID_uint64:
            setter_func_uint64(L, offset);
            break;
        case TYPEID_bool:
            setter_func_bool(L, offset);
            break;
        case TYPEID_ptr:
            setter_func_ptr(L, offset);
            break;
        case TYPEID_float:
            setter_func_float(L, offset);
            break;
        case TYPEID_double:
            setter_func_double(L, offset);
            break;
        default:
            return luaL_error(L, "Invalid type %d\n", type);
    }
    return 1;
}

static int setter_direct(lua_State *L) {
    int type = luaL_checkinteger(L, 1);
    if (type < 0 || type >= TYPEID_COUNT) return luaL_error(L, "Invalid type %d", type);
    int offset = luaL_checkinteger(L, 2);
    int stride = get_stride(type);
    if (offset % stride != 0) {
        return luaL_error(L, "Invalid offset %d for type %d", offset, type);
    }
    offset /= stride;
    return set_value(L, type, offset);
}

#define LUATYPEID(type, typen)         \
    lua_pushinteger(L, TYPEID_##type); \
    lua_setfield(L, -2, #typen);

static int ltypeid(lua_State *L) {
    lua_newtable(L);
    LUATYPEID(int8, int8_t);
    LUATYPEID(int16, int16_t);
    LUATYPEID(int32, int32_t);
    LUATYPEID(int64, int64_t);
    LUATYPEID(uint8, uint8_t);
    LUATYPEID(uint16, uint16_t);
    LUATYPEID(uint32, uint32_t);
    LUATYPEID(uint64, uint64_t);
    LUATYPEID(bool, bool);
    LUATYPEID(ptr, ptr);
    LUATYPEID(float, float);
    LUATYPEID(double, double);
    return 1;
}

static int loffset(lua_State *L) {
    if (!lua_isuserdata(L, 1)) return luaL_error(L, "Need userdata at 1");
    char *p = (char *)lua_touserdata(L, 1);
    size_t off = luaL_checkinteger(L, 2);
    lua_pushlightuserdata(L, (void *)(p + off));
    return 1;
}

struct address_path {
    uint8_t type;
    uint8_t offset[1];
};

static const uint8_t *get_offset(const uint8_t *offset, size_t sz, int *output) {
    if (sz == 0) return NULL;
    if (offset[0] < 128) {
        *output = offset[0];
        return offset + 1;
    }
    int t = offset[0] & 0x7f;
    size_t i;
    int shift = 7;
    for (i = 1; i < sz; i++) {
        if (offset[i] < 128) {
            t |= offset[i] << shift;
            *output = t;
            return offset + i + 1;
        } else {
            t |= (offset[i] & 0x7f) << shift;
            shift += 7;
        }
    }
    return NULL;
}

static void *address_ptr(lua_State *L, int *type, int *offset) {
    size_t sz;
    const uint8_t *buf = (const uint8_t *)lua_tolstring(L, lua_upvalueindex(1), &sz);
    if (sz == 0 || buf[0] >= TYPEID_COUNT) luaL_error(L, "Invalid type");
    void **p = (void **)lua_touserdata(L, 1);
    const uint8_t *endptr = buf + sz;
    sz--;
    const uint8_t *ptr = &buf[1];
    for (;;) {
        int off = 0;
        ptr = get_offset(ptr, sz, &off);
        if (ptr == NULL) luaL_error(L, "Invalid offset");
        sz = endptr - ptr;
        if (sz == 0) {
            *type = buf[0];
            *offset = off;
            return p;
        } else {
            p += off;
            if (*p == NULL) return NULL;
            p = (void **)*p;
        }
    }
}

static int lget_indirect(lua_State *L) {
    int type;
    int offset;
    void *p = address_ptr(L, &type, &offset);
    if (p == NULL) return 0;
    return getter(L, p, type, offset);
}

static int lset_indirect(lua_State *L) {
    int type;
    int offset;
    void *p = address_ptr(L, &type, &offset);
    if (p == NULL) return 0;
    return setter(L, p, type, offset);
}

static void address(lua_State *L) {
    int type = luaL_checkinteger(L, 1);
    if (type < 0 || type >= TYPEID_COUNT) luaL_error(L, "Invalid type %d", type);
    int top = lua_gettop(L);
    if (top <= 2) {
        luaL_error(L, "Need two or more offsets");
    }
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    luaL_addchar(&b, type);
    int i;
    for (i = 2; i <= top; i++) {
        unsigned int offset = (unsigned int)luaL_checkinteger(L, i);
        if (i != top) {
            if (offset % sizeof(void *) != 0) luaL_error(L, "%d is not align to pointer", offset);
            offset /= sizeof(void *);
        } else {
            int stride = get_stride(type);
            if (offset % stride != 0) luaL_error(L, "%d is not align to %d", offset, stride);
            offset /= stride;
        }

        if (offset < 128) {
            luaL_addchar(&b, offset);
        } else {
            while (offset >= 128) {
                luaL_addchar(&b, (char)(0x80 | (offset & 0x7f)));
                offset >>= 7;
            }
            luaL_addchar(&b, offset);
        }
    }
    luaL_pushresult(&b);
}

static int lcstruct(lua_State *L) {
    int top = lua_gettop(L);
    if (top <= 2) {
        getter_direct(L);
        setter_direct(L);
        return 2;
    } else {
        address(L);
        int cmd = lua_gettop(L);
        lua_pushvalue(L, cmd);
        lua_pushcclosure(L, lget_indirect, 1);
        lua_pushvalue(L, cmd);
        lua_pushcclosure(L, lset_indirect, 1);
        return 2;
    }
}

int neko_luaopen_cstruct_core(lua_State *L) {
    luaL_checkversion(L);
    luaL_Reg l[] = {
            {"cstruct", lcstruct},
            {"offset", loffset},
            {"typeid", ltypeid},
            {NULL, NULL},
    };
    // luaL_newlib(L, l);
    neko_lua_load(L, l, "__neko_cstruct_core");
    return 1;
}

static int newudata(lua_State *L) {
    size_t sz = luaL_checkinteger(L, 1);
    lua_newuserdatauv(L, sz, 0);
    return 1;
}

int neko_luaopen_cstruct_test(lua_State *L) {
    luaL_checkversion(L);
    luaL_Reg l[] = {
            {"udata", newudata},
            {"NULL", NULL},
            {NULL, NULL},
    };
    neko_lua_load(L, l, "__neko_cstruct_test");

    lua_getglobal(L, "__neko_cstruct_test");

    lua_pushlightuserdata(L, NULL);
    lua_setfield(L, -2, "NULL");

    lua_pop(L, 1);

    return 1;
}

#pragma endregion lua_struct

#pragma region lua_datalist

#define MAX_DEPTH 256
#define SHORT_STRING 1024
#define CONVERTER 2
#define REF_CACHE 3
#define REF_UNSOLVED 4
#define TAB_SPACE 4

typedef uintptr_t objectid;

enum token_type {
    TOKEN_OPEN,          // 0 { [
    TOKEN_CLOSE,         // 1 } ]
    TOKEN_CONVERTER,     // 2 $ ( $name something == [name, something] )
    TOKEN_MAP,           // 3 = :
    TOKEN_LIST,          // 4 ---
    TOKEN_STRING,        // 5
    TOKEN_ESCAPESTRING,  // 6
    TOKEN_ATOM,          // 7
    TOKEN_NEWLINE,       // 8 space \t
    TOKEN_TAG,           // 9   &badf00d  (64bit hex number)
    TOKEN_REF,           // 10  *badf00d
    TOKEN_EOF,           // 11 end of file
};

struct token {
    enum token_type type;
    ptrdiff_t from;
    ptrdiff_t to;
};

struct lex_state {
    const char *source;
    size_t sz;
    ptrdiff_t position;
    struct token c;
    struct token n;
    int newline;
    int aslist;
};

static const char *skip_line_comment(struct lex_state *LS) {
    const char *ptr = LS->source + LS->position;
    const char *endptr = LS->source + LS->sz;
    while (ptr < endptr) {
        if (*ptr == '\r' || *ptr == '\n') {
            LS->position = ptr - LS->source;
            LS->newline = 1;
            return ptr;
        }
        ++ptr;
    }
    return ptr;
}

static const char *parse_ident(struct lex_state *LS) {
    const char *ptr = LS->source + LS->position;
    const char *endptr = LS->source + LS->sz;
    while (ptr < endptr) {
        switch (*ptr) {
            case '\r':
            case '\n':
                LS->newline = 1;
                return ptr + 1;
            case '#':
                // empty line
                return ptr;
            case ' ':
            case '\t':
                break;
            default:
                LS->n.type = TOKEN_NEWLINE;
                LS->n.from = LS->position;
                LS->n.to = ptr - LS->source;
                LS->position = LS->n.to;
                return NULL;
        }
        ++ptr;
    }
    return ptr;
}

static int is_hexnumber(struct lex_state *LS) {
    const char *ptr = LS->source + LS->n.from + 1;
    const char *endptr = LS->source + LS->n.to;
    if (ptr == endptr) return 0;
    do {
        char c = *ptr;
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) return 0;
    } while (++ptr < endptr);
    return 1;
}

static void parse_atom(struct lex_state *LS) {
    static const char *separator = " \t\r\n,{}[]$:\"'";
    const char *ptr = LS->source + LS->position;
    const char *endptr = LS->source + LS->sz;
    char head = *ptr;
    LS->n.from = LS->position;
    while (ptr < endptr) {
        if (strchr(separator, *ptr)) {
            break;
        }
        ++ptr;
    }
    LS->n.to = ptr - LS->source;
    LS->position = LS->n.to;
    switch (head) {
        case '&':
            if (is_hexnumber(LS)) {
                LS->n.type = TOKEN_TAG;
                return;
            }
            break;
        case '*':
            if (is_hexnumber(LS)) {
                LS->n.type = TOKEN_REF;
                return;
            }
            break;
        default:
            break;
    }
    LS->n.type = TOKEN_ATOM;
}

static int parse_string(struct lex_state *LS) {
    const char *ptr = LS->source + LS->position;
    const char *endptr = LS->source + LS->sz;
    char open_string = *ptr++;
    LS->n.type = TOKEN_STRING;
    LS->n.from = LS->position + 1;
    while (ptr < endptr) {
        char c = *ptr;
        if (c == open_string) {
            LS->n.to = ptr - LS->source;
            LS->position = ptr - LS->source + 1;
            return 1;
        }
        if (c == '\r' || c == '\n') {
            return 0;
        }
        if (c == '\\') {
            LS->n.type = TOKEN_ESCAPESTRING;
            ++ptr;
        }
        ++ptr;
    }
    return 0;
}

// 0 : invalid source
// 1 : ok
static int next_token(struct lex_state *LS) {
    const char *ptr = LS->source + LS->position;
    const char *endptr = LS->source + LS->sz;
    while (ptr < endptr) {
        LS->position = ptr - LS->source;
        if (LS->newline) {
            // line head
            LS->newline = 0;
            const char *nextptr = parse_ident(LS);
            if (nextptr == NULL) return 1;
            // empty line
            ptr = nextptr;
            continue;
        }

        switch (*ptr) {
            case '#':
                // comment
                ptr = skip_line_comment(LS);
                continue;
            case '\r':
            case '\n':
                LS->newline = 1;
                ++ptr;
                continue;
            case ' ':
            case '\t':
            case ',':
                break;
            case '{':
            case '[':
                LS->n.type = TOKEN_OPEN;
                LS->n.from = LS->position;
                LS->n.to = ++LS->position;
                return 1;
            case '$':
                LS->n.type = TOKEN_CONVERTER;
                LS->n.from = LS->position;
                LS->n.to = ++LS->position;
                return 1;
            case '}':
            case ']':
                LS->n.type = TOKEN_CLOSE;
                LS->n.from = LS->position;
                LS->n.to = ++LS->position;
                return 1;
            case '-':
                do ++ptr;
                while (ptr < endptr && *ptr == '-');
                if (ptr >= endptr || strchr(" \t\r\n", *ptr)) {
                    LS->n.type = TOKEN_LIST;
                    LS->n.from = LS->position;
                    LS->n.to = ptr - LS->source;
                    LS->position = LS->n.to;
                } else {
                    // negative number
                    parse_atom(LS);
                }
                return 1;
            case ':':
                LS->n.type = TOKEN_MAP;
                LS->n.from = LS->position;
                LS->n.to = ++LS->position;
                return 1;
            case '"':
            case '\'':
                return parse_string(LS);
            default:
                parse_atom(LS);
                return 1;
        }
        ++ptr;
    }
    LS->n.type = TOKEN_EOF;
    LS->position = LS->sz;
    return 1;
}

static int invalid(lua_State *L, struct lex_state *LS, const char *err) {
    ptrdiff_t index;
    int line = 1;
    ptrdiff_t position = LS->n.from;
    for (index = 0; index < position; index++) {
        if (LS->source[index] == '\n') ++line;
    }
    return luaL_error(L, "Line %d : %s", line, err);
}

static inline int read_token(lua_State *L, struct lex_state *LS) {
    if (LS->c.type == TOKEN_EOF) {
        invalid(L, LS, "End of data");
    }
    LS->c = LS->n;
    if (!next_token(LS)) invalid(L, LS, "Invalid token");
    // neko_println("token %d %.*s", LS->c.type, (int)(LS->c.to-LS->c.from), LS->source + LS->c.from);
    return LS->c.type;
}

static inline int to_hex(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int push_token_string(lua_State *L, const char *ptr, size_t sz) {
    char tmp[SHORT_STRING];
    char *buffer = tmp;
    NEKO_ASSERT(sz > 0);
    if (sz > SHORT_STRING) {
        buffer = (char *)lua_newuserdatauv(L, sz, 0);
    }

    size_t i, n;
    for (n = i = 0; i < sz; ++i, ++ptr, ++n) {
        if (*ptr != '\\') {
            buffer[n] = *ptr;
        } else {
            ++ptr;
            ++i;
            NEKO_ASSERT(i < sz);
            char c = *ptr;
            if (c >= '0' && c <= '9') {
                // escape dec ascii
                int dec = c - '0';
                if (i + 1 < sz) {
                    int c2 = ptr[1];
                    if (c2 >= '0' && c2 <= '9') {
                        dec = dec * 10 + c2 - '0';
                        ++ptr;
                        ++i;
                    }
                }
                if (i + 1 < sz) {
                    int c2 = ptr[1];
                    if (c2 >= '0' && c2 <= '9') {
                        int tmp = dec * 10 + c2 - '0';
                        if (tmp <= 255) {
                            dec = tmp;
                            ++ptr;
                            ++i;
                        }
                    }
                }
                buffer[n] = dec;
            } else {
                switch (*ptr) {
                    case 'x':
                    case 'X': {
                        // escape hex ascii
                        if (i + 2 >= sz) {
                            return 1;
                        }
                        ++ptr;
                        ++i;
                        int hex = to_hex(*ptr);
                        if (hex < 0) {
                            return 1;
                        }
                        ++ptr;
                        ++i;
                        int hex2 = to_hex(*ptr);
                        if (hex2 > 0) {
                            hex = hex * 16 + hex2;
                        }
                        buffer[n] = hex;
                        break;
                    }
                    case 'n':
                        buffer[n] = '\n';
                        break;
                    case 'r':
                        buffer[n] = '\r';
                        break;
                    case 't':
                        buffer[n] = '\t';
                        break;
                    case 'a':
                        buffer[n] = '\a';
                        break;
                    case 'b':
                        buffer[n] = '\b';
                        break;
                    case 'v':
                        buffer[n] = '\v';
                        break;
                    case '\'':
                    case '"':
                    case '\n':
                    case '\r':
                        buffer[n] = *ptr;
                        break;
                    default:
                        return 1;
                }
            }
        }
    }
    lua_pushlstring(L, buffer, n);
    if (sz > SHORT_STRING) {
        lua_replace(L, -2);
    }
    return 0;
}

#define IS_KEYWORD(ptr, sz, str) (sizeof(str "") == sz + 1 && (memcmp(ptr, str, sz) == 0))

static void push_token(lua_State *L, struct lex_state *LS, struct token *t) {
    const char *ptr = LS->source + t->from;
    size_t sz = t->to - t->from;

    switch (t->type) {
        case TOKEN_STRING:
            lua_pushlstring(L, ptr, sz);
            return;
        case TOKEN_ESCAPESTRING:
            if (push_token_string(L, ptr, sz)) {
                invalid(L, LS, "Invalid quote string");
            }
            return;
        case TOKEN_ATOM:
            break;
        default:
            invalid(L, LS, "Invalid atom");
            return;
    }

    if (strchr("0123456789+-.", ptr[0])) {
        if (sz == 1) {
            char c = *ptr;
            if (c >= '0' && c <= '9') {
                lua_pushinteger(L, c - '0');
            } else {
                lua_pushlstring(L, ptr, 1);
            }
            return;
        }

        if (sz >= 3 && ptr[0] == '0' && (ptr[1] == 'x' || ptr[1] == 'X')) {
            // may be a hex integer
            lua_Integer v = 0;
            int hex = 1;
            size_t i;
            for (i = 2; i < sz; i++) {
                char c = ptr[i];
                v = v * 16;
                if (c >= '0' && c <= '9') {
                    v += c - '0';
                } else if (c >= 'a' && c <= 'f') {
                    v += c - 'a' + 10;
                } else if (c >= 'A' && c <= 'F') {
                    v += c - 'A' + 10;
                } else {
                    hex = 0;
                    break;
                }
            }
            if (hex) {
                lua_pushinteger(L, v);
                return;
            }
        }

        // may be a number
        // lua string always has \0 at the end, so strto* is safe
        char *endptr = NULL;
        lua_Integer v = strtoull(ptr, (const char *)(&endptr), 10);
        if (endptr - ptr == sz) {
            lua_pushinteger(L, v);
            return;
        }

        endptr = NULL;
        lua_Number f = strtod(ptr, &endptr);
        if (endptr - ptr == sz) {
            lua_pushnumber(L, f);
            return;
        }
    }

    if (t->type == TOKEN_ATOM) {
        if (IS_KEYWORD(ptr, sz, "true")) {
            lua_pushboolean(L, 1);
            return;
        } else if (IS_KEYWORD(ptr, sz, "false")) {
            lua_pushboolean(L, 0);
            return;
        } else if (IS_KEYWORD(ptr, sz, "nil")) {
            lua_pushnil(L);
            return;
        }
    }

    lua_pushlstring(L, ptr, sz);
}

static inline int token_length(struct token *t) { return (int)(t->to - t->from); }

static inline int token_ident(struct lex_state *LS) {
    struct token *t = &LS->c;
    const char *ptr = LS->source + t->from;
    const char *endptr = LS->source + t->to;
    int ident = token_length(t);
    while (endptr > ptr) {
        if (*ptr == '\t') ident += TAB_SPACE - 1;
        ++ptr;
    }
    return ident;
}

static inline int token_symbol(struct lex_state *LS) { return LS->source[LS->c.from]; }

static inline void push_key(lua_State *L, struct lex_state *LS) { lua_pushlstring(L, LS->source + LS->c.from, LS->c.to - LS->c.from); }

static inline void new_table_0(lua_State *L) {
    lua_newtable(L);
    // index 0 refer self
    // lua_pushvalue(L, -1);
    // lua_rawseti(L, -2, 0);
}

static void new_table(lua_State *L, int layer, objectid ref) {
    if (layer >= MAX_DEPTH) luaL_error(L, "too many layers");
    luaL_checkstack(L, 8, NULL);
    if (ref == 0) {
        new_table_0(L);
    } else {
        lua_rawgeti(L, REF_CACHE, ref);
    }
}

static objectid read_tag(struct lex_state *LS) {
    const char *ptr = LS->source + LS->c.from + 1;
    const char *endptr = LS->source + LS->c.to;
    objectid x = 0;
    while (ptr < endptr) {
        char c = *ptr;
        int n;
        if (c >= '0' && c <= '9')
            n = c - '0';
        else if (c >= 'a' && c <= 'f')
            n = c - 'a' + 10;
        else
            n = c - 'A' + 10;
        x = x * 16 + n;
        ++ptr;
    }
    if (x == 0) {
        x = ~(objectid)0;
    }
    return x;
}

static objectid parse_tag(lua_State *L, struct lex_state *LS) {
    objectid tag = read_tag(LS);
    if (lua_rawgeti(L, REF_CACHE, tag) != LUA_TNIL) {
        lua_pop(L, 1);
        if (lua_rawgeti(L, REF_UNSOLVED, tag) == LUA_TNIL) {
            invalid(L, LS, "Duplicate tag");
        }
        lua_pop(L, 1);
        // clear unsolved
        lua_pushnil(L);
        lua_rawseti(L, REF_UNSOLVED, tag);
    } else {
        lua_pop(L, 1);
        new_table_0(L);
        lua_rawseti(L, REF_CACHE, tag);
    }
    return tag;
}

static void parse_ref(lua_State *L, struct lex_state *LS) {
    objectid tag = read_tag(LS);
    read_token(L, LS);  //  consume ref tag
    if (lua_rawgeti(L, REF_CACHE, tag) != LUA_TNIL) {
        return;
    }
    lua_pop(L, 1);
    new_table_0(L);  // Create a table for future
    lua_pushvalue(L, -1);
    lua_rawseti(L, REF_CACHE, tag);
    // set unsolved flag
    lua_pushboolean(L, 1);
    lua_rawseti(L, REF_UNSOLVED, tag);
}

static void parse_bracket(lua_State *L, struct lex_state *LS, int layer, objectid tag);
static void parse_converter(lua_State *L, struct lex_state *LS, int layer, int ident);

static int closed_bracket(lua_State *L, struct lex_state *LS, int bracket) {
    for (;;) {
        switch (LS->c.type) {
            case TOKEN_CLOSE:
                if (token_symbol(LS) != bracket) {
                    invalid(L, LS, "Invalid close bracket");
                }
                read_token(L, LS);
                return 1;
            case TOKEN_NEWLINE:
                read_token(L, LS);
                break;
            default:
                return 0;
        }
    }
}

// table key value
static void set_keyvalue(lua_State *L) {
    lua_pushvalue(L, -2);
    // table key value key
    int oldv = lua_gettable(L, -4);
    // table key value oldv
    if (oldv == LUA_TNIL) {
        lua_pop(L, 1);
        lua_settable(L, -3);
    } else if (oldv == LUA_TTABLE) {
        lua_len(L, -1);
        int n = (int)lua_tointeger(L, -1);
        lua_pop(L, 1);
        lua_replace(L, -3);
        // table oldv value
        lua_seti(L, -2, n + 1);
        lua_pop(L, 1);
    } else {
        luaL_error(L, "Multi-key (%s) should be a table", lua_tostring(L, -3));
    }
}

static void parse_bracket_map(lua_State *L, struct lex_state *LS, int layer, int bracket) {
    int i = 1;
    int aslist = LS->aslist;
    do {
        if (LS->c.type != TOKEN_ATOM) {
            invalid(L, LS, "Invalid key");
        }
        push_key(L, LS);
        if (read_token(L, LS) != TOKEN_MAP) {
            invalid(L, LS, "Need a : or =");
        }
        objectid tag = 0;
        int t = read_token(L, LS);
        if (t == TOKEN_TAG) {
            tag = parse_tag(L, LS);
            t = read_token(L, LS);
        }
        while (t == TOKEN_NEWLINE) {
            t = read_token(L, LS);
        }
        switch (LS->c.type) {
            case TOKEN_REF:
                if (tag != 0) {
                    invalid(L, LS, "Invalid ref in bracket map");
                }
                parse_ref(L, LS);
                break;
            case TOKEN_OPEN:
                parse_bracket(L, LS, layer + 1, tag);
                break;
            case TOKEN_CONVERTER:
                parse_converter(L, LS, layer + 1, -1);
                break;
            default:
                push_token(L, LS, &LS->c);
                read_token(L, LS);
                break;
        }
        if (aslist) {
            lua_insert(L, -2);
            lua_seti(L, -3, i++);
            lua_seti(L, -2, i++);
        } else {
            set_keyvalue(L);
        }
    } while (!closed_bracket(L, LS, bracket));
}

static void parse_bracket_sequence(lua_State *L, struct lex_state *LS, int layer, int bracket) {
    int n = 1;
    for (;;) {
        switch (LS->c.type) {
            case TOKEN_NEWLINE:
                read_token(L, LS);
                continue;  // skip ident
            case TOKEN_CLOSE:
                if (token_symbol(LS) != bracket) {
                    invalid(L, LS, "Invalid close bracket");
                }
                read_token(L, LS);  // consume }
                return;
            case TOKEN_REF:
                parse_ref(L, LS);
                return;
            case TOKEN_OPEN:
                // No tag in sequence
                parse_bracket(L, LS, layer, 0);
                break;
            case TOKEN_CONVERTER:
                parse_converter(L, LS, layer, -1);
                break;
            default:
                push_token(L, LS, &LS->c);
                read_token(L, LS);
                break;
        }
        lua_seti(L, -2, n++);
    }
}

static inline void parse_bracket_(lua_State *L, struct lex_state *LS, int layer, objectid ref, int bracket) {
    new_table(L, layer, ref);
again:
    switch (read_token(L, LS)) {
        case TOKEN_NEWLINE:
            goto again;
        case TOKEN_CLOSE:
            if (token_symbol(LS) != bracket) {
                invalid(L, LS, "Invalid close bracket");
            }
            read_token(L, LS);  // consume }
            return;
        case TOKEN_ATOM:
            if (LS->n.type == TOKEN_MAP) {
                parse_bracket_map(L, LS, layer, bracket);
                return;
            }
            break;
        default:
            break;
    }
    parse_bracket_sequence(L, LS, layer, bracket);
}

static void parse_bracket(lua_State *L, struct lex_state *LS, int layer, objectid ref) {
    char bracket = token_symbol(LS);
    if (bracket == '[') {
        if (ref) {
            invalid(L, LS, "[] can't has a tag");
        }
        parse_bracket_(L, LS, layer, ref, ']');
        lua_pushvalue(L, CONVERTER);
        lua_insert(L, -2);
        lua_call(L, 1, 1);
    } else {
        parse_bracket_(L, LS, layer, ref, '}');
    }
}

static void parse_section(lua_State *L, struct lex_state *LS, int layer);

static void parse_converter(lua_State *L, struct lex_state *LS, int layer, int ident) {
    new_table(L, layer, 0);
    if (read_token(L, LS) != TOKEN_ATOM) {
        invalid(L, LS, "$ need an atom");
    }
    push_key(L, LS);
    lua_rawseti(L, -2, 1);  // $atom xxx === [ "atom",  xxx ]

    read_token(L, LS);

    int next_ident;

    switch (LS->c.type) {
        case TOKEN_NEWLINE:
            if (ident < 0) invalid(L, LS, "Invalid newline , Use { } for a struct instead");
            next_ident = token_ident(LS);
            if (next_ident < ident) {
                invalid(L, LS, "Invalid new section ident");
            }
            new_table(L, layer + 1, 0);
            parse_section(L, LS, layer + 1);
            break;
        case TOKEN_CLOSE:
            invalid(L, LS, "Invalid close bracket");
            break;
        case TOKEN_REF:
            parse_ref(L, LS);
            break;
        case TOKEN_OPEN:
            parse_bracket(L, LS, layer, 0);
            break;
        case TOKEN_CONVERTER:
            parse_converter(L, LS, layer, ident + 1);
            break;
        default:
            push_token(L, LS, &LS->c);
            read_token(L, LS);
            break;
    }

    lua_seti(L, -2, 2);
    lua_pushvalue(L, CONVERTER);
    lua_insert(L, -2);
    lua_call(L, 1, 1);
}

static int next_item(lua_State *L, struct lex_state *LS, int ident) {
    int t = LS->c.type;
    if (t == TOKEN_NEWLINE) {
        int next_ident = token_ident(LS);
        if (next_ident == ident) {
            if (LS->n.type == TOKEN_LIST) return 0;
            read_token(L, LS);
            return 1;
        } else if (next_ident > ident) {
            invalid(L, LS, "Invalid ident");
        } else {
            // end of sequence
            return 0;
        }
    } else if (t == TOKEN_EOF) {
        return 0;
    }
    return 1;
}

static void parse_section_map(lua_State *L, struct lex_state *LS, int ident, int layer) {
    int i = 1;
    int aslist = LS->aslist;
    do {
        if (LS->c.type != TOKEN_ATOM) invalid(L, LS, "Invalid key");
        push_key(L, LS);
        if (read_token(L, LS) != TOKEN_MAP) {
            invalid(L, LS, "Need a : or =");
        }
        objectid tag = 0;
        int t = read_token(L, LS);
        if (t == TOKEN_TAG) {
            tag = parse_tag(L, LS);
            t = read_token(L, LS);
        }

        switch (t) {
            case TOKEN_REF:
                if (tag != 0) {
                    invalid(L, LS, "Invalid ref after tag");
                }
                parse_ref(L, LS);
                break;
            case TOKEN_OPEN:
                parse_bracket(L, LS, layer + 1, tag);
                break;
            case TOKEN_CONVERTER:
                parse_converter(L, LS, layer + 1, ident + 1);
                break;
            case TOKEN_NEWLINE: {
                int next_ident = token_ident(LS);
                if (next_ident <= ident) {
                    invalid(L, LS, "Invalid new section ident");
                }
                new_table(L, layer + 1, tag);
                parse_section(L, LS, layer + 1);
                break;
            }
            default:
                push_token(L, LS, &LS->c);
                read_token(L, LS);
                break;
        }
        if (aslist) {
            lua_insert(L, -2);
            lua_seti(L, -3, i++);
            lua_seti(L, -2, i++);
        } else {
            set_keyvalue(L);
        }
    } while (next_item(L, LS, ident));
}

static void parse_section_sequence(lua_State *L, struct lex_state *LS, int ident, int layer) {
    int n = 1;
    do {
        switch (LS->c.type) {
            case TOKEN_REF:
                parse_ref(L, LS);
                break;
            case TOKEN_OPEN:
                parse_bracket(L, LS, layer + 1, 0);
                break;
            case TOKEN_CONVERTER:
                parse_converter(L, LS, layer + 1, ident + 1);
                break;
            case TOKEN_LIST:
                // end of this section
                return;
            default:
                push_token(L, LS, &LS->c);
                read_token(L, LS);
                break;
        }
        lua_seti(L, -2, n++);
    } while (next_item(L, LS, ident));
}

static int next_list(lua_State *L, struct lex_state *LS, int ident) {
    int t = LS->c.type;
    if (t == TOKEN_NEWLINE) {
        int next_ident = token_ident(LS);
        if (next_ident == ident) {
            switch (read_token(L, LS)) {
                case TOKEN_EOF:
                    return 0;
                case TOKEN_LIST:
                    // next list
                    return 1;
                default:
                    invalid(L, LS, "Invalid list");
                    break;
            }
        } else if (next_ident < ident) {
            // end of sequence
            return 0;
        }
    } else if (t == TOKEN_EOF)
        return 0;
    invalid(L, LS, "Invalid list");
    return 0;
}

static void empty_list(lua_State *L, int n, objectid tag) {
    new_table(L, 0, 0);
    lua_pushvalue(L, -1);
    lua_rawseti(L, REF_CACHE, tag);
    lua_seti(L, -2, n);
}

static void parse_section_list(lua_State *L, struct lex_state *LS, int ident, int layer) {
    int n = 1;
    do {
        int t = read_token(L, LS);
        objectid tag = 0;
        if (t == TOKEN_TAG) {
            tag = parse_tag(L, LS);
            t = read_token(L, LS);
        }
        switch (t) {
            case TOKEN_REF:
                if (tag != 0) {
                    invalid(L, LS, "Invalid ref after tag");
                }
                parse_ref(L, LS);
                break;
            case TOKEN_OPEN:
                parse_bracket(L, LS, layer + 1, tag);
                break;
            case TOKEN_CONVERTER:
                parse_converter(L, LS, layer + 1, ident);
                break;
            case TOKEN_NEWLINE: {
                int next_ident = token_ident(LS);
                if (next_ident >= ident) {
                    new_table(L, layer + 1, tag);
                    if (LS->n.type != TOKEN_LIST || next_ident > ident) {
                        // not an empty list
                        parse_section(L, LS, layer + 1);
                    }
                } else {
                    // end of list
                    empty_list(L, n, tag);
                    return;
                }
                break;
            }
            case TOKEN_EOF:
                // empty list
                empty_list(L, n, tag);
                return;
            default:
                push_token(L, LS, &LS->c);
                read_token(L, LS);
                break;
        }
        lua_seti(L, -2, n++);
    } while (next_list(L, LS, ident));
}

static void parse_section(lua_State *L, struct lex_state *LS, int layer) {
    int ident = token_ident(LS);
    switch (read_token(L, LS)) {
        case TOKEN_ATOM:
            if (LS->n.type == TOKEN_MAP) {
                parse_section_map(L, LS, ident, layer);
                return;
            }
            break;
        case TOKEN_EOF:
            return;
        case TOKEN_STRING:
        case TOKEN_ESCAPESTRING:
        case TOKEN_OPEN:
        case TOKEN_REF:
        case TOKEN_CONVERTER:
            break;
        case TOKEN_LIST:
            parse_section_list(L, LS, ident, layer);
            return;
        default:
            invalid(L, LS, "Invalid section");
    }
    // a sequence
    parse_section_sequence(L, LS, ident, layer);
}

static void init_lex(lua_State *L, int index, struct lex_state *LS) {
    LS->source = luaL_checklstring(L, 1, &LS->sz);
    LS->position = 0;
    LS->newline = 1;
    LS->aslist = 0;
    if (!next_token(LS)) invalid(L, LS, "Invalid token");
}

static int dummy_converter(lua_State *L) { return 1; }

static void parse_all(lua_State *L, struct lex_state *LS) {
    int t = lua_type(L, 2);
    if (t != LUA_TFUNCTION) {
        lua_pushcfunction(L, dummy_converter);
        lua_insert(L, 2);
    } else {
        t = lua_type(L, 3);
    }
    if (t == LUA_TTABLE || t == LUA_TUSERDATA) {
        lua_settop(L, 3);
    } else {
        new_table(L, 0, 0);
    }
    lua_newtable(L);  // ref cache (index 3/REF_CACHE)
    lua_newtable(L);  // unsolved ref (index 4/REF_UNSOLVED)
    lua_rotate(L, -3, 2);
    int tt = read_token(L, LS);
    if (tt == TOKEN_EOF) return;
    NEKO_ASSERT(tt == TOKEN_NEWLINE);
    parse_section(L, LS, 0);
    if (LS->c.type != TOKEN_EOF) {
        invalid(L, LS, "not end");
    }
    // check unsolved
    lua_pushnil(L);
    if (lua_next(L, REF_UNSOLVED) != 0) {
        objectid tag = lua_tointeger(L, -2);
        luaL_error(L, "Unsolved tag %p", (void *)tag);
    }
}

static int lparse(lua_State *L) {
    struct lex_state LS;
    init_lex(L, 1, &LS);
    parse_all(L, &LS);
    lua_pushvalue(L, REF_CACHE);
    return 2;
}

static int lparse_list(lua_State *L) {
    struct lex_state LS;
    init_lex(L, 1, &LS);
    LS.aslist = 1;
    parse_all(L, &LS);
    lua_pushvalue(L, REF_CACHE);
    return 2;
}

static int ltoken(lua_State *L) {
    struct lex_state LS;
    init_lex(L, 1, &LS);

    lua_newtable(L);

    int n = 1;
    while (read_token(L, &LS) != TOKEN_EOF) {
        lua_pushlstring(L, LS.source + LS.c.from, LS.c.to - LS.c.from);
        lua_seti(L, -2, n++);
    }
    return 1;
}

static int lquote(lua_State *L) {
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    luaL_addchar(&b, '"');
    size_t sz, i;
    const char *str = luaL_checklstring(L, 1, &sz);
    for (i = 0; i < sz; i++) {
        if ((unsigned char)str[i] < 32) {
            switch (str[i]) {
                case 0:
                    luaL_addchar(&b, '\\');
                    luaL_addchar(&b, '0');
                    break;
                case '\t':
                    luaL_addchar(&b, '\\');
                    luaL_addchar(&b, 't');
                    break;
                case '\n':
                    luaL_addchar(&b, '\\');
                    luaL_addchar(&b, 'n');
                    break;
                case '\r':
                    luaL_addchar(&b, '\\');
                    luaL_addchar(&b, 'r');
                    break;
                default:
                    luaL_addchar(&b, '\\');
                    luaL_addchar(&b, 'x');
                    luaL_addchar(&b, str[i] / 16 + '0');
                    luaL_addchar(&b, str[i] % 16 + '0');
                    break;
            }
        } else if (str[i] == '"') {
            luaL_addchar(&b, '\\');
            luaL_addchar(&b, '"');
        } else if (str[i] == '\\') {
            luaL_addchar(&b, '\\');
            luaL_addchar(&b, '\\');
        } else {
            luaL_addchar(&b, str[i]);
        }
    }
    luaL_addchar(&b, '"');
    luaL_pushresult(&b);
    return 1;
}

int neko_luaopen_nekonode(lua_State *L) {
    luaL_checkversion(L);
    luaL_Reg l[] = {
            {"parse", lparse}, {"parse_list", lparse_list}, {"token", ltoken}, {"quote", lquote}, {NULL, NULL},
    };

    // neko_lua_load(L, l, "__neko_datalist");
    luaL_newlib(L, l);

    return 1;
}

#pragma endregion lua_datalist

#pragma region lua_ds

#define REFCONSTANT 0x80000000
#define TYPE_SHIFT 28
#define TYPE_MASK (7 << TYPE_SHIFT)
#define VALUE_MASK ((1 << TYPE_SHIFT) - 1)
#define TYPE_BOOL 1
#define TYPE_INT 2
#define TYPE_REAL 3
#define TYPE_STRING 4
#define TYPE_USERDATA 5
#define TYPE_TABLE 6
#define MAX_VALUE ((1 << TYPE_SHIFT) - 1)
#define MAKE_CONSTANT(type, value) ((type) << TYPE_SHIFT | (value))
#define CHANGE_TYPE(v, t) (((v) & ~TYPE_MASK) | (t) << TYPE_SHIFT)

struct constant_table {
    int index;
    int id;
    size_t sz;
};

static u32 constant(lua_State *L, int idx, struct constant_table *ct) {
    u32 c = REFCONSTANT;
    switch (lua_type(L, idx)) {
        case LUA_TNUMBER:
            if (lua_isinteger(L, idx)) {
                lua_Integer n = lua_tointeger(L, idx);
                if (n >= 0 && n <= MAX_VALUE) {
                    return MAKE_CONSTANT(TYPE_INT, n);
                }
                c |= TYPE_INT << TYPE_SHIFT;
            } else {
                c |= TYPE_REAL << TYPE_SHIFT;
            }
            break;
        case LUA_TBOOLEAN:
            return MAKE_CONSTANT(TYPE_BOOL, lua_toboolean(L, idx));
        case LUA_TSTRING:
            c |= TYPE_STRING << TYPE_SHIFT;
            break;
        case LUA_TLIGHTUSERDATA:
            c |= TYPE_STRING << TYPE_USERDATA;
            break;
        default:
            luaL_error(L, "Invalid type %s", lua_typename(L, lua_type(L, idx)));
    }
    lua_pushvalue(L, idx);
    if (lua_rawget(L, ct->index) == LUA_TNUMBER) {
        c |= lua_tointeger(L, -1);
        lua_pop(L, 1);
    } else {
        int id = ct->id++;
        if (id > MAX_VALUE) {
            luaL_error(L, "Too many constant");
        }
        c |= id;
        lua_pop(L, 1);
        lua_pushvalue(L, idx);
        lua_pushinteger(L, id);
        lua_rawset(L, ct->index);
        if (lua_type(L, idx) == LUA_TSTRING) {
            size_t sz;
            lua_tolstring(L, idx, &sz);
            ct->sz += sz + 1;
        }
    }
    return c;
}

static u32 get_constant(lua_State *L, int idx, struct constant_table *ct) {
    lua_rawgeti(L, -1, idx);
    u32 c = constant(L, -1, ct);
    lua_pop(L, 1);
    return c;
}

static u32 get_int(lua_State *L, int index) {
    if (lua_rawgeti(L, -1, index) != LUA_TNUMBER || !lua_isinteger(L, -1)) {
        luaL_error(L, "Invalid type %s, need integer", lua_typename(L, lua_type(L, -1)));
    }
    u32 v = lua_tointeger(L, -1);
    lua_pop(L, 1);
    return v;
}

struct item {
    u32 sibling;
    u32 key;
    u32 value;
};

union constant {
    int64_t n;
    double f;
    void *p;
    const char *s;
};

typedef struct neko_lua_ds_s {
    size_t size;
    u32 maxid;
    u32 maxconstant;
} neko_lua_ds_t;

static inline union constant *ds_constant(const struct neko_lua_ds_s *t, u32 k) {
    union constant *c = (union constant *)(t + 1);
    return &c[k];
}

static inline struct item *ds_item(const struct neko_lua_ds_s *t, u32 index) {
    struct item *item = (struct item *)ds_constant(t, t->maxconstant);
    return &item[index];
}

static inline char *ds_stringbuffer(const struct neko_lua_ds_s *t) { return (char *)ds_item(t, t->maxid); }

static inline const char *ds_string(const struct neko_lua_ds_s *t, u32 index) {
    const char *stringtable = ds_stringbuffer(t);
    uintptr_t offset = ds_constant(t, index)->n;
    return &stringtable[offset];
}

static inline int ds_type(u32 k) { return (k & TYPE_MASK) >> TYPE_SHIFT; }

static union constant ds_value(const struct neko_lua_ds_s *t, u32 k) {
    u32 v = k & VALUE_MASK;
    union constant c = {0};
    int type = ds_type(k);
    if (k & REFCONSTANT) {
        if (type == TYPE_STRING) {
            c.s = ds_string(t, v);
        } else {
            c = *ds_constant(t, v);
        }
    }
    switch (type) {
        case TYPE_BOOL:
        case TYPE_INT:
        case TYPE_TABLE:
            c.n = v;
            break;
    }
    return c;
}

static void make_constant_table(lua_State *L, struct constant_table *ct, struct neko_lua_ds_s *dt) {
    char *buffer = ds_stringbuffer(dt);
    size_t offset = 0;
    lua_pushnil(L);
    while (lua_next(L, ct->index) != 0) {
        u32 id = lua_tointeger(L, -1);
        union constant *c = ds_constant(dt, id);
        switch (lua_type(L, -2)) {
            case LUA_TNUMBER:
                if (lua_isinteger(L, -2)) {
                    c->n = lua_tointeger(L, -2);
                } else {
                    c->f = lua_tonumber(L, -2);
                }
                break;
            case LUA_TSTRING: {
                size_t sz;
                const char *s = lua_tolstring(L, -2, &sz);
                sz += 1;
                memcpy(buffer, s, sz);
                c->n = offset;
                offset += sz;
                buffer += sz;
                break;
            }
            case LUA_TLIGHTUSERDATA:
                c->p = lua_touserdata(L, -2);
                break;
            default:
                luaL_error(L, "Invalid constant type %s", lua_typename(L, lua_type(L, -2)));
        }
        lua_pop(L, 1);
    }
}

static void print_value(struct neko_lua_ds_s *dt, u32 v) {
    union constant c = ds_value(dt, v);
    switch (ds_type(v)) {
        case TYPE_BOOL:
            printf("%s", c.n ? "true" : "false");
            break;
        case TYPE_INT:
            printf("%d", (int)c.n);
            break;
        case TYPE_REAL:
            printf("%f", c.f);
            break;
        case TYPE_STRING:
            printf("%s", c.s);
            break;
        case TYPE_USERDATA:
            printf("%p", c.p);
            break;
        case TYPE_TABLE:
            printf("[%d]", (int)c.n);
            break;
        default:
            printf("[Invalid]");
    }
}

static int ldump(lua_State *L) {
    luaL_checktype(L, 1, LUA_TUSERDATA);
    struct neko_lua_ds_s *dt = (struct neko_lua_ds_s *)lua_touserdata(L, 1);
    size_t sz = lua_rawlen(L, 1);
    if (sz < sizeof(struct neko_lua_ds_s) || dt->size != sz) return luaL_error(L, "Invalid datatree");
    int i;
    printf("ROOT");
    print_value(dt, ds_item(dt, 0)->value);
    printf("\n");
    for (i = 1; i < dt->maxid; i++) {
        struct item *item = ds_item(dt, i);
        if (item->value != 0) {
            printf("[%d] -> %d ", i + 1, item->sibling);
            print_value(dt, item->key);
            printf(" : ");
            print_value(dt, item->value);
            printf("\n");
        }
    }
    return 0;
}

static int lpack(lua_State *L) {
    u32 first_child = luaL_checkinteger(L, 1);
    if (first_child >= MAX_VALUE) {
        return luaL_error(L, "Too many items");
    }
    luaL_checktype(L, 2, LUA_TTABLE);
    int n = lua_rawlen(L, 2);
    lua_newtable(L);
    struct constant_table ct = {
            lua_gettop(L),
            0,
            0,
    };
    int i;
    int maxid = 0;
    for (i = 1; i <= n; i++) {
        if (lua_rawgeti(L, 2, i) != LUA_TTABLE) {
            return luaL_error(L, "Invalid item %d", i);
        }
        u32 nodeid = get_int(L, 1);
        if (nodeid > maxid) maxid = nodeid;
        if (nodeid <= 1) {
            return luaL_error(L, "Invalid node id %d", (int)nodeid);
        }
        get_constant(L, 3, &ct);  // 钥匙
        if (lua_rawgeti(L, -1, 5) == LUA_TNIL) {
            lua_pop(L, 1);
            get_constant(L, 4, &ct);  // 值
        } else {
            lua_pop(L, 1);
            get_constant(L, 5, &ct);  // 子表
        }
        lua_pop(L, 1);
    }
    maxid = maxid + 1;
    size_t sz = sizeof(struct neko_lua_ds_s) + maxid * sizeof(struct item) + ct.id * sizeof(union constant) + ct.sz;
    struct neko_lua_ds_s *dt = (struct neko_lua_ds_s *)lua_newuserdatauv(L, sz, 0);
    dt->size = sz;
    dt->maxid = maxid;
    dt->maxconstant = ct.id;
    struct item *root = ds_item(dt, 0);
    memset(root, 0, sizeof(struct item) * maxid);
    root->sibling = 0;
    root->key = 0;
    root->value = MAKE_CONSTANT(TYPE_TABLE, first_child);
    for (i = 1; i <= n; i++) {
        lua_rawgeti(L, 2, i);
        u32 nodeid = get_int(L, 1);
        struct item *item = ds_item(dt, nodeid - 1);
        item->sibling = get_int(L, 2);
        item->key = get_constant(L, 3, &ct);
        if (lua_rawgeti(L, -1, 5) == LUA_TNIL) {
            lua_pop(L, 1);
            item->value = get_constant(L, 4, &ct);  // 值
        } else {
            lua_pop(L, 1);
            item->value = CHANGE_TYPE(get_constant(L, 5, &ct), TYPE_TABLE);  // 子表
        }
        lua_pop(L, 1);
    }

    make_constant_table(L, &ct, dt);
    return 1;
}

static int push_value(lua_State *L, const struct neko_lua_ds_s *dt, u32 v) {
    union constant c = ds_value(dt, v);
    int t = ds_type(v);
    switch (t) {
        case TYPE_BOOL:
            lua_pushboolean(L, c.n);
            break;
        case TYPE_INT:
            lua_pushinteger(L, c.n);
            break;
        case TYPE_REAL:
            lua_pushnumber(L, c.f);
            break;
        case TYPE_USERDATA:
            lua_pushlightuserdata(L, c.p);
            break;
        case TYPE_STRING:
            lua_pushstring(L, c.s);
            break;
        case TYPE_TABLE:
            break;
        default:
            luaL_error(L, "Invalid datatree value type");
    }
    return t;
}

static int ldecode(lua_State *L) {
    const struct neko_lua_ds_s *dt = NULL;
    switch (lua_type(L, 1)) {
        case LUA_TSTRING: {
            size_t sz;
            dt = (const struct neko_lua_ds_s *)lua_tolstring(L, 1, &sz);
            if (sz < sizeof(struct neko_lua_ds_s) || dt->size != sz) {
                return luaL_error(L, "Invalid datatree size (string)");
            }
            break;
        }
        case LUA_TUSERDATA: {
            size_t sz = lua_rawlen(L, 1);
            dt = (const struct neko_lua_ds_s *)lua_touserdata(L, 1);
            if (sz < sizeof(struct neko_lua_ds_s) || dt->size != sz) {
                return luaL_error(L, "Invalid datatree size (userdata)");
            }
            break;
        }
        case LUA_TLIGHTUSERDATA:
            dt = (const struct neko_lua_ds_s *)lua_touserdata(L, 1);
            break;
        default:
            return luaL_error(L, "Invalid datatree type (%s)", lua_typename(L, lua_type(L, 1)));
    }
    luaL_checktype(L, 2, LUA_TTABLE);
    u32 id = luaL_checkinteger(L, 3);
    if (id == 0 || id >= dt->maxid) return luaL_error(L, "Invalid node id (%d)", id);
    struct item *item = ds_item(dt, id - 1);
    if (ds_type(item->value) != TYPE_TABLE) {
        return luaL_error(L, "Node (%d) is not a table", id);
    }
    id = ds_value(dt, item->value).n;
    while (id) {
        item = ds_item(dt, id - 1);
        push_value(L, dt, item->key);
        if (push_value(L, dt, item->value) == TYPE_TABLE) {
            lua_createtable(L, 1, 0);
            lua_pushinteger(L, id);
            lua_rawseti(L, -2, 1);
            // 触发元方法
            lua_settable(L, 2);
        } else {
            lua_rawset(L, 2);
        }
        id = item->sibling;
    }
    return 0;
}

int luaopen_ds_core(lua_State *L) {
    luaL_checkversion(L);
    luaL_Reg l[] = {
            {"pack", lpack},
            {"dump", ldump},
            {"decode", ldecode},
            {NULL, NULL},
    };
    luaL_newlib(L, l);
    return 1;
}

#pragma endregion lua_ds