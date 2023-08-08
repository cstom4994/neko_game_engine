
#include "engine/scripting/neko_lua_base.h"

#include <assert.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "engine/base/neko_engine.h"
#include "engine/common/neko_util.h"
#include "engine/platform/neko_platform.h"

#pragma region lua_safe_alloc

void *lua_simple_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
    (void)ud;
    (void)osize;
    if (nsize == 0) {
        free(ptr);
        return NULL;
    } else
        return realloc(ptr, nsize);
}

struct lua_allocator *new_allocator(void) {
    struct lua_allocator *alloc = (struct lua_allocator *)malloc(sizeof(struct lua_allocator));
    alloc->blocks = (struct lua_mem_block *)malloc(sizeof(struct lua_mem_block) * 4);
    alloc->nb_blocks = 0;
    alloc->size_blocks = 4;
    alloc->total_allocated = 0;
#ifdef _DEBUG_ALLOC
    fprintf(stderr, "Created allocator %p\n", alloc);
#endif
    return alloc;
}

void delete_allocator(struct lua_allocator *alloc) {
    size_t blk;
#ifdef _DEBUG_ALLOC
    fprintf(stderr, "Deleting allocator %p\n", alloc);
#endif
    for (blk = 0; blk < alloc->nb_blocks; ++blk) free(alloc->blocks[blk].ptr);
    free(alloc->blocks);
    free(alloc);
}

void *lua_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
    struct lua_allocator *alloc = (struct lua_allocator *)ud;
    size_t blk;
#ifdef _DEBUG_ALLOC
    {
        size_t i;
        for (i = 0; i < alloc->nb_blocks; ++i) {
            fprintf(stderr, "%p %u ", alloc->blocks[i].ptr, alloc->blocks[i].size);
            if (i % 4 == 0) fprintf(stderr, "\n");
        }
        if (i % 4 != 1)
            fprintf(stderr, "\n\n");
        else
            fprintf(stderr, "\n");
    }
    fprintf(stderr,
            "Lua request on allocator %p: "
            "ptr=%p, osize=%u, nsize=%u\n",
            alloc, ptr, osize, nsize);
#endif
    if (ptr == NULL) {
        if (nsize == 0) {
#ifdef _DEBUG_ALLOC
            fprintf(stderr, "Returning NULL...\n");
#endif
            return NULL;
        }
        blk = alloc->nb_blocks;
        alloc->nb_blocks++;
        if (alloc->nb_blocks > alloc->size_blocks) {
            alloc->size_blocks *= 2;
            alloc->blocks = (struct lua_mem_block *)realloc(alloc->blocks, sizeof(struct lua_mem_block) * alloc->size_blocks);
#ifdef _DEBUG_ALLOC
            fprintf(stderr, "Growing block table to %u blocks\n", alloc->size_blocks);
#endif
        }
        alloc->blocks[blk].ptr = NULL;
        alloc->blocks[blk].size = 0;
#ifdef _DEBUG_ALLOC
        fprintf(stderr, "Allocated new block %u\n", blk);
#endif
    } else {
        /* Bisect to the block */
        size_t low = 0, high = alloc->nb_blocks;
        while (low < high) {
            size_t mid = (low + high) / 2;
            if (alloc->blocks[mid].ptr < ptr)
                low = mid + 1;
            else
                high = mid;
        }
        blk = low;
#ifdef _DEBUG_ALLOC
        fprintf(stderr, "Found block %u\n", blk);
#endif
    }
    assert(alloc->blocks[blk].ptr == ptr && (!ptr || alloc->blocks[blk].size == osize));
    if (nsize == 0) {
        free(ptr);
        alloc->total_allocated -= osize;
        memmove(&alloc->blocks[blk], &alloc->blocks[blk + 1], sizeof(struct lua_mem_block) * (alloc->nb_blocks - blk - 1));
        alloc->nb_blocks--;
#ifdef _DEBUG_ALLOC
        fprintf(stderr, "Removed block, now have %u blocks, %u bytes\n\n", alloc->nb_blocks, alloc->total_allocated);
#endif
        return NULL;
    } else {
        ptr = realloc(ptr, nsize);
#ifdef _DEBUG_ALLOC
        fprintf(stderr, "ptr=%p ", ptr);
#endif
        alloc->blocks[blk].ptr = ptr;
        alloc->total_allocated += nsize - alloc->blocks[blk].size;
        alloc->blocks[blk].size = nsize;
        while (blk > 0 && alloc->blocks[blk].ptr < alloc->blocks[blk - 1].ptr) {
            struct lua_mem_block tmp = alloc->blocks[blk];
            alloc->blocks[blk] = alloc->blocks[blk - 1];
            alloc->blocks[blk - 1] = tmp;
            blk--;
#ifdef _DEBUG_ALLOC
            fprintf(stderr, "< ");
#endif
        }
        while (blk + 1 < alloc->nb_blocks && alloc->blocks[blk].ptr > alloc->blocks[blk + 1].ptr) {
            struct lua_mem_block tmp = alloc->blocks[blk];
            alloc->blocks[blk] = alloc->blocks[blk + 1];
            alloc->blocks[blk + 1] = tmp;
            blk++;
#ifdef _DEBUG_ALLOC
            fprintf(stderr, "> ");
#endif
        }
#ifdef _DEBUG_ALLOC
        fprintf(stderr, "\n");
        fprintf(stderr, "Moved block to %u, now have %u bytes\n\n", blk, alloc->total_allocated);
#endif
        return ptr;
    }
}

