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
