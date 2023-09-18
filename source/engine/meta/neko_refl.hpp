#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory_resource>
#include <set>
#include <shared_mutex>
#include <span>
#include <sstream>
#include <variant>

#include "engine/utility/name.hpp"
#include "engine/utility/neko_cpp_misc.hpp"
#include "engine/utility/neko_cpp_utils.hpp"

#ifndef NDEBUG
#include <unordered_set>
#endif  // !NDEBUG

namespace neko::cpp {
static constexpr std::size_t MaxArgNum = 64;
static_assert(MaxArgNum <= 256 - 2);
}  // namespace neko::cpp

#define NEKO_DYREFL_ENUM_BOOL_OPERATOR_DEFINE(Name)                          \
    constexpr Name operator&(const Name& lhs, const Name& rhs) noexcept {    \
        static_assert(std::is_enum_v<Name>);                                 \
        using T = std::underlying_type_t<Name>;                              \
        return static_cast<Name>(static_cast<T>(lhs) & static_cast<T>(rhs)); \
    }                                                                        \
    constexpr Name operator|(const Name& lhs, const Name& rhs) noexcept {    \
        static_assert(std::is_enum_v<Name>);                                 \
        using T = std::underlying_type_t<Name>;                              \
        return static_cast<Name>(static_cast<T>(lhs) | static_cast<T>(rhs)); \
    }                                                                        \
    constexpr Name operator~(const Name& e) noexcept {                       \
        static_assert(std::is_enum_v<Name>);                                 \
        using T = std::underlying_type_t<Name>;                              \
        return static_cast<Name>(~static_cast<T>(e));                        \
    }                                                                        \
    constexpr Name& operator&=(Name& lhs, const Name& rhs) noexcept {        \
        static_assert(std::is_enum_v<Name>);                                 \
        using T = std::underlying_type_t<Name>;                              \
        lhs = static_cast<Name>(static_cast<T>(lhs) & static_cast<T>(rhs));  \
        return lhs;                                                          \
    }                                                                        \
    constexpr Name& operator|=(Name& lhs, const Name& rhs) noexcept {        \
        static_assert(std::is_enum_v<Name>);                                 \
        using T = std::underlying_type_t<Name>;                              \
        lhs = static_cast<Name>(static_cast<T>(lhs) | static_cast<T>(rhs));  \
        return lhs;                                                          \
    }

namespace std {
template <typename T>
struct variant_size;
}

namespace neko::cpp {
using Offsetor = std::function<void*(void*)>;
using Destructor = std::function<void(const void*)>;
using DeleteFunc = std::function<void(void*)>;  // destruct + free

template <typename Obj, typename T>
std::size_t field_forward_offset_value(T Obj::*field_ptr) noexcept {
    static_assert(!std::is_function_v<T>);
    static_assert(!has_virtual_base_v<Obj>);
    return reinterpret_cast<std::size_t>(&(reinterpret_cast<const Obj*>(0)->*field_ptr));
}

template <auto fieldptr>
struct field_offsetor_impl;

template <typename Obj, typename T, T Obj::*fieldptr>
struct field_offsetor_impl<fieldptr> {
    static_assert(!std::is_function_v<T>);
    static constexpr auto get() noexcept {
        return [](void* ptr) noexcept -> void* { return &(reinterpret_cast<Obj*>(ptr)->*fieldptr); };
    }
};

template <auto fieldptr>
constexpr auto field_offsetor() noexcept {
    return field_offsetor_impl<fieldptr>::get();
}

// result size of field_offsetor(fieldptr) > result size of field_offsetor<fieldptr>
template <typename T, typename Obj>
constexpr auto field_offsetor(T Obj::*fieldptr) noexcept {
    static_assert(!std::is_function_v<T>);
    return [fieldptr](void* ptr) noexcept -> void* { return &(reinterpret_cast<Obj*>(ptr)->*fieldptr); };
}

struct InheritCastFunctions {
    Offsetor static_derived_to_base;
    Offsetor static_base_to_derived;
    Offsetor dynamic_base_to_derived;
};

template <typename From, typename To>
constexpr auto static_cast_functor() noexcept {
    static_assert(!is_virtual_base_of_v<From, To>);
    return [](void* obj) noexcept -> void* { return static_cast<To*>(reinterpret_cast<From*>(obj)); };
}

template <typename Base, typename Derived>
constexpr auto dynamic_cast_function() noexcept {
    static_assert(std::is_base_of_v<Base, Derived>);
    if constexpr (std::is_polymorphic_v<Base>) {
        return [](void* obj) noexcept -> void* { return dynamic_cast<Derived*>(reinterpret_cast<Base*>(obj)); };
    } else
        return static_cast_functor<Base, Derived>();
}

// polymorphic: dynamic_cast
// virtual    : no static_cast (Base -> Derived)
template <typename Derived, typename Base>
InheritCastFunctions inherit_cast_functions() {
    static_assert(std::is_base_of_v<Base, Derived>);
    if constexpr (std::is_polymorphic_v<Derived>) {
        if constexpr (is_virtual_base_of_v<Base, Derived>) {
            return {static_cast_functor<Derived, Base>(), nullptr, dynamic_cast_function<Base, Derived>()};
        } else {
            return {static_cast_functor<Derived, Base>(), static_cast_functor<Base, Derived>(), dynamic_cast_function<Base, Derived>()};
        }
    } else {
        if constexpr (is_virtual_base_of_v<Base, Derived>) {
            return {static_cast_functor<Derived, Base>(), nullptr, nullptr};
        } else {
            return {static_cast_functor<Derived, Base>(), static_cast_functor<Base, Derived>(), nullptr};
        }
    }
}

template <typename T>
Destructor destructor() {
    if constexpr (std::is_fundamental_v<T>)
        return {};
    else {
        static_assert(std::is_destructible_v<T>);
        if constexpr (!std::is_trivially_destructible_v<T>) {
            return [](const void* ptr) { reinterpret_cast<const T*>(ptr)->~T(); };
        } else
            return {};
    }
}

constexpr void* forward_offset(void* ptr, std::size_t offset) noexcept { return (std::uint8_t*)ptr + offset; }

constexpr const void* forward_offset(const void* ptr, std::size_t offset) noexcept { return forward_offset(const_cast<void*>(ptr), offset); }

constexpr void* backward_offset(void* ptr, std::size_t offset) noexcept { return (std::uint8_t*)ptr - offset; }

constexpr const void* backward_offset(const void* ptr, std::size_t offset) noexcept { return backward_offset(const_cast<void*>(ptr), offset); }

template <typename T>
constexpr T& buffer_get(void* buffer, std::size_t offset) noexcept {
    auto ptr = forward_offset(buffer, offset);
    return *reinterpret_cast<T*>(ptr);
}

template <typename T>
constexpr const T& buffer_get(const void* buffer, std::size_t offset) noexcept {
    return buffer_get<T>(const_cast<void*>(buffer), offset);
}

template <typename T>
constexpr T& buffer_as(void* buffer) noexcept {
    return buffer_get<T>(buffer, 0);
}

template <typename T>
constexpr const T& buffer_as(const void* buffer) noexcept {
    return buffer_get<T>(buffer, 0);
}

template <typename T>
constexpr T* ptr_const_cast(const T* ptr) noexcept {
    return const_cast<T*>(ptr);
}

template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr decltype(auto) enum_cast(Enum&& e) noexcept;
template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr bool enum_empty(const Enum& e) noexcept;
template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr bool enum_single(const Enum& e) noexcept;
template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr bool enum_contain_any(const Enum& e, const Enum& flag) noexcept;
template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr bool enum_contain(const Enum& e, const Enum& flag) noexcept;
template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr Enum enum_combine(std::initializer_list<Enum> flags) noexcept;
template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr Enum enum_remove(const Enum& e, const Enum& flag) noexcept;
template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr Enum enum_within(const Enum& e, const Enum& flag) noexcept;

// to <- from
// - same
// - reference
// > - 0 (invalid), 1 (convertible)
// > - table
//     |     -     | T | T & | const T & | T&& | const T&& | const T |
//     |       T   | - |  0  |     0     |  1  |     0     |    0    |
//     |       T & | 0 |  -  |     0     |  0  |     0     |    0    |
//     | const T & | 1 |  1  |     -     |  1  |     1     |    1    |
//     |       T&& | 1 |  0  |     0     |  -  |     0     |    0    |
//     | const T&& | 1 |  0  |     0     |  1  |     -     |    1    |
constexpr bool is_ref_compatible(Type to, Type from) noexcept;

// to <- copy from
// to can't be non-const reference
// remove_cvref for to and from, and then use below table
// - 0 (invalid), 1 (convertible)
// - table
//     |     -     | T * | const T * | T[] | const T[] |
//     |       T * |  -  |     0     |  1  |     0     |
//     | const T * |  1  |     -     |  1  |     1     |
//     |       T[] |  1  |     0     |  -  |     0     |
//     | const T[] |  1  |     1     |  1  |     -     |
constexpr bool is_pointer_array_compatible(std::string_view to, std::string_view from) noexcept;

////////////
// Traits //
////////////

template <typename From, typename To>
concept static_castable_to = requires(From from) { static_cast<To>(from); };

//
// operation
//////////////

template <typename T>
concept operator_bool = static_castable_to<T, bool>;

template <typename T>
concept operator_plus = requires(T t) { +t; };
template <typename T>
concept operator_minus = !std::is_unsigned_v<T> && requires(T t) { -t; };

template <typename T>
concept operator_add = requires(T lhs, T rhs) { lhs + rhs; };
template <typename T>
concept operator_sub = !std::is_same_v<std::decay_t<T>, void*> && requires(T lhs, T rhs) { lhs - rhs; };
template <typename T>
concept operator_mul = requires(T lhs, T rhs) { lhs* rhs; };
template <typename T>
concept operator_div = !std::is_same_v<std::remove_cvref_t<T>, bool> && requires(T lhs, T rhs) { lhs / rhs; };
template <typename T>
concept operator_mod = !std::is_same_v<std::remove_cvref_t<T>, bool> && requires(T lhs, T rhs) { lhs % rhs; };

template <typename T>
concept operator_bnot = !std::is_same_v<std::remove_cvref_t<T>, bool> && requires(T t) { ~t; };
template <typename T>
concept operator_band = requires(T lhs, T rhs) { lhs& rhs; };
template <typename T>
concept operator_bor = requires(T lhs, T rhs) { lhs | rhs; };
template <typename T>
concept operator_bxor = requires(T lhs, T rhs) { lhs ^ rhs; };
template <typename T, typename U>
concept operator_shl = requires(T lhs, U rhs) { lhs << rhs; };
template <typename T, typename U>
concept operator_shr = requires(T lhs, U rhs) { lhs >> rhs; };

template <typename T>
concept operator_pre_inc = !std::is_same_v<T, bool> && requires(T t) { ++t; };
template <typename T>
concept operator_post_inc = !std::is_same_v<T, bool> && requires(T t) { t++; };
template <typename T>
concept operator_pre_dec = !std::is_same_v<T, bool> && requires(T t) { --t; };
template <typename T>
concept operator_post_dec = !std::is_same_v<T, bool> && requires(T t) { t--; };

template <typename T, typename U>
concept operator_assignment = requires(T lhs, U rhs) {
    { lhs = std::forward<U>(rhs) } -> std::same_as<T&>;
};
template <typename T>
concept operator_assignment_copy = std::is_copy_assignable_v<T> && operator_assignment<T, const T&>;
template <typename T>
concept operator_assignment_move = std::is_move_assignable_v<T> && operator_assignment<T, T&&>;
template <typename T>
concept operator_assignment_add = !std::is_same_v<T, bool> && requires(T lhs, const T& rhs) {
    { lhs += rhs } -> std::same_as<T&>;
};
template <typename T>
concept operator_assignment_sub = !std::is_same_v<std::decay_t<T>, void*> && !std::is_same_v<T, void> && !std::is_same_v<T, bool> && requires(T lhs, const T& rhs) {
    { lhs -= rhs } -> std::same_as<T&>;
};
template <typename T>
concept operator_assignment_mul = !std::is_same_v<T, bool> && requires(T lhs, const T& rhs) {
    { lhs *= rhs } -> std::same_as<T&>;
};
template <typename T>
concept operator_assignment_div = !std::is_same_v<T, bool> && requires(T lhs, const T& rhs) {
    { lhs /= rhs } -> std::same_as<T&>;
};
template <typename T>
concept operator_assignment_mod = !std::is_same_v<T, bool> && requires(T lhs, const T& rhs) {
    { lhs %= rhs } -> std::same_as<T&>;
};
template <typename T>
concept operator_assignment_band = !std::is_same_v<T, bool> && requires(T lhs, const T& rhs) {
    { lhs &= rhs } -> std::same_as<T&>;
};
template <typename T>
concept operator_assignment_bor = !std::is_same_v<T, bool> && requires(T lhs, const T& rhs) {
    { lhs |= rhs } -> std::same_as<T&>;
};
template <typename T>
concept operator_assignment_bxor = !std::is_same_v<T, bool> && requires(T lhs, const T& rhs) {
    { lhs ^= rhs } -> std::same_as<T&>;
};
template <typename T>
concept operator_assignment_shl = !std::is_same_v<T, bool> && requires(T lhs, const T& rhs) {
    { lhs <<= rhs } -> std::same_as<T&>;
};
template <typename T>
concept operator_assignment_shr = !std::is_same_v<T, bool> && requires(T lhs, const T& rhs) {
    { lhs >>= rhs } -> std::same_as<T&>;
};

template <typename T>
concept operator_eq = !std::is_array_v<T> && requires(const T& lhs, const T& rhs) {
    { lhs == rhs } -> static_castable_to<bool>;
};
template <typename T>
concept operator_ne = !std::is_array_v<T> && requires(const T& lhs, const T& rhs) {
    { lhs != rhs } -> static_castable_to<bool>;
};
template <typename T>
concept operator_lt = !std::is_array_v<T> && requires(const T& lhs, const T& rhs) {
    { lhs < rhs } -> static_castable_to<bool>;
};
template <typename T>
concept operator_le = !std::is_array_v<T> && requires(const T& lhs, const T& rhs) {
    { lhs <= rhs } -> static_castable_to<bool>;
};
template <typename T>
concept operator_gt = !std::is_array_v<T> && requires(const T& lhs, const T& rhs) {
    { lhs > rhs } -> static_castable_to<bool>;
};
template <typename T>
concept operator_ge = !std::is_array_v<T> && requires(const T& lhs, const T& rhs) {
    { lhs >= rhs } -> static_castable_to<bool>;
};

template <typename T, typename U>
concept operator_subscript = !std::is_void_v<std::remove_pointer_t<T>> && requires(T lhs, const U& rhs) { lhs[rhs]; };
template <typename T>
concept operator_indirection = !std::is_same_v<std::decay_t<T>, void*> && requires(T t) { *t; };

//
// pair
///////////

template <typename T>
concept pair_first_type = requires { typename T::first_type; };
template <typename T>
concept pair_second_type = requires { typename T::second_type; };
template <typename T>
concept pair_first = std::is_member_object_pointer_v<decltype(&T::first)>;
template <typename T>
concept pair_second = std::is_member_object_pointer_v<decltype(&T::second)>;

//
// tuple
//////////

template <typename T>
concept tuple_size = requires() { std::tuple_size<T>::value; };

//
// variant
////////////

template <typename T>
concept variant_size = requires() { std::variant_size<T>::value; };

template <typename T>
concept variant_index = requires(const T& t) {
    { t.index() } -> static_castable_to<std::size_t>;
};

template <typename T>
concept variant_valueless_by_exception = requires(const T& t) {
    { t.valueless_by_exception() } -> static_castable_to<bool>;
};

//
// optional
/////////////

template <typename T>
concept optional_has_value = requires(const T& t) {
    { t.has_value() } -> static_castable_to<bool>;
};
template <typename T>
concept optional_value = requires(T t) { t.value(); };
template <typename T>
concept optional_reset = requires(T t) { t.reset(); };

//
// container
//////////////

// - member type
// - > key_type
// - > mapped_type
// - > value_type
// - > allocator_type
// - > size_type
// - > difference_type
// - > pointer
// - > const_pointer
// - > key_compare
// - > value_coompare
// - > iterator
// - > const_iterator
// - > local_iterator
// - > const_local_iterator
// - > reverse_iterator
// - > const_reverse_iterator
// - > node_type
// - > insert_return_type
// - > > position
// - > > inserted
// - > > node

template <typename T>
concept container_key_type = requires { typename T::key_type; };
template <typename T>
concept container_mapped_type = requires { typename T::mapped_type; };
template <typename T>
concept container_value_type = requires { typename T::value_type; };
template <typename T>
concept container_allocator_type = requires { typename T::allocator_type; };
template <typename T>
concept container_size_type = std::is_array_v<T> || requires { typename T::size_type; };
template <typename T>
concept container_difference_type = requires { typename T::difference_type; };
template <typename T>
concept container_pointer_type = requires { typename T::pointer; };
template <typename T>
concept container_const_pointer_type = requires { typename T::const_pointer; };
template <typename T>
concept container_key_compare = requires { typename T::key_compare; };
template <typename T>
concept container_value_compare = requires { typename T::value_coompare; };
template <typename T>
concept container_iterator = requires { typename T::iterator; };
template <typename T>
concept container_const_iterator = requires { typename T::const_iterator; };
template <typename T>
concept container_reverse_iterator = requires { typename T::reverse_iterator; };
template <typename T>
concept container_const_reverse_iterator = requires { typename T::const_reverse_iterator; };
template <typename T>
concept container_local_iterator = requires { typename T::local_iterator; };
template <typename T>
concept container_const_local_iterator = requires { typename T::const_local_iterator; };
template <typename T>
concept container_node_type = requires { typename T::node_type; };
template <typename T>
concept container_insert_return_type = requires { typename T::insert_return_type; };

template <typename T>
struct get_container_size_type;
template <typename T>
using get_container_size_type_t = typename get_container_size_type<T>::type;

// ctor

template <typename T, typename... Args>
concept type_ctor = std::is_constructible_v<T, Args...> && requires(Args... args) { T(std::forward<Args>(args)...); };
template <typename T>
concept type_ctor_copy = std::is_copy_constructible_v<T> && type_ctor<T, const T&>;
template <typename T>
concept type_ctor_move = std::is_move_constructible_v<T> && type_ctor<T, const T&>;

template <typename T>
concept container_ctor_cnt = container_size_type<T> && type_ctor<T, const typename T::size_type&>;

template <typename T>
concept container_ctor_clvalue = container_value_type<T> && type_ctor<T, const typename T::value_type&>;

template <typename T>
concept container_ctor_rvalue = container_value_type<T> && type_ctor<T, typename T::value_type&&>;

template <typename T>
concept container_ctor_cnt_value = container_size_type<T> && container_value_type<T> && type_ctor<T, const typename T::size_type&, const typename T::value_type&>;

template <typename T>
concept container_ctor_ptr_cnt = container_pointer_type<T> && container_size_type<T> && type_ctor<T, const typename T::pointer_type&, const typename T::size_type&>;

template <typename T>
concept container_ctor_ptr_ptr = container_pointer_type<T> && type_ctor<T, const typename T::pointer_type&, const typename T::pointer_type&>;

// assign

template <typename T>
concept container_assign = container_size_type<T> && container_value_type<T> && requires(T t, const typename T::size_type& s, const typename T::value_type& v) { t.assign(s, v); };

// - iterator

template <typename T>
concept container_begin = requires(T t) { std::begin(t); };
template <typename T>
concept container_cbegin = requires(const T& t) { std::cbegin(t); };
template <typename T>
concept container_rbegin = requires(T t) { std::rbegin(t); };
template <typename T>
concept container_crbegin = requires(const T& t) { std::crbegin(t); };
template <typename T>
concept container_end = requires(T t) { std::end(t); };
template <typename T>
concept container_cend = requires(const T& t) { std::cend(t); };
template <typename T>
concept container_rend = requires(T t) { std::rend(t); };
template <typename T>
concept container_crend = requires(const T& t) { std::crend(t); };

// - element access

template <typename T, typename U>
concept container_at = requires(T t, const U& key) { t.at(key); };
template <typename T>
concept container_at_size = container_size_type<T> && container_at<T, typename T::size_type>;
template <typename T>
concept container_at_key = container_key_type<T> && container_at<T, typename T::key_type>;
template <typename T, typename U>
concept container_subscript = requires(T t, U key) { t[std::forward<U>(key)]; };
template <typename T>
concept container_subscript_size = container_size_type<std::remove_reference_t<T>> && container_subscript<T, const get_container_size_type_t<T>&>;
template <typename T>
concept container_subscript_key_cl = container_key_type<T> && container_subscript<T, const typename T::key_type&>;
template <typename T>
concept container_subscript_key_r = container_key_type<T> && container_subscript<T, typename T::key_type>;
template <typename T>
concept container_data = requires(T t) { std::data(t); };
template <typename T>
concept container_front = requires(T t) { t.front(); };
template <typename T>
concept container_back = requires(T t) { t.back(); };
template <typename T>
concept container_top = requires(T t) { t.top(); };

// - capacity

template <typename T>
concept container_empty = requires(const T& t) {
    { std::empty(t) } -> static_castable_to<bool>;
};
template <typename T>
concept container_size = requires(const T& t) {
    { std::size(t) } -> static_castable_to<std::size_t>;
};
template <typename T>
concept container_size_bytes = requires(const T& t) {
    { t.size_bytes() } -> static_castable_to<std::size_t>;
};
template <typename T>
concept container_resize_cnt = container_size_type<T> && requires(T t, const typename T::size_type& cnt) { t.resize(cnt); };
template <typename T>
concept container_resize_cnt_value =
        container_size_type<T> && container_value_type<T> && requires(T t, const typename T::size_type& cnt, const typename T::value_type& value) { t.resize(cnt, value); };
template <typename T>
concept container_capacity = requires(const T& t) {
    { t.capacity() } -> static_castable_to<std::size_t>;
};
template <typename T>
concept container_bucket_count = requires(const T& t) {
    { t.bucket_count() } -> static_castable_to<std::size_t>;
};
template <typename T>
concept container_reserve = container_size_type<T> && requires(T t, const typename T::size_type& cnt) { t.reserve(cnt); };
template <typename T>
concept container_shrink_to_fit = container_size_type<T> && requires(T t) { t.shrink_to_fit(); };

// - modifiers

template <typename T>
concept container_clear = container_size_type<T> && requires(T t) { t.clear(); };

template <typename T, typename V>
concept container_insert = requires(T t, V value) { t.insert(std::forward<V>(value)); };
template <typename T>
concept container_insert_clvalue = container_value_type<T> && container_insert<T, const typename T::value_type&>;
template <typename T>
concept container_insert_rvalue = container_value_type<T> && container_insert<T, typename T::value_type&&>;
template <typename T>
concept container_insert_rnode = container_node_type<T> && container_insert<T, typename T::node_type&&>;

template <typename T, typename V>
concept container_insert_citer = container_const_iterator<T> && requires(T t, const typename T::const_iterator& iter, V value) { t.insert(iter, std::forward<V>(value)); };
template <typename T>
concept container_insert_citer_clvalue = container_value_type<T> && container_insert_citer<T, const typename T::value_type&>;
template <typename T>
concept container_insert_citer_rvalue = container_value_type<T> && container_insert_citer<T, typename T::value_type&&>;
template <typename T>
concept container_insert_citer_rnode = container_node_type<T> && container_insert_citer<T, typename T::node_type&&>;

template <typename T>
concept container_insert_citer_cnt = container_const_iterator<T> && container_value_type<T> && container_size_type<T> &&
                                     requires(T t, const typename T::const_iterator& iter, const typename T::size_type& cnt, const typename T::value_type& value) { t.insert(iter, cnt, value); };

template <typename T, typename U, typename V>
concept container_insert_or_assign = requires(T t, U u, V v) { t.insert_or_assign(std::forward<U>(u), std::forward<V>(v)); };
template <typename T>
concept container_insert_or_assign_clkey_rmap = container_key_type<T> && container_mapped_type<T> && container_insert_or_assign<T, const typename T::key_type&, typename T::mapped_type&&>;
template <typename T>
concept container_insert_or_assign_rkey_rmap = container_key_type<T> && container_mapped_type<T> && container_insert_or_assign<T, typename T::key_type&&, typename T::mapped_type&&>;

template <typename T, typename U, typename V>
concept container_insert_or_assign_citer =
        container_const_iterator<T> && requires(T t, const typename T::const_iterator& citer, U u, V v) { t.insert_or_assign(citer, std::forward<U>(u), std::forward<V>(v)); };
template <typename T>
concept container_insert_or_assign_citer_clkey_rmap = container_key_type<T> && container_mapped_type<T> && container_insert_or_assign_citer<T, const typename T::key_type&, typename T::mapped_type&&>;
template <typename T>
concept container_insert_or_assign_citer_rkey_rmap = container_key_type<T> && container_mapped_type<T> && container_insert_or_assign_citer<T, typename T::key_type&&, typename T::mapped_type&&>;

template <typename T, typename V>
concept container_insert_after = container_const_iterator<T> && requires(T t, const typename T::const_iterator& pos, V value) { t.insert_after(pos, std::forward<V>(value)); };
template <typename T>
concept container_insert_after_clvalue = container_value_type<T> && container_insert_after<T, const typename T::value_type&>;
template <typename T>
concept container_insert_after_rvalue = container_value_type<T> && container_insert_after<T, typename T::value_type&&>;

template <typename T>
concept container_insert_after_cnt = container_const_iterator<T> && container_size_type<T> && container_value_type<T> &&
                                     requires(T t, const typename T::const_iterator& pos, const typename T::size_type& cnt, const typename T::value_type& value) { t.insert_after(pos, cnt, value); };

template <typename T, typename U>
concept container_erase = requires(T t, const U& u) { t.erase(u); };
template <typename T>
concept container_erase_citer = container_const_iterator<T> && container_erase<T, typename T::const_iterator>;
template <typename T>
concept container_erase_key = container_key_type<T> && container_erase<T, typename T::key_type>;

template <typename T, typename U>
concept container_erase_range = requires(T t, const U& b, const U& e) { t.erase(b, e); };
template <typename T>
concept container_erase_range_citer = container_const_iterator<T> && container_erase_range<T, typename T::const_iterator>;

template <typename T>
concept container_erase_after = container_const_iterator<T> && requires(T t, const typename T::const_iterator& pos) { t.erase_after(pos); };

template <typename T>
concept container_erase_after_range = container_const_iterator<T> && requires(T t, const typename T::const_iterator& first, const typename T::const_iterator& last) { t.erase_after(first, last); };

template <typename T, typename U>
concept container_push_front = requires(T t, U u) { t.push_front(std::forward<U>(u)); };
template <typename T>
concept container_push_front_clvalue = container_value_type<T> && container_push_front<T, const typename T::value_type&>;
template <typename T>
concept container_push_front_rvalue = container_value_type<T> && container_push_front<T, typename T::value_type&&>;

template <typename T>
concept container_pop_front = requires(T t) { t.pop_front(); };

template <typename T, typename U>
concept container_push_back = requires(T t, U u) { t.push_back(std::forward<U>(u)); };
template <typename T>
concept container_push_back_clvalue = container_value_type<T> && container_push_back<T, const typename T::value_type&>;
template <typename T>
concept container_push_back_rvalue = container_value_type<T> && container_push_back<T, typename T::value_type&&>;

template <typename T>
concept container_pop_back = requires(T t) { t.pop_back(); };

template <typename T, typename U>
concept container_push = requires(T t, U u) { t.push(std::forward<U>(u)); };
template <typename T>
concept container_push_clvalue = container_value_type<T> && container_push<T, const typename T::value_type&>;
template <typename T>
concept container_push_rvalue = container_value_type<T> && container_push<T, typename T::value_type&&>;

template <typename T>
concept container_pop = requires(T t) { t.pop(); };

template <typename T>
concept container_swap = requires(T lhs, T rhs) { std::swap(lhs, rhs); };

template <typename T, typename U>
concept container_merge = requires(T t, U u) { t.merge(std::forward<U>(u)); };
template <typename T>
concept container_merge_l = container_merge<T, T&>;
template <typename T>
concept container_merge_r = container_merge<T, T&&>;

template <typename T, typename U>
concept container_extract = requires(T t, const U& u) { t.extract(u); };
template <typename T>
concept container_extract_citer = container_const_iterator<T> && container_extract<T, typename T::const_iterator>;
template <typename T>
concept container_extract_key = container_key_type<T> && container_extract<T, typename T::key_type>;

// - lookup

template <typename T>
concept container_count = container_key_type<T> && requires(const T& t, const typename T::key_type& u) {
    { t.count(u) } -> static_castable_to<std::size_t>;
};

template <typename T>
concept container_find = container_key_type<T> && requires(T t, const typename T::key_type& u) { t.find(u); };

template <typename T>
concept container_contains = container_key_type<T> && requires(const T& t, const typename T::key_type& u) {
    { t.count(u) } -> static_castable_to<bool>;
};

template <typename T>
concept container_lower_bound = container_key_type<T> && requires(T t, const typename T::key_type& u) { t.lower_bound(u); };

template <typename T>
concept container_upper_bound = container_key_type<T> && requires(T t, const typename T::key_type& u) { t.upper_bound(u); };

template <typename T>
concept container_equal_range = container_key_type<T> && requires(T t, const typename T::key_type& u) { t.equal_range(u); };

// - list operations

template <typename T, typename Other>
concept container_splice_after = container_const_iterator<T> && requires(T t, const typename T::const_iterator& pos, Other other) { t.splice_after(pos, std::forward<Other>(other)); };

template <typename T>
concept container_splice_after_l = container_splice_after<T, T&>;
template <typename T>
concept container_splice_after_r = container_splice_after<T, T&&>;

template <typename T, typename Other>
concept container_splice_after_it =
        container_const_iterator<T> && requires(T t, const typename T::const_iterator& pos, Other other, const typename T::const_iterator& it) { t.splice_after(pos, std::forward<Other>(other), it); };

template <typename T>
concept container_splice_after_it_l = container_splice_after_it<T, T&>;
template <typename T>
concept container_splice_after_it_r = container_splice_after_it<T, T&&>;

template <typename T, typename Other>
concept container_splice_after_range = container_const_iterator<T> && requires(T t, const typename T::const_iterator& pos, Other other, const typename T::const_iterator& first,
                                                                               const typename T::const_iterator& last) { t.splice_after(pos, std::forward<Other>(other), first, last); };

template <typename T>
concept container_splice_after_range_l = container_splice_after_range<T, T&>;
template <typename T>
concept container_splice_after_range_r = container_splice_after_range<T, T&&>;

template <typename T, typename Other>
concept container_splice = container_const_iterator<T> && requires(T t, const typename T::const_iterator& pos, Other other) { t.splice(pos, std::forward<Other>(other)); };

template <typename T>
concept container_splice_l = container_splice<T, T&>;
template <typename T>
concept container_splice_r = container_splice<T, T&&>;

template <typename T, typename Other>
concept container_splice_it =
        container_const_iterator<T> && requires(T t, const typename T::const_iterator& pos, Other other, const typename T::const_iterator& it) { t.splice(pos, std::forward<Other>(other), it); };

template <typename T>
concept container_splice_it_l = container_splice_it<T, T&>;
template <typename T>
concept container_splice_it_r = container_splice_it<T, T&&>;

template <typename T, typename Other>
concept container_splice_range = container_const_iterator<T> && requires(T t, const typename T::const_iterator& pos, Other other, const typename T::const_iterator& first,
                                                                         const typename T::const_iterator& last) { t.splice(pos, std::forward<Other>(other), first, last); };

template <typename T>
concept container_splice_range_l = container_splice_range<T, T&>;
template <typename T>
concept container_splice_range_r = container_splice_range<T, T&&>;

template <typename T>
concept container_remove = container_value_type<T> && requires(T t, const typename T::value_type& v) {
    { t.remove(v) } -> static_castable_to<std::size_t>;
};

template <typename T>
concept container_reverse = requires(T t) { t.reverse(); };

template <typename T>
concept container_unique = requires(T t) {
    { t.unique() } -> static_castable_to<std::size_t>;
};

template <typename T>
concept container_sort = requires(T t) { t.sort(); };
}  // namespace neko::cpp