#pragma endregion lua_safe_alloc

#pragma region LuaA

void __neko_lua_auto_open(lua_State *L) {

    lua_pushinteger(L, 0);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "type_index");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "type_ids");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "type_names");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "type_sizes");

    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "stack_push");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "stack_to");

    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "structs");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "structs_offset");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "enums");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "enums_sizes");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "enums_values");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "functions");

    lua_newuserdata(L, LUAA_RETURN_STACK_SIZE);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "call_ret_stk");
    lua_newuserdata(L, LUAA_ARGUMENT_STACK_SIZE);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "call_arg_stk");
    lua_pushinteger(L, 0);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "call_ret_ptr");
    lua_pushinteger(L, 0);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "call_arg_ptr");

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
    neko_lua_auto_conversion(L, unsigned long, neko_lua_auto_push_unsigned_long, neko_lua_auto_to_unsigned_long);
    neko_lua_auto_conversion(L, long long, neko_lua_auto_push_long_long, neko_lua_auto_to_long_long);
    neko_lua_auto_conversion(L, unsigned long long, neko_lua_auto_push_unsigned_long_long, neko_lua_auto_to_unsigned_long_long);
    neko_lua_auto_conversion(L, float, neko_lua_auto_push_float, neko_lua_auto_to_float);
    neko_lua_auto_conversion(L, double, neko_lua_auto_push_double, neko_lua_auto_to_double);
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

void __neko_lua_auto_close(lua_State *L) {

    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "type_index");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "type_ids");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "type_names");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "type_sizes");

    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "stack_push");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "stack_to");

    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "structs");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "structs_offset");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "enums");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "enums_sizes");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "enums_values");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "functions");

    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "call_ret_stk");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "call_arg_stk");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "call_ret_ptr");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "call_arg_ptr");
}

neko_lua_auto_Type neko_lua_auto_type_add(lua_State *L, const char *type, size_t size) {

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "type_ids");
    lua_getfield(L, -1, type);

    if (lua_isnumber(L, -1)) {

        neko_lua_auto_Type id = lua_tointeger(L, -1);
        lua_pop(L, 2);
        return id;

    } else {

        lua_pop(L, 2);

        lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "type_index");

        neko_lua_auto_Type id = lua_tointeger(L, -1);
        lua_pop(L, 1);
        id++;

        lua_pushinteger(L, id);
        lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "type_index");

        lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "type_ids");
        lua_pushinteger(L, id);
        lua_setfield(L, -2, type);
        lua_pop(L, 1);

        lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "type_names");
        lua_pushinteger(L, id);
        lua_pushstring(L, type);
        lua_settable(L, -3);
        lua_pop(L, 1);

        lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "type_sizes");
        lua_pushinteger(L, id);
        lua_pushinteger(L, size);
        lua_settable(L, -3);
        lua_pop(L, 1);

        return id;
    }
}

neko_lua_auto_Type neko_lua_auto_type_find(lua_State *L, const char *type) {

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "type_ids");
    lua_getfield(L, -1, type);

    neko_lua_auto_Type id = lua_isnil(L, -1) ? LUAA_INVALID_TYPE : lua_tointeger(L, -1);
    lua_pop(L, 2);

    return id;
}

const char *neko_lua_auto_typename(lua_State *L, neko_lua_auto_Type id) {

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "type_names");
    lua_pushinteger(L, id);
    lua_gettable(L, -2);

    const char *type = lua_isnil(L, -1) ? "LUAA_INVALID_TYPE" : lua_tostring(L, -1);
    lua_pop(L, 2);

    return type;
}

