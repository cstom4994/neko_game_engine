

#include "base/scripting/lua_wrapper.hpp"

extern "C" {
#include "lapi.h"
#include "ltable.h"
}

namespace Neko {

// 将TValue压入栈
void PushTValue(lua_State *L, const TValue *v) {
    int t = ttype(v);                   // 获取值的类型
    if (!iscollectable(v)) {            // 如果不是可回收的对象
        if (t == LUA_TLIGHTUSERDATA) {  // 不支持 lightuserdata
            luaL_error(L, "unsupport lightuserdata to pushvalue");
        }
        lua_lock(L);
        {
            TValue *io1 = s2v(L->top);                        // 获取栈顶的 TValue 结构
            const TValue *io2 = v;                            // v 作为输入值
            io1->value_ = io2->value_;                        // 复制 TValue 的值
            io1->tt_ = io2->tt_;                              // 复制类型标识
            lua_assert(!iscollectable(io1) || righttt(io1));  // Lua 5.4 GC 兼容性检查
        }
        api_incr_top(L);  // 增加栈顶
        lua_unlock(L);
    } else {
        switch (t) {
            case LUA_TTABLE:
                lua_pushlightuserdata(L, hvalue(v));  // 将table作为lightuserdata压入
                break;
            case LUA_TSTRING:
                lua_pushlstring(L, svalue(v), vslen(v));  // 压入字符串
                break;
            default:
                luaL_error(L, "unsupport %s", lua_typename(L, t));
        }
    }
}

// 获取数组部分的下一个非nil元素的索引
int LuaTableArrayNext(Table *t, int index) {
    while (t->alimit > index) {              // 遍历数组部分
        if (!ttisnil(&t->array[index++])) {  // 找到非 nil 的元素
            return index;
        }
    }
    return 0;
}

// 获取哈希部分的下一个非nil元素的索引
int LuaTableHashNext(const Table *t, int index) {
    index = -index;                               // 负数表示哈希部分
    int nrec = 1 << t->lsizenode;                 // 计算哈希表的大小
    while (nrec > index) {                        // 遍历哈希部分
        if (!ttisnil(gval(gnode(t, index++)))) {  // 找到非 nil 的值
            return -index;                        // 返回索引
        }
    }
    return 0;
}

}  // namespace Neko