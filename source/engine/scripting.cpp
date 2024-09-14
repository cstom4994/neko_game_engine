#include "engine/scripting.h"

#include <stdlib.h>
#include <string.h>

#include "engine/asset.h"
#include "engine/base.h"
#include "engine/bootstrap.h"
#include "engine/component.h"
#include "engine/edit.h"
#include "engine/entity.h"
#include "engine/event.h"
#include "engine/input.h"
#include "engine/luax.h"
#include "engine/luax.hpp"
#include "engine/scripting.h"
#include "engine/ui.h"

int load_embed_lua(lua_State *L, const u8 B[], const_str name) {
    if (luaL_loadbuffer(L, (const_str)B, neko_strlen((const_str)B), name) != LUA_OK) {
        luaL_error(L, "%s", lua_tostring(L, -1));
        return 0;
    }
    if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
        luaL_error(L, "%s", lua_tostring(L, -1));
        return 0;
    }
    return 1;
}

#define LUAOPEN_EMBED_DATA(func, name, compressed_data) \
    static int func(lua_State *L) {                     \
        i32 top = lua_gettop(L);                        \
        load_embed_lua(L, compressed_data, name);       \
        return lua_gettop(L) - top;                     \
    }

static const u8 g_lua_bootstrap_data[] = {
#include "bootstrap.lua.h"
};
static const u8 g_lua_nekogame_data[] = {
#include "nekogame.lua.h"
};
static const u8 g_lua_nekoeditor_data[] = {
#include "nekoeditor.lua.h"
};

// LUAOPEN_EMBED_DATA(open_embed_common, "common.lua", g_lua_common_data);
LUAOPEN_EMBED_DATA(open_embed_bootstrap, "bootstrap.lua", g_lua_bootstrap_data);
// LUAOPEN_EMBED_DATA(open_embed_nekogame, "nekogame.lua", g_lua_nekogame_data);

extern "C" {
int luaopen_http(lua_State *L);

#if defined(NEKO_CFFI)
int luaopen_cffi(lua_State *L);
int luaopen_bit(lua_State *L);
#endif
}

void package_preload_embed(lua_State *L) {

    luaL_Reg preloads[] = {
#if defined(NEKO_CFFI)
            {"ffi", luaopen_cffi},
            {"bit", luaopen_bit},
#endif
            {"http", luaopen_http},
    };

    for (int i = 0; i < NEKO_ARR_SIZE(preloads); i++) {
        luax_package_preload(L, preloads[i].name, preloads[i].func);
    }
}
void luax_run_bootstrap(lua_State *L) {
    if (luaL_loadbuffer(L, (const_str)g_lua_bootstrap_data, neko_strlen((const_str)g_lua_bootstrap_data), "<bootstrap>") != LUA_OK) {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        neko_panic("failed to load bootstrap");
    }
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char *errorMsg = lua_tostring(L, -1);
        fprintf(stderr, "bootstrap error: %s\n", errorMsg);
        lua_pop(L, 1);
        neko_panic("failed to run bootstrap");
    }
    console_log("loaded bootstrap");
}
void luax_run_nekogame(lua_State *L) {
    if (luaL_loadbuffer(L, (const_str)g_lua_nekogame_data, neko_strlen((const_str)g_lua_nekogame_data), "<nekogame>") != LUA_OK) {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        neko_panic("failed to load nekogame");
    }
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char *errorMsg = lua_tostring(L, -1);
        fprintf(stderr, "nekogame error: %s\n", errorMsg);
        lua_pop(L, 1);
        neko_panic("failed to run nekogame");
    }
    console_log("loaded nekogame");

    if (luaL_loadbuffer(L, (const_str)g_lua_nekoeditor_data, neko_strlen((const_str)g_lua_nekoeditor_data), "<nekoeditor>") != LUA_OK) {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        neko_panic("failed to load nekoeditor");
    }
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char *errorMsg = lua_tostring(L, -1);
        fprintf(stderr, "nekoeditor error: %s\n", errorMsg);
        lua_pop(L, 1);
        neko_panic("failed to run nekoeditor");
    }
    console_log("loaded nekoeditor");
}

