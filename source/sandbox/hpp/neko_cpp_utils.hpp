// Copyright(c) 2022-2023, KaoruXun All rights reserved.

#ifndef NEKO_CPP_TUTIL_HPP
#define NEKO_CPP_TUTIL_HPP

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <codecvt>
#include <cstdint>
#include <functional>
#include <locale>
#include <map>
#include <span>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "engine/neko.hpp"

#pragma region Template

namespace neko::cpp {
template <typename T>
constexpr bool always_false = false;

template <typename T, T V>
struct IValue {
    static constexpr T value = V;
};
template <typename T>
struct IsIValue;
template <typename T>
constexpr bool IsIValue_v = IsIValue<T>::value;
template <auto V>
using IValue_of = IValue<decltype(V), V>;

template <typename...>
struct typename_template_type;

template <typename T>
struct is_typename_template_type;
template <typename T>
static constexpr bool is_typename_template_type_v = is_typename_template_type<T>::value;

// use IValue to replace integral value in template arguments
// we provide some partial template specializations (see details/ToTTType.inl for more details)
// [example]
// template<typename T, std::size_t N>
// struct Array;
// to_typename_template_type_t<Array<T, N>> == typename_template_type<T, IValue<std::size_t, N>>
template <typename T>
struct to_typename_template_type : std::type_identity<T> {};
template <typename T>
using to_typename_template_type_t = typename to_typename_template_type<T>::type;

// type object
// type value
template <typename T>
struct member_pointer_traits;
template <typename T>
using member_pointer_traits_object = typename member_pointer_traits<T>::object;
template <typename T>
using member_pointer_traits_value = typename member_pointer_traits<T>::value;

template <template <typename...> typename T, typename... Ts>
struct is_instantiable;
template <template <typename...> typename T, typename... Ts>
constexpr bool is_instantiable_v = is_instantiable<T, Ts...>::value;

template <template <typename...> class TA, template <typename...> class TB>
struct is_same_typename_template;
template <template <typename...> class TA, template <typename...> class TB>
constexpr bool is_same_typename_template_v = is_same_typename_template<TA, TB>::value;

template <typename Instance, template <typename...> class T>
struct is_instance_of;
template <typename Instance, template <typename...> class T>
constexpr bool is_instance_of_v = is_instance_of<Instance, T>::value;

template <typename T, typename... Args>
struct is_list_initializable;
template <typename T, typename... Args>
static constexpr bool is_list_initializable_v = is_list_initializable<T, Args...>::value;

template <typename T>
struct is_defined;
template <typename T>
static constexpr bool is_defined_v = is_defined<T>::value;

template <typename T>
struct has_virtual_base;
template <typename T>
constexpr bool has_virtual_base_v = has_virtual_base<T>::value;

template <typename Base, typename Derived>
struct is_virtual_base_of;
template <typename Base, typename Derived>
constexpr bool is_virtual_base_of_v = is_virtual_base_of<Base, Derived>::value;

template <size_t N>
constexpr std::size_t lengthof(const char (&str)[N]) noexcept;

constexpr std::size_t string_hash_seed(std::size_t seed, const char *str, std::size_t N) noexcept;
constexpr std::size_t string_hash_seed(std::size_t seed, std::string_view str) noexcept { return string_hash_seed(seed, str.data(), str.size()); }
template <std::size_t N>
constexpr std::size_t string_hash_seed(std::size_t seed, const char (&str)[N]) noexcept {
    return string_hash_seed(seed, str, N - 1);
}
constexpr std::size_t string_hash_seed(std::size_t seed, const char *str) noexcept;

constexpr std::size_t string_hash(const char *str, std::size_t N) noexcept;
constexpr std::size_t string_hash(std::string_view str) noexcept { return string_hash(str.data(), str.size()); }
template <std::size_t N>
constexpr std::size_t string_hash(const char (&str)[N]) noexcept {
    return string_hash(str, N - 1);
}
constexpr std::size_t string_hash(const char *str) noexcept;

template <typename T>
struct is_function_pointer;
template <typename T>
constexpr bool is_function_pointer_v = is_function_pointer<T>::value;

template <template <class...> class Op, class... Args>
struct is_valid;
template <template <class...> class Op, class... Args>
constexpr bool is_valid_v = is_valid<Op, Args...>::value;

template <typename V1, typename Obj1, typename V2, typename Obj2>
constexpr bool member_pointer_equal(V1 Obj1::*p1, V2 Obj2::*p2) noexcept;

template <typename Y>
struct is_same_with {
    template <typename X>
    struct Ttype : std::is_same<X, Y> {};
};

enum class ReferenceMode { None, Left, Right };

enum class CVRefMode : std::uint8_t {
    None = 0b0000,
    Left = 0b0001,
    Right = 0b0010,
    Const = 0b0100,
    ConstLeft = 0b0101,
    ConstRight = 0b0110,
    Volatile = 0b1000,
    VolatileLeft = 0b1001,
    VolatileRight = 0b1010,
    CV = 0b1100,
    CVLeft = 0b1101,
    CVRight = 0b1110,
};

constexpr bool CVRefMode_IsLeft(CVRefMode mode) noexcept { return static_cast<std::uint8_t>(mode) & 0b0001; }
constexpr bool CVRefMode_IsRight(CVRefMode mode) noexcept { return static_cast<std::uint8_t>(mode) & 0b0010; }
constexpr bool CVRefMode_IsConst(CVRefMode mode) noexcept { return static_cast<std::uint8_t>(mode) & 0b0100; }
constexpr bool CVRefMode_IsVolatile(CVRefMode mode) noexcept { return static_cast<std::uint8_t>(mode) & 0b1000; }

template <typename T, std::size_t N>
class TempArray {
public:
    template <typename... Elems>
    constexpr TempArray(Elems &&...elems) : data{static_cast<T>(elems)...} {}

    constexpr operator std::add_lvalue_reference_t<T[N]>() & { return data; }
    constexpr operator std::add_lvalue_reference_t<const T[N]>() const & { return data; }
    constexpr operator std::add_rvalue_reference_t<T[N]>() && { return std::move(data); }
    constexpr operator std::add_rvalue_reference_t<const T[N]>() const && { return std::move(data); }

    constexpr operator std::span<T>() { return data; }
    constexpr operator std::span<const T>() const { return data; }
    constexpr operator std::span<T, N>() { return data; }
    constexpr operator std::span<const T, N>() const { return data; }

private:
    T data[N];
};

template <typename T, typename... Ts>
TempArray(T, Ts...) -> TempArray<T, sizeof...(Ts) + 1>;
}  // namespace neko::cpp

// To Typename Template Type

// 1
template <template <auto> class T, auto Int>
struct ::neko::cpp::to_typename_template_type<T<Int>> : std::type_identity<typename_template_type<IValue_of<Int>>> {};

// 1...
// template<template<auto...>class T, auto... Ints>
// struct ::neko::cpp::to_typename_template_type<T<Ints...>>
//  : std::type_identity<typename_template_type<IValue_of<Ints>...>> {};

// 1 0
template <template <auto, typename> class T, auto Int, typename U>
struct ::neko::cpp::to_typename_template_type<T<Int, U>> : std::type_identity<typename_template_type<IValue_of<Int>, U>> {};

// 1 0...
// template<template<auto, typename...>class T, auto Int, typename... Us>
// struct ::neko::cpp::to_typename_template_type<T<Int, Us...>>
//  : std::type_identity<typename_template_type<IValue_of<Int>, Us...>> {};

