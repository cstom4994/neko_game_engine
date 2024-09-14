#include "engine/component.h"
#include "engine/luax.hpp"

#define MAX_DEPTH 256
#define SHORT_STRING 1024
#define CONVERTER 2
#define REF_CACHE 3
#define REF_UNSOLVED 4
#define TAB_SPACE 4

#define UV_PROXY 1
#define UV_WEAK 2

#define NEKO_DATALUA_REGISTER "__neko_luadb"

typedef struct luadb {
    lua_State *L;
    const_str key;
} luadb;

static void luadb_get(lua_State *L, lua_State *Ldb, int index) {
    // 从 Ldb 复制表索引 并将 proxy 推入 L

    // luaL_checktype(Ldb, -1, LUA_TTABLE);
    const_str key = reinterpret_cast<const_str>(lua_topointer(Ldb, index));
    if (key == NULL) {
        luaL_error(L, "Not a table");
    }

    if (lua_rawgetp(Ldb, LUA_REGISTRYINDEX, key) == LUA_TNIL) {
        lua_pop(Ldb, 1);
        lua_pushvalue(Ldb, index);  // 将表放入 Ldb 的注册表中
        lua_rawsetp(Ldb, LUA_REGISTRYINDEX, key);
    } else {
        lua_pop(Ldb, 1);  // pop table
    }
    if (lua_rawgetp(L, LUA_REGISTRYINDEX, Ldb) != LUA_TTABLE) {
        lua_pop(L, 1);
        luaL_error(L, "Not an invalid L %p", Ldb);
    }
    if (lua_rawgetp(L, -1, key) == LUA_TNIL) {
        lua_pop(L, 1);
        luadb *t = (luadb *)lua_newuserdata(L, sizeof(*t));
        // auto t = neko::lua::newudata<luadb>(L);
        t->L = Ldb;
        t->key = key;
        lua_pushvalue(L, lua_upvalueindex(UV_PROXY));  // 代理的元表

        lua_setmetatable(L, -2);
        lua_pushvalue(L, -1);
        lua_rawsetp(L, -3, key);  // 缓存
    }
    lua_replace(L, -2);  // 删除代理缓存表
}

// kv推入luadb
static luadb *luadb_pretable(lua_State *L) {
    luadb *t = (luadb *)lua_touserdata(L, 1);
    // auto t = neko::lua::toudata<luadb>(L, 1);
    if (t->L == NULL) {
        luaL_error(L, "invalid proxy object");
    }
    if (lua_rawgetp(t->L, LUA_REGISTRYINDEX, t->key) != LUA_TTABLE) {
        lua_pop(t->L, 1);
        luaL_error(L, "invalid external table %p of L(%p)", t->key, t->L);
    }
    switch (lua_type(L, 2)) {
        case LUA_TNONE:
        case LUA_TNIL:
            lua_pushnil(t->L);
            break;
        case LUA_TNUMBER:
            if (lua_isinteger(L, 2)) {
                lua_pushinteger(t->L, lua_tointeger(L, 2));
            } else {
                lua_pushnumber(t->L, lua_tonumber(L, 2));
            }
            break;
        case LUA_TSTRING:
            lua_pushstring(t->L, lua_tostring(L, 2));
            break;
        case LUA_TBOOLEAN:
            lua_pushboolean(t->L, lua_toboolean(L, 2));
            break;
        default:
            lua_pop(t->L, 1);
            luaL_error(L, "Unsupport key type %s", lua_typename(L, lua_type(L, 2)));
    }
    return t;
}

static int luadb_copyvalue(lua_State *fromL, lua_State *toL, int index) {
    int t = lua_type(fromL, index);
    switch (t) {
        case LUA_TNIL:
            lua_pushnil(toL);
            break;
        case LUA_TNUMBER:
            if (lua_isinteger(fromL, index)) {
                lua_pushinteger(toL, lua_tointeger(fromL, index));
            } else {
                lua_pushnumber(toL, lua_tonumber(fromL, index));
            }
            break;
        case LUA_TSTRING:
            lua_pushstring(toL, lua_tostring(fromL, index));
            break;
        case LUA_TBOOLEAN:
            lua_pushboolean(toL, lua_toboolean(fromL, index));
            break;
        case LUA_TTABLE:
            luadb_get(toL, fromL, index);
            break;
        default:
            lua_pushfstring(toL, "Unsupport value type (%s)", lua_typename(fromL, lua_type(fromL, index)));
            return 1;
    }
    return 0;
}