#if LUA_VERSION_NUM < 504

void *lua_newuserdatauv(lua_State *L_, size_t sz_, int nuvalue_) {
    neko_assert(L_ && nuvalue_ <= 1);
    return lua_newuserdata(L_, sz_);
}

int lua_getiuservalue(lua_State *const L_, int const idx_, int const n_) {
    if (n_ > 1) {
        lua_pushnil(L_);
        return LUA_TNONE;
    }

#if LUA_VERSION_NUM == 501
    lua_getfenv(L_, idx_);
    lua_getglobal(L_, LUA_LOADLIBNAME);
    if (lua_rawequal(L_, -2, -1) || lua_rawequal(L_, -2, LUA_GLOBALSINDEX)) {
        lua_pop(L_, 2);
        lua_pushnil(L_);

        return LUA_TNONE;
    } else {
        lua_pop(L_, 1);
    }
#else
    lua_getuservalue(L_, idx_);
#endif

    int const _uvType = lua_type(L_, -1);

    return (LUA_VERSION_NUM == 502 && _uvType == LUA_TNIL) ? LUA_TNONE : _uvType;
}

int lua_setiuservalue(lua_State *L_, int idx_, int n_) {
    if (n_ > 1
#if LUA_VERSION_NUM == 501
        || lua_type(L_, -1) != LUA_TTABLE
#endif
    ) {
        lua_pop(L_, 1);
        return 0;
    }

#if LUA_VERSION_NUM == 501
    lua_setfenv(L_, idx_);
#else
    lua_setuservalue(L_, idx_);
#endif
    return 1;
}

#endif

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
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_sizes");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_values");
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
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_sizes");
    lua_pushnil(L);
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUA_AUTO_REGISTER_PREFIX "enums_values");
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

LUASTRUCT_BEGIN(vec4)
LUASTRUCT_FIELD(x, float)
LUASTRUCT_FIELD(y, float)
LUASTRUCT_FIELD(z, float)
LUASTRUCT_FIELD(w, float)
LUASTRUCT_END

// LUASTRUCT_BEGIN(neko_framebuffer_t)
// LUASTRUCT_FIELD(id, uint)
// LUASTRUCT_END

LUASTRUCT_BEGIN(gfx_texture_t)
LUASTRUCT_FIELD(id, uint)
LUASTRUCT_END

LUASTRUCT_BEGIN(Color)
LUASTRUCT_FIELD(r, float)
LUASTRUCT_FIELD(g, float)
LUASTRUCT_FIELD(b, float)
LUASTRUCT_FIELD(a, float)
LUASTRUCT_END

LUASTRUCT_BEGIN(NativeEntity)
LUASTRUCT_FIELD(id, uint)
LUASTRUCT_END