// 0 1
template <template <typename, auto> class T, typename U, auto Int>
struct ::neko::cpp::to_typename_template_type<T<U, Int>> : std::type_identity<typename_template_type<U, IValue_of<Int>>> {};

// 0 1...
// template<template<typename, auto...>class T, typename U, auto... Ints>
// struct ::neko::cpp::to_typename_template_type<T<U, Ints...>>
//  : std::type_identity<typename_template_type<U, IValue_of<Ints>...>> {};

// 1 1
template <template <auto, auto> class T, auto Int0, auto Int1>
struct ::neko::cpp::to_typename_template_type<T<Int0, Int1>> : std::type_identity<typename_template_type<IValue_of<Int0>, IValue_of<Int1>>> {};

// 1 1...
// template<template<auto, auto, typename...>class T, auto Int, auto... Ints>
// struct ::neko::cpp::to_typename_template_type<T<Int, Ints...>>
//  : std::type_identity<typename_template_type<IValue_of<Int>, IValue_of<Ints>...>> {};

// 1 0 0
template <template <auto, typename, typename> class T, auto Int, typename U0, typename U1>
struct ::neko::cpp::to_typename_template_type<T<Int, U0, U1>> : std::type_identity<typename_template_type<IValue_of<Int>, U0, U1>> {};

// 1 0 0 0...
template <template <auto, typename, typename, typename...> class T, auto Int, typename U0, typename U1, typename... Us>
struct ::neko::cpp::to_typename_template_type<T<Int, U0, U1, Us...>> : std::type_identity<typename_template_type<IValue_of<Int>, U0, U1, Us...>> {};

// 0 1 0
template <template <typename, auto, typename> class T, typename U0, auto Int, typename U1>
struct ::neko::cpp::to_typename_template_type<T<U0, Int, U1>> : std::type_identity<typename_template_type<U0, IValue_of<Int>, U1>> {};

// 0 1 0 0...
template <template <typename, auto, typename, typename...> class T, typename U0, auto Int, typename U1, typename... Us>
struct ::neko::cpp::to_typename_template_type<T<U0, Int, U1, Us...>> : std::type_identity<typename_template_type<U0, IValue_of<Int>, U1, Us...>> {};

// 0 0 1
template <template <typename, typename, auto> class T, typename U0, typename U1, auto Int>
struct ::neko::cpp::to_typename_template_type<T<U0, U1, Int>> : std::type_identity<typename_template_type<U0, U1, IValue_of<Int>>> {};

// 0 0 1 1...
template <template <typename, typename, auto, auto...> class T, typename U0, typename U1, auto Int, auto... Ints>
struct ::neko::cpp::to_typename_template_type<T<U0, U1, Int, Ints...>> : std::type_identity<typename_template_type<U0, U1, IValue_of<Int>, IValue_of<Ints>...>> {};

// 1 1 0
template <template <auto, auto, typename> class T, auto Int0, auto Int1, typename U>
struct ::neko::cpp::to_typename_template_type<T<Int0, Int1, U>> : std::type_identity<typename_template_type<IValue_of<Int0>, IValue_of<Int1>, U>> {};

// 1 1 0 0...
template <template <auto, auto, typename, typename...> class T, auto Int0, auto Int1, typename U, typename... Us>
struct ::neko::cpp::to_typename_template_type<T<Int0, Int1, U, Us...>> : std::type_identity<typename_template_type<IValue_of<Int0>, IValue_of<Int1>, U, Us...>> {};

// 1 0 1
template <template <auto, typename, auto> class T, auto Int0, typename U, auto Int1>
struct ::neko::cpp::to_typename_template_type<T<Int0, U, Int1>> : std::type_identity<typename_template_type<IValue_of<Int0>, U, IValue_of<Int1>>> {};

// 1 0 1 1...
template <template <auto, typename, auto, auto...> class T, auto Int0, typename U, auto Int1, auto... Ints>
struct ::neko::cpp::to_typename_template_type<T<Int0, U, Int1, Ints...>> : std::type_identity<typename_template_type<IValue_of<Int0>, U, IValue_of<Int1>, IValue_of<Ints>...>> {};

// 0 1 1
template <template <typename, auto, auto> class T, typename U, auto Int0, auto Int1>
struct ::neko::cpp::to_typename_template_type<T<U, Int0, Int1>> : std::type_identity<typename_template_type<U, IValue_of<Int0>, IValue_of<Int1>>> {};

// 0 1 1 1...
template <template <typename, auto, auto, auto...> class T, typename U, auto Int0, auto Int1, auto... Ints>
struct ::neko::cpp::to_typename_template_type<T<U, Int0, Int1, Ints...>> : std::type_identity<typename_template_type<U, IValue_of<Int0>, IValue_of<Int1>, IValue_of<Ints>...>> {};

// 1 1 1
template <template <auto, auto, auto> class T, auto Int0, auto Int1, auto Int2>
struct ::neko::cpp::to_typename_template_type<T<Int0, Int1, Int2>> : std::type_identity<typename_template_type<IValue_of<Int0>, IValue_of<Int1>, IValue_of<Int2>>> {};

// 1 1 1 1...
template <template <auto, auto, auto, auto...> class T, auto Int0, auto Int1, auto Int2, auto... Ints>
struct ::neko::cpp::to_typename_template_type<T<Int0, Int1, Int2, Ints...>> : std::type_identity<typename_template_type<IValue_of<Int0>, IValue_of<Int1>, IValue_of<Int2>, IValue_of<Ints>...>> {};

namespace neko::cpp::details {
template <typename Void, template <typename...> typename T, typename... Ts>
struct is_instantiable : std::false_type {};
template <template <typename...> typename T, typename... Ts>
struct is_instantiable<std::void_t<T<Ts...>>, T, Ts...> : std::true_type {};

template <typename Void, typename T, typename... Args>
struct is_list_initializable : std::false_type {};

template <typename T, typename... Args>
struct is_list_initializable<std::void_t<decltype(T{std::declval<Args>()...})>, T, Args...> : std::true_type {};

template <typename Void, typename T>
struct is_defined_helper : std::false_type {};

template <typename T>
struct is_defined_helper<std::void_t<decltype(sizeof(T))>, T> : std::true_type {};

template <std::size_t size>
struct fnv1a_traits;

template <>
struct fnv1a_traits<4> {
    using type = std::uint32_t;
    static constexpr std::uint32_t offset = 2166136261;
    static constexpr std::uint32_t prime = 16777619;
};

template <>
struct fnv1a_traits<8> {
    using type = std::uint64_t;
    static constexpr std::uint64_t offset = 14695981039346656037ull;
    static constexpr std::uint64_t prime = 1099511628211ull;
};

template <typename Void, typename T>
struct is_function_pointer : std::false_type {};

template <typename T>
struct is_function_pointer<std::enable_if_t<std::is_pointer_v<T> && std::is_function_v<std::remove_pointer_t<T>>>, T> : std::true_type {};

struct has_virtual_base_void {};
template <typename Void, typename Obj>
struct has_virtual_base_helper : std::true_type {};
template <typename Obj>
struct has_virtual_base_helper<std::void_t<decltype(reinterpret_cast<has_virtual_base_void has_virtual_base_void::*>(std::declval<has_virtual_base_void Obj::*>()))>, Obj> : std::false_type {};

template <typename Void, typename Base, typename Derived>
struct is_virtual_base_of_helper : std::is_base_of<Base, Derived> {};
template <typename Base, typename Derived>
struct is_virtual_base_of_helper<std::void_t<decltype(static_cast<Derived *>(std::declval<Base *>()))>, Base, Derived> : std::false_type {};

template <class Void, template <class...> class Op, class... Args>
struct is_valid : std::false_type {};
template <template <class...> class Op, class... Args>
struct is_valid<std::void_t<Op<Args...>>, Op, Args...> : std::true_type {};
}  // namespace neko::cpp::details

