
#include "lua_database.h"

#include "base/common/vfs.hpp"
#include "engine/component.h"
#include "base/scripting/lua_wrapper.hpp"

// 从另一个Lua状态复制表到当前状态 并创建代理对象 (LuaDatabase userdata)
void luadb_get(lua_State *toL, lua_State *fromL, int index) {
    // 从fromL复制表索引 并将proxy推入toL index是表的索引

    // luaL_checktype(fromL, -1, LUA_TTABLE);
    const_str key = reinterpret_cast<const_str>(lua_topointer(fromL, index));
    if (key == NULL) {
        luaL_error(toL, "Not a table");
    }

    // 在fromL中存储该表
    if (lua_rawgetp(fromL, LUA_REGISTRYINDEX, key) == LUA_TNIL) {
        lua_pop(fromL, 1);                           // # pop nil
        lua_pushvalue(fromL, index);                 // # push index
        lua_rawsetp(fromL, LUA_REGISTRYINDEX, key);  // 将表放入 fromL 的注册表中
    } else {
        lua_pop(fromL, 1);  // # pop table 已经有了
    }

    // 获取当前状态中与 fromL 关联的代理缓存表
    if (lua_rawgetp(toL, LUA_REGISTRYINDEX, fromL) != LUA_TTABLE) {
        lua_pop(toL, 1);
        luaL_error(toL, "Not an invalid L %p", fromL);
    }

    // 从代理缓存表中获取 *toL[*fromL][key] 如果代理不存在则创建
    if (lua_rawgetp(toL, -1, key) == LUA_TNIL) {
        lua_pop(toL, 1);  // # pop nil
        LuaDatabase *t = (LuaDatabase *)lua_newuserdata(toL, sizeof(LuaDatabase));
        t->L = fromL;
        t->key = key;
        lua_pushvalue(toL, lua_upvalueindex(UV_PROXY));  // 代理的元表

        lua_setmetatable(toL, -2);  // LuaDatabase.__metatable = UV_PROXY
        lua_pushvalue(toL, -1);     // # push (LuaDatabase userdata)
        lua_rawsetp(toL, -3, key);  // _G[*fromL][key] = (LuaDatabase userdata)
    }
    lua_replace(toL, -2);  // 删除代理缓存表
}

// kv推入luadb 准备跨状态访问表的键值
LuaDatabase *luadb_pretable(lua_State *L) {
    LuaDatabase *t = (LuaDatabase *)lua_touserdata(L, 1);
    if (t->L == NULL) luaL_error(L, "invalid proxy object");

    // 获取目标状态中的原始表
    if (lua_rawgetp(t->L, LUA_REGISTRYINDEX, t->key) != LUA_TTABLE) {  // 获取Ldb中存储的表 到Ldb#1
        lua_pop(t->L, 1);
        luaL_error(L, "invalid external table %p of L(%p)", t->key, t->L);
    }

    // 将当前键转换为目标状态的类型
    switch (lua_type(L, 2)) {  // 获取L中__index的查找键 到Ldb#2
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

int luadb_copyvalue(lua_State *fromL, lua_State *toL, int index) {
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
                LuaDatabase *t = luadb_pretable(L);
                lua_rawget(t->L, -2);                // 获取 Ldb[储存表][k]
                if (luadb_copyvalue(t->L, L, -1)) {  // L[top] = Ldb[储存表][k]
                    lua_pop(t->L, 2);
                    return lua_error(L);
                }
                lua_pop(t->L, 2);  // # pop K V
                return 1;
            },
            1);
    lua_setfield(L, -2, "__index");

    lua_pushvalue(L, -1);
    lua_pushcclosure(
            L,
            +[](lua_State *L) {
                lua_settop(L, 1);
                LuaDatabase *t = luadb_pretable(L);
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
                LuaDatabase *t = luadb_pretable(L);
                int top = lua_gettop(t->L);
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
                lua_pop(t->L, 3);
                neko_assert(lua_gettop(t->L) == top - 2);
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
        LuaDatabase *t = (LuaDatabase *)lua_touserdata(L, 1);
        lua_pushfstring(L, "[LuaDatabase %p:%p]", t->L, t->key);
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

                lua_newtable(L);                              // # push [代理缓存表]
                lua_pushvalue(L, lua_upvalueindex(UV_WEAK));  // # push [代理缓存表][弱表元表]
                lua_setmetatable(L, -2);                      // # pop  [代理缓存表]

                auto newdb = [](lua_State *L, String source) {
                    lua_State *Ldb = luaL_newstate();
                    String contents = {};
                    bool ok = the<VFS>().read_entire_file(&contents, source.cstr());
                    if (ok) {
                        neko_defer(contents.trash());
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

                // 将代理缓存表存入注册表 键为Ldb指针
                lua_rawsetp(L, LUA_REGISTRYINDEX, Ldb);

                lua_rawgeti(Ldb, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);

                // 将全局表转换为代理对象并压入主Lua栈
                luadb_get(L, Ldb, -1);  // # push [代理对象]
                lua_pop(Ldb, 1);

                // 将 Ldb 记录到 __datalua 中以进行关闭
                lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUADB_TABLE);  // # push [代理缓存表][注册表列表]
                int n = lua_rawlen(L, -1);
                lua_pushlightuserdata(L, Ldb);  // # push [代理缓存表][注册表列表][Ldb指针]
                lua_rawseti(L, -2, n + 1);      // 注册表列表[n+1] = Ldb指针
                lua_pop(L, 1);                  //

                return 1;
            },
            2);
    lua_setfield(L, modindex, "open");

    lua_newtable(L);  // 将所有可扩展键保存到 NEKO_LUADB_TABLE 中
    lua_setfield(L, LUA_REGISTRYINDEX, NEKO_LUADB_TABLE);

    static auto closeall = [](lua_State *L) {
        lua_getfield(L, LUA_REGISTRYINDEX, NEKO_LUADB_TABLE);
        int n = lua_rawlen(L, -1);
        for (int i = 1; i <= n; i++) {
            if (lua_rawgeti(L, -1, i) == LUA_TLIGHTUSERDATA) {
                lua_State *Ldb = (lua_State *)lua_touserdata(L, -1);
                lua_pop(L, 1);
                if (lua_rawgetp(L, LUA_REGISTRYINDEX, Ldb) == LUA_TTABLE) {
                    lua_pop(L, 1);                           // # pop table
                    lua_pushnil(L);                          //
                    lua_rawsetp(L, LUA_REGISTRYINDEX, Ldb);  // _G[*Ldb] = nil 回收*Ldb表

                    int top = lua_gettop(Ldb);
                    if (top != 0) {
                        LuaVM::Tools::ForEachStack(Ldb, []<typename T>(int i, T v) -> int {
                            std::cout << i << ':' << v << std::endl;
                            return 0;
                        });
                        LOG_TRACE("LuaDatabase stack leak {}", top);
                    }

                    lua_close(Ldb);
                } else {
                    lua_pop(L, 1);  // # pop nil
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