
#ifndef NEKO_LUA_UTIL_H
#define NEKO_LUA_UTIL_H

#include "base/scripting/lua_wrapper.hpp"
#include "base/scripting/lua_function.hpp"

struct TValue;
struct Table;

namespace Neko {

// 将TValue压入栈
void PushTValue(lua_State* L, const TValue* v);

// 获取数组部分的下一个非nil元素的索引
int LuaTableArrayNext(Table* t, int index);

// 获取哈希部分的下一个非nil元素的索引
int LuaTableHashNext(const Table* t, int index);

template <typename T>
class ClassBindBuilder {
    String name;

    lua_State* L;

public:
    explicit ClassBindBuilder(String name_) : name(name_) {
        L = ENGINE_LUA();
        lua_getglobal(L, "neko");
        if (!lua_istable(L, -1)) {
            error_assert("no neko table");
        }
    }

    bool Build() {
        lua_pop(L, 1);
        return true;
    }

    template <typename FUNC>
    ClassBindBuilder& Method(const String& name, FUNC ptr) {
        register_function<FUNC>(L, name.cstr(), ptr);
        lua_setfield(L, -2, name.cstr());
        return *this;
    }

    ClassBindBuilder& CClosure(const std::vector<luaL_Reg>& list) {
        for (auto& r : list) {
            lua_pushcclosure(L, r.func, 0);
            lua_setfield(L, -2, r.name);
        }
        return *this;
    }
};

}  // namespace Neko

// #define REGISTER_TYPE(Type) TypeRegistry::Instance().RegisterType<Type>(#Type)
#define BUILD_TYPE(Type) ClassBindBuilder<Type>(#Type)

#endif