void createStructTables(lua_State *L) {
#define XX(T) T##_create(L, #T)
    // vec4_create(L, "vec4");

    XX(vec4);
    // XX(neko_framebuffer_t);
    XX(gfx_texture_t);
    XX(Color);
    XX(NativeEntity);

    ARRAY_uchar8_create(L);
#undef XX
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

static const char **nekogame_ffi[] = {
        &nekogame_ffi_scalar,
        &nekogame_ffi_saveload,
        &nekogame_ffi_vec2,
        &nekogame_ffi_mat3,
        &nekogame_ffi_bbox,
        &nekogame_ffi_color,
        &nekogame_ffi_fs,
        &nekogame_ffi_game,
        &nekogame_ffi_system,
        &nekogame_ffi_input,
        &nekogame_ffi_entity,
        &nekogame_ffi_prefab,
        &nekogame_ffi_timing,
        &nekogame_ffi_transform,
        &nekogame_ffi_camera,
        &nekogame_ffi_sprite,
        &nekogame_ffi_tiled,
        &nekogame_ffi_gui,
        &nekogame_ffi_console,
        // &nekogame_ffi_sound,
        // &nekogame_ffi_physics,
        &nekogame_ffi_edit,
        &nekogame_ffi_inspector,

        &nekogame_ffi_keyboard_controlled,
};

static const unsigned int n_nekogame_ffi = sizeof(nekogame_ffi) / sizeof(nekogame_ffi[0]);

static int _traceback(lua_State *L) {
    if (!lua_isstring(L, 1)) {
        if (lua_isnoneornil(L, 1) || !luaL_callmeta(L, 1, "__tostring") || !lua_isstring(L, -1)) return 1;
        lua_remove(L, 1);
    }
    luaL_traceback(L, L, lua_tostring(L, 1), 1);
    return 1;
}

int luax_pcall_nothrow(lua_State *L, int nargs, int nresults) {
    int r;
#if 0
    int errfunc;
    // 将 _traceback 放在 function 和 args 下
    errfunc = lua_gettop(L) - nargs;
    lua_pushcfunction(L, _traceback);
    lua_insert(L, errfunc);
    // call, remove _traceback
    r = lua_pcall(L, nargs, nresults, errfunc);
    lua_remove(L, errfunc);
#endif
    r = lua_pcall(L, nargs, nresults, 1);
    return r;
}

void script_run_string(const char *s) {
    lua_State *L = ENGINE_LUA();
    luaL_loadstring(L, s);
    errcheck(luax_pcall_nothrow(L, 0, LUA_MULTRET));
}
void script_run_file(const char *filename) {
    lua_State *L = ENGINE_LUA();
    luaL_loadfile(L, filename);
    errcheck(luax_pcall_nothrow(L, 0, LUA_MULTRET));
}
void script_error(const char *s) {
    lua_State *L = ENGINE_LUA();
    luaL_error(L, s);
}

// 将对象推送为 cdata, t 必须是字符串形式的 FFI 类型说明符
// 如推入一个 vec2 应该为 _push_cdata("vec2 *", &v)
// 结果是堆栈上的 vec2 cdata (不是指针)
static void _push_cdata(const char *t, void *p) {
    // just call __deref_cdata(t, p)
    lua_State *L = ENGINE_LUA();
    lua_getglobal(L, "ng");
    lua_getfield(L, -1, "__deref_cdata");
    lua_remove(L, -2);
    lua_pushstring(L, t);
    lua_pushlightuserdata(L, p);
    errcheck(luax_pcall_nothrow(L, 2, 1));
}

void script_push_event(const char *event) {
    // call nekogame.__fire_event(event, ...)
    lua_State *L = ENGINE_LUA();
    lua_getglobal(L, "ng");
    lua_getfield(L, -1, "__fire_event");
    lua_remove(L, -2);
    lua_pushstring(L, event);
}

// 将命令行参数转发为 nekogame_args[0], nekogame_args[1], ...
static void _forward_args() {
    lua_State *L = ENGINE_LUA();

    int i, argc;
    char **argv;

    argc = game_get_argc();
    argv = game_get_argv();

    lua_createtable(L, argc, 0);
    for (i = 0; i < argc; ++i) {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i);
    }
    lua_setglobal(L, "nekogame_args");
}

static void _set_paths() {
    lua_State *L = ENGINE_LUA();
    lua_pushstring(L, data_path(""));
    lua_setglobal(L, "nekogame_data_path");
    lua_pushstring(L, usr_path(""));
    lua_setglobal(L, "nekogame_usr_path");
}

// LuaJIT FFI 解析器无法解析 'NEKO_EXPORT' -- 使其成为空白
static void _fix_exports(char *s) {
    static const char keyword[] = NEKO_STR(NEKO_EXPORT);
    unsigned int i;

    while ((s = strstr(s, keyword)))
        for (i = 0; i < sizeof(keyword) - 1; ++i) *s++ = ' ';
}