template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr decltype(auto) neko::cpp::enum_cast(Enum&& e) noexcept {
    using E = decltype(e);
    using T = std::underlying_type_t<std::remove_cvref_t<Enum>>;
    if constexpr (std::is_reference_v<E>) {
        using UnrefE = std::remove_reference_t<E>;
        if constexpr (std::is_lvalue_reference_v<E>) {
            if constexpr (std::is_const_v<UnrefE>)
                return static_cast<const T&>(e);
            else
                return static_cast<T&>(e);
        } else if constexpr (std::is_rvalue_reference_v<E>) {
            if constexpr (std::is_const_v<UnrefE>)
                return static_cast<const T&&>(e);
            else
                return static_cast<T&&>(e);
        } else {
            if constexpr (std::is_const_v<UnrefE>)
                return static_cast<const T>(e);
            else
                return static_cast<T>(e);
        }
    }
}

template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr bool neko::cpp::enum_empty(const Enum& e) noexcept {
    using T = std::underlying_type_t<Enum>;
    return static_cast<T>(e);
}

template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr bool neko::cpp::enum_single(const Enum& e) noexcept {
    using T = std::underlying_type_t<Enum>;
    return (static_cast<T>(e) & (static_cast<T>(e) - 1)) == static_cast<T>(0);
}

template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr bool neko::cpp::enum_contain_any(const Enum& e, const Enum& flag) noexcept {
    using T = std::underlying_type_t<Enum>;
    return static_cast<T>(e) & static_cast<T>(flag);
}

template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr bool neko::cpp::enum_contain(const Enum& e, const Enum& flag) noexcept {
    using T = std::underlying_type_t<Enum>;
    const auto flag_T = static_cast<T>(flag);
    return (static_cast<T>(e) & flag_T) == flag_T;
}

template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr Enum neko::cpp::enum_combine(std::initializer_list<Enum> flags) noexcept {
    using T = std::underlying_type_t<Enum>;
    T rst = 0;
    for (const auto& flag : flags) rst |= static_cast<T>(flag);
    return static_cast<Enum>(rst);
}

template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr Enum neko::cpp::enum_remove(const Enum& e, const Enum& flag) noexcept {
    using T = std::underlying_type_t<Enum>;
    return static_cast<Enum>(static_cast<T>(e) & (~static_cast<T>(flag)));
}

template <typename Enum>
    requires std::is_enum_v<Enum>
constexpr Enum neko::cpp::enum_within(const Enum& e, const Enum& flag) noexcept {
    using T = std::underlying_type_t<Enum>;
    return static_cast<Enum>(static_cast<T>(e) & (static_cast<T>(flag)));
}

constexpr bool neko::cpp::is_ref_compatible(Type lhs, Type rhs) noexcept {
    if (lhs == rhs) return true;

    if (lhs.IsLValueReference()) {                                           // &{T} | &{const{T}}
        const auto unref_lhs = lhs.Name_RemoveLValueReference();             // T | const{T}
        if (type_name_is_const(unref_lhs)) {                                 // &{const{T}}
            if (unref_lhs == rhs.Name_RemoveRValueReference()) return true;  // &{const{T}} <- &&{const{T}} | const{T}

            const auto raw_lhs = type_name_remove_const(unref_lhs);  // T

            if (rhs.Is(raw_lhs) || raw_lhs == rhs.Name_RemoveReference()) return true;  // &{const{T}} <- T | &{T} | &&{T}
        }
    } else if (lhs.IsRValueReference()) {                         // &&{T} | &&{const{T}}
        const auto unref_lhs = lhs.Name_RemoveRValueReference();  // T | const{T}

        if (type_name_is_const(unref_lhs)) {     // &&{const{T}}
            if (rhs.Is(unref_lhs)) return true;  // &&{const{T}} <- const{T}

            const auto raw_lhs = type_name_remove_const(unref_lhs);  // T

            if (rhs.Is(raw_lhs)) return true;  // &&{const{T}} <- T

            if (raw_lhs == rhs.Name_RemoveRValueReference()) return true;  // &&{const{T}} <- &&{T}
        } else {
            if (rhs.Is(unref_lhs)) return true;  // &&{T} <- T
        }
    } else {                                                        // T
        if (lhs.Is(rhs.Name_RemoveRValueReference())) return true;  // T <- &&{T}
    }

    return false;
}

constexpr bool neko::cpp::is_pointer_array_compatible(std::string_view lhs, std::string_view rhs) noexcept {
    if (type_name_is_reference(lhs)) {
        lhs = type_name_remove_reference(lhs);
        if (!type_name_is_const(lhs)) return false;
        lhs = type_name_remove_const(lhs);
    }
    rhs = type_name_remove_cvref(rhs);

    if (lhs == rhs) return true;

    std::string_view lhs_ele;
    if (type_name_is_pointer(lhs))
        lhs_ele = type_name_remove_pointer(lhs);
    else if (type_name_is_unbounded_array(lhs))
        lhs_ele = type_name_remove_extent(lhs);
    else
        return false;

    std::string_view rhs_ele;
    if (type_name_is_pointer(rhs))
        rhs_ele = type_name_remove_pointer(rhs);
    else if (type_name_is_array(rhs))
        rhs_ele = type_name_remove_extent(rhs);
    else
        return false;

    return lhs_ele == rhs_ele || type_name_remove_const(lhs_ele) == rhs_ele;
}

template <typename T>
struct neko::cpp::get_container_size_type : std::type_identity<typename T::size_type> {};
template <typename T>
struct neko::cpp::get_container_size_type<T&> : neko::cpp::get_container_size_type<T> {};
template <typename T>
struct neko::cpp::get_container_size_type<T&&> : neko::cpp::get_container_size_type<T> {};
template <typename T, std::size_t N>
struct neko::cpp::get_container_size_type<T[N]> : std::type_identity<std::size_t> {};
template <typename T>
struct neko::cpp::get_container_size_type<T[]> : std::type_identity<std::size_t> {};

namespace neko::cpp {
// TODO: list, forward_list, stack, queue

// RawArray -> Array -> Vector
// Stack -> PriorityQueue
// Tuple -> Array
// Tuple -> Pair
// MultiSet -> Set
// MultiSet -> MultiMap -> Map
// UnorderedMultiSet -> UnorderedSet
// UnorderedMultiSet -> UnorderedMultiMap -> UnorderedMap
enum class ContainerType {
    None,
    Array,
    Deque,
    ForwardList,
    List,
    Map,
    MultiMap,
    MultiSet,
    Optional,
    Pair,
    PriorityQueue,
    Queue,
    RawArray,
    Set,
    Span,
    Stack,
    Tuple,
    UnorderedMap,
    UnorderedMultiSet,
    UnorderedMultiMap,
    UnorderedSet,
    Variant,
    Vector
};

// SpecializeIsSet<std::set<...>> is std::true_type
template <typename T>
struct SpecializeIsSet : std::false_type {};
// traits
// ref : https://zh.cppreference.com/w/cpp/container
/*
template<typename T>
concept Is* = ...;
*/
}  // namespace neko::cpp

template <typename Key, typename Compare, typename Allocator>
struct neko::cpp::SpecializeIsSet<std::set<Key, Compare, Allocator>> : std::true_type {};

namespace neko::cpp {
template <typename T>
concept IsRawArray = true && container_begin<T&> && container_begin<const T&> && container_cbegin<T&>

                     && container_end<T&> && container_end<const T&> && container_cend<T&>

                     && container_rbegin<T&> && container_rbegin<const T&> && container_crbegin<T&>

                     && container_rend<T&> && container_rend<const T&> && container_crend<T&>

                     && container_subscript_size<T&> && container_subscript_size<const T&>

                     && container_data<T&> && container_data<const T&>

                     && container_empty<T> && container_size<T>

                     && container_swap<T&>;

template <typename T>
concept IsTuple = true && tuple_size<T>;

template <typename T>
concept IsArray = IsRawArray<T> && container_at_size<T> && container_at_size<const T>

                  && container_front<T> && container_front<const T>

                  && container_back<T> && container_back<const T>;

template <typename T>
concept IsVector = IsArray<T> && container_assign<T>

                   && container_resize_cnt<T> && container_resize_cnt_value<T> && container_capacity<T> && container_reserve<T> && container_shrink_to_fit<T>

                   && container_clear<T> && container_insert_citer_clvalue<T> && container_insert_citer_rvalue<T> && container_insert_citer_cnt<T> && container_erase_citer<T> &&
                   container_erase_range_citer<T> && container_push_back_clvalue<T> && container_push_back_rvalue<T> && container_pop_back<T>;

template <typename T>
concept IsDeque = true && container_assign<T>

                  && container_begin<T> && container_begin<const T> && container_cbegin<T>

                  && container_end<T> && container_end<const T> && container_cend<T>

                  && container_rbegin<T> && container_rbegin<const T> && container_crbegin<T>

                  && container_rend<T> && container_rend<const T> && container_crend<T>

                  && container_at_size<T> && container_at_size<const T>

                  && container_subscript_size<T> && container_subscript_size<const T>

                  && container_front<T> && container_front<const T>

                  && container_back<T> && container_back<const T>

                  && container_empty<T> && container_size<T>

                  && container_resize_cnt<T> && container_resize_cnt_value<T> && container_shrink_to_fit<T> && container_clear<T> && container_insert_citer_clvalue<T> &&
                  container_insert_citer_rvalue<T> && container_insert_citer_cnt<T> && container_erase_citer<T> && container_erase_range_citer<T> && container_push_front_clvalue<T> &&
                  container_push_front_rvalue<T> && container_pop_front<T> && container_push_back_clvalue<T> && container_push_back_rvalue<T> && container_pop_back<T>

                  && container_swap<T>;

template <typename T>
concept IsForwardList =
        true && container_assign<T>

        && container_begin<T> && container_begin<const T> && container_cbegin<T>

        && container_end<T> && container_end<const T> && container_cend<T>

        && container_front<T> && container_front<const T> && container_empty<T> && container_resize_cnt<T> && container_resize_cnt_value<T>

        && container_clear<T> && container_insert_after_clvalue<T> && container_insert_after_rvalue<T> && container_insert_after_cnt<T> && container_erase_after<T> && container_erase_after_range<T> &&
        container_push_front_clvalue<T> && container_push_front_rvalue<T> && container_pop_front<T> && container_swap<T> && container_merge_l<T> && container_merge_r<T>

        && container_splice_after_l<T> && container_splice_after_r<T> && container_splice_after_it_l<T> && container_splice_after_it_r<T> && container_splice_after_range_l<T> &&
        container_splice_after_range_r<T> && container_remove<T> && container_reverse<T> && container_unique<T> && container_sort<T>;

template <typename T>
concept IsList = true && container_assign<T>

                 && container_begin<T> && container_begin<const T> && container_cbegin<T>

                 && container_end<T> && container_end<const T> && container_cend<T>

                 && container_rbegin<T> && container_rbegin<const T> && container_crbegin<T>

                 && container_rend<T> && container_rend<const T> && container_crend<T>

                 && container_front<T> && container_front<const T> && container_back<T> && container_back<const T> && container_empty<T> && container_size<T> && container_resize_cnt<T> &&
                 container_resize_cnt_value<T>

                 && container_clear<T> && container_insert_citer_clvalue<T> && container_insert_citer_rvalue<T> && container_insert_citer_cnt<T> && container_erase_citer<T> &&
                 container_erase_range_citer<T> && container_push_front_clvalue<T> && container_push_front_rvalue<T> && container_pop_front<T> && container_push_back_clvalue<T> &&
                 container_push_back_rvalue<T> && container_pop_back<T> && container_swap<T> && container_merge_l<T> && container_merge_r<T>

                 && container_splice_l<T> && container_splice_r<T> && container_splice_it_l<T> && container_splice_it_r<T> && container_splice_range_l<T> && container_splice_range_r<T> &&
                 container_remove<T> && container_reverse<T> && container_unique<T> && container_sort<T>;

template <typename T>
concept IsMultiSet = true && container_begin<T> && container_begin<const T> && container_cbegin<T>

                     && container_end<T> && container_end<const T> && container_cend<T>

                     && container_rbegin<T> && container_rbegin<const T> && container_crbegin<T>

                     && container_rend<T> && container_rend<const T> && container_crend<T>

                     && container_empty<T> && container_size<T>

                     && container_clear<T> && container_insert_clvalue<T> && container_insert_rvalue<T> && container_insert_rnode<T> && container_insert_citer_clvalue<T> &&
                     container_insert_citer_rvalue<T> && container_insert_citer_rnode<T>

                     && container_erase_citer<T> && container_erase_key<T> && container_erase_range_citer<T>

                     && container_swap<T> && container_merge_l<T> && container_merge_r<T> && container_extract_citer<T> && container_extract_key<T>

                     && container_count<T> && container_find<T> && container_find<const T> && container_contains<T> && container_lower_bound<T> && container_lower_bound<const T> &&
                     container_upper_bound<T> && container_upper_bound<const T> && container_equal_range<T> && container_equal_range<const T>;

template <typename T>
concept IsSet = IsMultiSet<T> && SpecializeIsSet<T>::value;

template <typename T>
concept IsMultiMap = IsMultiSet<T> && container_mapped_type<T>;

template <typename T>
concept IsMap = IsMultiMap<T> && container_at_key<T> && container_at_key<const T>
        //&& container_subscript_key_cl<T>
        //&& container_subscript_key_r<T>
        ;

template <typename T>
concept IsUnorderedMultiSet = true && container_begin<T> && container_begin<const T> && container_cbegin<T>

                              && container_end<T> && container_end<const T> && container_cend<T>

                              && container_empty<T> && container_size<T> && container_bucket_count<T> && container_reserve<T>

                              && container_clear<T> && container_insert_clvalue<T> && container_insert_rvalue<T> && container_insert_rnode<T> && container_insert_citer_clvalue<T> &&
                              container_insert_citer_rvalue<T> && container_insert_citer_rnode<T>

                              && container_erase_citer<T> && container_erase_key<T> && container_erase_range_citer<T>

                              && container_swap<T> && container_merge_l<T> && container_merge_r<T> && container_extract_citer<T> && container_extract_key<T>

                              && container_count<T> && container_find<T> && container_find<const T> && container_contains<T> && container_equal_range<T> && container_equal_range<const T>;

template <typename T>
concept IsUnorderedSet = IsUnorderedMultiSet<T>;

template <typename T>
concept IsUnorderedMultiMap = IsUnorderedMultiSet<T> && container_mapped_type<T>;

template <typename T>
concept IsUnorderedMap = IsUnorderedMultiMap<T> && container_at_key<T> && container_at_key<const T>
        //&& container_subscript_key_cl<T>
        //&& container_subscript_key_r<T>
        ;

template <typename T>
concept IsPair = IsTuple<T> && pair_first<T> && pair_second<T>;

template <typename T>
concept IsStack = true && container_top<T> && container_top<const T> && container_empty<T> && container_size<T>

                  && container_push_clvalue<T> && container_push_rvalue<T> && container_pop<T> && container_swap<T>;

template <typename T>
concept IsPriorityQueue = IsStack<T> && requires { typename T::value_compare; };

template <typename T>
concept IsQueue = true && container_front<T> && container_front<const T> && container_back<T> && container_back<const T> && container_empty<T> && container_size<T>

                  && container_pop<T> && container_push_clvalue<T> && container_push_rvalue<T> && container_swap<T>;

template <typename T>
concept IsSpan = true && container_begin<T> && container_begin<const T>

                 && container_end<T> && container_end<const T>

                 && container_rbegin<T> && container_rbegin<const T>

                 && container_rend<T> && container_rend<const T>

                 && container_front<T> && container_front<const T>

                 && container_back<T> && container_back<const T>

                 && container_subscript_size<T> && container_subscript_size<const T>

                 && container_data<T> && container_data<const T>

                 && container_size<T> && container_size_bytes<T> && container_empty<T>;

template <typename T>
concept IsVariant = true && variant_size<T> && variant_index<T> && variant_valueless_by_exception<T>;

template <typename T>
concept IsOptional = true && container_value_type<T> && optional_has_value<T> && optional_value<T> && optional_value<const T> && optional_reset<T>;

template <typename T>
concept IsContainerType = false || IsRawArray<T> || IsDeque<T> || IsForwardList<T> || IsList<T> || IsMultiSet<T> || IsUnorderedMultiSet<T> || IsStack<T> || IsQueue<T> || IsTuple<T> || IsSpan<T> ||
                          IsVariant<T> || IsOptional<T>;
}  // namespace neko::cpp

