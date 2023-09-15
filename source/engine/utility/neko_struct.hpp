
#ifndef NEKO_STRUCT_HPP
#define NEKO_STRUCT_HPP

#include <type_traits>
#include <utility>

namespace neko {

// 计算结构体成员数量
#if !defined(_MSC_VER)  // 我不知道为什么 if constexpr (!requires { T{Args...}; }) {...} 方法会导致目前版本的vs代码感知非常卡

struct __any {
    // 无定义 我们需要一个可以转换为任何类型的在以下特殊语境中使用的辅助类
    template <typename T>
    constexpr operator T() const;
    template <typename T>
    constexpr operator T &() const;
    template <typename T>
    constexpr operator T &&() const;
};

template <typename T>
consteval size_t struct_size(auto &&...Args) {
    if constexpr (!requires { T{Args...}; }) {
        return sizeof...(Args) - 1;
    } else {
        return struct_size<T>(Args..., __any{});
    }
}

template <class T>
struct struct_member_count : std::integral_constant<std::size_t, struct_size<T>()> {};

#else

template <class T>
struct __any {
    // 无定义 我们需要一个可以转换为除了自身类型意外的任何类型的辅助类
    template <class U, class = typename std::enable_if<!std::is_same<typename std::decay<T>::type, typename std::decay<U>::type>::value>::type>
    constexpr operator U() const noexcept;
    // template <class U, class = typename std::enable_if<!std::is_same<typename std::decay<T>::type, typename std::decay<U>::type>::value>::type>
    // constexpr operator U &() const noexcept;
    // template <class U, class = typename std::enable_if<!std::is_same<typename std::decay<T>::type, typename std::decay<U>::type>::value>::type>
    // constexpr operator U &&() const noexcept;
};

template <class T, std::size_t I>
struct __any_tagged : __any<T> {};

// 判断T是否可以进行聚合初始化 T{std::declval<Args>()...}
template <class T, class... Args>
constexpr auto __is_aggregate_constructible_impl(T &&, Args &&...args) -> decltype(T{{args}...}, std::true_type());
constexpr auto __is_aggregate_constructible_impl(...) -> std::false_type;

template <class T, class... Args>
struct __is_aggregate_constructible : decltype(__is_aggregate_constructible_impl(std::declval<T>(), std::declval<Args>()...)) {};

template <class T, class seq>
struct __is_aggregate_constructible_with_n_args;
template <class T, std::size_t... I>
struct __is_aggregate_constructible_with_n_args<T, std::index_sequence<I...> > : __is_aggregate_constructible<T, __any_tagged<T, I>...> {};

template <class T, class seq = std::make_index_sequence<sizeof(T)> >
struct struct_member_count_impl;
template <class T, std::size_t... I>
struct struct_member_count_impl<T, std::index_sequence<I...> > : std::integral_constant<std::size_t, (... + !!(__is_aggregate_constructible_with_n_args<T, std::make_index_sequence<I + 1> >::value))> {
};

template <class T>
struct struct_member_count : struct_member_count_impl<T> {};

#endif

#include "neko_pp.inl"

#define STRUCT_APPLYER_DEF(N)                                                              \
    template <class T, class F>                                                            \
    auto __struct_apply_impl(T &&my_struct, F f, std::integral_constant<std::size_t, N>) { \
        auto &&[NEKO_PP_PARAMS(x, N)] = std::forward<T>(my_struct);                        \
        return std::invoke(f, NEKO_PP_PARAMS(x, N));                                       \
    }

// 使用宏打表 STRUCT_APPLYER_DEF 内使用结构化绑定得到结构体成员序列
NEKO_PP_FOR_EACH(STRUCT_APPLYER_DEF, 127)

// struct_apply 把结构体解包为变长参数调用可调用对象ApplyFunc
template <class T, class F>
auto struct_apply(T &&_struct, F f) {
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
}  // namespace neko

#endif  // !NEKO_STRUCT_HPP