template <template <typename...> typename T, typename... Ts>
struct ::neko::cpp::is_instantiable : details::is_instantiable<void, T, Ts...> {};

template <typename Instance, template <typename...> class T>
struct ::neko::cpp::is_instance_of : std::false_type {};

template <typename... Args, template <typename...> class T>
struct ::neko::cpp::is_instance_of<T<Args...>, T> : std::true_type {};

template <typename T, typename... Args>
struct ::neko::cpp::is_list_initializable : details::is_list_initializable<void, T, Args...> {};

template <template <typename...> class TA, template <typename...> class TB>
struct ::neko::cpp::is_same_typename_template : std::false_type {};

template <template <typename...> class T>
struct ::neko::cpp::is_same_typename_template<T, T> : std::true_type {};

template <typename T>
struct ::neko::cpp::is_defined : details::is_defined_helper<void, T> {};

template <typename T, typename U>
struct ::neko::cpp::member_pointer_traits<T U::*> {
    using object = U;
    using value = T;
};

template <typename T>
struct ::neko::cpp::is_typename_template_type : std::false_type {};

template <template <typename...> class T, typename... Ts>
struct ::neko::cpp::is_typename_template_type<T<Ts...>> : std::true_type {};

template <typename T>
struct ::neko::cpp::IsIValue : std::false_type {};
template <typename T, T v>
struct ::neko::cpp::IsIValue<::neko::cpp::IValue<T, v>> : std::true_type {};

template <size_t N>
constexpr std::size_t neko::cpp::lengthof(const char (&str)[N]) noexcept {
    static_assert(N > 0);
    neko_assert(str[N - 1] == 0);  // c-style string
    return N - 1;
}

constexpr std::size_t neko::cpp::string_hash_seed(std::size_t seed, const char *str, std::size_t N) noexcept {
    using Traits = details::fnv1a_traits<sizeof(std::size_t)>;
    std::size_t value = seed;

    for (std::size_t i = 0; i < N; i++) value = (value ^ static_cast<Traits::type>(str[i])) * Traits::prime;

    return value;
}

constexpr std::size_t neko::cpp::string_hash_seed(std::size_t seed, const char *curr) noexcept {
    using Traits = details::fnv1a_traits<sizeof(std::size_t)>;
    std::size_t value = seed;

    while (*curr) {
        value = (value ^ static_cast<Traits::type>(*(curr++))) * Traits::prime;
    }

    return value;
}

constexpr std::size_t neko::cpp::string_hash(const char *str, std::size_t N) noexcept {
    using Traits = details::fnv1a_traits<sizeof(std::size_t)>;
    return string_hash_seed(Traits::offset, str, N);
}

constexpr std::size_t neko::cpp::string_hash(const char *str) noexcept {
    using Traits = details::fnv1a_traits<sizeof(std::size_t)>;
    return string_hash_seed(Traits::offset, str);
}

template <typename T>
struct ::neko::cpp::is_function_pointer : ::neko::cpp::details::is_function_pointer<void, T> {};

template <typename T>
struct ::neko::cpp::has_virtual_base : ::neko::cpp::details::has_virtual_base_helper<void, T> {};

template <typename Base, typename Derived>
struct ::neko::cpp::is_virtual_base_of : ::neko::cpp::details::is_virtual_base_of_helper<void, Base, Derived> {};

template <template <class...> class Op, class... Args>
struct ::neko::cpp::is_valid : ::neko::cpp::details::is_valid<void, Op, Args...> {};

template <typename V1, typename Obj1, typename V2, typename Obj2>
constexpr bool ::neko::cpp::member_pointer_equal(V1 Obj1::*p1, V2 Obj2::*p2) noexcept {
    if constexpr (std::is_same_v<Obj1, Obj2> && std::is_same_v<V1, V2>)
        return p1 == p2;
    else
        return false;
}

#pragma endregion Template

#pragma region TypeList

namespace neko::cpp {
template <typename... Ts>
struct TypeList {};

template <template <typename...> class OtherListTemplate, typename OtherList>
struct ToTypeList;
template <template <typename...> class OtherListTemplate, typename OtherList>
using ToTypeList_t = typename ToTypeList<OtherListTemplate, OtherList>::type;

template <typename List, template <typename...> class OtherListTemplate>
struct ToOtherList;
template <typename List, template <typename...> class OtherListTemplate>
using ToOtherList_t = typename ToOtherList<List, OtherListTemplate>::type;

template <typename List>
struct IsTypeList;
template <typename List>
constexpr bool IsTypeList_v = IsTypeList<List>::value;

template <typename List>
struct Length;
template <typename List>
constexpr std::size_t Length_v = Length<List>::value;

template <typename List>
struct IsEmpty;
template <typename List>
constexpr bool IsEmpty_v = IsEmpty<List>::value;

template <typename List>
struct Front;
template <typename List>
using Front_t = typename Front<List>::type;

template <typename List, std::size_t N>
struct At;
template <typename List, std::size_t N>
using At_t = typename At<List, N>::type;

template <typename List, std::size_t... Indices>
struct Select;
template <typename List, std::size_t... Indices>
using Select_t = typename Select<List, Indices...>::type;

constexpr std::size_t Find_fail = static_cast<std::size_t>(-1);

template <typename List, typename T>
struct Find;
template <typename List, typename T>
constexpr std::size_t Find_v = Find<List, T>::value;

template <typename List, template <typename> class Op>
struct FindIf;
template <typename List, template <typename> class Op>
constexpr std::size_t FindIf_v = FindIf<List, Op>::value;

template <typename List, typename T>
struct Contain;
template <typename List, typename T>
constexpr bool Contain_v = Contain<List, T>::value;

template <typename List, typename... Ts>
struct ContainTs;
template <typename List, typename... Ts>
constexpr bool ContainTs_v = ContainTs<List, Ts...>::value;

template <typename List0, typename List1>
struct ContainList;
template <typename List0, typename List1>
constexpr bool ContainList_v = ContainList<List0, List1>::value;

template <typename List, template <typename...> class T>
struct CanInstantiate;
template <typename List, template <typename...> class T>
constexpr bool CanInstantiate_v = CanInstantiate<List, T>::value;

template <typename List, template <typename...> class T>
struct Instantiate;
template <typename List, template <typename...> class T>
using Instantiate_t = typename Instantiate<List, T>::type;

template <typename List, template <typename...> class T>
struct ExistInstance;
template <typename List, template <typename...> class T>
constexpr bool ExistInstance_v = ExistInstance<List, T>::value;

// get first template instantiable type
template <typename List, template <typename...> class T>
struct SearchInstance;
template <typename List, template <typename...> class T>
using SearchInstance_t = typename SearchInstance<List, T>::type;

template <typename List, typename T>
struct PushFront;
template <typename List, typename T>
using PushFront_t = typename PushFront<List, T>::type;

template <typename List, typename T>
struct PushBack;
template <typename List, typename T>
using PushBack_t = typename PushBack<List, T>::type;

template <typename List>
struct PopFront;
template <typename List>
using PopFront_t = typename PopFront<List>::type;

template <typename List>
struct Rotate;
template <typename List>
using Rotate_t = typename Rotate<List>::type;

template <typename List, template <typename I, typename X> class Op, typename I>
struct Accumulate;
template <typename List, template <typename I, typename X> class Op, typename I>
using Accumulate_t = typename Accumulate<List, Op, I>::type;

template <typename List, template <typename> class Op>
struct Filter;
template <typename List, template <typename> class Op>
using Filter_t = typename Filter<List, Op>::type;

template <typename List>
struct Reverse;
template <typename List>
using Reverse_t = typename Reverse<List>::type;

template <typename List0, typename List1>
struct Concat;
template <typename List0, typename List1>
using Concat_t = typename Concat<List0, List1>::type;

template <typename List, template <typename> class Op>
struct Transform;
template <typename List, template <typename> class Op>
using Transform_t = typename Transform<List, Op>::type;

template <typename List, template <typename X, typename Y> typename Less>
struct QuickSort;
template <typename List, template <typename X, typename Y> typename Less>
using QuickSort_t = typename QuickSort<List, Less>::type;

template <typename List>
struct IsUnique;
template <typename List>
constexpr bool IsUnique_v = IsUnique<List>::value;
}  // namespace neko::cpp