namespace neko::cpp {
enum class MethodFlag {
    Variable = 0b001,
    Const = 0b010,
    Static = 0b100,

    None = 0b000,
    Member = 0b011,
    Priority = 0b101,
    All = 0b111
};
NEKO_DYREFL_ENUM_BOOL_OPERATOR_DEFINE(MethodFlag)

enum class FieldFlag {
    Basic = 0b00001,
    Virtual = 0b00010,
    Static = 0b00100,
    DynamicShared = 0b01000,
    DynamicBuffer = 0b10000,

    None = 0b00000,
    Owned = 0b00011,
    Unowned = 0b11100,
    All = 0b11111
};
NEKO_DYREFL_ENUM_BOOL_OPERATOR_DEFINE(FieldFlag)

using SharedBuffer = std::shared_ptr<void>;
class ObjectView;
class SharedObject;

template <typename T>
struct IsObjectOrView {
private:
    using U = std::remove_cvref_t<T>;

public:
    static constexpr bool value = std::is_same_v<U, ObjectView> || std::is_same_v<U, SharedObject>;
};
template <typename T>
constexpr bool IsObjectOrView_v = IsObjectOrView<T>::value;
template <typename T>
concept NonObjectAndView = !IsObjectOrView_v<T>;

template <typename T>
T MoveResult(Type type, void* result_buffer) noexcept(std::is_reference_v<T> || std::is_nothrow_destructible_v<T> && std::is_nothrow_move_constructible_v<T>) {
    if constexpr (!std::is_void_v<T>) {
        neko_assert(result_buffer);

        if constexpr (!std::is_reference_v<T> && std::is_default_constructible_v<T>) {
            if (type != Type_of<T>) return {};
        } else
            neko_assert(type == Type_of<T>);

        if constexpr (std::is_reference_v<T>)
            return std::forward<T>(*buffer_as<std::add_pointer_t<T>>(result_buffer));
        else {
            T rst = std::move(buffer_as<T>(result_buffer));
            if constexpr (std::is_compound_v<T> && !std::is_trivially_destructible_v<T>) reinterpret_cast<const T*>(result_buffer)->~T();
            return rst;
        }
    }
}

struct TypeInfo;
struct FieldInfo;
struct MethodInfo;
class BaseInfo;

class ObjectTree;
class VarRange;
class FieldRange;
class MethodRange;

class neko_refl;
}  // namespace neko::cpp

namespace neko::cpp {
// name must end with 0
// thread-safe
template <typename T, typename U>
class IDRegistry {
public:
    IDRegistry();

    void RegisterUnmanaged(T ID, std::string_view name);
    T RegisterUnmanaged(std::string_view name);
    std::string_view Register(T ID, std::string_view name);
    U Register(std::string_view name);

    bool IsRegistered(T ID) const;
    std::string_view Viewof(T ID) const;

    void UnregisterUnmanaged(T ID);
    void Clear() noexcept;

protected:
    std::pmr::polymorphic_allocator<char> get_allocator() { return &resource; }
    mutable std::shared_mutex smutex;

private:
    std::pmr::monotonic_buffer_resource resource;
    std::pmr::unordered_map<T, std::string_view> id2name;

#ifndef NDEBUG
public:
    bool IsUnmanaged(T ID) const;
    void ClearUnmanaged() noexcept;

private:
    std::pmr::unordered_set<T> unmanagedIDs;
#endif  // NDEBUG
};

class NameIDRegistry : public IDRegistry<name_id, Name> {
public:
    struct Meta {
        // operators

        static constexpr Name operator_bool{"__bool"};

        static constexpr Name operator_add{"__add"};
        static constexpr Name operator_sub{"__sub"};
        static constexpr Name operator_mul{"__mul"};
        static constexpr Name operator_div{"__div"};
        static constexpr Name operator_mod{"__mod"};

        static constexpr Name operator_bnot{"__bnot"};
        static constexpr Name operator_band{"__band"};
        static constexpr Name operator_bor{"__bor"};
        static constexpr Name operator_bxor{"__bxor"};
        static constexpr Name operator_shl{"__shl"};
        static constexpr Name operator_shr{"__shr"};

        static constexpr Name operator_pre_inc{"__pre_inc"};
        static constexpr Name operator_pre_dec{"__pre_dec"};
        static constexpr Name operator_post_inc{"__post_inc"};
        static constexpr Name operator_post_dec{"__post_dec"};

        static constexpr Name operator_assignment{"__assignment"};
        static constexpr Name operator_assignment_add{"__assignment_add"};
        static constexpr Name operator_assignment_sub{"__assignment_sub"};
        static constexpr Name operator_assignment_mul{"__assignment_mul"};
        static constexpr Name operator_assignment_div{"__assignment_div"};
        static constexpr Name operator_assignment_mod{"__assignment_mod"};
        static constexpr Name operator_assignment_band{"__assignment_band"};
        static constexpr Name operator_assignment_bor{"__assignment_bor"};
        static constexpr Name operator_assignment_bxor{"__assignment_bxor"};
        static constexpr Name operator_assignment_shl{"__assignment_shl"};
        static constexpr Name operator_assignment_shr{"__assignment_shr"};

        static constexpr Name operator_eq{"__eq"};
        static constexpr Name operator_ne{"__ne"};
        static constexpr Name operator_lt{"__lt"};
        static constexpr Name operator_le{"__le"};
        static constexpr Name operator_gt{"__gt"};
        static constexpr Name operator_ge{"__ge"};

        static constexpr Name operator_and{"__and"};
        static constexpr Name operator_or{"__or"};
        static constexpr Name operator_not{"__not"};

        static constexpr Name operator_subscript{"__subscript"};
        static constexpr Name operator_indirection{"__indirection"};

        static constexpr Name operator_call{"__call"};

        // non-member functions

        static constexpr Name ctor{"__ctor"};
        static constexpr Name dtor{"__dtor"};

        static constexpr Name get{"__get"};
        static constexpr Name variant_visit_get{"__variant_visit_get"};  // std::visit + std::get

        static constexpr Name tuple_size{"__tuple_size"};
        static constexpr Name tuple_element{"__tuple_element"};

        static constexpr Name get_if{"__get_if"};
        static constexpr Name holds_alternative{"__holds_alternative"};
        static constexpr Name variant_size{"__variant_size"};
        static constexpr Name variant_alternative{"__variant_alternative"};

        static constexpr Name advance{"__advance"};
        static constexpr Name distance{"__distance"};
        static constexpr Name next{"__next"};
        static constexpr Name prev{"__prev"};

        // member functions

        static constexpr Name container_assign{"assign"};

        static constexpr Name container_begin{"begin"};
        static constexpr Name container_cbegin{"cbegin"};
        static constexpr Name container_end{"end"};
        static constexpr Name container_cend{"cend"};
        static constexpr Name container_rbegin{"rbegin"};
        static constexpr Name container_crbegin{"crbegin"};
        static constexpr Name container_rend{"rend"};
        static constexpr Name container_crend{"crend"};

        static constexpr Name container_at{"at"};
        static constexpr Name container_data{"data"};
        static constexpr Name container_front{"front"};
        static constexpr Name container_back{"back"};
        static constexpr Name container_top{"top"};

        static constexpr Name container_empty{"empty"};
        static constexpr Name container_size{"size"};
        static constexpr Name container_size_bytes{"size_bytes"};
        static constexpr Name container_resize{"resize"};
        static constexpr Name container_capacity{"capacity"};
        static constexpr Name container_bucket_count{"bucket_count"};
        static constexpr Name container_reserve{"reserve"};
        static constexpr Name container_shrink_to_fit{"shrink_to_fit"};

        static constexpr Name container_clear{"clear"};
        static constexpr Name container_insert{"insert"};
        static constexpr Name container_insert_after{"insert_after"};
        static constexpr Name container_insert_or_assign{"insert_or_assign"};
        static constexpr Name container_erase{"erase"};
        static constexpr Name container_erase_after{"erase_after"};
        static constexpr Name container_push_front{"push_front"};
        static constexpr Name container_pop_front{"pop_front"};
        static constexpr Name container_push_back{"push_back"};
        static constexpr Name container_pop_back{"pop_back"};
        static constexpr Name container_push{"push"};
        static constexpr Name container_pop{"pop"};
        static constexpr Name container_swap{"swap"};
        static constexpr Name container_merge{"merge"};
        static constexpr Name container_extract{"extract"};

        static constexpr Name container_splice_after{"splice_after"};
        static constexpr Name container_splice{"splice"};
        static constexpr Name container_remove{"remove"};
        static constexpr Name container_reverse{"reverse"};
        static constexpr Name container_unique{"unique"};
        static constexpr Name container_sort{"sort"};

        static constexpr Name container_count{"count"};
        static constexpr Name container_find{"find"};
        static constexpr Name container_lower_bound{"lower_bound"};
        static constexpr Name container_upper_bound{"upper_bound"};
        static constexpr Name container_equal_range{"equal_range"};

        static constexpr Name variant_index{"index"};
        static constexpr Name variant_valueless_by_exception{"valueless_by_exception"};

        static constexpr Name optional_has_value{"has_value"};
        static constexpr Name optional_value{"value"};
        static constexpr Name optional_reset{"reset"};
    };

    NameIDRegistry();

    using IDRegistry<name_id, Name>::Register;

    Name Register(Name n) { return Register(n.get_id(), n.get_view()); }
    Name Nameof(name_id ID) const;
};

class TypeIDRegistry : public IDRegistry<TypeID, Type> {
public:
    struct Meta {
        static constexpr Type global{"__global"};
    };

    using IDRegistry<TypeID, Type>::Register;
    using IDRegistry<TypeID, Type>::IsRegistered;

    TypeIDRegistry();

    // unmanaged
    template <typename T>
    void Register();

    Type Register(Type n) { return Register(n.get_id(), n.get_name()); }

    template <typename T>
    bool IsRegistered() const;

    Type Typeof(TypeID ID) const;

    //
    // Type Computation
    /////////////////////

    Type RegisterAddConst(Type type);
    Type RegisterAddLValueReference(Type type);
    Type RegisterAddLValueReferenceWeak(Type type);
    Type RegisterAddRValueReference(Type type);
    Type RegisterAddConstLValueReference(Type type);
    Type RegisterAddConstRValueReference(Type type);
};
}  // namespace neko::cpp

namespace neko::cpp {
template <typename T, typename U>
IDRegistry<T, U>::IDRegistry()
    : id2name{&resource}
#ifndef NDEBUG
      ,
      unmanagedIDs{&resource}
#endif  // !NDEBUG
{
}

template <typename T, typename U>
void IDRegistry<T, U>::RegisterUnmanaged(T ID, std::string_view name) {
    neko_assert(!name.empty());

    std::shared_lock rlock{smutex};  // read id2name
    auto target = id2name.find(ID);
    if (target != id2name.end()) {
        neko_assert(target->second == name);
        return;
    }
    rlock.unlock();

    neko_assert(name.data() && name.data()[name.size()] == 0);

    std::lock_guard wlock{smutex};           // write id2name, [DEBUG] unmanagedIDs
    id2name.emplace_hint(target, ID, name);  // target is thread-safe

#ifndef NDEBUG
    unmanagedIDs.insert(ID);
#endif  // !NDEBUG
}

template <typename T, typename U>
T IDRegistry<T, U>::RegisterUnmanaged(std::string_view name) {
    T ID{name};
    RegisterUnmanaged(ID, name);
    return ID;
}

template <typename T, typename U>
std::string_view IDRegistry<T, U>::Register(T ID, std::string_view name) {
    neko_assert(!name.empty());

    std::shared_lock rlock{smutex};  // read id2name
    auto target = id2name.find(ID);
    if (target != id2name.end()) {
        neko_assert(target->second == name);
        return target->second;
    }
    rlock.unlock();

    neko_assert(name.data() && name.data()[name.size()] == 0);

    std::lock_guard wlock{smutex};  // write resource, id2name
    auto buffer = reinterpret_cast<char*>(resource.allocate(name.size() + 1, alignof(char)));
    std::memcpy(buffer, name.data(), name.size());
    buffer[name.size()] = 0;

    std::string_view new_name{buffer, name.size()};

    id2name.emplace_hint(target, ID, new_name);  // target is thread-safe

#ifndef NDEBUG
    unmanagedIDs.erase(ID);
#endif  // !NDEBUG

    return new_name;
}

template <typename T, typename U>
U IDRegistry<T, U>::Register(std::string_view name) {
    T ID{name};
    auto new_name = Register(ID, name);
    return {new_name, ID};
}

template <typename T, typename U>
void IDRegistry<T, U>::UnregisterUnmanaged(T ID) {
    std::shared_lock rlock{smutex};  // read id2name
    auto target = id2name.find(ID);
    if (target == id2name.end()) return;
    rlock.unlock();

    neko_assert(IsUnmanaged(ID));

    std::lock_guard wlock{smutex};
    id2name.erase(target);  // target is thread-safe
}

template <typename T, typename U>
void IDRegistry<T, U>::Clear() noexcept {
    std::lock_guard wlock{smutex};

    id2name.clear();
#ifndef NDEBUG
    unmanagedIDs.clear();
#endif  // !NDEBUG
    resource.release();
}

#ifndef NDEBUG
template <typename T, typename U>
bool IDRegistry<T, U>::IsUnmanaged(T ID) const {
    std::shared_lock rlock{smutex};
    return unmanagedIDs.find(ID) != unmanagedIDs.end();
}

template <typename T, typename U>
void IDRegistry<T, U>::ClearUnmanaged() noexcept {
    std::lock_guard wlock{smutex};

    for (const auto& ID : unmanagedIDs) id2name.erase(ID);
    unmanagedIDs.clear();
}
#endif  // !NDEBUG

template <typename T, typename U>
bool IDRegistry<T, U>::IsRegistered(T ID) const {
    std::shared_lock rlock{smutex};
    return id2name.contains(ID);
}

template <typename T, typename U>
std::string_view IDRegistry<T, U>::Viewof(T ID) const {
    std::shared_lock rlock{smutex};
    if (auto target = id2name.find(ID); target != id2name.end()) return target->second;

    return {};
}

template <typename T>
void TypeIDRegistry::Register() {
    IDRegistry<TypeID, Type>::RegisterUnmanaged(TypeID_of<T>, type_name<T>());
}

template <typename T>
bool TypeIDRegistry::IsRegistered() const {
    static_assert(!std::is_volatile_v<T>);
    return IDRegistry<TypeID, Type>::IsRegistered(TypeID_of<T>);
}
}  // namespace neko::cpp

namespace neko::cpp {
// pointer const array type (pointer is const, and pointer to non - const / referenced object)
using ArgPtrBuffer = void* const*;

class ArgsView {
public:
    constexpr ArgsView() noexcept : buffer{nullptr} {}
    constexpr ArgsView(ArgPtrBuffer buffer, std::span<const Type> argTypes) noexcept : buffer{buffer}, argTypes{argTypes} {}
    constexpr ArgPtrBuffer Buffer() const noexcept { return buffer; }
    constexpr std::span<const Type> Types() const noexcept { return argTypes; }
    constexpr ObjectView operator[](size_t idx) const noexcept;

private:
    ArgPtrBuffer buffer;
    std::span<const Type> argTypes;
};

template <std::size_t N>
class TempArgsView {
public:
    template <typename... Args>
    TempArgsView(Args&&... args) noexcept;
    operator ArgsView() const&& noexcept { return {argptr_buffer, argTypes}; }

private:
    const Type argTypes[N];
    void* const argptr_buffer[N];
};
template <typename... Args>
TempArgsView(Args&&... args) -> TempArgsView<sizeof...(Args)>;

std::pmr::memory_resource* ReflMngr_GetTemporaryResource();

class ObjectView {
public:
    constexpr ObjectView() noexcept : ptr{nullptr} {}
    constexpr ObjectView(Type type, void* ptr) noexcept : type{type}, ptr{ptr} {}
    explicit constexpr ObjectView(Type type) noexcept : ObjectView{type, nullptr} {}
    template <typename T>
        requires std::negation_v<std::is_reference<T>> && std::negation_v<std::is_same<std::remove_cvref_t<T>, Type>> && NonObjectAndView<T>
    constexpr explicit ObjectView(T& obj) noexcept : ObjectView{Type_of<T>, const_cast<void*>(static_cast<const void*>(&obj))} {}

    constexpr const Type& GetType() const noexcept { return type; }
    constexpr void* const& GetPtr() const noexcept { return ptr; }

    explicit operator bool() const noexcept;

    template <typename T>
    constexpr auto* AsPtr() const noexcept;

    template <typename T>
    constexpr decltype(auto) As() const noexcept;

    //////////////
    // ReflMngr //
    //////////////

    //
    // Cast
    /////////

    ObjectView StaticCast_DerivedToBase(Type base) const;
    ObjectView StaticCast_BaseToDerived(Type derived) const;
    ObjectView DynamicCast_BaseToDerived(Type derived) const;
    ObjectView StaticCast(Type type) const;
    ObjectView DynamicCast(Type type) const;

    //
    // Invoke
    ///////////

    Type IsInvocable(Name method_name, std::span<const Type> argTypes = {}, MethodFlag flag = MethodFlag::All) const;

    Type BInvoke(Name method_name, void* result_buffer = nullptr, ArgsView args = {}, MethodFlag flag = MethodFlag::All,
                 std::pmr::memory_resource* temp_args_rsrc = ReflMngr_GetTemporaryResource()) const;

    SharedObject MInvoke(Name method_name, std::pmr::memory_resource* rst_rsrc, ArgsView args = {}, MethodFlag flag = MethodFlag::All,
                         std::pmr::memory_resource* temp_args_rsrc = ReflMngr_GetTemporaryResource()) const;

    SharedObject Invoke(Name method_name, ArgsView args = {}, MethodFlag flag = MethodFlag::All, std::pmr::memory_resource* temp_args_rsrc = ReflMngr_GetTemporaryResource()) const;

    // -- template --

    template <typename... Args>
    Type IsInvocable(Name method_name, MethodFlag flag = MethodFlag::All) const;

    template <typename T>
    T Invoke(Name method_name, ArgsView args = {}, MethodFlag flag = MethodFlag::All, std::pmr::memory_resource* temp_args_rsrc = ReflMngr_GetTemporaryResource()) const;

    //
    // Var
    ////////

    ObjectView Var(Name field_name, FieldFlag flag = FieldFlag::All) const;
    // for diamond inheritance
    ObjectView Var(Type base, Name field_name, FieldFlag flag = FieldFlag::All) const;

    //
    // Type
    //////////

    constexpr ObjectView RemoveConst() const noexcept;
    constexpr ObjectView RemoveLValueReference() const noexcept;
    constexpr ObjectView RemoveRValueReference() const noexcept;
    constexpr ObjectView RemoveReference() const noexcept;
    constexpr ObjectView RemoveConstReference() const noexcept;

    ObjectView AddConst() const;
    ObjectView AddLValueReference() const;
    ObjectView AddLValueReferenceWeak() const;
    ObjectView AddRValueReference() const;
    ObjectView AddConstLValueReference() const;
    ObjectView AddConstRValueReference() const;

    //
    // Ranges
    ///////////

    ObjectTree GetObjectTree() const;
    MethodRange GetMethods(MethodFlag flag = MethodFlag::All) const;
    FieldRange GetFields(FieldFlag flag = FieldFlag::All) const;
    VarRange GetVars(FieldFlag flag = FieldFlag::All) const;

    //////////
    // Meta //
    //////////

    //
    // operators
    //////////////

    template <typename T>
    SharedObject operator+(T&& rhs) const;
    template <typename T>
    SharedObject operator-(T&& rhs) const;
    template <typename T>
    SharedObject operator*(T&& rhs) const;
    template <typename T>
    SharedObject operator/(T&& rhs) const;
    template <typename T>
    SharedObject operator%(T&& rhs) const;

    template <typename T>
    SharedObject operator&(T&& rhs) const;
    template <typename T>
    SharedObject operator|(T&& rhs) const;
    template <typename T>
    SharedObject operator^(T&& rhs) const;
    template <typename T>
    SharedObject operator<<(T&& rhs) const;
    template <typename T>
    SharedObject operator>>(T&& rhs) const;

    template <typename T>
    bool operator<(const T& rhs) const;
    template <typename T>
    bool operator<=(const T& rhs) const;
    template <typename T>
    bool operator>(const T& rhs) const;
    template <typename T>
    bool operator>=(const T& rhs) const;

    template <typename T>
        requires NonObjectAndView<std::decay_t<T>>
    ObjectView operator=(T&& rhs) const;

    template <typename T>
    ObjectView operator+=(T&& rhs) const;
    template <typename T>
    ObjectView operator-=(T&& rhs) const;
    template <typename T>
    ObjectView operator*=(T&& rhs) const;
    template <typename T>
    ObjectView operator/=(T&& rhs) const;
    template <typename T>
    ObjectView operator%=(T&& rhs) const;
    template <typename T>
    ObjectView operator&=(T&& rhs) const;
    template <typename T>
    ObjectView operator|=(T&& rhs) const;
    template <typename T>
    ObjectView operator^=(T&& rhs) const;
    template <typename T>
    ObjectView operator<<=(T&& rhs) const;
    template <typename T>
    ObjectView operator>>=(T&& rhs) const;

    SharedObject operator++() const;
    SharedObject operator++(int) const;
    SharedObject operator--() const;
    SharedObject operator--(int) const;
    SharedObject operator+() const;
    SharedObject operator-() const;
    SharedObject operator~() const;
    SharedObject operator*() const;

    template <typename T>
    SharedObject operator[](T&& rhs) const;

    template <typename... Args>
    SharedObject operator()(Args&&... args) const;

    //
    // attrs
    //////////

    ContainerType get_container_type() const;

    //
    // non-member functions
    /////////////////////////

    // tuple & variant

    ObjectView get(std::size_t i) const;
    ObjectView get(Type type) const;

    // - tuple

    std::size_t tuple_size() const;
    Type tuple_element(std::size_t i) const;

    // - variant

    std::size_t variant_size() const;
    Type variant_alternative(std::size_t i) const;
    ObjectView variant_visit_get() const;

    // - iterator

    template <typename T>
    void advance(T&& arg) const;
    template <typename T>
    std::size_t distance(T&& arg) const;
    template <typename T>
    SharedObject next(T&& arg) const;
    template <typename T>
    SharedObject prev(T&& arg) const;
    SharedObject next() const;
    SharedObject prev() const;

    //
    // Container
    //////////////

    template <typename... Args>
    void assign(Args&&... args) const;

    // - iterator

    SharedObject begin() const;
    SharedObject end() const;
    SharedObject cbegin() const;
    SharedObject cend() const;
    SharedObject rbegin() const;
    SharedObject rend() const;
    SharedObject crbegin() const;
    SharedObject crend() const;

    // - capacity

    bool empty() const;
    std::size_t size() const;
    std::size_t capacity() const;
    std::size_t bucket_count() const;
    template <typename... Args>
    void resize(Args&&... args) const;
    void reserve(std::size_t n) const;
    void shrink_to_fit() const;

    // - element access

    template <typename T>
    SharedObject at(T&& rhs) const;
    SharedObject data() const;
    SharedObject front() const;
    SharedObject back() const;
    SharedObject top() const;

    // - modifiers

    void clear() const;
    template <typename... Args>
    SharedObject insert(Args&&... args) const;
    template <typename... Args>
    SharedObject insert_after(Args&&... args) const;
    template <typename... Args>
    SharedObject insert_or_assign(Args&&... args) const;
    template <typename... Args>
    SharedObject erase(Args&&... args) const;
    template <typename... Args>
    SharedObject erase_after(Args&&... args) const;
    template <typename T>
    void push_front(T&& arg) const;
    template <typename T>
    void push_back(T&& arg) const;
    void pop_front() const;
    void pop_back() const;
    template <typename T>
    void push(T&& arg) const;
    void pop() const;
    template <typename T>
    void swap(T&& arg) const;
    template <typename T>
    void merge(T&& arg) const;
    template <typename T>
    SharedObject extract(T&& rhs) const;