size_t neko_lua_auto_typesize(lua_State *L, neko_lua_auto_Type id) {

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "type_sizes");
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

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "stack_push");
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

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "stack_to");
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
    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "stack_push");
    lua_pushinteger(L, type_id);
    lua_pushlightuserdata(L, func);
    lua_settable(L, -3);
    lua_pop(L, 1);
}

void neko_lua_auto_conversion_to_type(lua_State *L, neko_lua_auto_Type type_id, neko_lua_auto_Tofunc func) {
    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "stack_to");
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

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "stack_push");
    lua_pushinteger(L, type_id);
    lua_gettable(L, -2);

    bool reg = !lua_isnil(L, -1);
    lua_pop(L, 2);

    return reg;
}

bool neko_lua_auto_conversion_to_registered_type(lua_State *L, neko_lua_auto_Type type_id) {

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "stack_to");
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

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "structs_offset");
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

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "structs");
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

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "structs_offset");
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

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "structs");
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

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "structs_offset");
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

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "structs");
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

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "structs_offset");
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

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "structs");
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
    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "structs");
    lua_pushinteger(L, type);
    lua_newtable(L);
    lua_settable(L, -3);
    lua_pop(L, 1);

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "structs_offset");
    lua_pushinteger(L, type);
    lua_newtable(L);
    lua_settable(L, -3);
    lua_pop(L, 1);
}

void neko_lua_auto_struct_member_type(lua_State *L, neko_lua_auto_Type type, const char *member, neko_lua_auto_Type mtype, size_t offset) {

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "structs");
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

        lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "structs_offset");
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

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "structs");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    bool reg = !lua_isnil(L, -1);
    lua_pop(L, 2);

    return reg;
}

int neko_lua_auto_struct_push_type(lua_State *L, neko_lua_auto_Type type, const void *c_in) {

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "structs");
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

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "structs");
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

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "enums_values");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "enums_sizes");
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

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "enums");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "enums_sizes");
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

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "enums_values");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "enums_sizes");
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
    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "enums");
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
    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "enums");
    lua_pushinteger(L, type);
    lua_newtable(L);
    lua_settable(L, -3);
    lua_pop(L, 1);

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "enums_values");
    lua_pushinteger(L, type);
    lua_newtable(L);
    lua_settable(L, -3);
    lua_pop(L, 1);

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "enums_sizes");
    lua_pushinteger(L, type);
    lua_pushinteger(L, size);
    lua_settable(L, -3);
    lua_pop(L, 1);
}

void neko_lua_auto_enum_value_type(lua_State *L, neko_lua_auto_Type type, const void *value, const char *name) {

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "enums");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);

    if (!lua_isnil(L, -1)) {

        lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "enums_sizes");
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

        lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "enums_values");
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
    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "enums");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);
    bool reg = !lua_isnil(L, -1);
    lua_pop(L, 2);
    return reg;
}

const char *neko_lua_auto_enum_next_value_name_type(lua_State *L, neko_lua_auto_Type type, const char *member) {

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "enums");
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

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "call_ret_stk");
    void *ret_stack = lua_touserdata(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "call_arg_stk");
    void *arg_stack = lua_touserdata(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "call_ret_ptr");
    lua_Integer ret_ptr = lua_tointeger(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "call_arg_ptr");
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
        lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "call_ret_ptr");
    }

    if (!arg_heap) {
        lua_pushinteger(L, arg_ptr + arg_size);
        lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "call_arg_ptr");
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
        lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "call_ret_ptr");
    } else {
        free(ret_data);
    }

    if (!arg_heap) {
        lua_pushinteger(L, arg_ptr);
        lua_setfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "argument_ptr");
    } else {
        free(arg_data);
    }

    return count;
}

int neko_lua_auto_call(lua_State *L, void *func_ptr) {
    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "functions");
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
    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "functions");
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

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "functions");
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

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "functions");
    lua_pushlightuserdata(L, src_func);

    lua_getfield(L, LUA_REGISTRYINDEX, LUAA_REGISTRYPREFIX "functions");
    lua_getfield(L, -1, name);
    lua_remove(L, -2);

    lua_settable(L, -3);
    lua_pop(L, 1);
}

#pragma endregion LuaA

int luaopen_debugger(lua_State *lua) {
    if (luaL_dofile(lua, neko_file_path("data/scripts/libs/debugger.lua"))) lua_error(lua);
    return 1;
}