namespace neko::cpp::details {
template <typename List, typename T, std::size_t N = 0, bool found = false>
struct Find;

template <typename List, template <typename> class Op, std::size_t N = 0, bool found = false>
struct FindIf;

template <typename List, template <typename I, typename X> class Op, typename I, bool = IsEmpty_v<List>>
struct Accumulate;

template <typename List, template <typename...> class T, bool found = false, bool = IsEmpty<List>::value>
struct ExistInstance;

template <typename List, typename LastT, template <typename...> class T, bool found = false, bool = IsEmpty<List>::value>
struct SearchInstance;

template <typename List, bool haveSame = false>
struct IsUnique;
}  // namespace neko::cpp::details

namespace neko::cpp {
template <template <typename...> class OtherListTemplate, typename... Ts>
struct ToTypeList<OtherListTemplate, OtherListTemplate<Ts...>> : std::type_identity<TypeList<Ts...>> {};

// =================================================

template <typename... Ts, template <typename...> class OtherListTemplate>
struct ToOtherList<TypeList<Ts...>, OtherListTemplate> : std::type_identity<OtherListTemplate<Ts...>> {};

// =================================================

template <typename List>
struct IsTypeList : is_instance_of<List, TypeList> {};

// =================================================

template <typename... Ts>
struct Length<TypeList<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)> {};

// =================================================

template <typename List>
struct IsEmpty : std::integral_constant<bool, Length_v<List> == 0> {};

// =================================================

template <typename Head, typename... Tail>
struct Front<TypeList<Head, Tail...>> : std::type_identity<Head> {};

// =================================================

template <typename List>
struct At<List, 0> : std::type_identity<Front_t<List>> {};

template <typename List, std::size_t N>
struct At : At<PopFront_t<List>, N - 1> {};

// =================================================

template <typename List, std::size_t... Indices>
struct Select : std::type_identity<TypeList<At_t<List, Indices>...>> {};

// =================================================

template <typename List, typename T>
struct Find : details::Find<List, T> {};

template <typename List, template <typename> class Op>
struct FindIf : details::FindIf<List, Op> {};

// =================================================

template <typename List, typename T>
struct Contain : std::integral_constant<bool, Find_v<List, T> != Find_fail> {};

// =================================================

template <typename List, typename... Ts>
struct ContainTs : std::integral_constant<bool, (Contain_v<List, Ts> && ...)> {};

// =================================================

template <typename List, typename... Ts>
struct ContainList<List, TypeList<Ts...>> : std::integral_constant<bool, (Contain_v<List, Ts> && ...)> {};

// =================================================

template <template <typename...> class T, typename... Args>
struct CanInstantiate<TypeList<Args...>, T> : is_instantiable<T, Args...> {};

// =================================================

template <template <typename...> class T, typename... Args>
struct Instantiate<TypeList<Args...>, T> : std::type_identity<T<Args...>> {};

// =================================================

template <typename List, template <typename...> class T>
struct ExistInstance : details::ExistInstance<List, T> {};

// =================================================

template <typename List, template <typename...> class T>
struct SearchInstance : details::SearchInstance<List, void, T> {};

// =================================================

template <typename T, typename... Ts>
struct PushFront<TypeList<Ts...>, T> : std::type_identity<TypeList<T, Ts...>> {};

// =================================================

template <typename T, typename... Ts>
struct PushBack<TypeList<Ts...>, T> : std::type_identity<TypeList<Ts..., T>> {};

// =================================================

template <typename Head, typename... Tail>
struct PopFront<TypeList<Head, Tail...>> : std::type_identity<TypeList<Tail...>> {};

// =================================================

template <typename Head, typename... Tail>
struct Rotate<TypeList<Head, Tail...>> : std::type_identity<TypeList<Tail..., Head>> {};

// =================================================

template <typename List, template <typename I, typename X> class Op, typename I>
struct Accumulate : details::Accumulate<List, Op, I> {};

// =================================================

template <typename List, template <typename> class Test>
struct Filter {
private:
    template <typename I, typename X>
    struct PushFrontIf : std::conditional<Test<X>::value, PushFront_t<I, X>, I> {};

public:
    using type = Accumulate_t<List, PushFrontIf, TypeList<>>;
};

// =================================================

template <typename List>
struct Reverse : Accumulate<List, PushFront, TypeList<>> {};

// =================================================

template <typename List, typename T>
struct PushBack : Reverse<PushFront_t<Reverse_t<List>, T>> {};

// =================================================

template <typename List0, typename List1>
struct Concat : Accumulate<List1, PushBack, List0> {};

// =================================================

template <template <typename T> class Op, typename... Ts>
struct Transform<TypeList<Ts...>, Op> : std::type_identity<TypeList<typename Op<Ts>::type...>> {};

// =================================================

template <template <typename X, typename Y> typename Less>
struct QuickSort<TypeList<>, Less> : std::type_identity<TypeList<>> {};

template <template <typename X, typename Y> typename Less, typename T>
struct QuickSort<TypeList<T>, Less> : std::type_identity<TypeList<T>> {};

template <template <typename X, typename Y> typename Less, typename Head, typename... Tail>
struct QuickSort<TypeList<Head, Tail...>, Less> {
private:
    template <typename X>
    using LessThanHead = Less<X, Head>;
    template <typename X>
    using GEThanHead = std::integral_constant<bool, !Less<X, Head>::value>;
    using LessList = Filter_t<TypeList<Tail...>, LessThanHead>;
    using GEList = Filter_t<TypeList<Tail...>, GEThanHead>;

public:
    using type = Concat_t<typename QuickSort<LessList, Less>::type, PushFront_t<typename QuickSort<GEList, Less>::type, Head>>;
};

// =================================================

template <typename List>
struct IsUnique : details::IsUnique<List> {};
}  // namespace neko::cpp