    // - list operations

    template <typename... Args>
    void splice_after(Args&&... args) const;
    template <typename... Args>
    void splice(Args&&... args) const;
    template <typename T>
    std::size_t remove(T&& arg) const;
    void reverse() const;
    template <typename T>
    std::size_t unique(T&& arg) const;
    void sort() const;

    // - lookup

    template <typename T>
    std::size_t count(T&& arg) const;
    template <typename T>
    SharedObject find(T&& rhs) const;
    template <typename T>
    SharedObject lower_bound(T&& rhs) const;
    template <typename T>
    SharedObject upper_bound(T&& rhs) const;
    template <typename T>
    SharedObject equal_range(T&& rhs) const;

    // - variant

    std::size_t index() const;
    bool holds_alternative(Type type) const;

    // - optional

    bool has_value() const;
    ObjectView value() const;
    void reset() const;

protected:
    Type type;
    void* ptr;  // if type is reference, ptr is a pointer of referenced object
};

class SharedObject : public ObjectView {
public:
    using ObjectView::ObjectView;
    using ObjectView::operator=;

    SharedObject(Type type, SharedBuffer buffer) noexcept : ObjectView{type}, buffer{std::move(buffer)} { ptr = this->buffer.get(); }
    constexpr explicit SharedObject(ObjectView obj) noexcept : ObjectView{obj} {}

    template <typename T>
    SharedObject(Type type, std::shared_ptr<T> buffer) noexcept : ObjectView{type, buffer.get()}, buffer{std::move(buffer)} {}

    template <typename T>
    explicit SharedObject(std::unique_ptr<T>&& buffer) noexcept : ObjectView{Type_of<T>, buffer.get()}, buffer{std::move(buffer)} {}

    template <typename Deleter>
    SharedObject(ObjectView obj, Deleter d) noexcept : ObjectView{obj}, buffer{obj.GetPtr(), std::move(d)} {}
    template <typename U, typename Deleter, typename Alloc>
    SharedObject(ObjectView obj, Deleter d, Alloc alloc) noexcept : ObjectView{obj}, buffer{obj.GetPtr(), std::move(d), alloc} {}

    // set pointer to nullptr
    void Reset() noexcept {
        ptr = nullptr;
        buffer.reset();
    }

    SharedBuffer& GetBuffer() noexcept { return buffer; }
    const SharedBuffer& GetBuffer() const noexcept { return buffer; }

    long UseCount() const noexcept { return buffer.use_count(); }

    bool IsObjectView() const noexcept { return !buffer; }

    void swap(SharedObject& rhs) noexcept {
        std::swap(type, rhs.type);
        std::swap(ptr, rhs.ptr);
        buffer.swap(rhs.buffer);
    }

    template <typename T>
    auto AsShared() const;

    //////////////
    // ReflMngr //
    //////////////

    //
    // Cast
    /////////

    SharedObject StaticCast_DerivedToBase(Type base) const;
    SharedObject StaticCast_BaseToDerived(Type derived) const;
    SharedObject DynamicCast_BaseToDerived(Type derived) const;
    SharedObject StaticCast(Type type) const;
    SharedObject DynamicCast(Type type) const;

private:
    SharedBuffer buffer;  // if type is reference/void, buffer is empty
};

template <typename T>
constexpr ObjectView ObjectView_of = {Type_of<T>, nullptr};
}  // namespace neko::cpp

