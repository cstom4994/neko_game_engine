
#pragma once

#include "engine/scripting/lua_wrapper.hpp"
#include "base/common/reflection.hpp"

namespace Neko::luabind {

template <typename Tuple, size_t... Is>
Tuple get_args_impl(lua_State* L, int start, std::index_sequence<Is...>) {
    return {
        [&]<size_t I>() -> auto {
            using T = std::tuple_element_t<I, Tuple>;  // 获取元组中第I个位置的类型
            auto value = LuaGet<T>(L, start + I);
            using RawType = std::remove_cv_t<decltype(value)>;
            if constexpr (std::is_pointer_v<RawType> && std::is_class_v<std::remove_pointer_t<RawType>>) {
                return *value;
            } else {
                return value;
            }
        }.template operator()<Is>()...
    };
}

template <typename Tuple>
Tuple get_args_from_lua(lua_State* L, int start) {
    return get_args_impl<Tuple>(L, start, std::make_index_sequence<std::tuple_size_v<Tuple>>{});
}

template <typename Func>
int lua_callback_impl(lua_State* L) {
    using traits = reflection::function_traits<Func>;
    using return_type = typename traits::return_type;
    using args_tuple = typename traits::args_type;
    constexpr int arity = traits::arity;

    if (lua_gettop(L) != arity) {
        return luaL_error(L, "Expected %d arguments", arity);
    }

    // 从upvalue获取函数指针
    Func** pfunc = static_cast<Func**>(lua_touserdata(L, lua_upvalueindex(1)));
    Func& func = **pfunc;

    args_tuple args = get_args_from_lua<args_tuple>(L, 1);  // 从Lua栈获取参数

    try {
        if constexpr (!std::is_same_v<return_type, void>) {
            auto result = std::apply(func, args);
            using RawType = std::remove_cv_t<decltype(result)>;
            if constexpr (std::is_class_v<RawType>) {
                LuaPush<RawType>(L, result);
            } else {
                LuaPush(L, result);
            }
            return 1;
        } else {
            std::apply(func, args);
            return 0;
        }
    } catch (const std::exception& e) {
        luaL_error(L, "C++ Exception: %s", e.what());
        return 0;
    }
}

template <typename Func>
int luabind_gc_impl(lua_State* L) {
    Func** p = static_cast<Func**>(lua_touserdata(L, 1));
    mem_del(*p);
    return 0;
}

template <typename Func>
void register_function(lua_State* L, const char* name, Func func) {
    using FuncType = std::decay_t<Func>;

    FuncType* pFunc = mem_new<FuncType>(std::move(func));

    void* p = lua_newuserdata(L, sizeof(FuncType*));
    *(FuncType**)p = pFunc;

    if (luaL_newmetatable(L, "Neko::luabind::Callable")) {
        lua_pushcfunction(L, &luabind_gc_impl<FuncType>);
        lua_setfield(L, -2, "__gc");
    }
    lua_setmetatable(L, -2);

    lua_pushcclosure(L, &lua_callback_impl<FuncType>, 1);
}

// 注册成员函数的辅助函数
template <typename T, typename R, typename... Args>
void register_member_function(lua_State* L, const char* name, T* obj, R (T::*mem_func)(Args...)) {
    auto func = [obj, mem_func](Args... args) -> R { return (obj->*mem_func)(std::forward<Args>(args)...); };
    register_function(L, name, func);
}

// 处理const成员函数的重载
template <typename T, typename R, typename... Args>
void register_member_function(lua_State* L, const char* name, T* obj, R (T::*mem_func)(Args...) const) {
    auto func = [obj, mem_func](Args... args) -> R { return (obj->*mem_func)(std::forward<Args>(args)...); };
    register_function(L, name, func);
}

}  // namespace Neko::luabind