namespace neko::cpp::details {
template <typename T, std::size_t N, typename... Ts>
struct Find<TypeList<Ts...>, T, N, true> : std::integral_constant<std::size_t, N - 1> {};
template <typename T, std::size_t N>
struct Find<TypeList<>, T, N, false> : std::integral_constant<std::size_t, Find_fail> {};
template <typename T, typename Head, std::size_t N, typename... Tail>
struct Find<TypeList<Head, Tail...>, T, N, false> : Find<TypeList<Tail...>, T, N + 1, std::is_same_v<T, Head>> {};

// =================================================

template <template <typename> class Op, std::size_t N, typename... Ts>
struct FindIf<TypeList<Ts...>, Op, N, true> : std::integral_constant<std::size_t, N - 1> {};
template <template <typename> class Op, std::size_t N>
struct FindIf<TypeList<>, Op, N, false> : std::integral_constant<std::size_t, Find_fail> {};
template <template <typename> class Op, typename Head, std::size_t N, typename... Tail>
struct FindIf<TypeList<Head, Tail...>, Op, N, false> : FindIf<TypeList<Tail...>, Op, N + 1, Op<Head>::value> {};

// =================================================

template <typename List, template <typename I, typename X> class Op, typename I>
struct Accumulate<List, Op, I, false> : Accumulate<PopFront_t<List>, Op, typename Op<I, Front_t<List>>::type> {};

template <typename List, template <typename X, typename Y> class Op, typename I>
struct Accumulate<List, Op, I, true> {
    using type = I;
};

// =================================================

template <typename List, template <typename...> class T>
struct ExistInstance<List, T, false, true> : std::false_type {};

template <typename List, template <typename...> class T, bool isEmpty>
struct ExistInstance<List, T, true, isEmpty> : std::true_type {};

template <typename List, template <typename...> class T>
struct ExistInstance<List, T, false, false> : ExistInstance<PopFront_t<List>, T, is_instance_of_v<Front_t<List>, T>> {};

// =================================================

template <typename List, typename LastT, template <typename...> class T>
struct SearchInstance<List, LastT, T, false, true> {};  // no 'type'

template <typename List, typename LastT, template <typename...> class T, bool isEmpty>
struct SearchInstance<List, LastT, T, true, isEmpty> {
    using type = LastT;
};

template <typename List, typename LastT, template <typename...> class T>
struct SearchInstance<List, LastT, T, false, false> : SearchInstance<PopFront_t<List>, Front_t<List>, T, is_instance_of_v<Front_t<List>, T>> {};

// =================================================

template <typename List>
struct IsUnique<List, true> : std::false_type {};
template <>
struct IsUnique<TypeList<>, false> : std::true_type {};
template <typename Head, typename... Tail>
struct IsUnique<TypeList<Head, Tail...>, false> : IsUnique<TypeList<Tail...>, Contain_v<TypeList<Tail...>, Head>> {};
}  // namespace neko::cpp::details

#pragma endregion TypeList

// TemplateList https://zhuanlan.zhihu.com/p/106982496
#pragma region TemplateList

namespace neko::cpp {
template <template <typename...> class... Ts>
struct TemplateList {};

template <typename TList>
struct TLength;
template <typename TList>
constexpr std::size_t TLength_v = TLength<TList>::value;

template <typename TList>
struct TIsEmpty;
template <typename TList>
constexpr bool TIsEmpty_v = TIsEmpty<TList>::value;

// TFront will introduce new template
// template<typename TList> struct TFront;

template <typename TList, template <typename...> class T>
struct TPushFront;
template <typename TList, template <typename...> class T>
using TPushFront_t = typename TPushFront<TList, T>::type;

template <typename TList, template <typename...> class T>
struct TPushBack;
template <typename TList, template <typename...> class T>
using TPushBack_t = typename TPushBack<TList, T>::type;

template <typename TList>
struct TPopFront;
template <typename TList>
using TPopFront_t = typename TPopFront<TList>::type;

// TAt will introduce new template
// template<typename TList, std::size_t N> struct TAt;

template <typename TList, template <typename...> class T>
struct TContain;
template <typename TList, template <typename...> class T>
static constexpr bool TContain_v = TContain<TList, T>::value;

template <typename TList, template <typename I, template <typename...> class X> class Op, typename I>
struct TAccumulate;
template <typename TList, template <typename I, template <typename...> class X> class Op, typename I>
using TAccumulate_t = typename TAccumulate<TList, Op, I>::type;

template <typename TList>
struct TReverse;
template <typename TList>
using TReverse_t = typename TReverse<TList>::type;

template <typename TList0, typename TList1>
struct TConcat;
template <typename TList0, typename TList1>
using TConcat_t = typename TConcat<TList0, TList1>::type;

template <typename TList, template <template <typename...> class T> class Op>
struct TTransform;

template <typename TList, template <template <typename...> class T> class Op>
using TTransform_t = typename TTransform<TList, Op>::type;

// TSelect will introduce new template
// template<typename TList, std::size_t... Indices> struct TSelect;

template <typename TList, typename Instance>
struct TExistGenericity;
template <typename TList, typename Instance>
constexpr bool TExistGenericity_v = TExistGenericity<TList, Instance>::value;

template <typename TList, typename InstanceList>
struct TExistGenericities;
template <typename TList, typename InstanceList>
constexpr bool TExistGenericities_v = TExistGenericities<TList, InstanceList>::value;

template <typename TList, typename ArgList>
struct TInstance;
template <typename TList, typename ArgList>
using TInstance_t = typename TInstance<TList, ArgList>::type;

template <typename TList, typename InstanceList>
struct TCanGeneralizeFromList;
template <typename TList, typename InstanceList>
constexpr bool TCanGeneralizeFromList_v = TCanGeneralizeFromList<TList, InstanceList>::value;
}  // namespace neko::cpp

namespace neko::cpp {
template <template <typename...> class... Ts>
struct TLength<TemplateList<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template <typename TList>
struct TIsEmpty : std::bool_constant<TLength_v<TList> == 0> {};

/*
// TFront will introduce new template
template<template<typename...> class Head, template<typename...> class... Tail>
struct TFront<TemplateList<Head, Tail...>> {
    template<typename... Ts>
    using Ttype = Head<Ts...>;
};
*/

template <template <typename...> class T, template <typename...> class... Ts>
struct TPushFront<TemplateList<Ts...>, T> : std::type_identity<TemplateList<T, Ts...>> {};

template <template <typename...> class T, template <typename...> class... Ts>
struct TPushBack<TemplateList<Ts...>, T> : std::type_identity<TemplateList<Ts..., T>> {};

template <template <typename...> class Head, template <typename...> class... Tail>
struct TPopFront<TemplateList<Head, Tail...>> : std::type_identity<TemplateList<Tail...>> {};

/*
// TAt will introduce new template
template<typename TList>
struct TAt<TList, 0> {
    template<typename... Ts>
    using Ttype = typename TFront<TList>::template Ttype<Ts...>;
};

template<typename TList, std::size_t N>
struct TAt : TAt<TPopFront_t<TList>, N - 1> { };
*/

template <template <typename I, template <typename...> class X> class Op, typename I>
struct TAccumulate<TemplateList<>, Op, I> : std::type_identity<I> {};

template <template <typename I, template <typename...> class X> class Op, typename I, template <typename...> class THead, template <typename...> class... TTail>
struct TAccumulate<TemplateList<THead, TTail...>, Op, I> : TAccumulate<TemplateList<TTail...>, Op, typename Op<I, THead>::type> {};

template <typename TList>
struct TReverse : TAccumulate<TList, TPushFront, TemplateList<>> {};

template <typename TList0, typename TList1>
struct TConcat : TAccumulate<TList1, TPushBack, TList0> {};

template <template <template <typename...> class T> class Op, template <typename...> class... Ts>
struct TTransform<TemplateList<Ts...>, Op> : std::type_identity<TemplateList<Op<Ts>::template Ttype...>> {};

// TSelect
// template<typename TList, std::size_t... Indices>
// struct TSelect : std::type_identity<TemplateList<TAt<TList, Indices>::template Ttype...>> {};

template <template <typename...> class... Ts, typename Instance>
struct TExistGenericity<TemplateList<Ts...>, Instance> : std::bool_constant<(is_instance_of_v<Instance, Ts> || ...)> {};

template <typename ArgList, template <typename...> class... Ts>
struct TInstance<TemplateList<Ts...>, ArgList> : std::type_identity<TypeList<Instantiate_t<ArgList, Ts>...>> {};

template <typename TList, typename... Instances>
struct TExistGenericities<TList, TypeList<Instances...>> : std::bool_constant<(TExistGenericity_v<TList, Instances> && ...)> {};

template <typename InstanceList, template <typename...> class... Ts>
struct TCanGeneralizeFromList<TemplateList<Ts...>, InstanceList> : std::bool_constant<(ExistInstance_v<InstanceList, Ts> && ...)> {};

template <template <typename...> class... Ts, template <typename...> class T>
struct TContain<TemplateList<Ts...>, T> : std::bool_constant<(is_same_typename_template_v<Ts, T> || ...)> {};
}  // namespace neko::cpp