#define OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(op, name)                                                                \
    template <typename T>                                                                                              \
    ObjectView ObjectView::operator op(T&& rhs) const {                                                                \
        Invoke<void>(NameIDRegistry::Meta::operator_##name, TempArgsView{std::forward<T>(rhs)}, MethodFlag::Variable); \
        return AddLValueReference();                                                                                   \
    }

#define OBJECT_VIEW_DEFINE_META_T(fname, mname)                                         \
    template <typename T>                                                               \
    SharedObject ObjectView::fname(T&& arg) const {                                     \
        return Invoke(NameIDRegistry::Meta::mname, TempArgsView{std::forward<T>(arg)}); \
    }

#define OBJECT_VIEW_DEFINE_OPERATOR_T(op, name) OBJECT_VIEW_DEFINE_META_T(operator op, operator_##name)

#define OBJECT_VIEW_DEFINE_CONTAINER_META_T(name) OBJECT_VIEW_DEFINE_META_T(name, container_##name)

#define OBJECT_VIEW_DEFINE_META_VARS_T(prefix, name)                                                     \
    template <typename... Args>                                                                          \
    SharedObject ObjectView::name(Args&&... args) const {                                                \
        return Invoke(NameIDRegistry::Meta::prefix##_##name, TempArgsView{std::forward<Args>(args)...}); \
    }

#define DEFINE_OPERATOR_LSHIFT(Lhs, Rhs)               \
    inline Lhs& operator<<(Lhs& lhs, const Rhs& rhs) { \
        rhs >> lhs;                                    \
        return lhs;                                    \
    }

#define DEFINE_OPERATOR_RSHIFT(Lhs, Rhs)               \
    inline Lhs& operator>>(Lhs& lhs, const Rhs& rhs) { \
        rhs << lhs;                                    \
        return lhs;                                    \
    }

namespace neko::cpp::details {
template <typename T>
constexpr Type ArgType(const std::remove_const_t<std::remove_reference_t<T>>& arg) noexcept {
    using U = std::remove_cvref_t<T>;
    if constexpr (std::is_same_v<U, ObjectView> || std::is_same_v<U, SharedObject>)
        return ObjectView{arg}.AddLValueReferenceWeak().GetType();
    else
        return Type_of<T>;
}

template <typename T>
constexpr void* ArgPtr(const T& arg) noexcept {
    if constexpr (std::is_same_v<T, ObjectView> || std::is_same_v<T, SharedObject>)
        return arg.GetPtr();
    else
        return const_cast<void*>(static_cast<const void*>(&arg));
}
}  // namespace neko::cpp::details

namespace neko::cpp {
constexpr ObjectView ArgsView::operator[](size_t idx) const noexcept { return {argTypes[idx], buffer[idx]}; }

template <std::size_t N>
template <typename... Args>
TempArgsView<N>::TempArgsView(Args&&... args) noexcept : argTypes{details::ArgType<decltype(args)>(args)...}, argptr_buffer{details::ArgPtr(args)...} {
    static_assert(sizeof...(Args) == N);
}

template <typename T>
constexpr auto* ObjectView::AsPtr() const noexcept {
    neko_assert(type.Is<T>());
    return reinterpret_cast<std::add_pointer_t<T>>(ptr);
}

template <typename T>
constexpr decltype(auto) ObjectView::As() const noexcept {
    neko_assert(ptr);
    auto* ptr = AsPtr<T>();
    if constexpr (std::is_reference_v<T>)
        return std::forward<T>(*ptr);
    else
        return *ptr;
}

template <typename T>
auto SharedObject::AsShared() const {
    static_assert(!std::is_reference_v<T>);
    neko_assert(!IsObjectView());
    neko_assert(GetType().template Is<T>());
    return std::reinterpret_pointer_cast<T>(buffer);
}

//////////////
// ReflMngr //
//////////////

constexpr ObjectView ObjectView::RemoveConst() const noexcept { return {type.RemoveConst(), ptr}; }
constexpr ObjectView ObjectView::RemoveLValueReference() const noexcept { return {type.RemoveLValueReference(), ptr}; }
constexpr ObjectView ObjectView::RemoveRValueReference() const noexcept { return {type.RemoveRValueReference(), ptr}; }
constexpr ObjectView ObjectView::RemoveReference() const noexcept { return {type.RemoveReference(), ptr}; }
constexpr ObjectView ObjectView::RemoveConstReference() const noexcept { return {type.RemoveCVRef(), ptr}; }

template <typename... Args>
Type ObjectView::IsInvocable(Name method_name, MethodFlag flag) const {
    if constexpr (sizeof...(Args) > 0) {
        constexpr Type argTypes[] = {Type_of<Args>...};
        return IsInvocable(method_name, argTypes, flag);
    } else
        return IsInvocable(method_name, {}, flag);
}

template <typename T>
T ObjectView::Invoke(Name method_name, ArgsView args, MethodFlag flag, std::pmr::memory_resource* temp_args_rsrc) const {
    if constexpr (!std::is_void_v<T>) {
        using U = std::conditional_t<std::is_reference_v<T>, std::add_pointer_t<T>, T>;
        std::aligned_storage_t<sizeof(U), alignof(U)> result_buffer;
        Type result_type = BInvoke(method_name, &result_buffer, args, flag, temp_args_rsrc);
        return MoveResult<T>(result_type, &result_buffer);
    } else
        BInvoke(method_name, nullptr, args, flag, temp_args_rsrc);
}

//////////
// Meta //
//////////

//
// operators
//////////////

inline SharedObject ObjectView::operator++() const { return Invoke(NameIDRegistry::Meta::operator_pre_inc); }
inline SharedObject ObjectView::operator++(int) const { return Invoke(NameIDRegistry::Meta::operator_post_inc); }
inline SharedObject ObjectView::operator--() const { return Invoke(NameIDRegistry::Meta::operator_pre_dec); }
inline SharedObject ObjectView::operator--(int) const { return Invoke(NameIDRegistry::Meta::operator_post_dec); }
inline SharedObject ObjectView::operator+() const { return Invoke(NameIDRegistry::Meta::operator_add); }
inline SharedObject ObjectView::operator-() const { return Invoke(NameIDRegistry::Meta::operator_sub); }
inline SharedObject ObjectView::operator~() const { return Invoke(NameIDRegistry::Meta::operator_bnot); }
inline SharedObject ObjectView::operator*() const { return Invoke(NameIDRegistry::Meta::operator_indirection); }

OBJECT_VIEW_DEFINE_OPERATOR_T(+, add)
OBJECT_VIEW_DEFINE_OPERATOR_T(-, sub)
OBJECT_VIEW_DEFINE_OPERATOR_T(*, mul)
OBJECT_VIEW_DEFINE_OPERATOR_T(/, div)
OBJECT_VIEW_DEFINE_OPERATOR_T(%, mod)
OBJECT_VIEW_DEFINE_OPERATOR_T(&, band)
OBJECT_VIEW_DEFINE_OPERATOR_T(|, bor)
OBJECT_VIEW_DEFINE_OPERATOR_T(^, bxor)
OBJECT_VIEW_DEFINE_OPERATOR_T(<<, shl)
OBJECT_VIEW_DEFINE_OPERATOR_T(>>, shr)

OBJECT_VIEW_DEFINE_OPERATOR_T([], subscript)

template <typename T>
bool ObjectView::operator<(const T& rhs) const {
    return Invoke<bool>(NameIDRegistry::Meta::operator_lt, TempArgsView{rhs}, MethodFlag::Const);
}
template <typename T>
bool ObjectView::operator<=(const T& rhs) const {
    return Invoke<bool>(NameIDRegistry::Meta::operator_le, TempArgsView{rhs}, MethodFlag::Const);
}
template <typename T>
bool ObjectView::operator>(const T& rhs) const {
    return Invoke<bool>(NameIDRegistry::Meta::operator_gt, TempArgsView{rhs}, MethodFlag::Const);
}
template <typename T>
bool ObjectView::operator>=(const T& rhs) const {
    return Invoke<bool>(NameIDRegistry::Meta::operator_ge, TempArgsView{rhs}, MethodFlag::Const);
}

template <typename T>
    requires NonObjectAndView<std::decay_t<T>>
ObjectView ObjectView::operator=(T&& rhs) const {
    Invoke<void>(NameIDRegistry::Meta::operator_assignment, TempArgsView{std::forward<T>(rhs)}, MethodFlag::Variable);
    return AddLValueReference();
}

OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(+=, assignment_add);
OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(-=, assignment_sub);
OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(*=, assignment_mul);
OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(/=, assignment_div);
OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(%=, assignment_mod);
OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(&=, assignment_band);
OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(|=, assignment_bor);
OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(^=, assignment_bxor);
OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(<<=, assignment_shl);
OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR(>>=, assignment_shr);

template <typename... Args>
SharedObject ObjectView::operator()(Args&&... args) const {
    return AInvoke(NameIDRegistry::Meta::operator_call, TempArgsView{std::forward<Args>(args)...});
}

//
// non-member functions
/////////////////////////

inline ObjectView ObjectView::get(std::size_t i) const { return Invoke<ObjectView>(NameIDRegistry::Meta::get, TempArgsView{std::move(i)}, MethodFlag::Member); }
inline ObjectView ObjectView::get(Type type) const { return Invoke<ObjectView>(NameIDRegistry::Meta::get, TempArgsView{std::move(type)}, MethodFlag::Member); }
inline ObjectView ObjectView::variant_visit_get() const { return Invoke<ObjectView>(NameIDRegistry::Meta::variant_visit_get, ArgsView{}, MethodFlag::Member); }

inline std::size_t ObjectView::tuple_size() const { return Invoke<std::size_t>(NameIDRegistry::Meta::tuple_size, ArgsView{}, MethodFlag::Static); }
inline Type ObjectView::tuple_element(std::size_t i) const { return Invoke<Type>(NameIDRegistry::Meta::tuple_element, TempArgsView{std::move(i)}, MethodFlag::Static); }

inline std::size_t ObjectView::variant_size() const { return Invoke<std::size_t>(NameIDRegistry::Meta::variant_size, ArgsView{}, MethodFlag::Static); }
inline Type ObjectView::variant_alternative(std::size_t i) const { return Invoke<Type>(NameIDRegistry::Meta::variant_alternative, TempArgsView{std::move(i)}, MethodFlag::Static); }

template <typename T>
void ObjectView::advance(T&& arg) const {
    Invoke<void>(NameIDRegistry::Meta::advance, TempArgsView{std::forward<T>(arg)}, MethodFlag::Variable);
};
template <typename T>
std::size_t ObjectView::distance(T&& arg) const {
    return Invoke<std::size_t>(NameIDRegistry::Meta::distance, TempArgsView{std::forward<T>(arg)}, MethodFlag::Const);
};
OBJECT_VIEW_DEFINE_META_T(next, next);
OBJECT_VIEW_DEFINE_META_T(prev, prev);
inline SharedObject ObjectView::next() const { return Invoke(NameIDRegistry::Meta::next); }
inline SharedObject ObjectView::prev() const { return Invoke(NameIDRegistry::Meta::prev); }

//
// member functions
/////////////////////

template <typename... Args>
void ObjectView::assign(Args&&... args) const {
    Invoke<void>(NameIDRegistry::Meta::container_assign, TempArgsView{std::forward<Args>(args)...}, MethodFlag::Variable);
};

// - element access

OBJECT_VIEW_DEFINE_CONTAINER_META_T(at)
inline SharedObject ObjectView::data() const { return Invoke(NameIDRegistry::Meta::container_data); }
inline SharedObject ObjectView::front() const { return Invoke(NameIDRegistry::Meta::container_front); }
inline SharedObject ObjectView::back() const { return Invoke(NameIDRegistry::Meta::container_back); }
inline SharedObject ObjectView::top() const { return Invoke(NameIDRegistry::Meta::container_top); }

// - iterator

inline SharedObject ObjectView::begin() const { return Invoke(NameIDRegistry::Meta::container_begin); }
inline SharedObject ObjectView::end() const { return Invoke(NameIDRegistry::Meta::container_end); }
inline SharedObject ObjectView::rbegin() const { return Invoke(NameIDRegistry::Meta::container_rbegin); }
inline SharedObject ObjectView::rend() const { return Invoke(NameIDRegistry::Meta::container_rend); }
inline SharedObject ObjectView::cbegin() const { return Invoke(NameIDRegistry::Meta::container_cbegin); }
inline SharedObject ObjectView::cend() const { return Invoke(NameIDRegistry::Meta::container_cend); }
inline SharedObject ObjectView::crbegin() const { return Invoke(NameIDRegistry::Meta::container_crbegin); }
inline SharedObject ObjectView::crend() const { return Invoke(NameIDRegistry::Meta::container_crend); }

// - capacity

inline bool ObjectView::empty() const { return Invoke<bool>(NameIDRegistry::Meta::container_empty, ArgsView{}, MethodFlag::Const); }
inline std::size_t ObjectView::size() const { return Invoke<std::size_t>(NameIDRegistry::Meta::container_size, ArgsView{}, MethodFlag::Const); }
inline std::size_t ObjectView::capacity() const { return Invoke<std::size_t>(NameIDRegistry::Meta::container_capacity, ArgsView{}, MethodFlag::Const); }
inline std::size_t ObjectView::bucket_count() const { return Invoke<std::size_t>(NameIDRegistry::Meta::container_bucket_count, ArgsView{}, MethodFlag::Const); }
template <typename... Args>
void ObjectView::resize(Args&&... args) const {
    Invoke<void>(NameIDRegistry::Meta::container_resize, TempArgsView{std::forward<Args>(args)...}, MethodFlag::Variable);
};
inline void ObjectView::reserve(std::size_t n) const { Invoke<void>(NameIDRegistry::Meta::container_reserve, TempArgsView{std::move(n)}, MethodFlag::Variable); }
inline void ObjectView::shrink_to_fit() const { Invoke<void>(NameIDRegistry::Meta::container_shrink_to_fit, ArgsView{}, MethodFlag::Variable); }

// - modifiers

inline void ObjectView::clear() const { Invoke<void>(NameIDRegistry::Meta::container_clear, ArgsView{}, MethodFlag::Variable); }
OBJECT_VIEW_DEFINE_META_VARS_T(container, insert)
OBJECT_VIEW_DEFINE_META_VARS_T(container, insert_after)
OBJECT_VIEW_DEFINE_META_VARS_T(container, insert_or_assign)
OBJECT_VIEW_DEFINE_META_VARS_T(container, erase)
OBJECT_VIEW_DEFINE_META_VARS_T(container, erase_after)
template <typename T>
void ObjectView::push_front(T&& arg) const {
    Invoke<void>(NameIDRegistry::Meta::container_push_front, TempArgsView{std::forward<T>(arg)}, MethodFlag::Variable);
};
template <typename T>
void ObjectView::push_back(T&& arg) const {
    Invoke<void>(NameIDRegistry::Meta::container_push_back, TempArgsView{std::forward<T>(arg)}, MethodFlag::Variable);
};
inline void ObjectView::pop_front() const { Invoke<void>(NameIDRegistry::Meta::container_pop_front, ArgsView{}, MethodFlag::Variable); }
inline void ObjectView::pop_back() const { Invoke<void>(NameIDRegistry::Meta::container_pop_back, ArgsView{}, MethodFlag::Variable); }
template <typename T>
void ObjectView::push(T&& arg) const {
    Invoke<void>(NameIDRegistry::Meta::container_push, TempArgsView{std::forward<T>(arg)}, MethodFlag::Variable);
};
inline void ObjectView::pop() const { Invoke<void>(NameIDRegistry::Meta::container_pop, ArgsView{}, MethodFlag::Variable); }
template <typename T>
void ObjectView::swap(T&& arg) const {
    Invoke<void>(NameIDRegistry::Meta::container_swap, TempArgsView{std::forward<T>(arg)}, MethodFlag::Variable);
};
template <typename T>
void ObjectView::merge(T&& arg) const {
    Invoke<void>(NameIDRegistry::Meta::container_merge, TempArgsView{std::forward<T>(arg)}, MethodFlag::Variable);
};
OBJECT_VIEW_DEFINE_CONTAINER_META_T(extract)

// - list operations

template <typename... Args>
void ObjectView::splice_after(Args&&... args) const {
    Invoke<void>(NameIDRegistry::Meta::container_splice_after, TempArgsView{std::forward<Args>(args)...}, MethodFlag::Variable);
};
template <typename... Args>
void ObjectView::splice(Args&&... args) const {
    Invoke<void>(NameIDRegistry::Meta::container_splice, TempArgsView{std::forward<Args>(args)...}, MethodFlag::Variable);
};
template <typename T>
std::size_t ObjectView::remove(T&& arg) const {
    return Invoke<std::size_t>(NameIDRegistry::Meta::container_remove, TempArgsView{std::forward<T>(arg)}, MethodFlag::Variable);
};
inline void ObjectView::reverse() const { Invoke<void>(NameIDRegistry::Meta::container_reverse, ArgsView{}, MethodFlag::Variable); }
template <typename T>
std::size_t ObjectView::unique(T&& arg) const {
    return Invoke<std::size_t>(NameIDRegistry::Meta::container_unique, TempArgsView{std::forward<T>(arg)}, MethodFlag::Variable);
};
inline void ObjectView::sort() const { Invoke<void>(NameIDRegistry::Meta::container_sort, ArgsView{}, MethodFlag::Variable); }

// - lookup

template <typename T>
std::size_t ObjectView::count(T&& arg) const {
    return Invoke<std::size_t>(NameIDRegistry::Meta::container_count, MethodFlag::Const, std::forward<T>(arg));
};
OBJECT_VIEW_DEFINE_CONTAINER_META_T(find)
OBJECT_VIEW_DEFINE_CONTAINER_META_T(lower_bound)
OBJECT_VIEW_DEFINE_CONTAINER_META_T(upper_bound)
OBJECT_VIEW_DEFINE_CONTAINER_META_T(equal_range)

// - variant

inline std::size_t ObjectView::index() const { return Invoke<std::size_t>(NameIDRegistry::Meta::variant_index, ArgsView{}, MethodFlag::Const); }
inline bool ObjectView::holds_alternative(Type type) const { return Invoke<bool>(NameIDRegistry::Meta::holds_alternative, TempArgsView{std::move(type)}, MethodFlag::Const); }

// - optional

inline bool ObjectView::has_value() const { return Invoke<bool>(NameIDRegistry::Meta::optional_has_value, ArgsView{}, MethodFlag::Const); }
inline ObjectView ObjectView::value() const { return Invoke<ObjectView>(NameIDRegistry::Meta::optional_value, ArgsView{}, MethodFlag::Member); }
inline void ObjectView::reset() const { Invoke<void>(NameIDRegistry::Meta::optional_reset, ArgsView{}, MethodFlag::Variable); }
}  // namespace neko::cpp

template <>
struct std::hash<neko::cpp::ObjectView> {
    std::size_t operator()(const neko::cpp::ObjectView& obj) const noexcept { return obj.GetType().get_id().GetValue() ^ std::hash<const void*>()(obj.GetPtr()); }
};

template <>
struct std::hash<neko::cpp::SharedObject> {
    std::size_t operator()(const neko::cpp::SharedObject& obj) const noexcept { return obj.GetType().get_id().GetValue() ^ std::hash<const void*>()(obj.GetPtr()); }
};

namespace std {
inline void swap(neko::cpp::SharedObject& left, neko::cpp::SharedObject& right) noexcept { left.swap(right); }
}  // namespace std

namespace neko::cpp {
inline bool operator==(const ObjectView& lhs, const ObjectView& rhs) {
    return lhs.Invoke<bool>(NameIDRegistry::Meta::operator_eq, TempArgsView{rhs}, MethodFlag::Const) || rhs.Invoke<bool>(NameIDRegistry::Meta::operator_eq, TempArgsView{lhs}, MethodFlag::Const);
}

inline bool operator!=(const ObjectView& lhs, const ObjectView& rhs) { return !(lhs == rhs); }

template <NonObjectAndView T>
bool operator==(const T& lhs, ObjectView ptr) {
    return ObjectView{lhs} == ptr;
}
template <NonObjectAndView T>
bool operator!=(const T& lhs, ObjectView ptr) {
    return ObjectView{lhs} != ptr;
}
template <NonObjectAndView T>
bool operator<(const T& lhs, ObjectView ptr) {
    return ObjectView{lhs} < ptr;
}
template <NonObjectAndView T>
bool operator<=(const T& lhs, ObjectView ptr) {
    return ObjectView{lhs} <= ptr;
}
template <NonObjectAndView T>
bool operator>(const T& lhs, ObjectView ptr) {
    return ObjectView{lhs} > ptr;
}
template <NonObjectAndView T>
bool operator>=(const T& lhs, ObjectView ptr) {
    return ObjectView{lhs} >= ptr;
}

DEFINE_OPERATOR_LSHIFT(std::ostream, ObjectView)
DEFINE_OPERATOR_LSHIFT(std::ostringstream, ObjectView)
DEFINE_OPERATOR_LSHIFT(std::ofstream, ObjectView)
DEFINE_OPERATOR_LSHIFT(std::iostream, ObjectView)
DEFINE_OPERATOR_LSHIFT(std::stringstream, ObjectView)
DEFINE_OPERATOR_LSHIFT(std::fstream, ObjectView)

DEFINE_OPERATOR_RSHIFT(std::istream, ObjectView)
DEFINE_OPERATOR_RSHIFT(std::istringstream, ObjectView)
DEFINE_OPERATOR_RSHIFT(std::ifstream, ObjectView)
DEFINE_OPERATOR_RSHIFT(std::iostream, ObjectView)
DEFINE_OPERATOR_RSHIFT(std::stringstream, ObjectView)
DEFINE_OPERATOR_RSHIFT(std::fstream, ObjectView)
}  // namespace neko::cpp

#undef DEFINE_OPERATOR_RSHIFT
#undef DEFINE_OPERATOR_LSHIFT
#undef OBJECT_VIEW_DEFINE_META_VARS_T
#undef OBJECT_VIEW_DEFINE_CONTAINER_META_T
#undef OBJECT_VIEW_DEFINE_OPERATOR_T
#undef OBJECT_VIEW_DEFINE_META_T
#undef OBJECT_VIEW_DEFINE_ASSIGN_OP_OPERATOR

namespace neko::cpp {
class FieldPtr {
public:
    //
    // Buffer
    ///////////

    static constexpr std::size_t BufferSize = std::max(sizeof(Offsetor), sizeof(SharedBuffer));  // maybe 64
    using Buffer = std::aligned_storage_t<BufferSize>;
    static_assert(sizeof(Buffer) == BufferSize);
    // raw offsetor
    using Data = std::variant<std::size_t,   // forward_offset_value 0 BASIC
                              Offsetor,      // offsetor             1 VIRTUAL
                              void*,         // static_obj           2 STATIC
                              SharedBuffer,  // dynamic_obj          3 DYNAMIC_SHARED
                              Buffer         // dynamic_obj          4 DYNAMIC_BUFFER
                              >;

    template <typename T>
    static constexpr bool IsBufferable() noexcept {
        return std::is_trivially_copyable_v<T> && sizeof(T) <= BufferSize && alignof(T) <= alignof(Buffer);
    }

    template <typename T>
    static constexpr Buffer ConvertToBuffer(const T& data) noexcept {
        static_assert(IsBufferable<T>());
        Buffer buffer;
        memcpy(&buffer, &data, sizeof(T));
        return buffer;
    }

    //
    // Constructor
    ////////////////

    FieldPtr() noexcept = default;

    FieldPtr(Type type, std::size_t forward_offset_value) noexcept : type{type}, data{forward_offset_value} { neko_assert(type); }

    FieldPtr(Type type, Offsetor offsetor) noexcept : type{type}, data{std::move(offsetor)} { neko_assert(type && std::get<1>(data)); }

    FieldPtr(Type type, void* ptr) noexcept : type{type}, data{ptr} { neko_assert(type && ptr); }

    explicit FieldPtr(ObjectView static_obj) noexcept : type{static_obj.GetType()}, data{static_obj.GetPtr()} { neko_assert(type && static_obj.GetPtr()); }

    explicit FieldPtr(SharedObject obj) noexcept : type{obj.GetType()}, data{std::move(obj.GetBuffer())} { neko_assert(type && std::get<3>(data)); }

    FieldPtr(Type type, const Buffer& buffer) noexcept : type{type}, data{buffer} { neko_assert(type); }

    Type GetType() const noexcept { return type; }

    FieldFlag GetFieldFlag() const noexcept;

    // unowned
    ObjectView Var();

    ObjectView Var(void* obj);

    // unowned without DYNAMIC_BUFFER
    ObjectView Var() const;

    // without DYNAMIC_BUFFER
    ObjectView Var(void* obj) const;

private:
    Type type;
    Data data;
};
}  // namespace neko::cpp

namespace neko::cpp {
using ParamList = std::vector<Type>;

class MethodPtr {
public:
    using Func = std::function<void(void*, void*, ArgsView)>;

    MethodPtr() = default;
    MethodPtr(Func func, MethodFlag flag, Type result_type = Type_of<void>, ParamList paramList = {});

    MethodFlag GetMethodFlag() const noexcept { return flag; }
    const Type& GetResultType() const noexcept { return result_type; }
    const ParamList& GetParamList() const noexcept { return paramList; }

    bool IsDistinguishableWith(const MethodPtr& rhs) const noexcept { return flag != rhs.flag || paramList != rhs.paramList; }

    // argTypes[i] == paramList[i] || paramList[i].Is<ObjectView>()
    bool IsMatch(std::span<const Type> argTypes) const noexcept;

    void Invoke(void* obj, void* result_buffer, ArgsView args) const;
    void Invoke(const void* obj, void* result_buffer, ArgsView args) const;
    void Invoke(void* result_buffer, ArgsView args) const;

private:
    Func func;
    MethodFlag flag{MethodFlag::None};
    Type result_type;
    ParamList paramList;
};
}  // namespace neko::cpp

namespace neko::cpp {
using Attr = SharedObject;
struct AttrLess {
    using is_transparent = int;
    bool operator()(const Attr& lhs, const Attr& rhs) const noexcept { return lhs.GetType() < rhs.GetType(); }
    bool operator()(const Attr& lhs, const Type& rhs) const noexcept { return lhs.GetType() < rhs; }
    bool operator()(const Type& lhs, const Attr& rhs) const noexcept { return lhs < rhs.GetType(); }
};
using AttrSet = std::set<Attr, AttrLess>;

class BaseInfo {
public:
    BaseInfo() noexcept = default;
    BaseInfo(InheritCastFunctions funcs) : funcs{std::move(funcs)} { neko_assert(this->funcs.static_derived_to_base); }

    bool IsVirtual() const noexcept { return !static_cast<bool>(funcs.static_base_to_derived); }
    bool IsPolymorphic() const noexcept { return static_cast<bool>(funcs.dynamic_base_to_derived); }

    void* StaticCast_DerivedToBase(void* ptr) const noexcept { return funcs.static_derived_to_base(ptr); }
    // require non virtual
    void* StaticCast_BaseToDerived(void* ptr) const noexcept { return IsVirtual() ? nullptr : funcs.static_base_to_derived(ptr); }
    // require polymorphic
    void* DynamicCast_BaseToDerived(void* ptr) const noexcept { return IsPolymorphic() ? funcs.dynamic_base_to_derived(ptr) : nullptr; }

private:
    InheritCastFunctions funcs;
};

struct FieldInfo {
    FieldPtr fieldptr;
    AttrSet attrs;
};

struct MethodInfo {
    MethodPtr methodptr;
    AttrSet attrs;
};

// trivial : https://docs.microsoft.com/en-us/cpp/cpp/trivial-standard-layout-and-pod-types?view=msvc-160
// if the type is trivial, it must contains a copy-ctor for type-convertion, and can't register default ctor, dtor
struct TypeInfo {
    size_t size;
    size_t alignment;
    bool is_polymorphic;
    bool is_trivial;
    std::unordered_map<Name, FieldInfo> fieldinfos;
    std::unordered_multimap<Name, MethodInfo> methodinfos;
    std::unordered_map<Type, BaseInfo> baseinfos;
    AttrSet attrs;
};
}  // namespace neko::cpp

template <>
constexpr auto neko::cpp::type_name<neko::cpp::AttrSet>() noexcept {
    return TSTR("neko::cpp::AttrSet");
}

namespace neko::cpp {
constexpr Type GlobalType = TypeIDRegistry::Meta::global;
constexpr ObjectView Global = {GlobalType, nullptr};

class neko_refl {
public:
    static neko_refl& Instance() noexcept;

    //
    // Data
    /////////
    //
    // enum is a special type (all member is static)
    //

    mutable NameIDRegistry nregistry;
    mutable TypeIDRegistry tregistry;

    std::unordered_map<Type, TypeInfo> typeinfos;

    TypeInfo* GetTypeInfo(Type type) const;
    SharedObject GetTypeAttr(Type type, Type attr_type) const;
    SharedObject GetFieldAttr(Type type, Name field_name, Type attr_type) const;
    SharedObject GetMethodAttr(Type type, Name method_name, Type attr_type) const;

    void SetTemporaryResource(std::shared_ptr<std::pmr::memory_resource> rsrc);
    void SetObjectResource(std::shared_ptr<std::pmr::memory_resource> rsrc);

    std::pmr::memory_resource* GetTemporaryResource() const { return temporary_resource.get(); }
    std::pmr::memory_resource* GetObjectResource() const { return object_resource.get(); }

    // clear order
    // - field attrs
    // - type attrs
    // - type dynamic shared field
    // - typeinfos
    // - temporary_resource
    // - object_resource
    void Clear() noexcept;

    //
    // Traits
    ///////////

    bool ContainsVirtualBase(Type type) const;

    //
    // Factory
    ////////////
    //
    // - we will register the value type when generating FieldPtr, so those APIs aren't static
    //

    // field_data can be:
    // - static field: pointer to **non-void** type
    // - member object pointer
    // - enumerator
    template <auto field_data, bool NeedRegisterFieldType = true>
    FieldPtr GenerateFieldPtr();

    template <auto field_data>
    FieldPtr SimpleGenerateFieldPtr() {
        return GenerateFieldPtr<field_data, false>();
    }

    // data can be:
    // 1. member object pointer
    // 2. pointer to **non-void** and **non-function** type
    // 3. functor : Value*(Object*)  / Value&(Object*)
    // 4. enumerator
    template <typename T, bool NeedRegisterFieldType = true>
    FieldPtr GenerateFieldPtr(T&& data);

    template <typename T>
    FieldPtr SimpleGenerateFieldPtr(T&& data) {
        return GenerateFieldPtr<T, false>(std::forward<T>(data));
    }

    // if T is bufferable, T will be stored as buffer,
    // else we will use MakeShared to store it
    template <typename T, typename... Args>
    FieldPtr GenerateDynamicFieldPtr(Args&&... args);

    template <typename T, typename... Args>
    FieldPtr SimpleGenerateDynamicFieldPtr(Args&&... args);

    template <typename... Params>
    static ParamList GenerateParamList() noexcept(sizeof...(Params) == 0);

    // funcptr can be
    // 1. member method : member function pointer
    // 2. static method : function pointer
    template <auto funcptr>
    static MethodPtr GenerateMethodPtr();

    // void(T&, Args...)
    template <typename T, typename... Args>
    static MethodPtr GenerateConstructorPtr();

    // void(const T&)
    template <typename T>
    static MethodPtr GenerateDestructorPtr();

    // Func: Ret(const? Object&, Args...)
    template <typename Func>
    static MethodPtr GenerateMemberMethodPtr(Func&& func);

    // Func: Ret(Args...)
    template <typename Func>
    static MethodPtr GenerateStaticMethodPtr(Func&& func);

    template <typename Derived, typename Base>
    static BaseInfo GenerateBaseInfo();

    //
    // Modifier
    /////////////

    // if is_trivial, register a trivial copy ctor
    Type RegisterType(Type type, size_t size, size_t alignment, bool is_polymorphic = false, bool is_trivial = false);
    Name AddField(Type type, Name field_name, FieldInfo fieldinfo);
    Name AddMethod(Type type, Name method_name, MethodInfo methodinfo);
    Type AddBase(Type derived, Type base, BaseInfo baseinfo);
    bool AddTypeAttr(Type type, Attr attr);
    bool AddFieldAttr(Type type, Name field_name, Attr attr);
    bool AddMethodAttr(Type type, Name method_name, Attr attr);

    Name AddTrivialDefaultConstructor(Type type);
    Name AddTrivialCopyConstructor(Type type);
    Name AddZeroDefaultConstructor(Type type);
    Name AddDefaultConstructor(Type type);
    Name AddDestructor(Type type);

    // - data-driven

    // require
    // - all bases aren't polymorphic and don't contain any virtual base
    // - field_types.size() == field_names.size()
    // auto compute
    // - size & alignment of type
    // - baseinfos
    // - fields' forward offset value
    Type RegisterType(Type type, std::span<const Type> bases, std::span<const Type> field_types, std::span<const Name> field_names, bool is_trivial = false);

    // -- template --

    // call
    // - RegisterType(type_name<T>(), sizeof(T), alignof(T), std::is_polymorphic<T>, std::is_trivial_v<T>)
    // - details::TypeAutoRegister<T>::run
    // you can custom type register by specialize details::TypeAutoRegister<T>
    template <typename T>
    void RegisterType();

    // get TypeID from field_data
    // field_data can be
    // 1. member object pointer
    // 2. enumerator
    template <auto field_data, bool NeedRegisterFieldType = true>
    bool AddField(Name name, AttrSet attrs = {});

    template <auto field_data>
    bool SimpleAddField(Name name, AttrSet attrs = {}) {
        return AddField<field_data, false>(name, std::move(attrs));
    }

    // data can be:
    // 1. member object pointer
    // 2. enumerator
    // 3. pointer to **non-void** and **non-function** type
    // 4. functor : Value*(Object*) / Value&(Object*)
    template <typename T, bool NeedRegisterFieldType = true>
        requires std::negation_v<std::is_same<std::decay_t<T>, FieldInfo>>
    bool AddField(Type type, Name name, T&& data, AttrSet attrs = {}) {
        return AddField(type, name, {GenerateFieldPtr<T, NeedRegisterFieldType>(std::forward<T>(data)), std::move(attrs)});
    }

    template <typename T>
        requires std::negation_v<std::is_same<std::decay_t<T>, FieldInfo>>
    bool SimpleAddField(Type type, Name name, T&& data, AttrSet attrs = {}) {
        return AddField<T, false>(type, name, std::forward<T>(data), std::move(attrs));
    }

    // data can be:
    // 1. member object pointer
    // 2. functor : Value*(Object*)
    // > - result must be an pointer of **non-void** type
    // 3. enumerator
    template <typename T, bool NeedRegisterFieldType = true>
        requires std::negation_v<std::is_same<std::decay_t<T>, FieldInfo>>
    bool AddField(Name name, T&& data, AttrSet attrs = {});

    template <typename T>
        requires std::negation_v<std::is_same<std::decay_t<T>, FieldInfo>>
    bool SimpleAddField(Name name, T&& data, AttrSet attrs = {}) {
        return AddField<T, false>(name, std::forward<T>(data), std::move(attrs));
    }

    template <typename T, typename... Args>
    bool AddDynamicFieldWithAttr(Type type, Name name, AttrSet attrs, Args&&... args) {
        return AddField(type, name, {GenerateDynamicFieldPtr<T>(std::forward<Args>(args)...), std::move(attrs)});
    }

    template <typename T, typename... Args>
    bool SimpleAddDynamicFieldWithAttr(Type type, Name name, AttrSet attrs, Args&&... args) {
        return AddField(type, name, {GenerateDynamicFieldPtr<T, false>(std::forward<Args>(args)...), std::move(attrs)});
    }

    template <typename T, typename... Args>
    bool AddDynamicField(Type type, Name name, Args&&... args) {
        return AddDynamicFieldWithAttr<T>(type, name, {}, std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    bool SimpleAddDynamicField(Type type, Name name, Args&&... args) {
        return SimpleAddDynamicFieldWithAttr<T>(type, name, {}, std::forward<Args>(args)...);
    }

    // funcptr is member function pointer
    // get TypeID from funcptr
    template <auto member_func_ptr>
    bool AddMethod(Name name, AttrSet attrs = {});

    // funcptr is function pointer
    template <auto func_ptr>
    bool AddMethod(Type type, Name name, AttrSet attrs = {});

    template <typename T, typename... Args>
    bool AddConstructor(AttrSet attrs = {});
    template <typename T>
    bool AddDestructor(AttrSet attrs = {});

    // Func: Ret(const? Object&, Args...)
    template <typename Func>
    bool AddMemberMethod(Name name, Func&& func, AttrSet attrs = {});

    // Func: Ret(Args...)
    template <typename Func>
    bool AddStaticMethod(Type type, Name name, Func&& func, AttrSet attrs = {}) {
        return AddMethod(type, name, {GenerateStaticMethodPtr(std::forward<Func>(func)), std::move(attrs)});
    }

    template <typename Derived, typename... Bases>
    bool AddBases();

    //
    // Cast
    /////////

    ObjectView StaticCast_DerivedToBase(ObjectView obj, Type type) const;
    ObjectView StaticCast_BaseToDerived(ObjectView obj, Type type) const;
    ObjectView DynamicCast_BaseToDerived(ObjectView obj, Type type) const;
    ObjectView StaticCast(ObjectView obj, Type type) const;
    ObjectView DynamicCast(ObjectView obj, Type type) const;

    //
    // Var
    ////////
    //
    // - result type of Var maintains the CVRefMode of the input
    //

    ObjectView Var(ObjectView obj, Name field_name, FieldFlag flag = FieldFlag::All) const;
    // for diamond inheritance
    ObjectView Var(ObjectView obj, Type base, Name field_name, FieldFlag flag = FieldFlag::All) const;

    //
    // Invoke
    ///////////
    //
    // - 'B' means basic
    // - 'M' means memory
    // - auto search methods in bases
    // - support overload
    // - require IsCompatible()
    // - MInvoke will allocate buffer for result, and move to SharedObject
    // - if result is a reference, SharedObject is a ObjectView actually
    // - if result is ObjectView or SharedObject, then MInvoke's result is it.
    // - temp_args_rsrc is used for temporary allocation of arguments (release before return)
    //

    // parameter <- argument
    // - ObjectView
    // - same
    // - reference
    // > - 0 (invalid), 1 (convertible), 2 (constructible)
    // > - table
    //     |    -     | T | T & | const T & | T&& | const T&& |
    //     |      T   | - |  2  |     2     |  1  |     2     |
    //     |      T & | 0 |  -  |     0     |  0  |     0     |
    //     |const T & | 1 |  1  |     -     |  1  |     1     |
    //     |      T&& | 1 |  0  |     0     |  -  |     0     |
    //     |const T&& | 1 |  0  |     0     |  1  |     -     |
    // - pointer and array (non cvref)
    // > - 0 (invalid), 1 (convertible)
    // > - table
    //     |     -     | T * | const T * | T[] | const T[] |
    //     |       T * |  -  |     0     |  1  |     0     |
    //     | const T * |  1  |     -     |  1  |     1     |
    //     |       T[] |  1  |     0     | -/1 |     0     |
    //     | const T[] |  1  |     1     |  1  |    -/1    |
    bool IsCompatible(std::span<const Type> paramTypes, std::span<const Type> argTypes) const;

    Type IsInvocable(Type type, Name method_name, std::span<const Type> argTypes = {}, MethodFlag flag = MethodFlag::All) const;

    Type BInvoke(ObjectView obj, Name method_name, void* result_buffer = nullptr, ArgsView args = {}, MethodFlag flag = MethodFlag::All,
                 std::pmr::memory_resource* temp_args_rsrc = ReflMngr_GetTemporaryResource()) const;

    SharedObject MInvoke(ObjectView obj, Name method_name, std::pmr::memory_resource* rst_rsrc, ArgsView args = {}, MethodFlag flag = MethodFlag::All,
                         std::pmr::memory_resource* temp_args_rsrc = ReflMngr_GetTemporaryResource()) const;

    SharedObject Invoke(ObjectView obj, Name method_name, ArgsView args = {}, MethodFlag flag = MethodFlag::All, std::pmr::memory_resource* temp_args_rsrc = ReflMngr_GetTemporaryResource()) const {
        return MInvoke(obj, method_name, object_resource.get(), args, flag, temp_args_rsrc);
    }

    // -- template --

    template <typename... Args>
    Type IsInvocable(Type type, Name method_name, MethodFlag flag = MethodFlag::All) const;

    template <typename T>
    T Invoke(ObjectView obj, Name method_name, ArgsView args = {}, MethodFlag flag = MethodFlag::All, std::pmr::memory_resource* temp_args_rsrc = ReflMngr_GetTemporaryResource()) const;

    //
    // Make
    /////////
    //
    // - if the type doesn't contains any ctor, then we use trivial ctor (do nothing)
    // - if the type doesn't contains any dtor, then we use trivial dtor (do nothing)
    //

    bool IsConstructible(Type type, std::span<const Type> argTypes = {}) const;
    bool IsCopyConstructible(Type type) const;
    bool IsMoveConstructible(Type type) const;
    bool IsDestructible(Type type) const;

    bool Construct(ObjectView obj, ArgsView args = {}) const;
    bool Destruct(ObjectView obj) const;

    ObjectView MNew(Type type, std::pmr::memory_resource* rsrc, ArgsView args = {}) const;
    SharedObject MMakeShared(Type type, std::pmr::memory_resource* rsrc, ArgsView args = {}) const;
    bool MDelete(ObjectView obj, std::pmr::memory_resource* rsrc) const;

    ObjectView New(Type type, ArgsView args = {}) const;
    SharedObject MakeShared(Type type, ArgsView args = {}) const;
    bool Delete(ObjectView obj) const;

    // -- template --

    template <typename... Args>
    bool IsConstructible(Type type) const;

private:
    neko_refl();
    ~neko_refl();

    // for
    // - argument copy
    // - user argument buffer
    std::shared_ptr<std::pmr::memory_resource> temporary_resource;

    // for
    // - New/MakeShared
    std::shared_ptr<std::pmr::memory_resource> object_resource;
};

inline static neko_refl& neko_refl_instance = neko_refl::Instance();
inline static const ObjectView neko_refl_view = {Type_of<neko_refl>, &neko_refl::Instance()};
}  // namespace neko::cpp

namespace neko::cpp::details {
template <bool NeedRegisterFieldType, typename T, typename... Args>
FieldPtr GenerateDynamicFieldPtr(neko_refl& mngr, Args&&... args) {
    static_assert(!std::is_reference_v<T> && !std::is_volatile_v<T>);
    using RawT = std::remove_const_t<T>;
    if constexpr (NeedRegisterFieldType)
        mngr.RegisterType<RawT>();
    else
        neko_assert(mngr.GetTypeInfo(Type_of<T>));
    mngr.AddConstructor<RawT, Args...>();
    if constexpr (FieldPtr::IsBufferable<RawT>()) {
        FieldPtr::Buffer buffer = FieldPtr::ConvertToBuffer(T{std::forward<Args>(args)...});
        return FieldPtr{Type_of<T>, buffer};
    } else
        return FieldPtr{mngr.MakeShared(Type_of<RawT>, TempArgsView{std::forward<Args>(args)...})};
}

template <typename F>
struct WrapFuncTraits;

template <typename Func, typename Obj>
struct WrapFuncTraits<Func Obj::*> : FuncTraits<Func Obj::*> {
private:
    using Traits = FuncTraits<Func>;

public:
    using Object = Obj;
    using ArgList = typename Traits::ArgList;
    using Return = typename Traits::Return;
    static constexpr bool is_const = Traits::is_const;
};

template <typename F>
struct WrapFuncTraits {
private:
    using Traits = FuncTraits<F>;
    using ObjectArgList = typename Traits::ArgList;
    static_assert(!IsEmpty_v<ObjectArgList>);
    using CVObjRef = Front_t<ObjectArgList>;
    using CVObj = std::remove_reference_t<CVObjRef>;

public:
    using ArgList = PopFront_t<ObjectArgList>;
    using Object = std::remove_cv_t<CVObj>;
    using Return = typename Traits::Return;
    static constexpr bool is_const = std::is_const_v<CVObj>;
    static_assert(is_const || !std::is_rvalue_reference_v<CVObjRef>);
};

template <typename ArgList>
struct wrap_function_call;

template <typename T>
decltype(auto) auto_get_arg(ObjectView obj) {
    if constexpr (std::is_same_v<T, ObjectView>)
        return obj;
    else
        return obj.As<T>();
}

template <typename... Args>
struct wrap_function_call<TypeList<Args...>> {
    template <typename Obj, auto func_ptr, typename MaybeConstVoidPtr, std::size_t... Ns>
    static constexpr decltype(auto) run(MaybeConstVoidPtr ptr, ArgsView args, std::index_sequence<Ns...>) {
        return (buffer_as<Obj>(ptr).*func_ptr)(auto_get_arg<Args>(args[Ns])...);
    }
    template <auto func_ptr, std::size_t... Ns>
    static constexpr decltype(auto) run(ArgsView args, std::index_sequence<Ns...>) {
        return func_ptr(auto_get_arg<Args>(args[Ns])...);
    }
    template <typename Obj, typename Func, typename MaybeConstVoidPtr, std::size_t... Ns>
    static constexpr decltype(auto) run(MaybeConstVoidPtr ptr, Func&& func, ArgsView args, std::index_sequence<Ns...>) {
        if constexpr (std::is_member_function_pointer_v<std::decay_t<Func>>)
            return (buffer_as<Obj>(ptr).*func)(auto_get_arg<Args>(args[Ns])...);
        else {
            return std::forward<Func>(func)(buffer_as<Obj>(ptr), auto_get_arg<Args>(args[Ns])...);
        }
    }
    template <typename Func, std::size_t... Ns>
    static constexpr decltype(auto) run(Func&& func, ArgsView args, std::index_sequence<Ns...>) {
        return std::forward<Func>(func)(auto_get_arg<Args>(args[Ns])...);
    }
};

// [func_ptr]
// - Func Obj::* : Func isn't && (const && is ok)
// - Func*
// [result]
// - type : void(void* obj, void* result_buffer, ArgsView args)
// - size : 1
template <auto func_ptr>
constexpr auto wrap_member_function() noexcept {
    using FuncPtr = decltype(func_ptr);
    static_assert(std::is_member_function_pointer_v<FuncPtr>);
    using Traits = FuncTraits<FuncPtr>;
    static_assert(!(Traits::ref == ReferenceMode::Right && !Traits::is_const));
    using Obj = typename Traits::Object;
    using Return = typename Traits::Return;
    using ArgList = typename Traits::ArgList;
    using IndexSeq = std::make_index_sequence<Length_v<ArgList>>;
    constexpr auto wrapped_function = [](void* obj, void* result_buffer, ArgsView args) {
        if constexpr (!std::is_void_v<Return>) {
            using NonCVReturn = std::remove_cv_t<Return>;
            NonCVReturn rst = details::wrap_function_call<ArgList>::template run<Obj, func_ptr>(obj, args, IndexSeq{});
            if (result_buffer) {
                if constexpr (std::is_reference_v<Return>)
                    buffer_as<std::add_pointer_t<Return>>(result_buffer) = &rst;
                else
                    buffer_as<NonCVReturn>(result_buffer) = std::move(rst);
            }
        } else
            details::wrap_function_call<ArgList>::template run<Obj, func_ptr>(obj, args, IndexSeq{});
    };
    return wrapped_function;
}

// [func_ptr]
// - Func*
// [result]
// - type : void(void*, void* result_buffer, ArgsView args)
// - size : 1
template <auto func_ptr>
constexpr auto wrap_static_function() noexcept {
    using FuncPtr = decltype(func_ptr);
    static_assert(is_function_pointer_v<FuncPtr>);
    using Traits = FuncTraits<FuncPtr>;
    using Return = typename Traits::Return;
    using ArgList = typename Traits::ArgList;
    using IndexSeq = std::make_index_sequence<Length_v<ArgList>>;
    constexpr auto wrapped_function = [](void*, void* result_buffer, ArgsView args) {
        if constexpr (!std::is_void_v<Return>) {
            using NonCVReturn = std::remove_cv_t<Return>;
            NonCVReturn rst = details::wrap_function_call<ArgList>::template run<func_ptr>(args, IndexSeq{});
            if (result_buffer) {
                if constexpr (std::is_reference_v<Return>)
                    buffer_as<std::add_pointer_t<Return>>(result_buffer) = &rst;
                else
                    new (result_buffer) NonCVReturn{std::move(rst)};
            }
        } else
            details::wrap_function_call<ArgList>::template run<func_ptr>(args, IndexSeq{});
    };
    return wrapped_function;
}

// static dispatch to
// - wrap_member_function
// - wrap_static_function
template <auto func_ptr>
constexpr auto wrap_function() noexcept {
    using FuncPtr = decltype(func_ptr);
    if constexpr (is_function_pointer_v<FuncPtr>)
        return wrap_static_function<func_ptr>();
    else if constexpr (std::is_member_function_pointer_v<FuncPtr>)
        return wrap_member_function<func_ptr>();
    else
        static_assert(always_false<decltype(func_ptr)>);
}

// Func: Ret(const? volatile? Object&, Args...)
// [result]
// - type : void(void* obj, void* result_buffer, ArgsView args)
// - size : sizeof(Func)
template <typename Func>
constexpr auto wrap_member_function(Func&& func) noexcept {
    using Traits = details::WrapFuncTraits<std::decay_t<Func>>;
    using Return = typename Traits::Return;
    using Obj = typename Traits::Object;
    using ArgList = typename Traits::ArgList;
    using IndexSeq = std::make_index_sequence<Length_v<ArgList>>;
    /*constexpr*/ auto wrapped_function = [f = std::forward<Func>(func)](void* obj, void* result_buffer, ArgsView args) mutable {
        if constexpr (!std::is_void_v<Return>) {
            using NonCVReturn = std::remove_cv_t<Return>;
            NonCVReturn rst = details::wrap_function_call<ArgList>::template run<Obj>(obj, std::forward<Func>(f), args, IndexSeq{});
            if (result_buffer) {
                if constexpr (std::is_reference_v<Return>)
                    buffer_as<std::add_pointer_t<Return>>(result_buffer) = &rst;
                else
                    new (result_buffer) NonCVReturn{std::move(rst)};
            }
        } else
            details::wrap_function_call<ArgList>::template run<Obj>(obj, std::forward<Func>(f), args, IndexSeq{});
    };
    return wrapped_function;
}

// Func: Ret(Args...)
// [result]
// - type : void(void*, void* result_buffer, ArgsView args)
// - size : sizeof(Func)
template <typename Func>
constexpr auto wrap_static_function(Func&& func) noexcept {
    using Traits = FuncTraits<std::decay_t<Func>>;
    using Return = typename Traits::Return;
    using ArgList = typename Traits::ArgList;
    using IndexSeq = std::make_index_sequence<Length_v<ArgList>>;
    /*constexpr*/ auto wrapped_function = [f = std::forward<Func>(func)](void*, void* result_buffer, ArgsView args) mutable {
        if constexpr (!std::is_void_v<Return>) {
            using NonCVReturn = std::remove_cv_t<Return>;
            NonCVReturn rst = details::wrap_function_call<ArgList>::template run(std::forward<Func>(f), args, IndexSeq{});
            if (result_buffer) {
                if constexpr (std::is_reference_v<Return>)
                    buffer_as<std::add_pointer_t<Return>>(result_buffer) = &rst;
                else
                    new (result_buffer) NonCVReturn{std::move(rst)};
            }
        } else
            details::wrap_function_call<ArgList>::template run(std::forward<Func>(f), args, IndexSeq{});
    };
    return wrapped_function;
}

template <typename ArgList>
struct GenerateParamListHelper;
template <typename... Args>
struct GenerateParamListHelper<TypeList<Args...>> {
    static ParamList get() noexcept(sizeof...(Args) == 0) { return neko_refl::GenerateParamList<Args...>(); }
};

template <typename T, typename FixedArgList, typename... LeftArgs>
struct register_ctor_impl;
template <typename T, typename... FixedArgs>
struct register_ctor_impl<T, TypeList<FixedArgs...>> {
    static void run(neko_refl& mngr) {
        if constexpr (type_ctor<T, FixedArgs...>) mngr.AddConstructor<T, FixedArgs...>();
    }
};
template <typename T, typename... FixedArgs, typename LeftArg0, typename... LeftArgs>
struct register_ctor_impl<T, TypeList<FixedArgs...>, LeftArg0, LeftArgs...> {
    static void run(neko_refl& mngr) {
        register_ctor_impl<T, TypeList<FixedArgs..., const LeftArg0&>, LeftArgs...>::run(mngr);
        if constexpr (!std::is_fundamental_v<LeftArg0>) register_ctor_impl<T, TypeList<FixedArgs..., LeftArg0&&>, LeftArgs...>::run(mngr);
    }
};

template <typename T, typename... Args>
void register_ctor(neko_refl& mngr) {
    register_ctor_impl<T, TypeList<>, Args...>::run(mngr);
}

template <template <typename> class get_size, std::size_t TargetIdx, typename T>
ObjectView runtime_get_impl(T&& obj, std::size_t i) {
    using U = std::remove_cvref_t<T>;
    if constexpr (TargetIdx == get_size<U>::value)
        return {};
    else {
        if (i == TargetIdx)
            return ObjectView{std::get<TargetIdx>(std::forward<T>(obj))};
        else
            return runtime_get_impl<get_size, TargetIdx + 1>(std::forward<T>(obj), i);
    }
}

template <template <typename> class get_size, typename T>
ObjectView runtime_get(T&& obj, std::size_t i) {
    using U = std::remove_cvref_t<T>;
    neko_assert(i < get_size<U>::value);
    return runtime_get_impl<get_size, 0>(std::forward<T>(obj), i);
}

template <template <typename> class get_size, template <std::size_t, typename> class get_type, std::size_t TargetIdx, typename T>
ObjectView runtime_get_impl(T&& obj, const Type& type) {
    using U = std::remove_cvref_t<T>;
    if constexpr (TargetIdx == get_size<U>::value)
        return {};
    else {
        if (type == Type_of<typename get_type<TargetIdx, U>::type>)
            return ObjectView{std::get<TargetIdx>(std::forward<T>(obj))};
        else
            return runtime_get_impl<get_size, get_type, TargetIdx + 1>(std::forward<T>(obj), type);
    }
}

template <template <typename> class get_size, template <std::size_t, typename> class get_type, typename T>
ObjectView runtime_get(T&& obj, const Type& type) {
    return runtime_get_impl<get_size, get_type, 0>(std::forward<T>(obj), type);
}

template <std::size_t TargetIdx, typename T>
Type runtime_tuple_element_impl(std::size_t i) noexcept {
    if constexpr (TargetIdx == std::tuple_size_v<T>)
        return {};
    else {
        if (i == TargetIdx)
            return Type_of<std::tuple_element_t<TargetIdx, T>>;
        else
            return runtime_tuple_element_impl<TargetIdx + 1, T>(i);
    }
}

template <typename T>
Type runtime_tuple_element(std::size_t i) noexcept {
    neko_assert(i < std::tuple_size_v<T>);
    return runtime_tuple_element_impl<0, T>(i);
}

template <typename T, std::size_t... Ns>
void register_tuple_elements(neko_refl& mngr, std::index_sequence<Ns...>) {
    (mngr.RegisterType<std::tuple_element_t<Ns, T>>(), ...);
    register_ctor<T, std::tuple_element_t<Ns, T>...>(mngr);
}

template <std::size_t TargetIdx, typename T>
bool runtime_variant_holds_alternative_impl(const T& obj, const Type& type) noexcept {
    if constexpr (TargetIdx == std::variant_size_v<T>)
        return false;
    else {
        using Elem = std::variant_alternative_t<TargetIdx, T>;
        if (type == Type_of<Elem>)
            return std::holds_alternative<Elem>(obj);
        else
            return runtime_variant_holds_alternative_impl<TargetIdx + 1>(obj, type);
    }
}

template <typename T>
bool runtime_variant_holds_alternative(const T& obj, const Type& type) noexcept {
    return runtime_variant_holds_alternative_impl<0>(obj, type);
}

template <std::size_t TargetIdx, typename T>
Type runtime_variant_alternative_impl(std::size_t i) {
    if constexpr (TargetIdx == std::variant_size_v<T>)
        return {};
    else {
        if (i == TargetIdx)
            return Type_of<std::variant_alternative_t<TargetIdx, T>>;
        else
            return runtime_variant_alternative_impl<TargetIdx + 1, T>(i);
    }
}

template <typename T>
Type runtime_variant_alternative(std::size_t i) {
    neko_assert(i < std::variant_size_v<T>);
    return runtime_variant_alternative_impl<0, T>(i);
}

template <typename T, std::size_t Idx>
void register_variant_ctor_assign(neko_refl& mngr) {
    using Elem = std::variant_alternative_t<Idx, T>;
    if constexpr (type_ctor<T, const Elem&>) mngr.AddConstructor<T, const Elem&>();
    if constexpr (operator_assignment<T, const Elem&>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assignment, [](T& t, const Elem& elem) -> T& { return t = elem; });

    if constexpr (!std::is_fundamental_v<Elem>) {
        if constexpr (type_ctor<T, Elem&&>) mngr.AddConstructor<T, Elem&&>();
        if constexpr (operator_assignment<T, Elem&&>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assignment, [](T& t, Elem&& elem) -> T& { return t = std::move(elem); });
    }
}

template <typename T, std::size_t... Ns>
void register_variant_alternatives(neko_refl& mngr, std::index_sequence<Ns...>) {
    (mngr.RegisterType<std::variant_alternative_t<Ns, T>>(), ...);
    (register_variant_ctor_assign<T, Ns>(mngr), ...);
}

template <typename T>
struct TypeAutoRegister_Default {
    static void run(neko_refl& mngr) {
        if constexpr (std::is_default_constructible_v<T> && !std::is_trivial_v<T>) mngr.AddConstructor<T>();
        if constexpr (type_ctor_copy<T> && !std::is_trivial_v<T>) mngr.AddConstructor<T, const T&>();
        if constexpr (type_ctor_move<T> && !std::is_trivial_v<T>) mngr.AddConstructor<T, T&&>();
        if constexpr (std::is_destructible_v<T> && !std::is_trivial_v<T>) mngr.AddDestructor<T>();

        if constexpr (std::is_pointer_v<T>) {
            mngr.RegisterType<std::remove_pointer_t<T>>();
            if constexpr (std::is_const_v<std::remove_pointer_t<T>>) mngr.AddConstructor<T, const std::add_pointer_t<std::remove_const_t<std::remove_pointer_t<T>>>&>();
        }

        if constexpr (operator_bool<const T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_bool, [](const T& obj) { return static_cast<bool>(obj); });

        if constexpr (operator_plus<const T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_add, [](const T& lhs) { return +lhs; });
        if constexpr (operator_minus<const T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_sub, [](const T& lhs) { return -lhs; });

        if constexpr (std::is_array_v<T> && std::rank_v<T> == 0) {
            using Ele = std::remove_extent_t<T>;
            mngr.AddConstructor<T, const std::add_pointer_t<Ele>&>();
            if constexpr (std::is_const_v<Ele>) mngr.AddConstructor<T, const std::add_pointer_t<std::remove_const_t<Ele>>&>();
        }

        if constexpr (operator_add<const T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_add, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs + rhs; });
        if constexpr (operator_sub<const T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_sub, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs - rhs; });
        if constexpr (operator_mul<const T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_mul, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs * rhs; });
        if constexpr (operator_div<const T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_div, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs / rhs; });
        if constexpr (operator_mod<const T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_mod, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs % rhs; });

        if constexpr (operator_bnot<const T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_bnot, [](const T& lhs) -> decltype(auto) { return ~lhs; });
        if constexpr (operator_band<const T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_band, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs & rhs; });
        if constexpr (operator_bor<const T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_bor, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs & rhs; });
        if constexpr (operator_bxor<const T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_bxor, [](const T& lhs, const T& rhs) -> decltype(auto) { return lhs & rhs; });
        if constexpr (operator_shl<const T&, const std::size_t&>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::operator_shl, [](const T& lhs, const std::size_t& rhs) -> decltype(auto) { return lhs << rhs; });
        if constexpr (operator_shr<const T&, const std::size_t&>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::operator_shr, [](const T& lhs, const std::size_t& rhs) -> decltype(auto) { return lhs >> rhs; });

        if constexpr (operator_shr<std::istream&, T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_shl, [](T& lhs, std::istream& rhs) -> decltype(auto) { return rhs >> lhs; });
        if constexpr (operator_shr<std::istringstream&, T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_shl, [](T& lhs, std::istringstream& rhs) -> decltype(auto) { return rhs >> lhs; });
        if constexpr (operator_shr<std::ifstream&, T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_shl, [](T& lhs, std::ifstream& rhs) -> decltype(auto) { return rhs >> lhs; });
        if constexpr (operator_shr<std::iostream&, T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_shl, [](T& lhs, std::iostream& rhs) -> decltype(auto) { return rhs >> lhs; });
        if constexpr (operator_shr<std::stringstream&, T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_shl, [](T& lhs, std::stringstream& rhs) -> decltype(auto) { return rhs >> lhs; });
        if constexpr (operator_shr<std::fstream&, T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_shl, [](T& lhs, std::fstream& rhs) -> decltype(auto) { return rhs >> lhs; });

        if constexpr (operator_shl<std::ostream&, const T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_shr, [](const T& lhs, std::ostream& rhs) -> decltype(auto) { return rhs << lhs; });
        if constexpr (operator_shl<std::ostringstream&, const T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::operator_shr, [](const T& lhs, std::ostringstream& rhs) -> decltype(auto) { return rhs << lhs; });
        if constexpr (operator_shl<std::ofstream&, const T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_shr, [](const T& lhs, std::ofstream& rhs) -> decltype(auto) { return rhs << lhs; });
        if constexpr (operator_shl<std::iostream&, const T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_shr, [](const T& lhs, std::iostream& rhs) -> decltype(auto) { return rhs << lhs; });
        if constexpr (operator_shl<std::stringstream&, const T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::operator_shr, [](const T& lhs, std::stringstream& rhs) -> decltype(auto) { return rhs << lhs; });
        if constexpr (operator_shl<std::fstream&, const T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_shr, [](const T& lhs, std::fstream& rhs) -> decltype(auto) { return rhs << lhs; });

        if constexpr (operator_pre_inc<T&>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_pre_inc, [](T& lhs) -> decltype(auto) { return ++lhs; });
        if constexpr (operator_post_inc<T&>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_post_inc, [](T& lhs) -> decltype(auto) { return lhs++; });
        if constexpr (operator_pre_dec<T&>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_pre_dec, [](T& lhs) -> decltype(auto) { return --lhs; });
        if constexpr (operator_post_dec<T&>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_post_dec, [](T& lhs) -> decltype(auto) { return lhs--; });

        if constexpr (operator_assignment_copy<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assignment, [](T& lhs, const T& rhs) -> T& { return lhs = rhs; });
        if constexpr (operator_assignment_move<T> && (!std::is_trivially_move_assignable_v<T> || !std::is_trivially_copy_assignable_v<T>))
            mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assignment, [](T& lhs, T&& rhs) -> T& { return lhs = std::move(rhs); });
        if constexpr (operator_assignment_add<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assignment_add, [](T& lhs, const T& rhs) -> T& { return lhs += rhs; });
        if constexpr (operator_assignment_sub<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assignment_sub, [](T& lhs, const T& rhs) -> T& { return lhs -= rhs; });
        if constexpr (operator_assignment_mul<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assignment_mul, [](T& lhs, const T& rhs) -> T& { return lhs *= rhs; });
        if constexpr (operator_assignment_div<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assignment_div, [](T& lhs, const T& rhs) -> T& { return lhs /= rhs; });
        if constexpr (operator_assignment_mod<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assignment_mod, [](T& lhs, const T& rhs) -> T& { return lhs %= rhs; });
        if constexpr (operator_assignment_band<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assignment_band, [](T& lhs, const T& rhs) -> T& { return lhs &= rhs; });
        if constexpr (operator_assignment_bor<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assignment_bor, [](T& lhs, const T& rhs) -> T& { return lhs |= rhs; });
        if constexpr (operator_assignment_bxor<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assignment_bxor, [](T& lhs, const T& rhs) -> T& { return lhs ^= rhs; });
        if constexpr (operator_assignment_shl<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assignment_shl, [](T& lhs, const T& rhs) -> T& { return lhs <<= rhs; });
        if constexpr (operator_assignment_shl<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assignment_shr, [](T& lhs, const T& rhs) -> T& { return lhs >>= rhs; });

        if constexpr (!IsContainerType<T> && operator_eq<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_eq, [](const T& lhs, const T& rhs) { return static_cast<bool>(lhs == rhs); });
        if constexpr (!IsContainerType<T> && operator_ne<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_ne, [](const T& lhs, const T& rhs) { return static_cast<bool>(lhs != rhs); });
        if constexpr (!IsContainerType<T> && operator_lt<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_lt, [](const T& lhs, const T& rhs) { return static_cast<bool>(lhs < rhs); });
        if constexpr (!IsContainerType<T> && operator_gt<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_gt, [](const T& lhs, const T& rhs) { return static_cast<bool>(lhs > rhs); });
        if constexpr (!IsContainerType<T> && operator_le<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_le, [](const T& lhs, const T& rhs) { return static_cast<bool>(lhs <= rhs); });
        if constexpr (!IsContainerType<T> && operator_ge<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_ge, [](const T& lhs, const T& rhs) { return static_cast<bool>(lhs >= rhs); });

        if constexpr (operator_subscript<T, const std::size_t>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::operator_subscript, [](T& lhs, const std::size_t& rhs) -> decltype(auto) { return lhs[rhs]; });
        if constexpr (operator_subscript<const T, const std::size_t>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::operator_subscript, [](const T& lhs, const std::size_t& rhs) -> decltype(auto) { return lhs[rhs]; });
        if constexpr (operator_indirection<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_indirection, [](T& lhs) -> decltype(auto) { return *lhs; });
        if constexpr (operator_indirection<const T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_indirection, [](const T& lhs) -> decltype(auto) { return *lhs; });

        // iterator
        if constexpr (std::input_iterator<T>) {
            mngr.AddMemberMethod(NameIDRegistry::Meta::advance, [](T& lhs, const std::iter_difference_t<T>& rhs) { std::advance(lhs, rhs); });
            mngr.AddMemberMethod(NameIDRegistry::Meta::next, [](const T& lhs, const std::iter_difference_t<T>& rhs) -> decltype(auto) { return std::next(lhs, rhs); });
            mngr.AddMemberMethod(NameIDRegistry::Meta::prev, [](const T& lhs, const std::iter_difference_t<T>& rhs) -> decltype(auto) { return std::prev(lhs, rhs); });
            if constexpr (std::is_convertible_v<std::iter_difference_t<T>, std::size_t>) {
                mngr.AddMemberMethod(NameIDRegistry::Meta::distance, [](const T& lhs, const T& rhs) { return static_cast<std::size_t>(std::distance(lhs, rhs)); });
            }
            if constexpr (std::random_access_iterator<T>) {
                mngr.AddMemberMethod(NameIDRegistry::Meta::operator_add, [](const T& lhs, const std::iter_difference_t<T>& rhs) -> decltype(auto) { return lhs + rhs; });
                mngr.AddMemberMethod(NameIDRegistry::Meta::operator_sub, [](const T& lhs, const std::iter_difference_t<T>& rhs) -> decltype(auto) { return lhs - rhs; });
            }
        }

        // pair

        if constexpr (IsPair<T>) {
            mngr.RegisterType<typename T::first_type>();
            mngr.RegisterType<typename T::second_type>();
            mngr.AddField<&T::first>("first");
            mngr.AddField<&T::second>("second");
        }

        // tuple

        if constexpr (IsTuple<T> && !IsArray<T>) {
            mngr.AddStaticMethod(Type_of<T>, NameIDRegistry::Meta::tuple_size, []() { return static_cast<std::size_t>(std::tuple_size_v<T>); });
            mngr.AddMemberMethod(NameIDRegistry::Meta::get, [](T& t, const std::size_t& i) { return runtime_get<std::tuple_size>(t, i); });
            mngr.AddMemberMethod(NameIDRegistry::Meta::get, [](const T& t, const std::size_t& i) { return runtime_get<std::tuple_size>(t, i); });
            mngr.AddMemberMethod(NameIDRegistry::Meta::get, [](T& t, const Type& type) { return runtime_get<std::tuple_size, std::tuple_element>(t, type); });
            mngr.AddMemberMethod(NameIDRegistry::Meta::get, [](const T& t, const Type& type) { return runtime_get<std::tuple_size, std::tuple_element>(t, type); });
            mngr.AddStaticMethod(Type_of<T>, NameIDRegistry::Meta::tuple_element, [](const std::size_t& i) { return runtime_tuple_element<T>(i); });
            register_tuple_elements<T>(mngr, std::make_index_sequence<std::tuple_size_v<T>>{});
        }

        // variant

        if constexpr (IsVariant<T>) {
            mngr.AddMemberMethod(NameIDRegistry::Meta::variant_index, [](const T& t) { return static_cast<std::size_t>(t.index()); });
            mngr.AddMemberMethod(NameIDRegistry::Meta::variant_valueless_by_exception, [](const T& t) { return static_cast<bool>(t.valueless_by_exception()); });
            mngr.AddStaticMethod(Type_of<T>, NameIDRegistry::Meta::variant_size, []() { return static_cast<std::size_t>(std::variant_size_v<T>); });
            mngr.AddMemberMethod(NameIDRegistry::Meta::get, [](T& t, const std::size_t& i) { return runtime_get<std::variant_size>(t, i); });
            mngr.AddMemberMethod(NameIDRegistry::Meta::get, [](const T& t, const std::size_t& i) { return runtime_get<std::variant_size>(t, i); });
            mngr.AddMemberMethod(NameIDRegistry::Meta::holds_alternative, [](const T& t, const Type& type) { return runtime_variant_holds_alternative(t, type); });
            mngr.AddMemberMethod(NameIDRegistry::Meta::get, [](T& t, const Type& type) { return runtime_get<std::variant_size, std::variant_alternative>(t, type); });
            mngr.AddMemberMethod(NameIDRegistry::Meta::get, [](const T& t, const Type& type) { return runtime_get<std::variant_size, std::variant_alternative>(t, type); });
            mngr.AddStaticMethod(Type_of<T>, NameIDRegistry::Meta::variant_alternative, [](const std::size_t& i) { return runtime_variant_alternative<T>(i); });
            mngr.AddMemberMethod(NameIDRegistry::Meta::variant_visit_get, [](T& t) { return runtime_get<std::variant_size>(t, t.index()); });
            mngr.AddMemberMethod(NameIDRegistry::Meta::variant_visit_get, [](const T& t) { return runtime_get<std::variant_size>(t, t.index()); });
            register_variant_alternatives<T>(mngr, std::make_index_sequence<std::variant_size_v<T>>{});
        }

        // optional

        if constexpr (IsOptional<T>) {
            mngr.AddMemberMethod(NameIDRegistry::Meta::optional_has_value, [](const T& t) { return static_cast<bool>(t.has_value()); });
            mngr.AddMemberMethod(NameIDRegistry::Meta::optional_value, [](T& t) { return ObjectView{t.value()}; });
            mngr.AddMemberMethod(NameIDRegistry::Meta::optional_value, [](const T& t) { return ObjectView{t.value()}; });
            mngr.AddMemberMethod(NameIDRegistry::Meta::optional_reset, [](T& t) { t.reset(); });

            using Elem = typename T::value_type;
            if constexpr (type_ctor<T, const Elem&>) mngr.AddConstructor<T, const Elem&>();
            if constexpr (operator_assignment<T, const Elem&>) {

                mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assignment, [](T& t, const Elem& elem) -> T& {
                    t = elem;
                    return t;
                });
            }

            if constexpr (!std::is_fundamental_v<Elem>) {
                if constexpr (type_ctor<T, Elem&&>) mngr.AddConstructor<T, Elem&&>();
                if constexpr (operator_assignment<T, Elem&&>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_assignment, [](T& t, Elem&& elem) -> T& { return t = std::move(elem); });
            }
        }

        // container

        // - assign

        if constexpr (container_assign<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_assign, [](T& lhs, const typename T::size_type& s, const typename T::value_type& v) { lhs.assign(s, v); });

        // - iterator

        if constexpr (container_begin<T&>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_begin, [](T& lhs) -> decltype(auto) { return std::begin(lhs); });
        if constexpr (container_begin<const T&>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_begin, [](const T& lhs) -> decltype(auto) { return std::begin(lhs); });
        if constexpr (container_cbegin<const T&>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_cbegin, [](const T& lhs) -> decltype(auto) { return std::cbegin(lhs); });
        if constexpr (container_end<T&>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_end, [](T& lhs) -> decltype(auto) { return std::end(lhs); });
        if constexpr (container_end<const T&>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_end, [](const T& lhs) -> decltype(auto) { return std::end(lhs); });
        if constexpr (container_cend<const T&>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_cend, [](const T& lhs) -> decltype(auto) { return std::cend(lhs); });

        if constexpr (container_rbegin<T&>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_rbegin, [](T& lhs) -> decltype(auto) { return std::rbegin(lhs); });
        if constexpr (container_rbegin<const T&>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_rbegin, [](const T& lhs) -> decltype(auto) { return std::rbegin(lhs); });
        if constexpr (container_crbegin<const T&>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_crbegin, [](const T& lhs) -> decltype(auto) { return std::rbegin(lhs); });
        if constexpr (container_rend<T&>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_rend, [](T& lhs) -> decltype(auto) { return std::rend(lhs); });
        if constexpr (container_rend<const T&>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_rend, [](const T& lhs) -> decltype(auto) { return std::rend(lhs); });
        if constexpr (container_crend<const T&>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_crend, [](const T& lhs) -> decltype(auto) { return std::crend(lhs); });

        // - element access

        if constexpr (container_at_size<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_at, [](T& lhs, const std::size_t& n) -> decltype(auto) { return lhs.at(n); });
        if constexpr (container_at_size<const T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_at, [](const T& lhs, const std::size_t& n) -> decltype(auto) { return lhs.at(n); });

        if constexpr (container_at_key<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_at, [](T& lhs, const typename T::key_type& key) -> decltype(auto) { return lhs.at(key); });
        if constexpr (container_at_key<const T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_at, [](const T& lhs, const typename T::key_type& key) -> decltype(auto) { return lhs.at(key); });

        if constexpr (container_subscript_size<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::operator_subscript, [](T& lhs, const get_container_size_type_t<T>& rhs) -> decltype(auto) { return lhs[rhs]; });
        if constexpr (container_subscript_size<const T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::operator_subscript, [](const T& lhs, const get_container_size_type_t<T>& rhs) -> decltype(auto) { return lhs[rhs]; });

        if constexpr (container_subscript_key_cl<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::operator_subscript, [](T& lhs, const typename T::key_type& key) -> decltype(auto) { return lhs[key]; });
        if constexpr (container_subscript_key_r<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::operator_subscript, [](T& lhs, typename T::key_type&& key) -> decltype(auto) { return lhs[std::move(key)]; });

        if constexpr (container_data<T&>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_data, [](T& lhs) -> decltype(auto) { return std::data(lhs); });
        if constexpr (container_data<const T&>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_data, [](const T& lhs) -> decltype(auto) { return std::data(lhs); });

        if constexpr (container_front<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_front, [](T& lhs) -> decltype(auto) { return lhs.front(); });
        if constexpr (container_front<const T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_front, [](const T& lhs) -> decltype(auto) { return lhs.front(); });

        if constexpr (container_back<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_back, [](T& lhs) -> decltype(auto) { return lhs.back(); });
        if constexpr (container_back<const T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_back, [](const T& lhs) -> decltype(auto) { return lhs.back(); });

        if constexpr (container_top<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_top, [](T& lhs) -> decltype(auto) { return lhs.top(); });
        if constexpr (container_top<const T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_top, [](const T& lhs) -> decltype(auto) { return lhs.top(); });

        if constexpr (container_empty<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_empty, [](const T& lhs) { return static_cast<bool>(std::empty(lhs)); });

        if constexpr (container_size<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_size, [](const T& lhs) { return static_cast<std::size_t>(std::size(lhs)); });

        if constexpr (container_size_bytes<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_size_bytes, [](const T& lhs) { return static_cast<std::size_t>(lhs.size_bytes()); });

        if constexpr (container_resize_cnt<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_resize, [](T& lhs, const typename T::size_type& n) { lhs.resize(n); });

        if constexpr (container_resize_cnt_value<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_resize, [](T& lhs, const typename T::size_type& n, const typename T::value_type& value) { lhs.resize(n, value); });

        if constexpr (container_capacity<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_capacity, [](const T& lhs) { return static_cast<std::size_t>(lhs.capacity()); });

        if constexpr (container_bucket_count<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_bucket_count, [](const T& lhs) { return static_cast<std::size_t>(lhs.bucket_count()); });

        if constexpr (container_reserve<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_reserve, [](T& lhs, const typename T::size_type& n) { lhs.reserve(n); });

        if constexpr (container_shrink_to_fit<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_shrink_to_fit, [](T& lhs) { lhs.shrink_to_fit(); });

        // - modifiers

        if constexpr (container_clear<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_clear, [](T& lhs) { lhs.clear(); });

        if constexpr (container_insert_clvalue<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_insert, [](T& lhs, const typename T::value_type& value) -> decltype(auto) { return lhs.insert(value); });

        if constexpr (container_insert_rvalue<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_insert, [](T& lhs, typename T::value_type&& value) -> decltype(auto) { return lhs.insert(std::move(value)); });

        if constexpr (container_insert_rnode<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_insert, [](T& lhs, typename T::node_type&& node) -> decltype(auto) { return lhs.insert(std::move(node)); });

        if constexpr (container_insert_citer_clvalue<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_insert,
                                 [](T& lhs, const typename T::const_iterator& iter, const typename T::value_type& value) -> decltype(auto) { return lhs.insert(iter, value); });

        if constexpr (container_insert_citer_rvalue<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_insert,
                                 [](T& lhs, const typename T::const_iterator& iter, typename T::value_type&& value) -> decltype(auto) { return lhs.insert(iter, std::move(value)); });

        if constexpr (container_insert_citer_rnode<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_insert,
                                 [](T& lhs, const typename T::const_iterator& iter, typename T::node_type&& node) -> decltype(auto) { return lhs.insert(iter, std::move(node)); });

        if constexpr (container_insert_citer_cnt<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_insert,
                                 [](T& lhs, const typename T::const_iterator& iter, const typename T::size_type& cnt, const typename T::value_type& value) -> decltype(auto) {
                                     return lhs.insert(iter, cnt, value);
                                 });

        if constexpr (container_insert_after_clvalue<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_insert_after,
                                 [](T& lhs, const typename T::const_iterator& pos, const typename T::value_type& value) -> decltype(auto) { return lhs.insert_after(pos, value); });

        if constexpr (container_insert_after_rvalue<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_insert_after,
                                 [](T& lhs, const typename T::const_iterator& pos, typename T::value_type&& value) -> decltype(auto) { return lhs.insert_after(pos, std::move(value)); });

        if constexpr (container_insert_after_cnt<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_insert_after,
                                 [](T& lhs, const typename T::const_iterator& pos, const typename T::size_type& cnt, const typename T::value_type& value) -> decltype(auto) {
                                     return lhs.insert_after(pos, cnt, value);
                                 });

        if constexpr (container_erase_citer<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_erase, [](T& lhs, const typename T::const_iterator& rhs) -> decltype(auto) { return lhs.erase(rhs); });

        if constexpr (container_erase_key<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_erase, [](T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.erase(rhs); });

        if constexpr (container_erase_range_citer<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_erase,
                                 [](T& lhs, const typename T::const_iterator& start, const typename T::const_iterator& end) -> decltype(auto) { return lhs.erase(start, end); });

        if constexpr (container_erase_after<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_erase_after, [](T& lhs, const typename T::const_iterator& pos) -> decltype(auto) { return lhs.erase_after(pos); });

        if constexpr (container_erase_after_range<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_erase_after,
                                 [](T& lhs, const typename T::const_iterator& first, const typename T::const_iterator& last) -> decltype(auto) { return lhs.erase_after(first, last); });

        if constexpr (container_push_front_clvalue<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_push_front, [](T& lhs, const typename T::value_type& value) { lhs.push_front(value); });

        if constexpr (container_push_front_rvalue<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_push_front, [](T& lhs, typename T::value_type&& value) { lhs.push_front(std::move(value)); });

        if constexpr (container_pop_front<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_pop_front, [](T& lhs) { lhs.pop_front(); });

        if constexpr (container_push_back_clvalue<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_push_back, [](T& lhs, const typename T::value_type& value) { lhs.push_back(value); });

        if constexpr (container_push_back_rvalue<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_push_back, [](T& lhs, typename T::value_type&& value) { lhs.push_back(std::move(value)); });

        if constexpr (container_pop_back<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_pop_back, [](T& lhs) { lhs.pop_back(); });

        if constexpr (container_push_clvalue<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_push, [](T& lhs, const typename T::value_type& value) { lhs.push(value); });

        if constexpr (container_push_rvalue<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_push, [](T& lhs, typename T::value_type&& value) { lhs.push(std::move(value)); });

        if constexpr (container_pop<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_pop, [](T& lhs) { lhs.pop(); });

        if constexpr (container_swap<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_swap, [](T& lhs, T& rhs) { std::swap(lhs, rhs); });

        if constexpr (container_merge_l<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_merge, [](T& lhs, T& rhs) { lhs.merge(rhs); });

        if constexpr (container_merge_r<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_merge, [](T& lhs, T&& rhs) { lhs.merge(std::move(rhs)); });

        if constexpr (container_extract_citer<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_extract, [](T& lhs, const typename T::const_iterator& iter) -> decltype(auto) { return lhs.extract(iter); });

        if constexpr (container_extract_key<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_extract, [](T& lhs, const typename T::key_type& key) -> decltype(auto) { return lhs.extract(key); });

        // - list operations

        if constexpr (container_splice_after_l<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_splice_after, [](T& lhs, const typename T::const_iterator& pos, T& other) { lhs.splice_after(pos, other); });

        if constexpr (container_splice_after_r<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_splice_after, [](T& lhs, const typename T::const_iterator& pos, T&& other) { lhs.splice_after(pos, std::move(other)); });

        if constexpr (container_splice_after_it_l<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_splice_after,
                                 [](T& lhs, const typename T::const_iterator& pos, T& other, const typename T::const_iterator& it) { lhs.splice_after(pos, other, it); });

        if constexpr (container_splice_after_it_r<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_splice_after,
                                 [](T& lhs, const typename T::const_iterator& pos, T&& other, const typename T::const_iterator& it) { lhs.splice_after(pos, std::move(other), it); });

        if constexpr (container_splice_after_range_l<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_splice_after, [](T& lhs, const typename T::const_iterator& pos, T& other, const typename T::const_iterator& first,
                                                                                  const typename T::const_iterator& last) { lhs.splice_after(pos, other, first, last); });

        if constexpr (container_splice_after_range_l<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_splice_after, [](T& lhs, const typename T::const_iterator& pos, T&& other, const typename T::const_iterator& first,
                                                                                  const typename T::const_iterator& last) { lhs.splice_after(pos, std::move(other), first, last); });

        if constexpr (container_splice_l<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_splice, [](T& lhs, const typename T::const_iterator& pos, T& other) { lhs.splice(pos, other); });

        if constexpr (container_splice_r<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_splice, [](T& lhs, const typename T::const_iterator& pos, T&& other) { lhs.splice(pos, std::move(other)); });

        if constexpr (container_splice_it_l<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_splice,
                                 [](T& lhs, const typename T::const_iterator& pos, T& other, const typename T::const_iterator& it) { lhs.splice(pos, other, it); });

        if constexpr (container_splice_it_r<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_splice,
                                 [](T& lhs, const typename T::const_iterator& pos, T&& other, const typename T::const_iterator& it) { lhs.splice(pos, std::move(other), it); });

        if constexpr (container_splice_range_l<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_splice, [](T& lhs, const typename T::const_iterator& pos, T& other, const typename T::const_iterator& first,
                                                                            const typename T::const_iterator& last) { lhs.splice(pos, other, first, last); });

        if constexpr (container_splice_range_l<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_splice, [](T& lhs, const typename T::const_iterator& pos, T&& other, const typename T::const_iterator& first,
                                                                            const typename T::const_iterator& last) { lhs.splice(pos, std::move(other), first, last); });

        if constexpr (container_remove<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_remove, [](T& lhs, const typename T::value_type& v) { return static_cast<std::size_t>(lhs.remove(v)); });

        if constexpr (container_reverse<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_reverse, [](T& lhs) { lhs.reverse(); });

        if constexpr (container_unique<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_unique, [](T& lhs) { return static_cast<std::size_t>(lhs.unique()); });

        if constexpr (container_sort<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_sort, [](T& lhs) { lhs.sort(); });

        // - lookup

        if constexpr (container_count<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_count, [](const T& lhs, const typename T::key_type& rhs) { return static_cast<std::size_t>(lhs.count(rhs)); });

        if constexpr (container_find<T>) mngr.AddMemberMethod(NameIDRegistry::Meta::container_find, [](T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.find(rhs); });
        if constexpr (container_find<const T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_find, [](const T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.find(rhs); });

        if constexpr (container_lower_bound<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_lower_bound, [](T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.lower_bound(rhs); });
        if constexpr (container_lower_bound<const T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_lower_bound, [](const T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.lower_bound(rhs); });

        if constexpr (container_upper_bound<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_upper_bound, [](T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.upper_bound(rhs); });
        if constexpr (container_upper_bound<const T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_upper_bound, [](const T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.upper_bound(rhs); });

        if constexpr (container_equal_range<T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_equal_range, [](T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.equal_range(rhs); });
        if constexpr (container_equal_range<const T>)
            mngr.AddMemberMethod(NameIDRegistry::Meta::container_equal_range, [](const T& lhs, const typename T::key_type& rhs) -> decltype(auto) { return lhs.equal_range(rhs); });

        if constexpr (IsVector<T>)
            mngr.AddTypeAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, TempArgsView{ContainerType::Vector}));
        else if constexpr (IsArray<T>)
            mngr.AddTypeAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, TempArgsView{ContainerType::Array}));
        else if constexpr (IsRawArray<T>)
            mngr.AddTypeAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, TempArgsView{ContainerType::RawArray}));
        else if constexpr (IsDeque<T>)
            mngr.AddTypeAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, TempArgsView{ContainerType::Deque}));
        else if constexpr (IsForwardList<T>)
            mngr.AddTypeAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, TempArgsView{ContainerType::ForwardList}));
        else if constexpr (IsList<T>)
            mngr.AddTypeAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, TempArgsView{ContainerType::List}));
        else if constexpr (IsMap<T>)
            mngr.AddTypeAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, TempArgsView{ContainerType::Map}));
        else if constexpr (IsMultiMap<T>)
            mngr.AddTypeAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, TempArgsView{ContainerType::MultiMap}));
        else if constexpr (IsSet<T>)
            mngr.AddTypeAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, TempArgsView{ContainerType::Set}));
        else if constexpr (IsMultiSet<T>)
            mngr.AddTypeAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, TempArgsView{ContainerType::MultiSet}));
        else if constexpr (IsUnorderedMap<T>)
            mngr.AddTypeAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, TempArgsView{ContainerType::UnorderedMap}));
        else if constexpr (IsUnorderedMultiMap<T>)
            mngr.AddTypeAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, TempArgsView{ContainerType::UnorderedMultiMap}));
        else if constexpr (IsUnorderedSet<T>)
            mngr.AddTypeAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, TempArgsView{ContainerType::UnorderedSet}));
        else if constexpr (IsUnorderedMultiSet<T>)
            mngr.AddTypeAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, TempArgsView{ContainerType::UnorderedMultiSet}));
        else if constexpr (IsStack<T>)
            mngr.AddTypeAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, TempArgsView{ContainerType::Stack}));
        else if constexpr (IsPriorityQueue<T>)
            mngr.AddTypeAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, TempArgsView{ContainerType::PriorityQueue}));
        else if constexpr (IsQueue<T>)
            mngr.AddTypeAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, TempArgsView{ContainerType::Queue}));
        else if constexpr (IsPair<T>)
            mngr.AddTypeAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, TempArgsView{ContainerType::Pair}));
        else if constexpr (IsTuple<T>)
            mngr.AddTypeAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, TempArgsView{ContainerType::Tuple}));
        else if constexpr (IsSpan<T>)
            mngr.AddTypeAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, TempArgsView{ContainerType::Span}));
        else if constexpr (IsVariant<T>)
            mngr.AddTypeAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, TempArgsView{ContainerType::Variant}));
        else if constexpr (IsOptional<T>)
            mngr.AddTypeAttr(Type_of<T>, mngr.MakeShared(Type_of<ContainerType>, TempArgsView{ContainerType::Optional}));

        // - type

        if constexpr (std::is_array_v<T>) {
            using value_type = std::remove_extent_t<T>;
            using pointer = value_type*;
            using const_pointer = const value_type*;
            mngr.RegisterType<value_type>();
            mngr.RegisterType<pointer>();
            mngr.RegisterType<const_pointer>();
        } else {

            if constexpr (container_key_type<T>) mngr.RegisterType<typename T::key_type>();
            if constexpr (container_mapped_type<T>) mngr.RegisterType<typename T::mapped_type>();
            if constexpr (container_value_type<T>) mngr.RegisterType<typename T::value_type>();
            if constexpr (container_size_type<T>) mngr.RegisterType<typename T::size_type>();
            if constexpr (container_difference_type<T>) mngr.RegisterType<typename T::difference_type>();
            if constexpr (!is_instance_of_v<T, std::allocator>) {
                if constexpr (container_pointer_type<T>) mngr.RegisterType<typename T::pointer>();
                if constexpr (container_const_pointer_type<T>) mngr.RegisterType<typename T::const_pointer>();
            }
            if constexpr (container_iterator<T>) {
                mngr.RegisterType<typename T::iterator>();
                if constexpr (IsMultiSet<T> || IsUnorderedMultiSet<T>) mngr.RegisterType<std::pair<typename T::iterator, bool>>();
            }
            if constexpr (container_const_iterator<T>) mngr.RegisterType<typename T::const_iterator>();
            if constexpr (container_local_iterator<T>) mngr.RegisterType<typename T::local_iterator>();
            if constexpr (container_const_local_iterator<T>) mngr.RegisterType<typename T::const_local_iterator>();
            if constexpr (container_node_type<T>) mngr.RegisterType<typename T::node_type>();
            if constexpr (container_insert_return_type<T>) {
                mngr.RegisterType<typename T::insert_return_type>();
                mngr.AddField<&T::insert_return_type::position>("position");
                mngr.AddField<&T::insert_return_type::inserted>("inserted");
                mngr.AddField<&T::insert_return_type::node>("node");
            }

            if constexpr (container_iterator<T> && container_const_iterator<T>) mngr.AddConstructor<typename T::const_iterator, const typename T::iterator&>();
            if constexpr (container_local_iterator<T> && container_const_local_iterator<T>) mngr.AddConstructor<typename T::const_local_iterator, const typename T::local_iterator&>();
        }
    }
};

