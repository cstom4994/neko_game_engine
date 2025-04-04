
#pragma once

#include "base/scripting/lua_wrapper.hpp"
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

template <typename R>
struct CallFunction {
    template <typename Func, typename Tuple>
    static int Do(lua_State* L, Func func, Tuple&& args) {
        R result = std::apply(func, std::forward<Tuple>(args));
        LuaPush(L, result);
        return 1;
    }
};

template <>
struct CallFunction<void> {
    template <typename Func, typename Tuple>
    static int Do(lua_State* L, Func func, Tuple&& args) {
        std::apply(func, std::forward<Tuple>(args));
        return 0;
    }
};

template <typename Func>
int lua_callback_impl(lua_State* L) {
    using traits = reflection::function_traits<Func>;
    using return_type = typename traits::return_type;
    using args_tuple = typename traits::args_type;
    constexpr int arity = traits::arity;

    if (lua_gettop(L) != arity) {
        return luaL_error(L, "Expected %d arguments", arity);
    }

    Func* func = static_cast<Func*>(lua_touserdata(L, lua_upvalueindex(1)));  // 从upvalue获取函数指针
    args_tuple args = get_args_from_lua<args_tuple>(L, 1);                    // 从Lua栈获取参数

    try {
        if constexpr (!std::is_same_v<return_type, void>) {
            auto result = std::apply(*func, args);
            LuaPush(L, result);
            return 1;
        } else {
            std::apply(*func, args);
            return 0;
        }
    } catch (const std::exception& e) {
        luaL_error(L, "C++ Exception: %s", e.what());
        return 0;
    }
}

template <typename Func>
void register_function(lua_State* L, const char* name, Func func) {
    void* p = lua_newuserdata(L, sizeof(Func));
    new (p) Func(func);
    lua_pushcclosure(L, &lua_callback_impl<Func>, 1);
}

}  // namespace Neko::luabind