#pragma endregion TemplateList

#pragma region TSTR

// marco
// - NEKO_TSTR
// - NEKO_TSTR_UTIL

#ifndef NEKO_TSTR
#define NEKO_TSTR

#include <concepts>
#include <string_view>
#include <utility>

namespace neko::cpp {
template <typename Char, std::size_t N>
struct fixed_cstring {
    using value_type = Char;

    constexpr fixed_cstring(const value_type (&str)[N + 1]) noexcept {
        for (std::size_t i{0}; i < size; ++i) data[i] = str[i];
        data[size] = 0;
    }

    constexpr fixed_cstring(std::basic_string_view<value_type> str) noexcept {
        for (std::size_t i{0}; i < size; ++i) data[i] = str[i];
        data[size] = 0;
    }

    template <std::size_t N_ = N, std::enable_if_t<N_ == 0, int> = 0>
    constexpr fixed_cstring() noexcept : data{0} {}

    template <std::size_t N_ = N, std::enable_if_t<N_ == 1, int> = 0>
    constexpr fixed_cstring(value_type c) noexcept : data{c, 0} {}

    template <typename... Chars>
    constexpr fixed_cstring(std::in_place_t, Chars... chars) noexcept : data{static_cast<value_type>(chars)..., 0} {
        static_assert(sizeof...(chars) == size);
    }

    value_type data[N + 1]{};
    static constexpr std::size_t size = N;
};

template <typename Char, std::size_t N>
fixed_cstring(const Char (&)[N]) -> fixed_cstring<Char, N - 1>;
}  // namespace neko::cpp

// 编译期字符串
//
// 创建一个字符串的方法是这样
// TStr<char, 'h', 'e', 'l', 'l', 'o', 'w'> str;
//
// 这里的模板参数fixed_cstring是指字符类型
namespace neko::cpp {
template <fixed_cstring str>
struct TStr {
    using Char = typename decltype(str)::value_type;

    template <typename T>
    static constexpr bool Is(T = {}) noexcept {
        return std::is_same_v<T, TStr>;
    }
    static constexpr const Char *Data() noexcept { return str.data; }
    static constexpr std::size_t Size() noexcept { return str.size; }
    static constexpr std::basic_string_view<Char> View() noexcept { return str.data; }
    constexpr operator std::basic_string_view<Char>() { return View(); }
};

template <char... chars>
using TStrC_of = TStr<fixed_cstring<char, sizeof...(chars)>{std::in_place, chars...}>;
template <auto c>
using TStr_of_a = TStr<fixed_cstring<decltype(c), 1>(c)>;
}  // namespace neko::cpp

#define TSTR_IMPL(s)                                                                                                 \
    ([] {                                                                                                            \
        constexpr std::basic_string_view str{s};                                                                     \
        return ::neko::cpp::TStr<::neko::cpp::fixed_cstring<typename decltype(str)::value_type, str.size()>{str}>{}; \
    }())

#define TSTR(s) TSTR_IMPL(s)

#endif  // NEKO_TSTR

#ifndef NEKO_TSTR_UTIL
#define NEKO_TSTR_UTIL