template <typename T>
struct TypeAutoRegister : TypeAutoRegister_Default<T> {};
};  // namespace neko::cpp::details

namespace neko::cpp {
//
// Factory
////////////

template <auto field_data, bool NeedRegisterFieldType>
FieldPtr neko_refl::GenerateFieldPtr() {
    using FieldData = decltype(field_data);
    if constexpr (std::is_pointer_v<FieldData>) {
        using Value = std::remove_pointer_t<FieldData>;
        if constexpr (NeedRegisterFieldType) RegisterType<Value>();
        return {Type_of<Value>, ptr_const_cast(field_data)};
    } else if constexpr (std::is_member_object_pointer_v<FieldData>) {
        using Traits = member_pointer_traits<FieldData>;
        using Obj = typename Traits::object;
        using Value = typename Traits::value;

        if constexpr (NeedRegisterFieldType) RegisterType<Value>();
        if constexpr (has_virtual_base_v<Obj>) {
            return FieldPtr{Type_of<Value>, field_offsetor<field_data>()};
        } else {
            return FieldPtr{Type_of<Value>, field_forward_offset_value(field_data)};
        }
    } else if constexpr (std::is_enum_v<FieldData>) {
        if constexpr (NeedRegisterFieldType) RegisterType<FieldData>();
        auto buffer = FieldPtr::ConvertToBuffer(field_data);
        return {Type_of<const FieldData>, buffer};
    } else
        static_assert(always_false<FieldData>);
}

template <typename T, bool NeedRegisterFieldType>
FieldPtr neko_refl::GenerateFieldPtr(T&& data) {
    using RawT = std::remove_cv_t<std::remove_reference_t<T>>;
    static_assert(!std::is_same_v<RawT, std::size_t>);
    if constexpr (std::is_member_object_pointer_v<RawT>) {
        using Traits = member_pointer_traits<RawT>;
        using Obj = typename Traits::object;
        using Value = typename Traits::value;
        tregistry.Register<Value>();
        if constexpr (NeedRegisterFieldType) RegisterType<Value>();
        if constexpr (has_virtual_base_v<Obj>) {
            return {Type_of<Value>, field_offsetor(data)};
        } else {
            return {Type_of<Value>, field_forward_offset_value(data)};
        }
    } else if constexpr (std::is_pointer_v<RawT> && !is_function_pointer_v<RawT> && !std::is_void_v<std::remove_pointer_t<RawT>>) {
        using Value = std::remove_pointer_t<RawT>;
        tregistry.Register<Value>();
        if constexpr (NeedRegisterFieldType) RegisterType<Value>();
        return {Type_of<Value>, ptr_const_cast(data)};
    } else if constexpr (std::is_enum_v<RawT>) {
        tregistry.Register<RawT>();
        if constexpr (NeedRegisterFieldType) RegisterType<RawT>();
        auto buffer = FieldPtr::ConvertToBuffer(data);
        return {Type_of<const RawT>, buffer};
    } else {
        using Traits = FuncTraits<RawT>;

        using ArgList = typename Traits::ArgList;
        static_assert(Length_v<ArgList> == 1);
        using ObjPtr = Front_t<ArgList>;
        static_assert(std::is_pointer_v<ObjPtr>);
        using Obj = std::remove_pointer_t<ObjPtr>;
        static_assert(!std::is_const_v<Obj>);

        using Ret = typename Traits::Return;
        if constexpr (std::is_pointer_v<Ret>) {
            using Value = std::remove_pointer_t<Ret>;
            static_assert(!std::is_void_v<Value>);

            tregistry.Register<Value>();
            if constexpr (NeedRegisterFieldType) RegisterType<Value>();

            auto offsetor = [f = std::forward<T>(data)](void* obj) -> void* { return f(reinterpret_cast<Obj*>(obj)); };

            return {Type_of<Value>, offsetor};
        } else if constexpr (std::is_reference_v<Ret>) {
            tregistry.Register<Ret>();
            if constexpr (NeedRegisterFieldType) RegisterType<Ret>();

            auto offsetor = [f = std::forward<T>(data)](void* obj) -> void* { return &f(reinterpret_cast<Obj*>(obj)); };

            return {Type_of<Ret>, offsetor};
        } else
            static_assert(always_false<T>);
    }
}

template <typename T, typename... Args>
FieldPtr neko_refl::GenerateDynamicFieldPtr(Args&&... args) {
    return details::GenerateDynamicFieldPtr<true, T>(*this, std::forward<Args>(args)...);
}

template <typename T, typename... Args>
FieldPtr neko_refl::SimpleGenerateDynamicFieldPtr(Args&&... args) {
    return details::GenerateDynamicFieldPtr<false, T>(*this, std::forward<Args>(args)...);
}

template <typename... Params>
ParamList neko_refl::GenerateParamList() noexcept(sizeof...(Params) == 0) {
    if constexpr (sizeof...(Params) > 0) {
        static_assert(((!std::is_const_v<Params> && !std::is_volatile_v<Params>)&&...), "parameter type shouldn't be const.");
        return {Type_of<Params>...};
    } else
        return {};
}

template <auto funcptr>
MethodPtr neko_refl::GenerateMethodPtr() {
    using FuncPtr = decltype(funcptr);
    using Traits = FuncTraits<decltype(funcptr)>;
    using ArgList = typename Traits::ArgList;
    using Return = typename Traits::Return;
    constexpr MethodFlag flag = Traits::is_const ? MethodFlag::Const : MethodFlag::Variable;
    return {details::wrap_function<funcptr>(), flag, Type_of<Return>, details::GenerateParamListHelper<ArgList>::get()};
}

template <typename T, typename... Args>
MethodPtr neko_refl::GenerateConstructorPtr() {
    return GenerateMemberMethodPtr([](T& obj, Args... args) {
        if constexpr (std::is_constructible_v<T, Args...>)
            new (&obj) T(std::forward<Args>(args)...);
        else if constexpr (std::is_aggregate_v<T>)
            new (&obj) T{std::forward<Args>(args)...};
        else
            static_assert(always_false<T>);
    });
}

template <typename T>
MethodPtr neko_refl::GenerateDestructorPtr() {
    return GenerateMemberMethodPtr([](T& obj) {
        if constexpr (!std::is_trivially_destructible_v<T>) obj.~T();
    });
}

template <typename Func>
MethodPtr neko_refl::GenerateMemberMethodPtr(Func&& func) {
    using Traits = details::WrapFuncTraits<std::decay_t<Func>>;
    using ArgList = typename Traits::ArgList;
    using Return = typename Traits::Return;
    constexpr MethodFlag flag = Traits::is_const ? MethodFlag::Const : MethodFlag::Variable;
    return {details::wrap_member_function(std::forward<Func>(func)), flag, Type_of<Return>, details::GenerateParamListHelper<ArgList>::get()};
}

template <typename Func>
MethodPtr neko_refl::GenerateStaticMethodPtr(Func&& func) {
    using Traits = FuncTraits<std::decay_t<Func>>;
    using Return = typename Traits::Return;
    using ArgList = typename Traits::ArgList;
    return {details::wrap_static_function(std::forward<Func>(func)), MethodFlag::Static, Type_of<Return>, details::GenerateParamListHelper<ArgList>::get()};
}

//
// Modifier
/////////////

template <typename T>
void neko_refl::RegisterType() {
    static_assert(!std::is_volatile_v<T>);
    if constexpr (std::is_void_v<T>)
        return;
    else {
        if constexpr (std::is_const_v<T>)
            RegisterType<std::remove_const_t<T>>();
        else if constexpr (std::is_reference_v<T>)
            RegisterType<std::remove_cvref_t<T>>();
        else {
            if (typeinfos.contains(Type_of<T>)) return;

            tregistry.Register<T>();
            RegisterType(Type_of<T>, std::is_empty_v<T> ? 0 : sizeof(T), alignof(T), std::is_polymorphic_v<T>, std::is_trivial_v<T>);

            details::TypeAutoRegister<T>::run(*this);
        }
    }
}

template <auto field_data, bool NeedRegisterFieldType>
bool neko_refl::AddField(Name name, AttrSet attrs) {
    using FieldData = decltype(field_data);
    if constexpr (std::is_enum_v<FieldData>) {
        return AddField(Type_of<const FieldData>, name, {GenerateFieldPtr<field_data, NeedRegisterFieldType>(), std::move(attrs)});
    } else if constexpr (std::is_member_object_pointer_v<FieldData>) {
        return AddField(Type_of<member_pointer_traits_object<FieldData>>, name, {GenerateFieldPtr<field_data, NeedRegisterFieldType>(), std::move(attrs)});
    } else
        static_assert(always_false<FieldData>, "if field_data is a static field, use AddField(Type, name, field_data, attrs)");
}

template <typename T, bool NeedRegisterFieldType>
    requires std::negation_v<std::is_same<std::decay_t<T>, FieldInfo>>
bool neko_refl::AddField(Name name, T&& data, AttrSet attrs) {
    using RawT = std::remove_cv_t<std::remove_reference_t<T>>;
    if constexpr (std::is_member_object_pointer_v<RawT>)
        return AddField<T, NeedRegisterFieldType>(Type_of<member_pointer_traits_object<RawT>>, name, std::forward<T>(data), std::move(attrs));
    else if constexpr (std::is_enum_v<RawT>)
        return AddField<T, NeedRegisterFieldType>(Type_of<const RawT>, name, std::forward<T>(data), std::move(attrs));
    else {
        using Traits = FuncTraits<RawT>;

        using ArgList = typename Traits::ArgList;
        static_assert(Length_v<ArgList> == 1);
        using ObjPtr = Front_t<ArgList>;
        static_assert(std::is_pointer_v<ObjPtr>);
        using Obj = std::remove_pointer_t<ObjPtr>;

        return AddField<T, NeedRegisterFieldType>(Type_of<Obj>, name, std::forward<T>(data), std::move(attrs));
    }
}

template <auto member_func_ptr>
bool neko_refl::AddMethod(Name name, AttrSet attrs) {
    using MemberFuncPtr = decltype(member_func_ptr);
    static_assert(std::is_member_function_pointer_v<MemberFuncPtr>);
    using Obj = member_pointer_traits_object<MemberFuncPtr>;
    return AddMethod(Type_of<Obj>, name, {GenerateMethodPtr<member_func_ptr>(), std::move(attrs)});
}

template <auto func_ptr>
bool neko_refl::AddMethod(Type type, Name name, AttrSet attrs) {
    using FuncPtr = decltype(func_ptr);
    static_assert(is_function_pointer_v<FuncPtr>);
    return AddMethod(type, name, {GenerateMethodPtr<func_ptr>(), std::move(attrs)});
}

template <typename T, typename... Args>
bool neko_refl::AddConstructor(AttrSet attrs) {
    return AddMethod(Type_of<T>, NameIDRegistry::Meta::ctor, {GenerateConstructorPtr<T, Args...>(), std::move(attrs)});
}
template <typename T>
bool neko_refl::AddDestructor(AttrSet attrs) {
    return AddMethod(Type_of<T>, NameIDRegistry::Meta::dtor, {GenerateDestructorPtr<T>(), std::move(attrs)});
}

template <typename Func>
bool neko_refl::AddMemberMethod(Name name, Func&& func, AttrSet attrs) {
    return AddMethod(Type_of<typename details::WrapFuncTraits<std::decay_t<Func>>::Object>, name, {GenerateMemberMethodPtr(std::forward<Func>(func)), std::move(attrs)});
}

template <typename Derived, typename Base>
BaseInfo neko_refl::GenerateBaseInfo() {
    return {inherit_cast_functions<Derived, Base>()};
}

template <typename Derived, typename... Bases>
bool neko_refl::AddBases() {
    return (AddBase(Type_of<Derived>, Type_of<Bases>, GenerateBaseInfo<Derived, Bases>()) && ...);
}

//
// Invoke
///////////

template <typename... Args>
Type neko_refl::IsInvocable(Type type, Name method_name, MethodFlag flag) const {
    constexpr Type argTypes[] = {Type_of<Args>...};
    return IsInvocable(type, method_name, argTypes, flag);
}

template <typename T>
T neko_refl::Invoke(ObjectView obj, Name method_name, ArgsView args, MethodFlag flag, std::pmr::memory_resource* temp_args_rsrc) const {
    if constexpr (!std::is_void_v<T>) {
        using U = std::conditional_t<std::is_reference_v<T>, std::add_pointer_t<T>, T>;
        std::aligned_storage_t<sizeof(U), alignof(U)> result_buffer;
        Type result_type = BInvoke(obj, method_name, &result_buffer, args, flag, temp_args_rsrc);
        return MoveResult<T>(result_type, &result_buffer);
    } else
        BInvoke(obj, method_name, nullptr, args, flag, temp_args_rsrc);
}

//
// Make
/////////

template <typename... Args>
bool neko_refl::IsConstructible(Type type) const {
    constexpr Type argTypes[] = {Type_of<Args>...};
    return IsConstructible(type, std::span<const Type>{argTypes});
}
}  // namespace neko::cpp