int open_db(lua_State *L) {

    lua_newtable(L);

    int modindex = lua_gettop(L);

    lua_createtable(L, 0, 1);  // 创建代理元表 (upvalue 1:UV_PROXY)

    lua_pushvalue(L, -1);
    lua_pushcclosure(
            L,
            +[](lua_State *L) {
                struct luadb *t = luadb_pretable(L);
                lua_rawget(t->L, -2);
                if (luadb_copyvalue(t->L, L, -1)) {
                    lua_pop(t->L, 2);
                    return lua_error(L);
                }
                lua_pop(t->L, 2);
                return 1;
            },
            1);
    lua_setfield(L, -2, "__index");

    lua_pushvalue(L, -1);
    lua_pushcclosure(
            L,
            +[](lua_State *L) {
                lua_settop(L, 1);
                luadb *t = luadb_pretable(L);
                size_t n = lua_rawlen(t->L, -2);
                lua_pushinteger(L, n);
                lua_pop(t->L, 2);
                return 1;
            },
            1);
    lua_setfield(L, -2, "__len");

    lua_pushvalue(L, -1);
    lua_pushcclosure(
            L,
            +[](lua_State *L) {
                luadb *t = luadb_pretable(L);
                if (lua_next(t->L, -2) == 0) {
                    lua_pop(t->L, 1);
                    return 0;
                }
                if (luadb_copyvalue(t->L, L, -2)) {
                    lua_pop(t->L, 3);
                    return lua_error(L);
                }
                if (luadb_copyvalue(t->L, L, -1)) {
                    lua_pop(t->L, 3);
                    return lua_error(L);
                }
                return 2;
            },
            1);
    lua_pushcclosure(
            L,
            +[](lua_State *L) {
                lua_pushvalue(L, lua_upvalueindex(1));  // proxynext
                lua_pushvalue(L, 1);                    // table
                lua_pushnil(L);
                return 3;
            },
            1);
    lua_setfield(L, -2, "__pairs");

    static auto tostring = [](lua_State *L) {
        luadb *t = (luadb *)lua_touserdata(L, 1);
        // auto t = neko::lua::toudata<luadb>(L, 1);
        lua_pushfstring(L, "[luadb %p:%p]", t->L, t->key);
        return 1;
    };

    lua_pushcfunction(L, tostring);
    lua_setfield(L, -2, "__tostring");

    lua_createtable(L, 0, 1);  // 弱表 (upvalue 2:UV_WEAK)
    lua_pushstring(L, "kv");
    lua_setfield(L, -2, "__mode");

    lua_pushcclosure(
            L,
            +[](lua_State *L) {
                const char *source = luaL_checkstring(L, 1);

                lua_newtable(L);  // 代理缓存表
                lua_pushvalue(L, lua_upvalueindex(UV_WEAK));
                lua_setmetatable(L, -2);

                auto newdb = [](lua_State *L, std::string source) {
                    lua_State *Ldb = luaL_newstate();
                    String contents = {};
                    bool ok = vfs_read_entire_file(&contents, source.c_str());
                    if (ok) {
                        neko_defer(mem_free(contents.data));
                        if (luaL_dostring(Ldb, contents.data)) {
                            // 引发错误
                            lua_pushstring(L, lua_tostring(Ldb, -1));
                            lua_close(Ldb);
                            lua_error(L);
                        }
                    }
                    lua_gc(Ldb, LUA_GCCOLLECT, 0);
                    return Ldb;
                };

                lua_State *Ldb = newdb(L, source);

                lua_rawsetp(L, LUA_REGISTRYINDEX, Ldb);

#if LUA_VERSION_NUM >= 502
                lua_rawgeti(Ldb, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
#else
                lua_pushvalue(L, LUA_GLOBALSINDEX);
#endif
                luadb_get(L, Ldb, -1);
                lua_pop(Ldb, 1);

                // 将 Ldb 记录到 __datalua 中以进行关闭
                lua_getfield(L, LUA_REGISTRYINDEX, NEKO_DATALUA_REGISTER);
                int n = lua_rawlen(L, -1);
                lua_pushlightuserdata(L, Ldb);
                lua_rawseti(L, -2, n + 1);
                lua_pop(L, 1);

                return 1;
            },
            2);
    lua_setfield(L, modindex, "open");

    lua_newtable(L);  // 将所有可扩展键保存到 NEKO_DATALUA_REGISTER 中
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_DATALUA_REGISTER);

    static auto closeall = [](lua_State *L) {
        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_DATALUA_REGISTER);
        int n = lua_rawlen(L, -1);
        int i;
        for (i = 1; i <= n; i++) {
            if (lua_rawgeti(L, -1, i) == LUA_TLIGHTUSERDATA) {
                const void *Ldb = lua_touserdata(L, -1);
                lua_pop(L, 1);
                if (lua_rawgetp(L, LUA_REGISTRYINDEX, Ldb) == LUA_TTABLE) {
                    lua_pop(L, 1);
                    lua_pushnil(L);
                    lua_rawsetp(L, LUA_REGISTRYINDEX, Ldb);  // 清除缓存
                    lua_close((lua_State *)Ldb);
                } else {
                    lua_pop(L, 1);
                }
            }
        }
        return 0;
    };

    lua_createtable(L, 0, 1);  // 关闭虚拟机时设置收集功能
    lua_pushcfunction(L, closeall);
    lua_setfield(L, -2, "__gc");

    lua_setmetatable(L, -2);

    return 1;
}