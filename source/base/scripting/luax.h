#ifndef NEKO_LUAX_H
#define NEKO_LUAX_H

#include "base/common/base.hpp"
#include "base/common/mutex.hpp"
#include "base/common/string.hpp"
#include "base/common/arena.hpp"

// lua
#include <atomic>
#include <cstdint>
#ifdef __cplusplus
extern "C" {
#endif
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#ifndef NEKO_CFFI
#include <luajit.h>
#endif

#ifdef __cplusplus
}
#endif

#ifndef NEKO_CFFI
#define NEKO_LUAJIT
#else
#define LUAJIT_VERSION "NOJIT"
#endif

#define LUA_FUNCTION(F) static int F(lua_State *L)

#define NEKO_LUA_AUTO_REGISTER_PREFIX "neko_luabind_"

#if LUA_VERSION_NUM < 502

typedef size_t lua_Unsigned;

#define luaL_tolstring lua_tolstring
#define lua_getfield(L, i, k) (lua_getfield((L), (i), (k)), lua_type((L), -1))
#define lua_gettable(L, i) (lua_gettable((L), (i)), lua_type((L), -1))
#define lua_numbertointeger(n, p) ((*(p) = (lua_Integer)(n)), 1)
#define lua_rawget(L, i) (lua_rawget((L), (i)), lua_type((L), -1))
#define lua_rawgeti(L, i, n) (lua_rawgeti((L), (i), (n)), lua_type((L), -1))
#define luaL_getmetafield(L, o, e) (luaL_getmetafield((L), (o), (e)) ? lua_type((L), -1) : LUA_TNIL)
#define luaL_newmetatable(L, tn) (luaL_newmetatable((L), (tn)) ? (lua_pushstring((L), (tn)), lua_setfield((L), -2, "__name"), 1) : 0)

#define lua_pushunsigned(L, n) lua_pushinteger((L), (lua_Integer)(n))
#define lua_tounsignedx(L, i, is) ((lua_Unsigned)lua_tointegerx((L), (i), (is)))
#define lua_tounsigned(L, i) lua_tounsignedx((L), (i), NULL)
#define luaL_checkunsigned(L, a) ((lua_Unsigned)luaL_checkinteger((L), (a)))
#define luaL_optunsigned(L, a, d) ((lua_Unsigned)luaL_optinteger((L), (a), (lua_Integer)(d)))

#ifndef luaL_newlibtable
#define luaL_newlibtable(L, l) (lua_createtable((L), 0, sizeof((l)) / sizeof(*(l)) - 1))
#endif

#ifndef luaL_newlib
#define luaL_newlib(L, l) (luaL_newlibtable((L), (l)), luaL_register((L), NULL, (l)))
#endif

#ifndef LUA_OK
#define LUA_OK 0
#endif

inline int lua_absindex(lua_State *L, int idx) {
    if (idx > 0 || idx <= LUA_REGISTRYINDEX) {
        return idx;
    }
    return lua_gettop(L) + idx + 1;
}

#define LUAI_MAXALIGN \
    lua_Number n;     \
    double u;         \
    void *s;          \
    lua_Integer i;    \
    long l

inline void lua_pushglobaltable(lua_State *L) {
    lua_pushvalue(L, LUA_GLOBALSINDEX);  // 将全局表压入栈中
}

inline size_t lua_rawlen(lua_State *L, int index) { return lua_objlen(L, index); }

inline size_t lua_len(lua_State *L, int index) {
    int type = lua_type(L, index);
    if (type == LUA_TTABLE || type == LUA_TSTRING) {
        return lua_objlen(L, index);
    }
    return 0;  // 不支持的类型返回 0
}

inline int lua_rawgetp(lua_State *L, int index, const void *p) {
    index = lua_absindex(L, index);
    lua_pushlightuserdata(L, (void *)p);  // 将指针转换为轻量用户数据
    lua_rawget(L, index);                 // 从表中获取值
    return lua_type(L, -1);
}

