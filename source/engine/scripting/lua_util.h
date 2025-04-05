
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
    String subname;

    lua_State* L;

    bool is_sub_system{};

public:
    explicit ClassBindBuilder(String name_) : name(name_), is_sub_system(false) {
        L = ENGINE_LUA();
        lua_getglobal(L, "neko");
        if (!lua_istable(L, -1)) {
            error_assert("no neko table");
        }
    }

    explicit ClassBindBuilder(String name_, String subname_) : name(name_), subname(subname_), is_sub_system(true) {
        L = ENGINE_LUA();
        lua_getglobal(L, "neko");
        if (!lua_istable(L, -1)) {
            error_assert("no neko table");
        }
        lua_createtable(L, 0, 0);
    }

    bool Build() {
        if (is_sub_system) {
            lua_setfield(L, -2, subname.cstr());
        }
        lua_pop(L, 1);
        return true;
    }

    template <typename FUNC>
    ClassBindBuilder& Method(const String& name, FUNC ptr) {
        register_function<FUNC>(L, name.cstr(), ptr);
        lua_setfield(L, -2, name.cstr());
        return *this;
    }

    template <typename FUNC>
    ClassBindBuilder& MemberMethod(const String& name, T* obj, FUNC ptr) {
        register_member_function(L, name.cstr(), obj, ptr);
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

    ClassBindBuilder& CustomBind(const String& name, lua_CFunction func) {
        func(L);
        return *this;
    }
};

}  // namespace Neko

// #define REGISTER_TYPE(Type) TypeRegistry::Instance().RegisterType<Type>(#Type)
#define BUILD_TYPE(Type) ClassBindBuilder<Type>(#Type)
#define BUILD_TYPE_SUB(Type, Subname) ClassBindBuilder<Type>(#Type, Subname)

#endif