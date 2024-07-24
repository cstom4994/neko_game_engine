#include "neko_lua_struct.h"

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

#define LUASTRUCT_access_u8 LUASTRUCT_access_uchar

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

LUASTRUCT_BEGIN(Color)
LUASTRUCT_FIELD(r, u8)
LUASTRUCT_FIELD(g, u8)
LUASTRUCT_FIELD(b, u8)
LUASTRUCT_FIELD(a, u8)
LUASTRUCT_END

void createStructTables(lua_State *L) {
    Vector4_create(L, "Vector4");
    Color_create(L, "Color");

    ARRAY_uchar8_create(L);
}

// luabind

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

// lua struct

#include "engine/neko_luabind.hpp"

enum class TYPEID { f_int8, f_int16, f_int32, f_int64, f_uint8, f_uint16, f_uint32, f_uint64, f_bool, f_ptr, f_float, f_double, f_COUNT };

#define TYPE_ID_(type) ((int)TYPEID::f_##type)

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
    switch ((TYPEID)type) {
        case TYPEID::f_int8:
            return sizeof(int8_t);
        case TYPEID::f_int16:
            return sizeof(int16_t);
        case TYPEID::f_int32:
            return sizeof(int32_t);
        case TYPEID::f_int64:
            return sizeof(int64_t);
        case TYPEID::f_uint8:
            return sizeof(uint8_t);
        case TYPEID::f_uint16:
            return sizeof(uint16_t);
        case TYPEID::f_uint32:
            return sizeof(uint32_t);
        case TYPEID::f_uint64:
            return sizeof(uint64_t);
        case TYPEID::f_ptr:
            return sizeof(void *);
        case TYPEID::f_bool:
            return 1;
        case TYPEID::f_float:
            return sizeof(float);
        case TYPEID::f_double:
            return sizeof(double);
        default:
            return 0;
    }
}