namespace neko::cpp::Ranges {
struct Derived {
    ObjectView obj;
    TypeInfo* typeinfo;  // not nullptr
    std::unordered_map<Type, BaseInfo>::iterator curbase;

    friend bool operator==(const Derived& lhs, const Derived& rhs) { return lhs.obj.GetType() == rhs.obj.GetType() && lhs.obj.GetPtr() == rhs.obj.GetPtr() && lhs.curbase == rhs.curbase; }
};
}  // namespace neko::cpp::Ranges

namespace neko::cpp {
class ObjectTree {
public:
    // DFS
    // TypeInfo* and BaseInfo* maybe nullptr
    class iterator {
    public:
        using value_type = std::tuple<TypeInfo*, ObjectView>;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::forward_iterator_tag;

        // true: begin
        // false: end
        iterator(ObjectView obj, bool begin_or_end);

        iterator& operator++();
        iterator operator++(int);

        reference operator*() const noexcept { return value; }
        pointer operator->() const noexcept { return &value; }

        friend bool operator==(const iterator& lhs, const iterator& rhs);
        friend bool operator!=(const iterator& lhs, const iterator& rhs);

        bool Valid() const noexcept { return mode != -1; }
        std::span<const Ranges::Derived> GetDeriveds() const noexcept { return {deriveds.begin(), deriveds.end()}; }

