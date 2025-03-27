
#ifndef NEKO_LUA_UTIL_H
#define NEKO_LUA_UTIL_H

#include "base/scripting/lua_wrapper.hpp"

struct TValue;
struct Table;

// 将TValue压入栈
void PushTValue(lua_State *L, const TValue *v);

// 获取数组部分的下一个非nil元素的索引
int LuaTableArrayNext(Table *t, int index);

// 获取哈希部分的下一个非nil元素的索引
int LuaTableHashNext(const Table *t, int index);

#endif