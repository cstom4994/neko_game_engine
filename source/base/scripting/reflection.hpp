
#pragma once

#include "base/scripting/lua_wrapper.hpp"

namespace Neko::reflection {

#define STRUCT_APPLYER_DEF(N)                                                              \
    template <class T, class F>                                                            \
    auto __struct_apply_impl(T &&my_struct, F f, std::integral_constant<std::size_t, N>) { \
        auto &&[NEKO_PP_PARAMS(x, N)] = std::forward<T>(my_struct);                        \
        return std::invoke(f, NEKO_PP_PARAMS(x, N));                                       \
    }

NEKO_PP_FOR_EACH(STRUCT_APPLYER_DEF, 63)

// struct_apply 把结构体解包为变长参数调用可调用对象ApplyFunc
template <class T, class F>
auto struct_apply(T &&_struct, F f) {
    constexpr auto N = field_count<typename std::decay<T>::type>;
    return __struct_apply_impl(std::forward<T>(_struct), f, std::integral_constant<std::size_t, N>{});
}

// StructTransformMeta 把结构体各成员的类型作为变长参数调用元函数 F
template <class T, template <class...> class F>
struct struct_transform_meta {
    struct __fake_applyer {
        template <class... Args>
        auto operator()(Args... args) -> F<decltype(args)...>;
    };
    using type = decltype(struct_apply(std::declval<T>(), __fake_applyer()));
};

template <class T>
struct StructMetaData {
    using U = std::void_t<>;
    inline constexpr auto __gen_struct_meta() { return std::make_tuple(); }
};

#define NEKO_STRUCT(__struct, ...)                                                         \
    template <>                                                                            \
    struct ::Neko::reflection::StructMetaData<__struct> {                                  \
        using U = __struct;                                                                \
        inline constexpr auto __gen_struct_meta() { return std::make_tuple(__VA_ARGS__); } \
    };

// 这里_F*用于生成__gen_struct_meta内tuple叠入的tuple
#define _F(field) (std::make_tuple(#field, &U::field))
#define _Fs(field, ...) (std::make_tuple(#field, &U::field, std::make_tuple(__VA_ARGS__)))

#define FIELD_TYPES() bool, i32, f32, f32 *, String

template <typename T, typename Fields, typename F, size_t... Is>
inline constexpr void struct_foreach_impl(T &&obj, Fields &&fields, F &&f, std::index_sequence<Is...>) {
    auto ff = [&](auto index) {
        auto &t = std::get<index>(fields);
        constexpr static std::size_t t_size = std::tuple_size_v<std::decay_t<decltype(t)>>;
        if constexpr (t_size == 3)
            std::apply([&](auto &&arg1, auto &&arg2, auto &&info) { f(arg1, obj.*arg2, info); }, t);
        else if constexpr (t_size == 2)
            std::apply([&](auto &&arg1, auto &&arg2) { f(arg1, obj.*arg2, std::make_tuple("default")); }, t);
        // std::cout << "^^ 傻逼 " << std::tuple_size_v<std::decay_t<decltype(fields)>> << std::endl;
    };
    // 逗号双层表达式 因为ff没有返回值则Is作为里层逗号表达式的结果
    auto _ = ((ff(std::integral_constant<size_t, Is>{}), Is), ...);
    std::ignore = _;
}

template <typename T, typename F>
inline constexpr void struct_foreach(T &&obj, F &&f) {
    // 获取宏生成的元数据 tuple
    constexpr auto fields = StructMetaData<std::decay_t<T>>{}.__gen_struct_meta();
    // 调用 neko_struct_foreach_impl 函数 并传递 obj/fields/f
    // std::make_index_sequence 来确认范围
    struct_foreach_impl(std::forward<T>(obj), fields, std::forward<F>(f), std::make_index_sequence<std::tuple_size_v<decltype(fields)>>{});
}

template <typename T>
constexpr bool is_refl_struct = std::is_class_v<T> && requires {
    typename StructMetaData<T>::U;                                   // 检查是否定义了 U
    { StructMetaData<T>{}.__gen_struct_meta() };                     // 检查 __gen_struct_meta
} && !std::is_same_v<typename StructMetaData<T>::U, std::void_t<>>;  // 确保 U 不为 std::void_t

template <typename T, typename F, typename Fields = std::tuple<>>
void struct_foreach_rec(F func, T &&obj, int depth = 0, const char *fieldName = "", Fields &&fields = std::make_tuple()) {
    if constexpr (reflection::is_refl_struct<std::decay_t<T>>) {
        reflection::struct_foreach(obj, [&](auto &&fieldName, auto &&value, auto &&info) { struct_foreach_rec(func, value, depth + 1, fieldName, info); });
    } else {
        auto f = [&]<typename S>(const char *name, auto &var, S &t) {
            if constexpr (std::is_same_v<std::decay_t<decltype(var)>, S>) {
                func(name, var, t, fields);
            }
        };
        std::apply([&](auto &&...args) { (f(fieldName, obj, args), ...); }, std::tuple<FIELD_TYPES()>());
    }
}

}  // namespace Neko::reflection
