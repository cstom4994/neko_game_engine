
#ifndef NEKO_STRUCT_HPP
#define NEKO_STRUCT_HPP

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

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
#define _Fs(field, ...) (std::make_tuple(#field, &T::field, std::make_tuple(__VA_ARGS__)))

template <typename T, typename Fields, typename F, size_t... Is>
inline constexpr void neko_struct_foreach_impl(T&& obj, Fields&& fields, F&& f, std::index_sequence<Is...>) {
    auto ff = [&](auto index) {
        auto& t = std::get<index>(fields);
        constexpr static std::size_t t_size = std::tuple_size_v<std::decay_t<decltype(t)>>;
        if constexpr (t_size == 3)
            std::apply([&](auto&& arg1, auto&& arg2, auto&& info) { f(arg1, obj.*arg2, info); }, t);
        else if constexpr (t_size == 2)
            std::apply([&](auto&& arg1, auto&& arg2) { f(arg1, obj.*arg2, std::make_tuple("default")); }, t);
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

#define STRUCT_APPLYER_DEF(N)                                                              \
    template <class T, class F>                                                            \
    auto __struct_apply_impl(T&& my_struct, F f, std::integral_constant<std::size_t, N>) { \
        auto&& [NEKO_PP_PARAMS(x, N)] = std::forward<T>(my_struct);                        \
        return std::invoke(f, NEKO_PP_PARAMS(x, N));                                       \
    }

#if (!defined NEKO_PP_FOR_EACH || !defined NEKO_PP_PARAMS)

// 横向迭代专用，NEKO_PP_PARAMS(x, 3) => x1, x2, x3

#define NEKO_PP_PARAMS_0(x)
#define NEKO_PP_PARAMS_1(x) x##1
#define NEKO_PP_PARAMS_2(x) NEKO_PP_PARAMS_1(x), x##2
#define NEKO_PP_PARAMS_3(x) NEKO_PP_PARAMS_2(x), x##3
#define NEKO_PP_PARAMS_4(x) NEKO_PP_PARAMS_3(x), x##4
#define NEKO_PP_PARAMS_5(x) NEKO_PP_PARAMS_4(x), x##5
#define NEKO_PP_PARAMS_6(x) NEKO_PP_PARAMS_5(x), x##6
#define NEKO_PP_PARAMS_7(x) NEKO_PP_PARAMS_6(x), x##7
#define NEKO_PP_PARAMS_8(x) NEKO_PP_PARAMS_7(x), x##8
#define NEKO_PP_PARAMS_9(x) NEKO_PP_PARAMS_8(x), x##9
#define NEKO_PP_PARAMS_10(x) NEKO_PP_PARAMS_9(x), x##10
#define NEKO_PP_PARAMS_11(x) NEKO_PP_PARAMS_10(x), x##11
#define NEKO_PP_PARAMS_12(x) NEKO_PP_PARAMS_11(x), x##12
#define NEKO_PP_PARAMS_13(x) NEKO_PP_PARAMS_12(x), x##13
#define NEKO_PP_PARAMS_14(x) NEKO_PP_PARAMS_13(x), x##14
#define NEKO_PP_PARAMS_15(x) NEKO_PP_PARAMS_14(x), x##15
#define NEKO_PP_PARAMS_16(x) NEKO_PP_PARAMS_15(x), x##16
#define NEKO_PP_PARAMS_17(x) NEKO_PP_PARAMS_16(x), x##17
#define NEKO_PP_PARAMS_18(x) NEKO_PP_PARAMS_17(x), x##18
#define NEKO_PP_PARAMS_19(x) NEKO_PP_PARAMS_18(x), x##19
#define NEKO_PP_PARAMS_20(x) NEKO_PP_PARAMS_19(x), x##20
#define NEKO_PP_PARAMS_21(x) NEKO_PP_PARAMS_20(x), x##21
#define NEKO_PP_PARAMS_22(x) NEKO_PP_PARAMS_21(x), x##22
#define NEKO_PP_PARAMS_23(x) NEKO_PP_PARAMS_22(x), x##23
#define NEKO_PP_PARAMS_24(x) NEKO_PP_PARAMS_23(x), x##24
#define NEKO_PP_PARAMS_25(x) NEKO_PP_PARAMS_24(x), x##25
#define NEKO_PP_PARAMS_26(x) NEKO_PP_PARAMS_25(x), x##26
#define NEKO_PP_PARAMS_27(x) NEKO_PP_PARAMS_26(x), x##27
#define NEKO_PP_PARAMS_28(x) NEKO_PP_PARAMS_27(x), x##28
#define NEKO_PP_PARAMS_29(x) NEKO_PP_PARAMS_28(x), x##29
#define NEKO_PP_PARAMS_30(x) NEKO_PP_PARAMS_29(x), x##30
#define NEKO_PP_PARAMS_31(x) NEKO_PP_PARAMS_30(x), x##31
#define NEKO_PP_PARAMS_32(x) NEKO_PP_PARAMS_31(x), x##32
#define NEKO_PP_PARAMS_33(x) NEKO_PP_PARAMS_32(x), x##33
#define NEKO_PP_PARAMS_34(x) NEKO_PP_PARAMS_33(x), x##34
#define NEKO_PP_PARAMS_35(x) NEKO_PP_PARAMS_34(x), x##35
#define NEKO_PP_PARAMS_36(x) NEKO_PP_PARAMS_35(x), x##36
#define NEKO_PP_PARAMS_37(x) NEKO_PP_PARAMS_36(x), x##37
#define NEKO_PP_PARAMS_38(x) NEKO_PP_PARAMS_37(x), x##38
#define NEKO_PP_PARAMS_39(x) NEKO_PP_PARAMS_38(x), x##39
#define NEKO_PP_PARAMS_40(x) NEKO_PP_PARAMS_39(x), x##40
#define NEKO_PP_PARAMS_41(x) NEKO_PP_PARAMS_40(x), x##41
#define NEKO_PP_PARAMS_42(x) NEKO_PP_PARAMS_41(x), x##42
#define NEKO_PP_PARAMS_43(x) NEKO_PP_PARAMS_42(x), x##43
#define NEKO_PP_PARAMS_44(x) NEKO_PP_PARAMS_43(x), x##44
#define NEKO_PP_PARAMS_45(x) NEKO_PP_PARAMS_44(x), x##45
#define NEKO_PP_PARAMS_46(x) NEKO_PP_PARAMS_45(x), x##46
#define NEKO_PP_PARAMS_47(x) NEKO_PP_PARAMS_46(x), x##47
#define NEKO_PP_PARAMS_48(x) NEKO_PP_PARAMS_47(x), x##48
#define NEKO_PP_PARAMS_49(x) NEKO_PP_PARAMS_48(x), x##49
#define NEKO_PP_PARAMS_50(x) NEKO_PP_PARAMS_49(x), x##50
#define NEKO_PP_PARAMS_51(x) NEKO_PP_PARAMS_50(x), x##51
#define NEKO_PP_PARAMS_52(x) NEKO_PP_PARAMS_51(x), x##52
#define NEKO_PP_PARAMS_53(x) NEKO_PP_PARAMS_52(x), x##53
#define NEKO_PP_PARAMS_54(x) NEKO_PP_PARAMS_53(x), x##54
#define NEKO_PP_PARAMS_55(x) NEKO_PP_PARAMS_54(x), x##55
#define NEKO_PP_PARAMS_56(x) NEKO_PP_PARAMS_55(x), x##56
#define NEKO_PP_PARAMS_57(x) NEKO_PP_PARAMS_56(x), x##57
#define NEKO_PP_PARAMS_58(x) NEKO_PP_PARAMS_57(x), x##58
#define NEKO_PP_PARAMS_59(x) NEKO_PP_PARAMS_58(x), x##59
#define NEKO_PP_PARAMS_60(x) NEKO_PP_PARAMS_59(x), x##60
#define NEKO_PP_PARAMS_61(x) NEKO_PP_PARAMS_60(x), x##61
#define NEKO_PP_PARAMS_62(x) NEKO_PP_PARAMS_61(x), x##62
#define NEKO_PP_PARAMS_63(x) NEKO_PP_PARAMS_62(x), x##63

#define NEKO_PP_PARAMS(x, N) NEKO_PP_PARAMS_##N(x)

// 纵向迭代专用

#define NEKO_PP_FOR_EACH_0(x)
#define NEKO_PP_FOR_EACH_1(x) NEKO_PP_FOR_EACH_0(x) x(1)
#define NEKO_PP_FOR_EACH_2(x) NEKO_PP_FOR_EACH_1(x) x(2)
#define NEKO_PP_FOR_EACH_3(x) NEKO_PP_FOR_EACH_2(x) x(3)
#define NEKO_PP_FOR_EACH_4(x) NEKO_PP_FOR_EACH_3(x) x(4)
#define NEKO_PP_FOR_EACH_5(x) NEKO_PP_FOR_EACH_4(x) x(5)
#define NEKO_PP_FOR_EACH_6(x) NEKO_PP_FOR_EACH_5(x) x(6)
#define NEKO_PP_FOR_EACH_7(x) NEKO_PP_FOR_EACH_6(x) x(7)
#define NEKO_PP_FOR_EACH_8(x) NEKO_PP_FOR_EACH_7(x) x(8)
#define NEKO_PP_FOR_EACH_9(x) NEKO_PP_FOR_EACH_8(x) x(9)
#define NEKO_PP_FOR_EACH_10(x) NEKO_PP_FOR_EACH_9(x) x(10)
#define NEKO_PP_FOR_EACH_11(x) NEKO_PP_FOR_EACH_10(x) x(11)
#define NEKO_PP_FOR_EACH_12(x) NEKO_PP_FOR_EACH_11(x) x(12)
#define NEKO_PP_FOR_EACH_13(x) NEKO_PP_FOR_EACH_12(x) x(13)
#define NEKO_PP_FOR_EACH_14(x) NEKO_PP_FOR_EACH_13(x) x(14)
#define NEKO_PP_FOR_EACH_15(x) NEKO_PP_FOR_EACH_14(x) x(15)
#define NEKO_PP_FOR_EACH_16(x) NEKO_PP_FOR_EACH_15(x) x(16)
#define NEKO_PP_FOR_EACH_17(x) NEKO_PP_FOR_EACH_16(x) x(17)
#define NEKO_PP_FOR_EACH_18(x) NEKO_PP_FOR_EACH_17(x) x(18)
#define NEKO_PP_FOR_EACH_19(x) NEKO_PP_FOR_EACH_18(x) x(19)
#define NEKO_PP_FOR_EACH_20(x) NEKO_PP_FOR_EACH_19(x) x(20)
#define NEKO_PP_FOR_EACH_21(x) NEKO_PP_FOR_EACH_20(x) x(21)
#define NEKO_PP_FOR_EACH_22(x) NEKO_PP_FOR_EACH_21(x) x(22)
#define NEKO_PP_FOR_EACH_23(x) NEKO_PP_FOR_EACH_22(x) x(23)
#define NEKO_PP_FOR_EACH_24(x) NEKO_PP_FOR_EACH_23(x) x(24)
#define NEKO_PP_FOR_EACH_25(x) NEKO_PP_FOR_EACH_24(x) x(25)
#define NEKO_PP_FOR_EACH_26(x) NEKO_PP_FOR_EACH_25(x) x(26)
#define NEKO_PP_FOR_EACH_27(x) NEKO_PP_FOR_EACH_26(x) x(27)
#define NEKO_PP_FOR_EACH_28(x) NEKO_PP_FOR_EACH_27(x) x(28)
#define NEKO_PP_FOR_EACH_29(x) NEKO_PP_FOR_EACH_28(x) x(29)
#define NEKO_PP_FOR_EACH_30(x) NEKO_PP_FOR_EACH_29(x) x(30)
#define NEKO_PP_FOR_EACH_31(x) NEKO_PP_FOR_EACH_30(x) x(31)
#define NEKO_PP_FOR_EACH_32(x) NEKO_PP_FOR_EACH_31(x) x(32)
#define NEKO_PP_FOR_EACH_33(x) NEKO_PP_FOR_EACH_32(x) x(33)
#define NEKO_PP_FOR_EACH_34(x) NEKO_PP_FOR_EACH_33(x) x(34)
#define NEKO_PP_FOR_EACH_35(x) NEKO_PP_FOR_EACH_34(x) x(35)
#define NEKO_PP_FOR_EACH_36(x) NEKO_PP_FOR_EACH_35(x) x(36)
#define NEKO_PP_FOR_EACH_37(x) NEKO_PP_FOR_EACH_36(x) x(37)
#define NEKO_PP_FOR_EACH_38(x) NEKO_PP_FOR_EACH_37(x) x(38)
#define NEKO_PP_FOR_EACH_39(x) NEKO_PP_FOR_EACH_38(x) x(39)
#define NEKO_PP_FOR_EACH_40(x) NEKO_PP_FOR_EACH_39(x) x(40)
#define NEKO_PP_FOR_EACH_41(x) NEKO_PP_FOR_EACH_40(x) x(41)
#define NEKO_PP_FOR_EACH_42(x) NEKO_PP_FOR_EACH_41(x) x(42)
#define NEKO_PP_FOR_EACH_43(x) NEKO_PP_FOR_EACH_42(x) x(43)
#define NEKO_PP_FOR_EACH_44(x) NEKO_PP_FOR_EACH_43(x) x(44)
#define NEKO_PP_FOR_EACH_45(x) NEKO_PP_FOR_EACH_44(x) x(45)
#define NEKO_PP_FOR_EACH_46(x) NEKO_PP_FOR_EACH_45(x) x(46)
#define NEKO_PP_FOR_EACH_47(x) NEKO_PP_FOR_EACH_46(x) x(47)
#define NEKO_PP_FOR_EACH_48(x) NEKO_PP_FOR_EACH_47(x) x(48)
#define NEKO_PP_FOR_EACH_49(x) NEKO_PP_FOR_EACH_48(x) x(49)
#define NEKO_PP_FOR_EACH_50(x) NEKO_PP_FOR_EACH_49(x) x(50)
#define NEKO_PP_FOR_EACH_51(x) NEKO_PP_FOR_EACH_50(x) x(51)
#define NEKO_PP_FOR_EACH_52(x) NEKO_PP_FOR_EACH_51(x) x(52)
#define NEKO_PP_FOR_EACH_53(x) NEKO_PP_FOR_EACH_52(x) x(53)
#define NEKO_PP_FOR_EACH_54(x) NEKO_PP_FOR_EACH_53(x) x(54)
#define NEKO_PP_FOR_EACH_55(x) NEKO_PP_FOR_EACH_54(x) x(55)
#define NEKO_PP_FOR_EACH_56(x) NEKO_PP_FOR_EACH_55(x) x(56)
#define NEKO_PP_FOR_EACH_57(x) NEKO_PP_FOR_EACH_56(x) x(57)
#define NEKO_PP_FOR_EACH_58(x) NEKO_PP_FOR_EACH_57(x) x(58)
#define NEKO_PP_FOR_EACH_59(x) NEKO_PP_FOR_EACH_58(x) x(59)
#define NEKO_PP_FOR_EACH_60(x) NEKO_PP_FOR_EACH_59(x) x(60)
#define NEKO_PP_FOR_EACH_61(x) NEKO_PP_FOR_EACH_60(x) x(61)
#define NEKO_PP_FOR_EACH_62(x) NEKO_PP_FOR_EACH_61(x) x(62)
#define NEKO_PP_FOR_EACH_63(x) NEKO_PP_FOR_EACH_62(x) x(63)

#define NEKO_PP_FOR_EACH(x, N) NEKO_PP_FOR_EACH_##N(x)

#endif

// 使用宏打表 STRUCT_APPLYER_DEF 内使用结构化绑定得到结构体成员序列
NEKO_PP_FOR_EACH(STRUCT_APPLYER_DEF, 63)

#endif  // !NEKO_STRUCT_HPP
