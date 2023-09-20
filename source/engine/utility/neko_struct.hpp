
#ifndef NEKO_STRUCT_HPP
#define NEKO_STRUCT_HPP

#include <cstddef>
#include <type_traits>
#include <utility>

namespace neko {

struct __any {
    // 无定义 我们需要一个可以转换为任何类型的在以下特殊语境中使用的辅助类
    template <typename T>
    constexpr operator T() const;
};

// 计算结构体成员数量
#if !defined(_MSC_VER) || 0  // 我不知道为什么 if constexpr (!requires { T{Args...}; }) {...} 方法会导致目前版本的vs代码感知非常卡

template <typename T>
consteval size_t struct_size(auto&&... Args) {
    if constexpr (!requires { T{Args...}; }) {
        return sizeof...(Args) - 1;
    } else {
        return struct_size<T>(Args..., __any{});
    }
}

template <class T>
struct struct_member_count : std::integral_constant<std::size_t, struct_size<T>()> {};

#else

template <typename T, typename = void, typename... Ts>
struct struct_size {
    constexpr static size_t value = sizeof...(Ts) - 1;
};

template <typename T, typename... Ts>
struct struct_size<T, std::void_t<decltype(T{Ts{}...})>, Ts...> {
    constexpr static size_t value = struct_size<T, void, Ts..., __any>::value;
};

template <class T>
struct struct_member_count : std::integral_constant<std::size_t, struct_size<T>::value> {};

#endif

#include "neko_pp.inl"

#define STRUCT_APPLYER_DEF(N)                                                              \
    template <class T, class F>                                                            \
    auto __struct_apply_impl(T&& my_struct, F f, std::integral_constant<std::size_t, N>) { \
        auto&& [NEKO_PP_PARAMS(x, N)] = std::forward<T>(my_struct);                        \
        return std::invoke(f, NEKO_PP_PARAMS(x, N));                                       \
    }

// 使用宏打表 STRUCT_APPLYER_DEF 内使用结构化绑定得到结构体成员序列
NEKO_PP_FOR_EACH(STRUCT_APPLYER_DEF, 127)

// struct_apply 把结构体解包为变长参数调用可调用对象ApplyFunc
template <class T, class F>
auto struct_apply(T&& _struct, F f) {
    return __struct_apply_impl(std::forward<T>(_struct), f, struct_member_count<typename std::decay<T>::type>());
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

template <typename T>
inline constexpr auto __neko_gen_struct_meta() {
    return std::make_tuple();
}

#define neko_struct(__struct, ...)                             \
    template <>                                                \
    inline constexpr auto __neko_gen_struct_meta<__struct>() { \
        using T = __struct;                                    \
        return std::make_tuple(__VA_ARGS__);                   \
    };

// 这里_F*用于生成__neko_gen_struct_meta内tuple叠入的tuple
#define _F(field) (std::make_tuple(#field, &T::field))
#define _Fs(field, metainfo) (std::make_tuple(#field, &T::field, metainfo))

template <typename T, typename Fields, typename F, size_t... Is>
inline constexpr void neko_struct_foreach_impl(T&& obj, Fields&& fields, F&& f, std::index_sequence<Is...>) {
    auto ff = [&](auto index) {
        auto& t = std::get<index>(fields);
        if constexpr (std::tuple_size_v<std::decay_t<decltype(t)>> == 3) {
            std::apply([&](auto&& arg1, auto&& arg2, auto&& info) { f(arg1, obj.*arg2, info); }, t);
        } else if constexpr (std::tuple_size_v<std::decay_t<decltype(t)>> == 2) {
            std::apply([&](auto&& arg1, auto&& arg2) { f(arg1, obj.*arg2, "default"); }, t);
        }
        // std::cout << "^^ 傻逼 " << std::tuple_size_v<std::decay_t<decltype(fields)>> << std::endl;
    };
    // 逗号双层表达式 因为ff没有返回值则Is作为里层逗号表达式的结果
    auto _ = ((ff(std::integral_constant<size_t, Is>{}), Is), ...);
}

template <typename T, typename F>
inline constexpr void neko_struct_foreach(T&& obj, F&& f) {
    // 获取宏生成的元数据 tuple
    constexpr auto fields = __neko_gen_struct_meta<std::decay_t<T>>();
    // 调用 neko_struct_foreach_impl 函数 并传递 obj/fields/f
    // std::make_index_sequence 来确认范围
    neko_struct_foreach_impl(std::forward<T>(obj), fields, std::forward<F>(f), std::make_index_sequence<std::tuple_size_v<decltype(fields)>>{});
}

}  // namespace neko

#endif  // !NEKO_STRUCT_HPP