    private:
        friend ObjectTree;

        void update();

        std::vector<Type> visitedVBs;
        std::vector<Ranges::Derived> deriveds;
        bool curbase_valid;
        int mode;

        value_type value;
    };

    constexpr explicit ObjectTree(ObjectView obj) noexcept : obj{obj.RemoveConstReference()} {}

    constexpr explicit ObjectTree(Type type) noexcept : obj{ObjectView{type}} {}

    iterator begin() const { return {obj, true}; }
    iterator end() const noexcept { return {obj, false}; }

private:
    ObjectView obj;
};

template <typename T>
static constexpr ObjectTree ObjectTree_of = ObjectTree{ObjectView_of<T>};
}  // namespace neko::cpp

namespace neko::cpp {
// DFS
class FieldRange {
public:
    class iterator {
    public:
        using value_type = std::pair<const Name, FieldInfo>;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::forward_iterator_tag;

        iterator(ObjectTree::iterator typeiter, FieldFlag flag = FieldFlag::All);

        iterator& operator++();
        iterator operator++(int);

        reference operator*() const noexcept { return *curfield; }
        pointer operator->() const noexcept { return curfield.operator->(); }

        friend bool operator==(const iterator& lhs, const iterator& rhs);
        friend bool operator!=(const iterator& lhs, const iterator& rhs);

        bool Valid() const noexcept { return typeiter.Valid(); }
        std::span<const Ranges::Derived> GetDeriveds() const noexcept { return typeiter.GetDeriveds(); }
        ObjectView GetObjectView() const { return std::get<ObjectView>(*typeiter); }
        TypeInfo* GetTypeInfo() const { return std::get<TypeInfo*>(*typeiter); }

    private:
        void update();
        ObjectTree::iterator typeiter;
        FieldFlag flag;
        int mode;
        std::unordered_map<Name, FieldInfo>::iterator curfield;
    };

    constexpr FieldRange(ObjectView obj, FieldFlag flag) noexcept : objtree{ObjectTree{obj}}, flag{flag} {}
    constexpr explicit FieldRange(ObjectView obj) noexcept : FieldRange{obj, FieldFlag::All} {}
    constexpr explicit FieldRange(Type type) noexcept : FieldRange{ObjectView{type}, FieldFlag::All} {}
    constexpr FieldRange(Type type, FieldFlag flag) noexcept : FieldRange{ObjectView{type}, flag} {}

    iterator begin() const { return {objtree.begin(), flag}; }
    iterator end() const noexcept { return {objtree.end(), flag}; }

private:
    ObjectTree objtree;
    FieldFlag flag;
};

template <typename T, FieldFlag flag = FieldFlag::All>
static constexpr FieldRange FieldRange_of = FieldRange{ObjectView_of<T>, flag};
}  // namespace neko::cpp

namespace neko::cpp {
// DFS
class MethodRange {
public:
    class iterator {
    public:
        using value_type = std::pair<const Name, MethodInfo>;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::forward_iterator_tag;

        iterator(ObjectTree::iterator typeiter, MethodFlag flag = MethodFlag::All);

        iterator& operator++();
        iterator operator++(int);

        reference operator*() const noexcept { return *curmethod; }
        pointer operator->() const noexcept { return curmethod.operator->(); }

        friend bool operator==(const iterator& lhs, const iterator& rhs);
        friend bool operator!=(const iterator& lhs, const iterator& rhs);

        bool Valid() const noexcept { return typeiter.Valid(); }
        std::span<const Ranges::Derived> GetDeriveds() const noexcept { return typeiter.GetDeriveds(); }
        ObjectView GetObjectView() const { return std::get<ObjectView>(*typeiter); }
        TypeInfo* GetTypeInfo() const { return std::get<TypeInfo*>(*typeiter); }

    private:
        void update();
        ObjectTree::iterator typeiter;
        MethodFlag flag;
        int mode;
        std::unordered_map<Name, MethodInfo>::iterator curmethod;
    };

    constexpr MethodRange(ObjectView obj, MethodFlag flag) noexcept : objtree{ObjectTree{obj}}, flag{flag} {}
    constexpr explicit MethodRange(ObjectView obj) noexcept : MethodRange{obj, MethodFlag::All} {}
    constexpr explicit MethodRange(Type type) noexcept : MethodRange{ObjectView{type}, MethodFlag::All} {}
    constexpr MethodRange(Type type, MethodFlag flag) noexcept : MethodRange{ObjectView{type}, flag} {}

    iterator begin() const { return {objtree.begin(), flag}; }
    iterator end() const noexcept { return {objtree.end(), flag}; }

private:
    ObjectTree objtree;
    MethodFlag flag;
};

template <typename T, MethodFlag flag = MethodFlag::All>
static constexpr MethodRange MethodRange_of = MethodRange{ObjectView_of<T>, flag};
}  // namespace neko::cpp

namespace neko::cpp {
class VarRange {
public:
    // DFS
    // TypeInfo* and BaseInfo* maybe nullptr
    class iterator {
    public:
        using value_type = std::tuple<Name, ObjectView>;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::forward_iterator_tag;

        struct Derived {
            Type type;
            TypeInfo* typeinfo;  // not nullptr
            std::unordered_map<Type, BaseInfo>::iterator curbase;
        };

        iterator(ObjectTree::iterator typeiter, CVRefMode cvref_mode = CVRefMode::None, FieldFlag flag = FieldFlag::All);

        iterator& operator++();
        iterator operator++(int);

        reference operator*() const noexcept { return value; }
        pointer operator->() const noexcept { return &value; }

        friend bool operator==(const iterator& lhs, const iterator& rhs);
        friend bool operator!=(const iterator& lhs, const iterator& rhs);

        bool Valid() const noexcept { return mode != -1; }

        std::span<const Ranges::Derived> GetDeriveds() const noexcept { return typeiter.GetDeriveds(); }

        ObjectView GetObjectView() const { return std::get<ObjectView>(*typeiter); }
        TypeInfo* GetTypeInfo() const { return std::get<TypeInfo*>(*typeiter); }
        FieldInfo& GetFieldInfo() const { return curfield->second; }

    private:
        void update();

        CVRefMode cvref_mode;  // fix
        FieldFlag flag;        // fix

        ObjectTree::iterator typeiter;
        std::unordered_map<Name, FieldInfo>::iterator curfield;
        int mode;

        value_type value;
    };

    constexpr VarRange(ObjectView obj, FieldFlag flag) noexcept
        : objtree{ObjectTree{obj}}, flag{obj.GetPtr() ? flag : enum_within(flag, FieldFlag::Unowned)}, cvref_mode{obj.GetType().get_cvref_mode()} {
        neko_assert(!CVRefMode_IsVolatile(cvref_mode));
    }

    constexpr explicit VarRange(ObjectView obj) noexcept : VarRange{obj, FieldFlag::All} {}
    constexpr explicit VarRange(Type type) noexcept : VarRange{ObjectView{type}, FieldFlag::Unowned} {}
    constexpr VarRange(Type type, FieldFlag flag) noexcept : VarRange{ObjectView{type}, flag} {}

    iterator begin() const { return {objtree.begin(), cvref_mode, flag}; }
    iterator end() const noexcept { return {objtree.end(), cvref_mode, flag}; }

private:
    ObjectTree objtree;
    FieldFlag flag;
    CVRefMode cvref_mode;
};

template <typename T, FieldFlag flag = FieldFlag::Unowned>
static constexpr VarRange VarRange_of = VarRange{ObjectView_of<T>, flag};
}  // namespace neko::cpp

namespace neko::cpp::details {
// parameter <- argument
// - same
// - reference
// > - 0 (invalid), 1 (convertible)
// > - table
//     |    -     | T | T & | const T & | T&& | const T&& | const T |
//     |      T   | - |  0  |     0     |  1  |     0     |    0    |
//     |      T & | 0 |  -  |     0     |  0  |     0     |    0    |
//     |const T & | 0 |  0  |     -     |  0  |     0     |    1    |
//     |      T&& | 1 |  0  |     0     |  -  |     0     |    0    |
//     |const T&& | 0 |  0  |     0     |  0  |     -     |    0    |
bool IsPriorityCompatible(std::span<const Type> params, std::span<const Type> argTypes);

// parameter <- argument
// - same
// - reference
// > - 0 (invalid), 1 (convertible)
// > - table
//     |     -     | T | T & | const T & | T&& | const T&& | const T |
//     |       T   | - |  0  |     0     |  1  |     0     |    0    |
//     |       T & | 0 |  -  |     0     |  0  |     0     |    0    |
//     | const T & | 1 |  1  |     -     |  1  |     1     |    1    |
//     |       T&& | 1 |  0  |     0     |  -  |     0     |    0    |
//     | const T&& | 1 |  0  |     0     |  1  |     -     |    1    |
bool IsRefCompatible(std::span<const Type> paramTypes, std::span<const Type> argTypes);

// parameter <- argument
// - same
// - reference
// > - 0 (invalid), 1 (convertible)
// > - table
//     |     -     | T | T & | const T & | T&& | const T&& | const T |
//     |       T   | - |  0  |     0     |  1  |     0     |    0    |
//     |       T & | 0 |  -  |     0     |  0  |     0     |    0    |
//     | const T & | 1 |  1  |     -     |  1  |     1     |    1    |
//     |       T&& | 1 |  0  |     0     |  -  |     0     |    0    |
//     | const T&& | 1 |  0  |     0     |  1  |     -     |    1    |
bool IsRefCompatible(std::span<const Type> paramTypes, std::span<const TypeID> argTypeIDs);

bool IsRefConstructible(Type paramType, std::span<const Type> argTypes);
bool RefConstruct(ObjectView obj, ArgsView args);

class BufferGuard {
public:
    BufferGuard() : rsrc{nullptr}, size{0}, alignment{0}, buffer{nullptr} {}
    BufferGuard(std::pmr::memory_resource* rsrc, std::size_t size, std::size_t alignment) : rsrc{rsrc}, size{size}, alignment{alignment}, buffer{rsrc->allocate(size, alignment)} {}

    ~BufferGuard() {
        if (rsrc) rsrc->deallocate(buffer, size, alignment);
    }

    void* Get() const noexcept { return buffer; }
    operator void*() const noexcept { return Get(); }

    BufferGuard(const BufferGuard&) = delete;
    BufferGuard& operator=(BufferGuard&&) noexcept = delete;

private:
    std::pmr::memory_resource* rsrc;
    std::size_t size;
    std::size_t alignment;
    void* buffer;
};

class NewArgsGuard {
    struct ArgInfo {
        const char* name;
        std::size_t name_hash;
        std::uint32_t offset;
        std::uint16_t name_size;
        std::uint8_t idx;
        bool is_pointer_or_array;
        Type GetType() const noexcept { return {std::string_view{name, name_size}, TypeID{name_hash}}; }
    };  // 24 bytes
    // MaxArgNum <= 2^8
    static_assert(sizeof(ArgInfo) * MaxArgNum < 16384);

public:
    NewArgsGuard(bool is_priority, std::pmr::memory_resource* rsrc, std::span<const Type> paramTypes, ArgsView args);

    ~NewArgsGuard();

    NewArgsGuard(const NewArgsGuard&) = delete;
    NewArgsGuard& operator=(NewArgsGuard&&) noexcept = delete;

    bool IsCompatible() const noexcept { return is_compatible; }

    ArgsView GetArgsView() const noexcept {
        neko_assert(IsCompatible());
        return new_args;
    }

private:
    bool is_compatible{false};
    BufferGuard buffer;
    std::span<ArgInfo> nonptr_arg_infos;
    ArgsView new_args;
    BufferGuard type_buffer;
};
}  // namespace neko::cpp::details