// 相当于:
// ffi = require 'ffi'
// ffi.cdef(nekogame_ffi[0] .. nekogame_ffi[1] .. ...)
// with 'NEKO_EXPORT's fixed -- after this nekogame.lua can bind the FFI
static void _load_nekogame_ffi() {
    lua_State *L = ENGINE_LUA();

    unsigned int i;
    char *fixed;
    luaL_Buffer buf;  // 将累积 nekogame_ffi cdefs 到这里

    // get ffi.cdef
    lua_getglobal(L, "require");
    lua_pushstring(L, "ffi");
    errcheck(luax_pcall_nothrow(L, 1, 1));
    lua_getfield(L, lua_gettop(L), "cdef");

    // accumulate nekogame_ffi cdefs
    luaL_buffinit(L, &buf);
    for (i = 0; i < n_nekogame_ffi; ++i) {
        fixed = (char *)mem_alloc(strlen(*nekogame_ffi[i]) + 1);
        strcpy(fixed, *nekogame_ffi[i]);
        _fix_exports(fixed);
        luaL_addstring(&buf, fixed);
        mem_free(fixed);
    }
    luaL_pushresult(&buf);

    errcheck(luax_pcall_nothrow(L, 1, 0));
}

int app_stop(App *app, event_t evt) {
    console_log("app_stop %p %s %f", app, event_string(evt.type), std::get<f64>(evt.p0.v));
    return 0;
}

void script_init() {
    PROFILE_FUNC();

    neko::timer timer;
    timer.start();

    ENGINE_LUA() = neko::neko_lua_create();

    auto L = ENGINE_LUA();

    open_neko_api(L);

    lua_atpanic(
            L, +[](lua_State *L) {
                auto msg = lua_tostring(L, -1);
                printf("LUA: neko_panic error: %s", msg);
                return 0;
            });

    lua_pushcfunction(L, luax_msgh);  // 添加错误消息处理程序 始终位于堆栈底部

    _load_nekogame_ffi();
    _forward_args();
    _set_paths();

    g_app->g_lua_callbacks_table_ref = LUA_NOREF;

    lua_channels_setup();

    event_register(eventhandler_instance(), g_app, quit, (evt_callback_t)app_stop, NULL);

    luax_run_bootstrap(L);

    timer.stop();
    console_log(std::format("lua init done in {0:.3f} ms", timer.get()).c_str());
}

void script_fini() {
    lua_State *L = ENGINE_LUA();

    {
        PROFILE_BLOCK("before quit");

        luax_neko_get(L, "before_quit");
        if (lua_pcall(L, 0, 0, 1) != LUA_OK) {
            String err = luax_check_string(L, -1);
            neko_panic("%s", err.data);
        }

        event_dispatch(eventhandler_instance(), event_t{.type = on_quit, .p0 = {.v = 199.14f}});
    }

    luax_get(ENGINE_LUA(), "neko", "game_fini");
    luax_pcall(ENGINE_LUA(), 0, 0);

    script_push_event("fini");
    errcheck(luax_pcall_nothrow(L, 1, 0));

    lua_pop(L, 1);  // FFI
    lua_pop(L, 1);  // luax_msgh

    neko::neko_lua_fini(ENGINE_LUA());
}

int script_update_all(App *app, event_t evt) {
    lua_State *L = ENGINE_LUA();

    luax_get(ENGINE_LUA(), "neko", "game_pre_update");
    luax_pcall(ENGINE_LUA(), 0, 0);

    luax_get(ENGINE_LUA(), "neko", "game_loop");
    neko::luabind::LuaStack<f32>::push(ENGINE_LUA(), get_timing_instance()->dt);
    luax_pcall(ENGINE_LUA(), 1, 0);

    script_push_event("update_all");
    errcheck(luax_pcall_nothrow(L, 1, 0));

    return 0;
}

int script_post_update_all(App *app, event_t evt) {
    lua_State *L = ENGINE_LUA();

    script_push_event("post_update_all");
    errcheck(luax_pcall_nothrow(L, 1, 0));

    return 0;
}

void script_draw_ui() {
    lua_State *L = ENGINE_LUA();

    luax_get(ENGINE_LUA(), "neko", "game_ui");
    luax_pcall(ENGINE_LUA(), 0, 0);

    script_push_event("draw_ui");
    errcheck(luax_pcall_nothrow(L, 1, 0));
}

void script_draw_all() {
    lua_State *L = ENGINE_LUA();

    luax_get(ENGINE_LUA(), "neko", "game_render");
    luax_pcall(ENGINE_LUA(), 0, 0);

    script_push_event("draw_all");
    errcheck(luax_pcall_nothrow(L, 1, 0));
}