static const char *MODULE_NAME = "DEBUGGER_LUA_MODULE";
static const char *MSGH = "DEBUGGER_LUA_MSGH";

void neko_lua_debug_setup(lua_State *lua, const char *name, const char *globalName, lua_CFunction readFunc, lua_CFunction writeFunc) {
    // Check that the module name was not already defined.
    lua_getfield(lua, LUA_REGISTRYINDEX, MODULE_NAME);
    assert(lua_isnil(lua, -1) || strcmp(name, luaL_checkstring(lua, -1)));
    lua_pop(lua, 1);

    // Push the module name into the registry.
    lua_pushstring(lua, name);
    lua_setfield(lua, LUA_REGISTRYINDEX, MODULE_NAME);

    // Preload the module
    luaL_requiref(lua, name, luaopen_debugger, false);

    // Insert the msgh function into the registry.
    lua_getfield(lua, -1, "msgh");
    lua_setfield(lua, LUA_REGISTRYINDEX, MSGH);

    if (readFunc) {
        lua_pushcfunction(lua, readFunc);
        lua_setfield(lua, -2, "read");
    }

    if (writeFunc) {
        lua_pushcfunction(lua, writeFunc);
        lua_setfield(lua, -2, "write");
    }

    if (globalName) {
        lua_setglobal(lua, globalName);
    } else {
        lua_pop(lua, 1);
    }
}