namespace neko::cpp {
template <typename T>
concept TStrLike = requires {
    { T::View() } -> std::same_as<std::basic_string_view<typename T::Char>>;
};

template <typename Str>
constexpr auto empty_of(Str = {}) noexcept {
    return TStr<fixed_cstring<typename Str::Char, 0>{}>{};
}

template <typename Str0, typename Str1>
struct concat_helper;
template <typename Str0, typename Str1>
using concat_helper_t = typename concat_helper<Str0, Str1>::type;

template <typename Str0, typename Str1>
struct concat_helper {
private:
    template <std::size_t... LNs, std::size_t... RNs>
    static constexpr auto get(std::index_sequence<LNs...>, std::index_sequence<RNs...>) noexcept {
        return TStrC_of<Str0::View()[LNs]..., Str1::View()[RNs]...>{};
    }

public:
    using type = decltype(get(std::make_index_sequence<Str0::Size()>{}, std::make_index_sequence<Str1::Size()>{}));
};

template <typename Str0, typename Str1>
constexpr auto concat(Str0 = {}, Str1 = {}) noexcept {
    return typename concat_helper<Str0, Str1>::type{};
}

template <typename... Strs>
struct concat_seq_helper;
template <typename... Strs>
using concat_seq_helper_t = typename concat_seq_helper<Strs...>::type;
template <typename Str>
struct concat_seq_helper<Str> {
    using type = Str;
};
template <typename Str, typename... Strs>
struct concat_seq_helper<Str, Strs...> {
    using type = concat_helper_t<Str, concat_seq_helper_t<Strs...>>;
};
template <typename... Strs>
constexpr auto concat_seq(Strs...) noexcept {
    return concat_seq_helper_t<Strs...>{};
}

template <typename Seperator, typename... Strs>
struct concat_seq_seperator_helper;
template <typename Seperator, typename... Strs>
using concat_seq_seperator_helper_t = typename concat_seq_seperator_helper<Seperator, Strs...>::type;
template <typename Seperator>
struct concat_seq_seperator_helper<Seperator> {
    using type = decltype(empty_of<Seperator>());
};
template <typename Seperator, typename Str>
struct concat_seq_seperator_helper<Seperator, Str> {
    using type = Str;
};
template <typename Seperator, typename Str, typename... Strs>
struct concat_seq_seperator_helper<Seperator, Str, Strs...> {
    using type = concat_helper_t<concat_helper_t<Str, Seperator>, concat_seq_seperator_helper_t<Seperator, Strs...>>;
};
template <typename Seperator, typename... Strs>
constexpr auto concat_seq_seperator(Seperator, Strs...) noexcept {
    return concat_seq_seperator_helper_t<Seperator, Strs...>{};
}

template <typename Str, typename X>
constexpr std::size_t find(Str = {}, X = {}) noexcept {
    if constexpr (Str::Size() >= X::Size()) {
        for (std::size_t i = 0; i < Str::Size() - X::Size() + 1; i++) {
            bool flag = true;
            for (std::size_t k = 0; k < X::Size(); k++) {
                if (Str::View()[i + k] != X::View()[k]) {
                    flag = false;
                    break;
                }
            }
            if (flag) return i;
        }
    }
    return static_cast<std::size_t>(-1);
}

template <typename Str, typename X>
constexpr std::size_t find_last(Str = {}, X = {}) noexcept {
    if constexpr (Str::Size() >= X::Size()) {
        for (std::size_t i = 0; i < Str::Size() - X::Size() + 1; i++) {
            std::size_t idx = Str::Size() - X::Size() - i;
            bool flag = true;
            for (std::size_t k = 0; k < X::Size(); k++) {
                if (Str::View()[idx + k] != X::View()[k]) {
                    flag = false;
                    break;
                }
            }
            if (flag) return idx;
        }
    }
    return static_cast<std::size_t>(-1);
}

template <typename Str, typename X>
constexpr bool starts_with(Str = {}, X = {}) noexcept {
    if constexpr (Str::Size() < X::Size())
        return false;
    else {
        for (std::size_t i = 0; i < X::Size(); i++) {
            if (Str::View()[i] != X::View()[i]) return false;
        }
        return true;
    }
}

template <typename Str, typename X>
constexpr bool ends_with(Str = {}, X = {}) noexcept {
    if constexpr (Str::Size() < X::Size())
        return false;
    else {
        for (std::size_t i = 0; i < X::Size(); i++) {
            if (Str::View()[Str::Size() - X::Size() + i] != X::View()[i]) return false;
        }
        return true;
    }
}

template <std::size_t N, typename Str>
constexpr auto remove_prefix(Str = {}) {
    if constexpr (Str::Size() >= N) {
        return TSTR(Str::Data() + N);
    } else
        return empty_of(Str{});
}

template <typename Str, typename X>
constexpr auto remove_prefix(Str = {}, X = {}) {
    if constexpr (starts_with<Str, X>())
        return remove_prefix<X::Size(), Str>();
    else
        return Str{};
}

template <std::size_t N, typename Char>
constexpr std::basic_string_view<Char> detail_remove_suffix(std::basic_string_view<Char> str) {
    str.remove_suffix(N);
    return str;
}

template <std::size_t N, typename Str>
constexpr auto remove_suffix(Str = {}) {
    if constexpr (Str::Size() >= N)
        return TSTR(detail_remove_suffix<N>(Str::View()));
    else
        return empty_of(Str{});
}

template <typename Str, typename X>
constexpr auto remove_suffix(Str = {}, X = {}) {
    if constexpr (ends_with<Str, X>())
        return remove_suffix<X::Size(), Str>();
    else
        return Str{};
}

template <std::size_t N, typename Char>
constexpr std::basic_string_view<Char> detail_get_prefix(std::basic_string_view<Char> str) {
    return {str.data(), N};
}

template <std::size_t N, typename Str>
constexpr auto get_prefix(Str = {}) {
    if constexpr (Str::Size() >= N)
        return TSTR(detail_get_prefix<N>(Str::View()));
    else
        return Str{};
}

template <std::size_t N, typename Str>
constexpr auto get_suffix(Str = {}) {
    if constexpr (Str::Size() >= N)
        return TSTR(decltype(Str::View()){Str::Data() + Str::Size() - N});
    else
        return Str{};
}

// [Left, Right)
template <std::size_t Idx, std::size_t Cnt, typename Str, typename X>
constexpr auto replace(Str = {}, X = {}) {
    constexpr auto prefix = remove_suffix<Str::Size() - Idx>(Str{});
    constexpr auto suffix = remove_prefix<Idx + Cnt>(Str{});

    return concat(concat(prefix, X{}), suffix);
}

template <typename Str, typename From, typename To>
constexpr auto replace(Str = {}, From = {}, To = {}) {
    constexpr std::size_t idx = find(Str{}, From{});
    if constexpr (idx != static_cast<std::size_t>(-1))
        return replace(replace<idx, From::Size()>(Str{}, To{}), From{}, To{});
    else
        return Str{};
}

template <typename Str, typename X>
constexpr auto remove(Str = {}, X = {}) {
    return replace(Str{}, X{}, empty_of(Str{}));
}

template <std::size_t Idx, std::size_t Cnt, typename Str>
constexpr auto substr(Str = {}) {
    return get_prefix<Cnt>(remove_prefix<Idx, Str>());
}

template <auto V, std::enable_if_t<std::is_integral_v<decltype(V)>, int> = 0>
constexpr auto int_to_TSTR() {
    if constexpr (V == 0)
        return TStr_of_a<'0'>{};
    else {  // not zero
        using T = decltype(V);
        if constexpr (std::is_signed_v<T>) {
            if constexpr (V < 0)
                return concat(TStr_of_a<'-'>{}, int_to_TSTR<static_cast<std::make_unsigned_t<T>>(-V)>());
            else
                return int_to_TSTR<static_cast<std::make_unsigned_t<T>>(V)>();
        } else {  // unsigned
            if constexpr (V < 10) {
                return TStr_of_a<static_cast<char>('0' + V)>{};
            } else
                return concat(int_to_TSTR<V / 10>(), int_to_TSTR<V % 10>());
        }
    }
}
}  // namespace neko::cpp

#endif  // !NEKO_TSTR_UTIL

#pragma endregion TSTR

template <typename F>
struct function_traits : public function_traits<decltype(&F::operator())> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType (ClassType::*)(Args...) const> {
    using return_type = ReturnType;
    using pointer = ReturnType (*)(Args...);
    using std_function = std::function<ReturnType(Args...)>;
};

template <typename F>
typename function_traits<F>::std_function to_function(F &lambda) {
    return typename function_traits<F>::std_function(lambda);
}

namespace neko {

template <typename T>
class moveonly {
public:
    static_assert(std::is_default_constructible_v<T>);
    static_assert(std::is_trivially_copy_constructible_v<T>);
    static_assert(std::is_trivially_copy_assignable_v<T>);
    static_assert(std::is_trivially_move_constructible_v<T>);
    static_assert(std::is_trivially_move_assignable_v<T>);
    static_assert(std::is_swappable_v<T>);

    moveonly() = default;

    ~moveonly() = default;

    explicit moveonly(const T &val) noexcept { m_value = val; }

    moveonly(const moveonly &) = delete;

    moveonly &operator=(const moveonly &) = delete;

    moveonly(moveonly &&other) noexcept { m_value = std::exchange(other.m_value, T{}); }

    moveonly &operator=(moveonly &&other) noexcept {
        std::swap(m_value, other.m_value);
        return *this;
    }

    operator const T &() const { return m_value; }

    moveonly &operator=(const T &val) {
        m_value = val;
        return *this;
    }

    // MoveOnly<T> has the same memory layout as T
    // So it's perfectly safe to overload operator&
    T *operator&() { return &m_value; }

    const T *operator&() const { return &m_value; }

    T *operator->() { return &m_value; }

    const T *operator->() const { return &m_value; }

    bool operator==(const T &val) const { return m_value == val; }

