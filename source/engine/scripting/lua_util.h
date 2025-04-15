
#ifndef NEKO_LUA_UTIL_H
#define NEKO_LUA_UTIL_H

#include "engine/scripting/lua_wrapper.hpp"
#include "engine/scripting/lua_function.hpp"
#include "engine/bootstrap.h"

struct TValue;
struct Table;

namespace Neko {

// 将TValue压入栈
void PushTValue(lua_State *L, const TValue *v);

// 获取数组部分的下一个非nil元素的索引
int LuaTableArrayNext(Table *t, int index);

// 获取哈希部分的下一个非nil元素的索引
int LuaTableHashNext(const Table *t, int index);

class ClassBindBuilder {
    String name;
    String subname;

    lua_State *L;

    enum class ClassType {
        CLASS_MODULE,
        CLASS_SUBSYSTEM,
        CLASS_OO,
    } classType;

public:
    explicit ClassBindBuilder(String name_) : name(name_), classType(ClassType::CLASS_MODULE) {
        L = ENGINE_LUA();
        lua_getglobal(L, "neko");
        if (!lua_istable(L, -1)) {
            error_assert("no neko table");
        }
    }

    explicit ClassBindBuilder(String name_, String subname_) : name(name_), subname(subname_), classType(ClassType::CLASS_SUBSYSTEM) {
        L = ENGINE_LUA();
        lua_getglobal(L, "neko");
        if (!lua_istable(L, -1)) {
            error_assert("no neko table");
        }
        lua_createtable(L, 0, 0);
    }

    bool Build() {
        if (classType == ClassType::CLASS_SUBSYSTEM) {
            lua_setfield(L, -2, subname.cstr());
        }
        lua_pop(L, 1);
        return true;
    }

    template <typename FUNC>
    ClassBindBuilder &Method(const String &name, FUNC ptr) {
        register_function<FUNC>(L, name.cstr(), ptr);
        lua_setfield(L, -2, name.cstr());
        return *this;
    }

    template <typename T, typename FUNC>
    ClassBindBuilder &MemberMethod(const String &name, T *obj, FUNC ptr) {
        register_member_function(L, name.cstr(), obj, ptr);
        lua_setfield(L, -2, name.cstr());
        return *this;
    }

    ClassBindBuilder &CClosure(const std::vector<luaL_Reg> &list) {
        for (auto &r : list) {
            lua_pushcclosure(L, r.func, 0);
            lua_setfield(L, -2, r.name);
        }
        return *this;
    }

    ClassBindBuilder &CustomBind(const String &name, lua_CFunction func) {
        func(L);
        return *this;
    }

    template <typename T>
    ClassBindBuilder &RegClass() {
        auto name = typeid(T).name();
        luaL_newmetatable(L, name);

        lua_pushstring(L, "__gc");
        lua_pushcfunction(L, &FreeClass<T>);
        lua_settable(L, -3);

        lua_pushstring(L, "__index");
        lua_pushvalue(L, -2);  // Push metatable
        lua_settable(L, -3);   // metatable.__index = metatable

        lua_pop(L, 1);

        lua_pushcclosure(L, NewClass<T>, 0);
        lua_setfield(L, -2, "new_A");

        return *this;
    }

    template <typename T, typename return_type, typename... arg_types>
    ClassBindBuilder &RegClassFunction(const char *func_name, return_type (T::*func)(arg_types...)) {
        auto name = typeid(T).name();
        if (!luaL_getmetatable(L, name)) {
            lua_pop(L, 1);

            return *this;
        }

        auto func_mem = new char[sizeof(func)];
        new (func_mem)(return_type(T::*)(arg_types...))(func);

        lua_pushstring(L, func_name);
        lua_pushlightuserdata(L, func_mem);
        lua_pushcclosure(L, CallClassFunc<T, return_type, arg_types...>, 1);
        lua_settable(L, -3);

        lua_pop(L, 1);

        return *this;
    }

    template <typename T, typename... arg_types>
    ClassBindBuilder &RegClassFunction(const char *func_name, void (T::*func)(arg_types...)) {
        auto name = typeid(T).name();
        if (!luaL_getmetatable(L, name)) {
            lua_pop(L, 1);

            return *this;
        }

        auto func_mem = new char[sizeof(func)];
        new (func_mem)(void(T::*)(arg_types...))(func);

        lua_pushstring(L, func_name);
        lua_pushlightuserdata(L, func_mem);
        lua_pushcclosure(L, CallClassFuncVoid<T, arg_types...>, 1);
        lua_settable(L, -3);

        lua_pop(L, 1);

        return *this;
    }

public:
    template <typename T>
    static T *CheckClassUd(lua_State *L, int i) noexcept {
        auto name = typeid(T).name();
        void *user_data = luaL_checkudata(L, i, name);
        luaL_argcheck(L, user_data != NULL, i, (std::string(name) + " expected").c_str());
        return *static_cast<T **>(user_data);
    }

    template <typename T>
    static int FreeClass(lua_State *L) {
        auto t = CheckClassUd<T>(L, 1);
        delete t;
        return 0;
    }

    template <typename T>
    static int NewClass(lua_State *L) {
        T *v = new T;
        auto userdata = static_cast<T **>(lua_newuserdata(L, sizeof(T *)));
        *userdata = v;
        auto name = typeid(T).name();
        luaL_setmetatable(L, name);
        return 1;
    }

private:
    template <typename T, typename return_type, size_t... I, typename... arg_types>
    static return_type ClassFuncCaller(lua_State *L, T *obj, return_type (T::*func)(arg_types...), std::index_sequence<I...> &&) {
        return ((obj)->*func)(LuaGet<arg_types>(L, I + 2)...);
    }

    template <typename T, typename return_type, typename... arg_types>
    static int CallClassFunc(lua_State *L) {
        auto obj = CheckClassUd<T>(L, 1);
        auto func = *(return_type(T::**)(arg_types...))lua_touserdata(L, lua_upvalueindex(1));
        LuaPush<return_type>(L, ClassFuncCaller(L, obj, func, std::make_index_sequence<sizeof...(arg_types)>()));
        return 1;
    }

    template <typename T, typename... arg_types>
    static int CallClassFuncVoid(lua_State *L) {
        auto obj = CheckClassUd<T>(L, 1);
        auto func = *(void(T::**)(arg_types...))lua_touserdata(L, lua_upvalueindex(1));
        ClassFuncCaller(L, obj, func, std::make_index_sequence<sizeof...(arg_types)>());
        return 0;
    }
};

}  // namespace Neko

// #define REGISTER_TYPE(Type) TypeRegistry::Instance().RegisterType<Type>(#Type)
#define BUILD_TYPE(Type) ClassBindBuilder(#Type)
#define BUILD_TYPE_SUB(Type, Subname) ClassBindBuilder(#Type, Subname)

#endif