inline void lua_rawsetp(lua_State *L, int index, const void *p) {
    index = lua_absindex(L, index);
    lua_pushlightuserdata(L, (void *)p);  // 将指针转换为轻量用户数据
    lua_insert(L, -2);                    // 将轻量用户数据移动到值的下面
    lua_rawset(L, index);                 // 将键值对设置到表中
}

inline int lua_isinteger(lua_State *L, int index) {
    if (lua_type(L, index) == LUA_TNUMBER) {
        lua_Number n = lua_tonumber(L, index);
        lua_Integer i = lua_tointeger(L, index);
        if (i == n) return 1;
    }
    return 0;
}

inline int lua_geti(lua_State *L, int index, lua_Integer i) {
    index = lua_absindex(L, index);
    lua_pushinteger(L, i);
    lua_gettable(L, index);
    return lua_type(L, -1);
}

inline void luaL_checkversion(lua_State *L) { (void)L; }

inline int luaL_getsubtable(lua_State *L, int i, const char *name) {
    int abs_i = lua_absindex(L, i);
    luaL_checkstack(L, 3, "not enough stack slots");
    lua_pushstring(L, name);
    lua_gettable(L, abs_i);
    if (lua_istable(L, -1)) return 1;
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushstring(L, name);
    lua_pushvalue(L, -2);
    lua_settable(L, abs_i);
    return 0;
}

inline void lua_seti(lua_State *L, int index, lua_Integer i) {
    luaL_checkstack(L, 1, "not enough stack slots available");
    index = lua_absindex(L, index);
    lua_pushinteger(L, i);
    lua_insert(L, -2);
    lua_settable(L, index);
}

inline void luaL_requiref(lua_State *L, const char *modname, lua_CFunction openf, int glb) {
    luaL_checkstack(L, 3, "not enough stack slots available");
    luaL_getsubtable(L, LUA_REGISTRYINDEX, "_LOADED");
    if (lua_getfield(L, -1, modname) == LUA_TNIL) {
        lua_pop(L, 1);
        lua_pushcfunction(L, openf);
        lua_pushstring(L, modname);
        lua_call(L, 1, 1);
        lua_pushvalue(L, -1);
        lua_setfield(L, -3, modname);
    }
    if (glb) {
        lua_pushvalue(L, -1);
        lua_setglobal(L, modname);
    }
    lua_replace(L, -2);
}

#else

#define lua_objlen lua_rawlen

#endif

#if LUA_VERSION_NUM < 504

void *lua_newuserdatauv(lua_State *L_, size_t sz_, int nuvalue_);
int lua_getiuservalue(lua_State *L_, int idx_, int n_);
int lua_setiuservalue(lua_State *L_, int idx_, int n_);

#define LUA_GNAME "_G"

#define luaL_typeerror luaL_typerror

#endif  // LUA_VERSION_NUM < 504

#ifndef LUA_TCDATA
#define LUA_TCDATA 10
#endif

#ifndef LUA_LOADED_TABLE
#define LUA_LOADED_TABLE "_LOADED"  // doesn't exist before Lua 5.3
#endif                              // LUA_LOADED_TABLE

inline void luax_pushloadedtable(lua_State *L) {
#if LUA_VERSION_NUM < 502
    // 获取全局表 _G
    lua_getfield(L, LUA_GLOBALSINDEX, "package");
    lua_getfield(L, -1, "loaded");
    lua_remove(L, -2);  // 移除 package 表，只保留 loaded 表在栈顶
#else
    lua_pushliteral(L, LUA_LOADED_TABLE);
    lua_gettable(L, LUA_REGISTRYINDEX);
#endif
}

using namespace Neko;