    bool operator!=(const T &val) const { return m_value != val; }

private:
    T m_value{};
};

static_assert(sizeof(int) == sizeof(moveonly<int>));

class noncopyable {
public:
    noncopyable(const noncopyable &) = delete;
    noncopyable &operator=(const noncopyable &) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

struct format_str {
    constexpr format_str(const char *str) noexcept : str(str) {}
    const char *str;
};

template <format_str F>
constexpr auto operator""_f() {
    return [=]<typename... T>(T... args) { return std::format(F.str, args...); };
}

// 成员函数返回值类型确定
// https://stackoverflow.com/questions/26107041/how-can-i-determine-the-return-type-of-a-c11-member-function

template <typename T>
struct return_type;
template <typename R, typename... Args>
struct return_type<R (*)(Args...)> {
    using type = R;
};
template <typename R, typename C, typename... Args>
struct return_type<R (C::*)(Args...)> {
    using type = R;
};
template <typename R, typename C, typename... Args>
struct return_type<R (C::*)(Args...) const> {
    using type = R;
};
template <typename R, typename C, typename... Args>
struct return_type<R (C::*)(Args...) volatile> {
    using type = R;
};
template <typename R, typename C, typename... Args>
struct return_type<R (C::*)(Args...) const volatile> {
    using type = R;
};
template <typename T>
using return_type_t = typename return_type<T>::type;

struct any_function {
public:
    struct type {
        const std::type_info *info;
        bool is_lvalue_reference, is_rvalue_reference;
        bool is_const, is_volatile;
        bool operator==(const type &r) const {
            return info == r.info && is_lvalue_reference == r.is_lvalue_reference && is_rvalue_reference == r.is_rvalue_reference && is_const == r.is_const && is_volatile == r.is_volatile;
        }
        bool operator!=(const type &r) const { return !(*this == r); }
        template <class T>
        static type capture() {
            return {&typeid(T), std::is_lvalue_reference<T>::value, std::is_rvalue_reference<T>::value, std::is_const<typename std::remove_reference<T>::type>::value,
                    std::is_volatile<typename std::remove_reference<T>::type>::value};
        }
    };

    class result {
        struct result_base {
            virtual ~result_base() {}
            virtual std::unique_ptr<result_base> clone() const = 0;
            virtual type get_type() const = 0;
            virtual void *get_address() = 0;
        };
        template <class T>
        struct typed_result : result_base {
            T x;
            typed_result(T x) : x(get((void *)&x, tag<T>{})) {}
            std::unique_ptr<result_base> clone() const { return std::unique_ptr<typed_result>(new typed_result(get((void *)&x, tag<T>{}))); }
            type get_type() const { return type::capture<T>(); }
            void *get_address() { return (void *)&x; }
        };
        std::unique_ptr<result_base> p;

    public:
        result() {}
        result(result &&r) : p(move(r.p)) {}
        result(const result &r) { *this = r; }
        result &operator=(result &&r) {
            p.swap(r.p);
            return *this;
        }
        result &operator=(const result &r) {
            p = r.p ? r.p->clone() : nullptr;
            return *this;
        }

        type get_type() const { return p ? p->get_type() : type::capture<void>(); }
        void *get_address() { return p ? p->get_address() : nullptr; }
        template <class T>
        T get_value() {
            ME_ASSERT(get_type() == type::capture<T>());
            return get(p->get_address(), tag<T>{});
        }

        template <class T>
        static result capture(T x) {
            result r;
            r.p.reset(new typed_result<T>(static_cast<T>(x)));
            return r;
        }
    };
    any_function() : result_type{} {}
    any_function(std::nullptr_t) : result_type{} {}
    template <class R, class... A>
    any_function(R (*p)(A...)) : any_function(p, tag<R>{}, tag<A...>{}, build_indices<sizeof...(A)>{}) {}
    template <class R, class... A>
    any_function(std::function<R(A...)> f) : any_function(f, tag<R>{}, tag<A...>{}, build_indices<sizeof...(A)>{}) {}
    template <class F>
    any_function(F f) : any_function(f, &F::operator()) {}

    explicit operator bool() const { return static_cast<bool>(func); }
    const std::vector<type> &get_parameter_types() const { return parameter_types; }
    const type &get_result_type() const { return result_type; }
    result invoke(void *const args[]) const { return func(args); }
    result invoke(std::initializer_list<void *> args = {}) const { return invoke(args.begin()); }

private:
    template <class... T>
    struct tag {};
    template <std::size_t... IS>
    struct indices {};
    template <std::size_t N, std::size_t... IS>
    struct build_indices : build_indices<N - 1, N - 1, IS...> {};
    template <std::size_t... IS>
    struct build_indices<0, IS...> : indices<IS...> {};

    template <class T>
    static T get(void *arg, tag<T>) {
        return *reinterpret_cast<T *>(arg);
    }
    template <class T>
    static T &get(void *arg, tag<T &>) {
        return *reinterpret_cast<T *>(arg);
    }
    template <class T>
    static T &&get(void *arg, tag<T &&>) {
        return std::move(*reinterpret_cast<T *>(arg));
    }
    template <class F, class R, class... A, size_t... I>
    any_function(F f, tag<R>, tag<A...>, indices<I...>) : parameter_types({type::capture<A>()...}), result_type(type::capture<R>()) {
        func = [f](void *const args[]) mutable { return result::capture<R>(f(get(args[I], tag<A>{})...)); };
    }
    template <class F, class... A, size_t... I>
    any_function(F f, tag<void>, tag<A...>, indices<I...>) : parameter_types({type::capture<A>()...}), result_type(type::capture<void>()) {
        func = [f](void *const args[]) mutable { return f(get(args[I], tag<A>{})...), result{}; };
    }
    template <class F, class R>
    any_function(F f, tag<R>, tag<>, indices<>) : parameter_types({}), result_type(type::capture<R>()) {
        func = [f](void *const args[]) mutable { return result::capture<R>(f()); };
    }
    template <class F>
    any_function(F f, tag<void>, tag<>, indices<>) : parameter_types({}), result_type(type::capture<void>()) {
        func = [f](void *const args[]) mutable { return f(), result{}; };
    }
    template <class F, class R, class... A>
    any_function(F f, R (F::*p)(A...)) : any_function(f, tag<R>{}, tag<A...>{}, build_indices<sizeof...(A)>{}) {}
    template <class F, class R, class... A>
    any_function(F f, R (F::*p)(A...) const) : any_function(f, tag<R>{}, tag<A...>{}, build_indices<sizeof...(A)>{}) {}

    std::function<result(void *const *)> func;
    std::vector<type> parameter_types;
    type result_type;
};

// std::function 合并方法

template <typename, typename...>
struct lastFnType;

template <typename F0, typename F1, typename... Fn>
struct lastFnType<F0, F1, Fn...> {
    using type = typename lastFnType<F1, Fn...>::type;
};

template <typename T1, typename T2>
struct lastFnType<neko_function<T2(T1)>> {
    using type = T1;
};

template <typename T1, typename T2>
neko_function<T1(T2)> func_combine(neko_function<T1(T2)> conv) {
    return conv;
}

template <typename T1, typename T2, typename T3, typename... Fn>
auto func_combine(neko_function<T1(T2)> conv1, neko_function<T2(T3)> conv2, Fn... fn) -> neko_function<T1(typename lastFnType<neko_function<T2(T3)>, Fn...>::type)> {
    using In = typename lastFnType<neko_function<T2(T3)>, Fn...>::type;

    return [=](In const &in) { return conv1(func_combine(conv2, fn...)(in)); };
}

}  // namespace neko

template <typename T>
struct neko_is_vector : std::false_type {};

template <typename T, typename Alloc>
struct neko_is_vector<std::vector<T, Alloc>> : std::true_type {};

namespace detail {
// 某些旧版本的 GCC 需要
template <typename...>
struct voider {
    using type = void;
};

// std::void_t 将成为 C++17 的一部分 但在这里我还是自己实现吧
template <typename... T>
using void_t = typename voider<T...>::type;

template <typename T, typename U = void>
struct is_mappish_impl : std::false_type {};

template <typename T>
struct is_mappish_impl<T, void_t<typename T::key_type, typename T::mapped_type, decltype(std::declval<T &>()[std::declval<const typename T::key_type &>()])>> : std::true_type {};
}  // namespace detail

template <typename T>
struct neko_is_mappish : detail::is_mappish_impl<T>::type {};

template <class... Ts>
struct neko_overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
neko_overloaded(Ts...) -> neko_overloaded<Ts...>;

#endif
