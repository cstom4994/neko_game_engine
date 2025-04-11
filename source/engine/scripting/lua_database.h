
#pragma once

#include "engine/scripting/lua_util.h"

static constexpr int MAX_DEPTH = 256;      // 最大递归深度
static constexpr int SHORT_STRING = 1024;  // 短字符串长度
static constexpr int CONVERTER = 2;        // upvalue 索引
static constexpr int REF_CACHE = 3;        // 引用缓存
static constexpr int REF_UNSOLVED = 4;     // 未解决引用
static constexpr int TAB_SPACE = 4;        // 缩进空格数

#define NEKO_LUADB_TABLE "__neko_luadb"

struct LuaDatabase {
    lua_State *L;   // 关联的 Lua 状态机
    const_str key;  // 在注册表中的唯一键
};

enum LuaDatabaseUpvalue {
    UV_PROXY = 1,  // 代理元表在upvalue中的位置
    UV_WEAK = 2,   // 弱表在upvalue中的位置
};

void luadb_get(lua_State *toL, lua_State *fromL, int index);
LuaDatabase *luadb_pretable(lua_State *L);
int luadb_copyvalue(lua_State *fromL, lua_State *toL, int index);

int open_db(lua_State *L);