int neko_lua_debug_pcall(lua_State *lua, int nargs, int nresults, int msgh) {
    // Call regular lua_pcall() if a message handler is provided.
    if (msgh) return lua_pcall(lua, nargs, nresults, msgh);

    // Grab the msgh function out of the registry.
    lua_getfield(lua, LUA_REGISTRYINDEX, MSGH);
    if (lua_isnil(lua, -1)) {
        luaL_error(lua, "Tried to call dbg_call() before calling dbg_setup().");
    }

    // Move the error handler just below the function.
    msgh = lua_gettop(lua) - (1 + nargs);
    lua_insert(lua, msgh);

    // Call the function.
    int err = lua_pcall(lua, nargs, nresults, msgh);

    // Remove the debug handler.
    lua_remove(lua, msgh);

    return err;
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

#pragma region LuaMemSafe

const char *const REG_POLICY = "neko_lua_safe_policy";
const char *const REG_CANCEL = "neko_lua_safe_cancel";
const char *const REG_CANCELUDATA = "neko_lua_safe_canceludata";
const char *const REG_CANCELJMP = "neko_lua_safe_canceljmp";

#define setudregistry(state, value, name)                \
    do {                                                 \
        lua_State *state_ = (state);                     \
        lua_pushlightuserdata(state_, (value));          \
        lua_setfield(state_, LUA_REGISTRYINDEX, (name)); \
    } while (0)

static void *getudregistry(lua_State *state, const char *name) {
    void *ret;
    lua_getfield(state, LUA_REGISTRYINDEX, name);
    ret = lua_touserdata(state, -1);
    lua_pop(state, 1);
    return ret;
}

struct Handler {
    neko_lua_safe_Handler callback;
    void *udata;
};

struct Policy {
    struct Allocator *allocator;
    struct Handler *handlers;
    size_t nb_handlers, size_handlers;
};

lua_State *neko_lua_safe_open(void) {
    struct Policy *policy = (Policy *)malloc(sizeof(struct Policy));
    policy->allocator = (Allocator *)new_allocator();
    policy->handlers = NULL;
    policy->nb_handlers = policy->size_handlers = 0;

    /* Create state with allocator */
    lua_State *state = lua_newstate(lua_alloc, policy->allocator);

    /* Open standard libraries */
    luaL_requiref(state, "", luaopen_base, 0);
    luaL_requiref(state, "coroutine", luaopen_coroutine, 1);
    luaL_requiref(state, "package", luaopen_package, 1);
    luaL_requiref(state, "string", luaopen_string, 1);
    luaL_requiref(state, "table", luaopen_table, 1);
    luaL_requiref(state, "math", luaopen_math, 1);
    luaL_requiref(state, "utf8", luaopen_utf8, 1);
    luaL_requiref(state, "io", luaopen_io, 1);
    luaL_requiref(state, "os", luaopen_os, 1);
    // without luaopen_debug

    setudregistry(state, policy, REG_POLICY);

    return state;
}

static void free_resources(struct Policy *policy, int why) {
    size_t i;
    for (i = 0; i < policy->nb_handlers; ++i) policy->handlers[i].callback(why, policy->handlers[i].udata);
    free(policy->handlers);
}

void neko_lua_safe_close(lua_State *state) {
    struct Policy *policy = (Policy *)getudregistry(state, REG_POLICY);
    lua_close(state);
    free_resources(policy, SAFELUA_FINISHED);
    delete_allocator((struct lua_allocator *)policy->allocator);
    free(policy);
}

static void cancel_hook(lua_State *state, lua_Debug *ar) { neko_lua_safe_checkcancel(state); }

int neko_lua_safe_pcallk(lua_State *state, int nargs, int nresults, int errfunc, int ctx, lua_KFunction k, neko_lua_safe_CancelCheck cancel, void *canceludata) {
    struct Policy *policy = (Policy *)getudregistry(state, REG_POLICY);
    jmp_buf env;
    int ret;

    /* If catch point is already set, do a normal pcallk */
    if (getudregistry(state, REG_CANCELJMP) != NULL)
        return lua_pcallk(state, nargs, nresults, errfunc, ctx, k);
    else {
        /* Set hook from which we will be able to exit */
        lua_sethook(state, cancel_hook, LUA_MASKCOUNT, 10);

        if (setjmp(env) == 0) {
            setudregistry(state, &env, REG_CANCELJMP);
            setudregistry(state, cancel, REG_CANCEL);
            setudregistry(state, canceludata, REG_CANCELUDATA);
            ret = lua_pcallk(state, nargs, nresults, errfunc, ctx, k);
            lua_pushnil(state);
            lua_setfield(state, LUA_REGISTRYINDEX, REG_CANCEL);
            lua_pushnil(state);
            lua_setfield(state, LUA_REGISTRYINDEX, REG_CANCELJMP);
            lua_sethook(state, cancel_hook, 0, 0);
        } else {
            free_resources(policy, SAFELUA_CANCELED);
            delete_allocator((struct lua_allocator *)policy->allocator);
            free(policy);
            ret = SAFELUA_CANCELED;
        }

        return ret;
    }
}

void neko_lua_safe_checkcancel(lua_State *state) {
    if (neko_lua_safe_shouldcancel(state)) neko_lua_safe_cancel(state);
}

void neko_lua_safe_cancel(lua_State *state) {
    jmp_buf *env = (jmp_buf *)getudregistry(state, REG_CANCELJMP);
    if (env) longjmp(*env, 1);
}

int neko_lua_safe_shouldcancel(lua_State *state) {
    neko_lua_safe_CancelCheck cancel = (neko_lua_safe_CancelCheck)getudregistry(state, REG_CANCEL);
    void *canceludata = getudregistry(state, REG_CANCELUDATA);

    return cancel && cancel(state, canceludata);
}

void neko_lua_safe_add_handler(lua_State *state, neko_lua_safe_Handler handler, void *handlerudata) {
    struct Policy *policy = (Policy *)getudregistry(state, REG_POLICY);
    if (++policy->nb_handlers > policy->size_handlers) {
        if (policy->size_handlers == 0)
            policy->size_handlers = 4;
        else
            policy->size_handlers *= 2;
        policy->handlers = (Handler *)realloc(policy->handlers, sizeof(struct Handler) * policy->size_handlers);
    }
    {
        struct Handler *h = &policy->handlers[policy->nb_handlers - 1];
        h->callback = handler;
        h->udata = handlerudata;
    }
}

int neko_lua_safe_remove_handler(lua_State *state, neko_lua_safe_Handler handler, void *handlerudata) {
    struct Policy *policy = (Policy *)getudregistry(state, REG_POLICY);
    size_t found = 0;
    size_t pos = 0;
    for (; pos < policy->nb_handlers; ++pos) {
        if (policy->handlers[pos].callback == handler && policy->handlers[pos].udata == handlerudata)
            found++;
        else
            policy->handlers[pos - found] = policy->handlers[pos];
    }
    policy->nb_handlers -= found;
    return found;
}

#pragma endregion LuaMemSafe

#pragma region LuaStructLoader

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

int luaopen_cstruct_core(lua_State *L) {
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

int luaopen_cstruct_test(lua_State *L) {
    luaL_checkversion(L);
    luaL_Reg l[] = {
            {"udata", newudata},
            {"NULL", NULL},
            {NULL, NULL},
    };
    // luaL_newlib(L, l);
    neko_lua_load(L, l, "__neko_cstruct_test");
    // lua_pushlightuserdata(L, NULL);
    // lua_setfield(L, -2, "NULL");
    return 1;
}

#pragma endregion LuaStructLoader
