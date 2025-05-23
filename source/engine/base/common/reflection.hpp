#pragma once

#include "base/common/base.hpp"
#include "base/common/pp.hpp"
#include "base/common/hashmap.hpp"
#include "base/common/string.hpp"

namespace std {
template <typename E, typename = std::enable_if_t<std::is_enum_v<E>>>
constexpr std::underlying_type_t<E> to_underlying(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}
}  // namespace std

namespace Neko {

namespace reflection {

template <unsigned short N>
struct cstring {
    constexpr explicit cstring(std::string_view str) noexcept : cstring{str, std::make_integer_sequence<unsigned short, N>{}} {}
    constexpr const char *data() const noexcept { return chars_; }
    constexpr unsigned short size() const noexcept { return N; }
    constexpr operator std::string_view() const noexcept { return {data(), size()}; }
    template <unsigned short... I>
    constexpr cstring(std::string_view str, std::integer_sequence<unsigned short, I...>) noexcept : chars_{str[I]..., '\0'} {}
    char chars_[static_cast<size_t>(N) + 1];
};
template <>
struct cstring<0> {
    constexpr explicit cstring(std::string_view) noexcept {}
    constexpr const char *data() const noexcept { return nullptr; }
    constexpr unsigned short size() const noexcept { return 0; }
    constexpr operator std::string_view() const noexcept { return {}; }
};

template <typename T>
constexpr auto name_raw() noexcept {
#if defined(__clang__) || defined(__GNUC__)
    std::string_view name = __PRETTY_FUNCTION__;
    size_t start = name.find('=') + 2;
    size_t end = name.size() - 1;
    return std::string_view{name.data() + start, end - start};
#elif defined(_MSC_VER)
    std::string_view name = __FUNCSIG__;
    size_t start = name.find('<') + 1;
    size_t end = name.rfind(">(");
    name = std::string_view{name.data() + start, end - start};
    start = name.find(' ');
    return start == std::string_view::npos ? name : std::string_view{name.data() + start + 1, name.size() - start - 1};
#else
#error Unsupported compiler
#endif
}

template <typename T>
constexpr auto name() noexcept {
    constexpr auto name = name_raw<T>();
    return cstring<name.size()>{name};
}
template <typename T>
constexpr auto name_v = name<T>();

struct DummyFlag {};
template <typename Enum, typename T, Enum enumValue>
inline int get_enum_value(std::unordered_map<int, std::string> &values) {
#if defined _MSC_VER && !defined __clang__
    std::string func(__FUNCSIG__);
    std::string mark = "DummyFlag";
    auto pos = func.find(mark) + mark.size();
    std::string enumStr = func.substr(pos);

    auto start = enumStr.find_first_not_of(", ");
    auto end = enumStr.find('>');
    if (start != enumStr.npos && end != enumStr.npos && enumStr[start] != '(') {
        enumStr = enumStr.substr(start, end - start);
        values.insert({(int)enumValue, enumStr});
    }

#else  // gcc, clang
    std::string func(__PRETTY_FUNCTION__);
    std::string mark = "enumValue = ";
    auto pos = func.find(mark) + mark.size();
    std::string enumStr = func.substr(pos, func.size() - pos - 1);
    char ch = enumStr[0];
    if (!(ch >= '0' && ch <= '9') && ch != '(') values.insert({(int)enumValue, enumStr});
#endif
    return 0;
}

template <typename Enum, int min_value, int... ints>
void guess_enum_range(std::unordered_map<int, std::string> &values, const std::integer_sequence<int, ints...> &) {
    auto dummy = {get_enum_value<Enum, DummyFlag, (Enum)(ints + min_value)>(values)...};
}

template <typename Enum, int... ints>
void guess_enum_bit_range(std::unordered_map<int, std::string> &values, const std::integer_sequence<int, ints...> &) {
    auto dummy = {get_enum_value<Enum, DummyFlag, (Enum)0>(values), get_enum_value<Enum, DummyFlag, (Enum)(1 << (int)ints)>(values)...};
}

template <typename T>
constexpr auto GetTypeName() {
    return reflection::name_v<T>.data();
}

template <typename T>
struct wrapper {
    T a;
};
template <typename T>
wrapper(T) -> wrapper<T>;

template <typename T>
static inline T storage = {};

template <auto T>
constexpr auto main_name_of_pointer() {
    constexpr auto is_identifier = [](char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_'; };
#if __GNUC__ && (!__clang__) && (!_MSC_VER)
    std::string_view str = __PRETTY_FUNCTION__;
    std::size_t start = str.rfind("::") + 2;
    std::size_t end = start;
    for (; end < str.size() && is_identifier(str[end]); end++) {
        1
    }
    return str.substr(start, end - start);
#elif __clang__
    std::string_view str = __PRETTY_FUNCTION__;
    std::size_t start = str.rfind(".") + 1;
    std::size_t end = start;
    for (; end < str.size() && is_identifier(str[end]); end++) {
    }
    return str.substr(start, end - start);
#elif _MSC_VER
    std::string_view str = __FUNCSIG__;
    std::size_t start = str.rfind("->") + 2;
    std::size_t end = start;
    for (; end < str.size() && is_identifier(str[end]); end++) {
    }
    return str.substr(start, end - start);
#else
    static_assert(false, "Not supported compiler");
#endif
}

// 无定义 我们需要一个可以转换为任何类型的在以下特殊语境中使用的辅助类
struct __Any {
    constexpr __Any() {}
    constexpr __Any(int) {}
    template <typename T>
        requires std::is_copy_constructible_v<T>
    constexpr operator T &() const;
    template <typename T>
        requires std::is_move_constructible_v<T>
    constexpr operator T &&() const;
    template <typename T>
        requires(!std::is_copy_constructible_v<T> && !std::is_move_constructible_v<T>)
    constexpr operator T() const;
};

// 计算结构体成员数量
#if !defined(_MSC_VER) || 0  // 我不知道为什么 if constexpr (!requires { T{Args...}; }) {...} 方法会导致目前版本的vs代码感知非常卡

template <typename T>
consteval size_t struct_size(auto &&...Args) {
    if constexpr (!requires { T{Args...}; }) {
        return sizeof...(Args) - 1;
    } else {
        return struct_size<T>(Args..., __Any{1});
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
    constexpr static size_t value = struct_size<T, void, Ts..., __Any>::value;
};

template <class T>
struct struct_member_count : std::integral_constant<std::size_t, struct_size<T>::value> {};

#endif

template <typename T, std::size_t N>
constexpr std::size_t try_initialize_with_n() {
    return []<std::size_t... Is>(std::index_sequence<Is...>) { return requires { T{__Any(Is)...}; }; }(std::make_index_sequence<N>{});
}

template <typename T, std::size_t N = 0>
constexpr auto field_count_impl() {
    if constexpr (try_initialize_with_n<T, N>() && !try_initialize_with_n<T, N + 1>()) {
        return N;
    } else {
        return field_count_impl<T, N + 1>();
    }
}

struct UniversalType {
    template <typename T>
    operator T();
};

template <typename T, typename... Args>
consteval auto memberCount() {
    static_assert(std::is_aggregate_v<std::remove_cvref_t<T>>);

    if constexpr (requires { T{{Args{}}..., {UniversalType{}}}; } == false) {
        return sizeof...(Args);
    } else {
        return memberCount<T, Args..., UniversalType>();
    }
}

// template <typename T>
//     requires std::is_aggregate_v<T>
// static constexpr auto field_count = field_count_impl<T>();

template <typename T>
    requires std::is_aggregate_v<T>
static constexpr auto field_count = memberCount<T>();

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4101)
#endif

#define STRUCT_FIELD_TYPE_DEF(N)                                                                               \
    template <typename T, std::size_t I>                                                                       \
    constexpr auto __struct_field_type_impl(T my_struct, std::integral_constant<std::size_t, N>) {             \
        auto [NEKO_PP_PARAMS(x, N)] = my_struct;                                                               \
        return std::type_identity<std::tuple_element_t<I, std::tuple<NEKO_PP_CALL_PARAMS(decltype, x, N)>>>{}; \
    }

NEKO_PP_FOR_EACH(STRUCT_FIELD_TYPE_DEF, 63)

template <typename T, std::size_t I>
constexpr auto field_type_impl(T object) {
    constexpr auto N = field_count<T>;
    return __struct_field_type_impl<T, I>(object, std::integral_constant<std::size_t, N>{});
}

#define STRUCT_FIELD_ACCESS_DEF(N)                                                                          \
    template <std::size_t I>                                                                                \
    constexpr auto &&__struct_field_access_impl(auto &&my_struct, std::integral_constant<std::size_t, N>) { \
        auto &&[NEKO_PP_PARAMS(x, N)] = std::forward<decltype(my_struct)>(my_struct);                       \
        return std::get<I>(std::forward_as_tuple(NEKO_PP_PARAMS(x, N)));                                    \
    }

NEKO_PP_FOR_EACH(STRUCT_FIELD_ACCESS_DEF, 63)

template <std::size_t I>
constexpr auto &&field_access(auto &&object) {
    using T = std::remove_cvref_t<decltype(object)>;
    constexpr auto N = field_count<T>;
    return __struct_field_access_impl<I>(object, std::integral_constant<std::size_t, N>{});
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

template <typename T, std::size_t I>
constexpr auto field_name_impl() noexcept {
    constexpr auto name = main_name_of_pointer<wrapper{&field_access<I>(storage<T>)}>();
    return cstring<name.size()>{name};
}

template <typename T, std::size_t I>
    requires std::is_aggregate_v<T>
using field_type = typename decltype(field_type_impl<T, I>(std::declval<T>()))::type;

template <typename T, std::size_t I>
    requires std::is_aggregate_v<T>
static constexpr auto field_name = field_name_impl<T, I>();

// 主模板声明
template <typename T>
struct function_traits;

// 普通函数特化
template <typename R, typename... Args>
struct function_traits<R(Args...)> {
    using return_type = R;
    using args_type = std::tuple<Args...>;
    static constexpr size_t arity = sizeof...(Args);
    using std_function = std::function<R(Args...)>;
};

// 函数指针特化
template <typename R, typename... Args>
struct function_traits<R (*)(Args...)> : function_traits<R(Args...)> {};

// 成员函数指针特化
template <typename T, typename R, typename... Args>
struct function_traits<R (T::*)(Args...)> {
    using return_type = R;
    using args_type = std::tuple<Args...>;
    using class_type = T;
    static constexpr size_t arity = sizeof...(Args);
    using std_function = std::function<R (T::*)(Args...)>;
};

// const成员函数特化
template <typename T, typename R, typename... Args>
struct function_traits<R (T::*)(Args...) const> : function_traits<R (T::*)(Args...)> {};

// 仿函数lambda
template <typename Functor>
struct function_traits : function_traits<decltype(&Functor::operator())> {};

// noexcept限定符
template <typename R, typename... Args>
struct function_traits<R(Args...) noexcept> : function_traits<R(Args...)> {};

// std::function
template <typename R, typename... Args>
struct function_traits<std::function<R(Args...)>> : function_traits<R(Args...)> {};

template <typename F>
typename function_traits<F>::std_function to_function(F &lambda) {
    return typename function_traits<F>::std_function(lambda);
}

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

}  // namespace reflection

}  // namespace Neko