void script_key_down(KeyCode key) {
    lua_State *L = ENGINE_LUA();

    script_push_event("key_down");
    _push_cdata("KeyCode *", &key);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}
void script_key_up(KeyCode key) {
    lua_State *L = ENGINE_LUA();

    script_push_event("key_up");
    _push_cdata("KeyCode *", &key);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}

void script_mouse_down(MouseCode mouse) {
    lua_State *L = ENGINE_LUA();

    script_push_event("mouse_down");
    _push_cdata("MouseCode *", &mouse);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}
void script_mouse_up(MouseCode mouse) {
    lua_State *L = ENGINE_LUA();

    script_push_event("mouse_up");
    _push_cdata("MouseCode *", &mouse);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}

void script_mouse_move(vec2 pos) {
    lua_State *L = ENGINE_LUA();

    script_push_event("mouse_move");
    _push_cdata("vec2 *", &pos);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}

void script_scroll(vec2 scroll) {
    lua_State *L = ENGINE_LUA();

    script_push_event("scroll");
    _push_cdata("vec2 *", &scroll);
    errcheck(luax_pcall_nothrow(L, 2, 0));
}

void script_save_all(Store *s) {
    lua_State *L = ENGINE_LUA();

    Store *t;
    const char *str;

    if (store_child_save(&t, "script", s)) {
        // get string from Lua
        lua_getglobal(L, "ng");
        lua_getfield(L, -1, "__save_all");
        lua_remove(L, -2);
        errcheck(luax_pcall_nothrow(L, 0, 1));
        str = lua_tostring(L, -1);

        // save it
        string_save(&str, "str", t);

        // release
        lua_pop(L, 1);
    }
}

void script_load_all(Store *s) {
    lua_State *L = ENGINE_LUA();

    Store *t;
    char *str;

    if (store_child_load(&t, "script", s))
        if (string_load(&str, "str", NULL, t)) {
            // send it to Lua
            lua_getglobal(L, "ng");
            lua_getfield(L, -1, "__load_all");
            lua_remove(L, -2);
            lua_pushstring(L, str);
            errcheck(luax_pcall_nothrow(L, 1, 0));

            // release
            mem_free(str);
        }
}

#ifdef NEKO_IS_WIN32
#include <windows.h>

static void alice_init_script_context_library(alice_script_context_t *context, const char *assembly_path) {
    assert(context);

    context->handle = LoadLibraryA(assembly_path);
    if (!context->handle) {
        console_log("Failed to load script assembly: `%s'", assembly_path);
    }
}

static void *alice_get_script_proc(alice_script_context_t *context, const char *name) {
    assert(context);

    void *function = GetProcAddress((HMODULE)context->handle, name);
    if (!function) {
        console_log("Failed to locate function `%s'", name);
    }

    return function;
}

static void alice_deinit_script_context_library(alice_script_context_t *context) {
    assert(context);

    FreeLibrary((HMODULE)context->handle);
}

#else

#endif

#if 0

alice_script_context_t *alice_new_script_context(App *scene, const char *assembly_path) {
    assert(scene);

    alice_script_context_t *new_ctx = (alice_script_context_t *)mem_alloc(sizeof(alice_script_context_t));

    new_ctx->scripts = NULL;
    new_ctx->script_count = 0;
    new_ctx->script_capacity = 0;

    new_ctx->scene = scene;

    alice_init_script_context_library(new_ctx, assembly_path);

    return new_ctx;
}

void alice_free_script_context(alice_script_context_t *context) {
    assert(context);

    alice_deinit_script_context_library(context);

    if (context->script_capacity > 0) {
        free(context->scripts);
    }

    free(context);
}

