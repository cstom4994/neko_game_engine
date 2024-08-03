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

// LUASTRUCT_BEGIN(Color)
// LUASTRUCT_FIELD(r, u8)
// LUASTRUCT_FIELD(g, u8)
// LUASTRUCT_FIELD(b, u8)
// LUASTRUCT_FIELD(a, u8)
// LUASTRUCT_END

void createStructTables(lua_State *L) {
    Vector4_create(L, "Vector4");
    // Color_create(L, "Color");

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

    // if (neko_luabind_struct_registered_type(L, type_id)) {
    //     return neko_luabind_struct_push_type(L, type_id, c_in);
    // }

    if (neko_lua_enum_registered_type(L, type_id)) {
        return neko_lua_enum_push_type(L, type_id, c_in);
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

    // if (neko_luabind_struct_registered_type(L, type_id)) {
    //     neko_luabind_struct_to_type(L, type_id, c_out, index);
    //     return;
    // }

    if (neko_lua_enum_registered_type(L, type_id)) {
        neko_lua_enum_to_type(L, type_id, c_out, index);
        return;
    }

    lua_pushfstring(L, "neko_luabind_to: conversion from Lua object to type '%s' not registered!", neko_luabind_typename(L, type_id));
    lua_error(L);
}

int neko_lua_enum_push_type(lua_State *L, neko_luabind_Type type, const void *value) {

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
        lua_pushfstring(L, "neko_lua_enum_push: Enum '%s' value %d not registered!", neko_luabind_typename(L, type), lvalue);
        lua_error(L);
        return 0;
    }

    lua_pop(L, 2);
    lua_pushfstring(L, "neko_lua_enum_push: Enum '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
    return 0;
}

void neko_lua_enum_to_type(lua_State *L, neko_luabind_Type type, void *c_out, int index) {

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
        lua_pushfstring(L, "neko_lua_enum_to: Enum '%s' field '%s' not registered!", neko_luabind_typename(L, type), name);
        lua_error(L);
        return;
    }

    lua_pop(L, 3);
    lua_pushfstring(L, "neko_lua_enum_to: Enum '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
    return;
}

bool neko_lua_enum_has_value_type(lua_State *L, neko_luabind_Type type, const void *value) {

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
    lua_pushfstring(L, "neko_lua_enum_has_value: Enum '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
    return false;
}

bool neko_lua_enum_has_name_type(lua_State *L, neko_luabind_Type type, const char *name) {
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
    lua_pushfstring(L, "neko_lua_enum_has_name: Enum '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
    return false;
}

void neko_lua_enum_type(lua_State *L, neko_luabind_Type type, size_t size) {
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

void neko_lua_enum_value_type(lua_State *L, neko_luabind_Type type, const void *value, const char *name) {

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
    lua_pushfstring(L, "neko_lua_enum_value: Enum '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
}

bool neko_lua_enum_registered_type(lua_State *L, neko_luabind_Type type) {
    lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums");
    lua_pushinteger(L, type);
    lua_gettable(L, -2);
    bool reg = !lua_isnil(L, -1);
    lua_pop(L, 2);
    return reg;
}

const char *neko_lua_enum_next_value_name_type(lua_State *L, neko_luabind_Type type, const char *member) {

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
    lua_pushfstring(L, "neko_lua_enum_next_enum_name_type: Enum '%s' not registered!", neko_luabind_typename(L, type));
    lua_error(L);
    return NULL;
}