#define neko_lua_register(FUNCTIONS)                              \
    for (unsigned i = 0; i < NEKO_ARR_SIZE(FUNCTIONS) - 1; ++i) { \
        lua_pushcfunction(L, FUNCTIONS[i].func);                  \
        lua_setglobal(L, FUNCTIONS[i].name);                      \
    }

bool neko_lua_equal(lua_State *state, int index1, int index2);

int neko_lua_preload(lua_State *L, lua_CFunction f, const char *name);
int neko_lua_preload_auto(lua_State *L, lua_CFunction f, const char *name);
void neko_lua_load(lua_State *L, const luaL_Reg *l, const char *name);
void neko_lua_loadover(lua_State *L, const luaL_Reg *l, const char *name);
int neko_lua_get_table_pairs_count(lua_State *L, int index);

int __neko_bind_callback_save(lua_State *L);
int __neko_bind_callback_call(lua_State *L);

void luax_get(lua_State *L, const_str tb, const_str field);
void luax_pcall(lua_State *L, i32 args, i32 results);

template <typename F>
inline void luax_package_preload(lua_State *L, const_str name, F function) {
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_pushcfunction(L, function);
    lua_setfield(L, -2, name);
    lua_pop(L, 2);
}

template <typename F>
inline void luax_setfunc2table(lua_State *L, const char *n, F func) {
    lua_pushstring(L, n);
    lua_pushcfunction(L, func);
    lua_settable(L, -3);
}

#define create_lua_class(L, metaname, init, methods) \
    luaL_newmetatable(L, metaname);                  \
    lua_pushvalue(L, -1);                            \
    lua_setfield(L, -2, "__index");                  \
    luaL_setfuncs(L, methods, 0);                    \
    lua_newtable(L);                                 \
    lua_pushcfunction(L, init);                      \
    lua_setfield(L, -2, "__call");                   \
    lua_setmetatable(L, -2);                         \
    lua_setglobal(L, metaname)

struct LuaThread {
    Mutex mtx;
    String contents;
    String name;
    Thread thread;

    void make(String code, String thread_name);
    void join();
};

struct lua_State;
struct LuaTableEntry;
struct LuaVariant {
    i32 type;
    union {
        bool boolean;
        double number;
        String string;
        Slice<LuaTableEntry> table;
        struct {
            void *ptr;
            String tname;
        } udata;
    };

    void make(lua_State *L, i32 arg);
    void trash();
    void push(lua_State *L);
};

struct LuaTableEntry {
    LuaVariant key;
    LuaVariant value;
};

struct LuaChannel {
    std::atomic<char *> name;

    Mutex mtx;
    Cond received;
    Cond sent;

    u64 received_total;
    u64 sent_total;

    Slice<LuaVariant> items;
    u64 front;
    u64 back;
    u64 len;

    void make(String n, u64 buf);
    void trash();
    void send(LuaVariant item);
    LuaVariant recv();
    bool try_recv(LuaVariant *v);
};

LuaChannel *lua_channel_make(String name, u64 buf);
LuaChannel *lua_channel_get(String name);
LuaChannel *lua_channels_select(lua_State *L, LuaVariant *v);
void lua_channels_setup();
void lua_channels_shutdown();

void luax_neko_get(lua_State *L, const char *field);
int luax_msgh(lua_State *L);
i32 luax_require_script(lua_State *L, String filepath);

enum W_LUA_UPVALUES { NEKO_W_COMPONENTS_NAME = 1, NEKO_W_UPVAL_N };

struct W_LUA_REGISTRY_CONST {
    static constexpr i32 W_CORE_IDX = 1;                             // neko_instance_t* index
    static constexpr const_str W_CORE = "__NEKO_W_CORE";             // neko_instance_t* reg
    static constexpr const_str ENG_UDATA_NAME = "__NEKO_ENGINE_UD";  // neko_instance_t* udata
    static constexpr const_str CVAR_MAP = "cvar_map";

    static constexpr i32 CVAR_MAP_MAX = 64;
};

#endif