alice_script_t *alice_new_script(alice_script_context_t *context, NativeEntity entity, const char *get_instance_size_name, const char *on_init_name, const char *on_update_name,
                                 const char *on_physics_update_name, const char *on_free_name, bool init_on_create) {
    assert(context);

    if (context->script_count >= context->script_capacity) {
        context->script_capacity = alice_grow_capacity(context->script_capacity);
        context->scripts = (alice_script_t *)mem_realloc(context->scripts, context->script_capacity * sizeof(alice_script_t));
    }

    alice_script_t *new_ctx = &context->scripts[context->script_count++];

    new_ctx->instance = NULL;

    new_ctx->entity = entity;

    NativeEntity *entity_ptr = alice_get_entity_ptr(context->scene, entity);
    entity_ptr->script = new_ctx;

    new_ctx->get_instance_size_name = NULL;
    new_ctx->on_init_name = NULL;
    new_ctx->on_update_name = NULL;
    new_ctx->on_physics_update_name = NULL;
    new_ctx->on_free_name = NULL;

    new_ctx->on_init = NULL;
    new_ctx->on_update = NULL;
    new_ctx->on_physics_update = NULL;
    new_ctx->on_free = NULL;

    alice_script_get_instance_size_f get_size = NULL;
    if (get_instance_size_name) {
        new_ctx->get_instance_size_name = alice_copy_string(get_instance_size_name);

        get_size = alice_get_script_proc(context, get_instance_size_name);
    }

    if (on_init_name) {
        new_ctx->on_init_name = alice_copy_string(on_init_name);
        new_ctx->on_init = alice_get_script_proc(context, on_init_name);
    }

    if (on_update_name) {
        new_ctx->on_update_name = alice_copy_string(on_update_name);
        new_ctx->on_update = alice_get_script_proc(context, on_update_name);
    }

    if (on_physics_update_name) {
        new_ctx->on_physics_update_name = alice_copy_string(on_physics_update_name);
        new_ctx->on_physics_update = alice_get_script_proc(context, on_physics_update_name);
    }

    if (on_free_name) {
        new_ctx->on_free_name = alice_copy_string(on_free_name);
        new_ctx->on_free = alice_get_script_proc(context, on_free_name);
    }

    if (get_size) {
        new_ctx->instance = malloc(get_size());
    }

    if (init_on_create && new_ctx->on_init) {
        new_ctx->on_init(context->scene, entity, new_ctx->instance);
    }

    return new_ctx;
}

void alice_delete_script(alice_script_context_t *context, alice_script_t *script) {
    assert(context);
    assert(script);

    i32 index = -1;

    for (u32 i = 0; i < context->script_count; i++) {
        if (&context->scripts[i] == script) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return;
    }

    alice_deinit_script(context, &context->scripts[index]);

    for (u32 i = index; i < context->script_count - 1; i++) {
        context->scripts[i] = context->scripts[i + 1];
    }

    context->script_count--;
}

void alice_deinit_script(alice_script_context_t *context, alice_script_t *script) {
    assert(context);
    assert(script);

    if (script->on_free) {
        script->on_free(context->scene, script->entity, script->instance);
    }

    if (script->instance) {
        free(script->instance);
    }

    NativeEntity *entity_ptr = alice_get_entity_ptr(context->scene, script->entity);
    entity_ptr->script = NULL;

    free(script->get_instance_size_name);
    free(script->on_init_name);
    free(script->on_update_name);
    free(script->on_free_name);
}

void alice_init_scripts(alice_script_context_t *context) {
    assert(context);

    for (u32 i = 0; i < context->script_count; i++) {
        alice_script_t *script = &context->scripts[i];
        if (script->on_init) {
            script->on_init(context->scene, script->entity, script->instance);
        }
    }
}

void alice_update_scripts(alice_script_context_t *context, double timestep) {
    assert(context);

    for (u32 i = 0; i < context->script_count; i++) {
        alice_script_t *script = &context->scripts[i];
        if (script->on_update) {
            script->on_update(context->scene, script->entity, script->instance, timestep);
        }
    }
}

void alice_physics_update_scripts(alice_script_context_t *context, double timestep) {
    assert(context);

    for (u32 i = 0; i < context->script_count; i++) {
        alice_script_t *script = &context->scripts[i];
        if (script->on_physics_update) {
            script->on_physics_update(context->scene, script->entity, script->instance, timestep);
        }
    }
}

void alice_free_scripts(alice_script_context_t *context) {
    assert(context);

    for (u32 i = 0; i < context->script_count; i++) {
        alice_script_t *script = &context->scripts[i];
        alice_deinit_script(context, script);
    }
}

#endif