static int setter(lua_State *L, void *p, int type, int offset) {
    p = (char *)p + offset * get_stride(type);
    switch (type) {
        case TYPE_ID_(int8):
            set_int8(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(int16):
            set_int16(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(int32):
            set_int32(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(int64):
            set_int64(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(uint8):
            set_uint8(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(uint16):
            set_uint16(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(uint32):
            set_uint32(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(uint64):
            set_uint64(p, luaL_checkinteger(L, 2));
            break;
        case TYPE_ID_(bool):
            set_bool(p, lua_toboolean(L, 2));
            break;
        case TYPE_ID_(ptr):
            set_ptr(p, lua_touserdata(L, 2));
            break;
        case TYPE_ID_(float):
            set_float(p, luaL_checknumber(L, 2));
            break;
        case TYPE_ID_(double):
            set_double(p, luaL_checknumber(L, 2));
            break;
    }
    return 0;
}

static inline int getter(lua_State *L, void *p, int type, int offset) {
    p = (char *)p + offset * get_stride(type);
    switch (type) {
        case TYPE_ID_(int8):
            lua_pushinteger(L, get_int8(p));
            break;
        case TYPE_ID_(int16):
            lua_pushinteger(L, get_int16(p));
            break;
        case TYPE_ID_(int32):
            lua_pushinteger(L, get_int32(p));
            break;
        case TYPE_ID_(int64):
            lua_pushinteger(L, (lua_Integer)get_int64(p));
            break;
        case TYPE_ID_(uint8):
            lua_pushinteger(L, get_int8(p));
            break;
        case TYPE_ID_(uint16):
            lua_pushinteger(L, get_int16(p));
            break;
        case TYPE_ID_(uint32):
            lua_pushinteger(L, get_int32(p));
            break;
        case TYPE_ID_(uint64):
            lua_pushinteger(L, (lua_Integer)get_int64(p));
            break;
        case TYPE_ID_(bool):
            lua_pushboolean(L, get_bool(p));
            break;
        case TYPE_ID_(ptr):
            lua_pushlightuserdata(L, get_ptr(p));
            break;
        case TYPE_ID_(float):
            lua_pushnumber(L, get_float(p));
            break;
        case TYPE_ID_(double):
            lua_pushnumber(L, get_double(p));
            break;
    }
    return 1;
}

#define ACCESSOR_FUNC(TYPE, OFF)                                                                                 \
    static int get_##TYPE##_##OFF(lua_State *L) { return getter(L, lua_touserdata(L, 1), TYPE_ID_(TYPE), OFF); } \
    static int set_##TYPE##_##OFF(lua_State *L) { return setter(L, lua_touserdata(L, 1), TYPE_ID_(TYPE), OFF); }

#define ACCESSOR_OFFSET(TYPE)                                                                                                                       \
    static int get_##TYPE##_offset(lua_State *L) { return getter(L, lua_touserdata(L, 1), TYPE_ID_(TYPE), lua_tointeger(L, lua_upvalueindex(1))); } \
    static int set_##TYPE##_offset(lua_State *L) { return setter(L, lua_touserdata(L, 1), TYPE_ID_(TYPE), lua_tointeger(L, lua_upvalueindex(1))); }

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

#define XX(TYPE)           \
    ACCESSOR_FUNC(TYPE, 0) \
    ACCESSOR_FUNC(TYPE, 1) \
    ACCESSOR_FUNC(TYPE, 2) \
    ACCESSOR_FUNC(TYPE, 3) \
    ACCESSOR_FUNC(TYPE, 4) \
    ACCESSOR_FUNC(TYPE, 5) \
    ACCESSOR_FUNC(TYPE, 6) \
    ACCESSOR_FUNC(TYPE, 7) \
    ACCESSOR_OFFSET(TYPE)  \
    ACCESSOR(get, TYPE)    \
    ACCESSOR(set, TYPE)

XX(float)
XX(double)
XX(int8)
XX(int16)
XX(int32)
XX(int64)
XX(uint8)
XX(uint16)
XX(uint32)
XX(uint64)
XX(bool)
XX(ptr)

#undef XX

static inline int get_value(lua_State *L, int type, int offset) {
    switch ((TYPEID)type) {
        case TYPEID::f_int8:
            getter_func_int8(L, offset);
            break;
        case TYPEID::f_int16:
            getter_func_int16(L, offset);
            break;
        case TYPEID::f_int32:
            getter_func_int32(L, offset);
            break;
        case TYPEID::f_int64:
            getter_func_int64(L, offset);
            break;
        case TYPEID::f_uint8:
            getter_func_uint8(L, offset);
            break;
        case TYPEID::f_uint16:
            getter_func_uint16(L, offset);
            break;
        case TYPEID::f_uint32:
            getter_func_uint32(L, offset);
            break;
        case TYPEID::f_uint64:
            getter_func_uint64(L, offset);
            break;
        case TYPEID::f_bool:
            getter_func_bool(L, offset);
            break;
        case TYPEID::f_ptr:
            getter_func_ptr(L, offset);
            break;
        case TYPEID::f_float:
            getter_func_float(L, offset);
            break;
        case TYPEID::f_double:
            getter_func_double(L, offset);
            break;
        default:
            return luaL_error(L, "Invalid type %d\n", type);
    }
    return 1;
}

static int getter_direct(lua_State *L) {
    int type = luaL_checkinteger(L, 1);
    if (type < 0 || type >= (int)TYPEID::f_COUNT) return luaL_error(L, "Invalid type %d", type);
    int offset = luaL_checkinteger(L, 2);
    int stride = get_stride(type);
    if (offset % stride != 0) {
        return luaL_error(L, "Invalid offset %d for type %d", offset, type);
    }
    offset /= stride;
    return get_value(L, type, offset);
}

static inline int set_value(lua_State *L, int type, int offset) {
    switch ((TYPEID)type) {
        case TYPEID::f_int8:
            setter_func_int8(L, offset);
            break;
        case TYPEID::f_int16:
            setter_func_int16(L, offset);
            break;
        case TYPEID::f_int32:
            setter_func_int32(L, offset);
            break;
        case TYPEID::f_int64:
            setter_func_int64(L, offset);
            break;
        case TYPEID::f_uint8:
            setter_func_uint8(L, offset);
            break;
        case TYPEID::f_uint16:
            setter_func_uint16(L, offset);
            break;
        case TYPEID::f_uint32:
            setter_func_uint32(L, offset);
            break;
        case TYPEID::f_uint64:
            setter_func_uint64(L, offset);
            break;
        case TYPEID::f_bool:
            setter_func_bool(L, offset);
            break;
        case TYPEID::f_ptr:
            setter_func_ptr(L, offset);
            break;
        case TYPEID::f_float:
            setter_func_float(L, offset);
            break;
        case TYPEID::f_double:
            setter_func_double(L, offset);
            break;
        default:
            return luaL_error(L, "Invalid type %d\n", type);
    }
    return 1;
}

static int setter_direct(lua_State *L) {
    int type = luaL_checkinteger(L, 1);
    if (type < 0 || type >= (int)TYPEID::f_COUNT) return luaL_error(L, "Invalid type %d", type);
    int offset = luaL_checkinteger(L, 2);
    int stride = get_stride(type);
    if (offset % stride != 0) {
        return luaL_error(L, "Invalid offset %d for type %d", offset, type);
    }
    offset /= stride;
    return set_value(L, type, offset);
}

#define LUATYPEID(type, typen)                 \
    lua_pushinteger(L, (int)TYPEID::f_##type); \
    lua_setfield(L, -2, #typen);

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
    if (sz == 0 || buf[0] >= (int)TYPEID::f_COUNT) luaL_error(L, "Invalid type");
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

static int get_indirect(lua_State *L) {
    int type;
    int offset;
    void *p = address_ptr(L, &type, &offset);
    if (p == NULL) return 0;
    return getter(L, p, type, offset);
}

static int set_indirect(lua_State *L) {
    int type;
    int offset;
    void *p = address_ptr(L, &type, &offset);
    if (p == NULL) return 0;
    return setter(L, p, type, offset);
}

static void address(lua_State *L) {
    int type = luaL_checkinteger(L, 1);
    if (type < 0 || type >= (int)TYPEID::f_COUNT) luaL_error(L, "Invalid type %d", type);
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

static int open_embed_luastruct(lua_State *L) {
    luaL_checkversion(L);
    luaL_Reg l[] = {
            {"cstruct",
             [](lua_State *L) {
                 int top = lua_gettop(L);
                 if (top <= 2) {
                     getter_direct(L);
                     setter_direct(L);
                     return 2;
                 } else {
                     address(L);
                     int cmd = lua_gettop(L);
                     lua_pushvalue(L, cmd);
                     lua_pushcclosure(L, get_indirect, 1);
                     lua_pushvalue(L, cmd);
                     lua_pushcclosure(L, set_indirect, 1);
                     return 2;
                 }
             }},
            {"offset",
             [](lua_State *L) {
                 if (!lua_isuserdata(L, 1)) return luaL_error(L, "Need userdata at 1");
                 char *p = (char *)lua_touserdata(L, 1);
                 size_t off = luaL_checkinteger(L, 2);
                 lua_pushlightuserdata(L, (void *)(p + off));
                 return 1;
             }},
            {"typeid",
             [](lua_State *L) {
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
             }},
            {NULL, NULL},
    };
    luaL_newlib(L, l);
    // neko_lua_load(L, l, "__neko_cstruct_core");
    return 1;
}

static int luastruct_newudata(lua_State *L) {
    size_t sz = luaL_checkinteger(L, 1);
    lua_newuserdatauv(L, sz, 0);
    return 1;
}

static int open_embed_luastruct_test(lua_State *L) {
    luaL_checkversion(L);
    luaL_Reg l[] = {
            {"udata", luastruct_newudata},
            {"NULL", NULL},
            {NULL, NULL},
    };
    luaL_newlib(L, l);
    lua_pushlightuserdata(L, NULL);
    lua_setfield(L, -2, "NULL");
    return 1;
}

namespace neko::lua::__struct {
LUABIND_MODULE() { return open_embed_luastruct(L); }
}  // namespace neko::lua::__struct

namespace neko::lua::__struct_test {
LUABIND_MODULE() { return open_embed_luastruct_test(L); }
}  // namespace neko::lua::__struct_test
