#pragma once

#include "base/common/base.hpp"

#include <chrono>

#define DeferLoop(start, end) for (int _i_ = ((start), 0); _i_ == 0; _i_ += 1, (end))

#define ENABLE_IF(x) typename std::enable_if<(x), bool>::type

template <typename... Args>
auto println(const std::string& fmt, Args&&... args) {
    std::string str = std::vformat(fmt, std::make_format_args(args...));
    std::cout << str << std::endl;
}

namespace Neko {

namespace util {

template <std::size_t N>
constexpr auto BitSet(const char (&str)[N]) {
    std::bitset<N - 1> bits;  // N - 1 是因为包含 null 终止符
    for (std::size_t i = 0; i < N - 1; ++i) {
        bits.set(i, (str[i] == '1'));
    }
    return bits;
}

template <std::size_t N>
constexpr auto BitSet(const char* str) {
    std::bitset<N> bits;
    std::size_t len = std::strlen(str);  // 计算字符串长度
    for (std::size_t i = 0; i < len && i < bits.size(); ++i) {
        bits.set(i, (str[i] == '1'));
    }
    return bits;
}

template <typename F>
struct Defer {
    F f;
    Defer(F f) : f(f) {}
    ~Defer() { f(); }
};

template <typename F>
Defer<F> defer_func(F f) {
    return Defer<F>(f);
}

template <typename T>
using Scope = std::unique_ptr<T>;
template <typename T, typename... Args>
constexpr Scope<T> create_scope(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T>
using Ref = std::shared_ptr<T>;
template <typename T, typename... Args>
constexpr Ref<T> create_ref(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

struct FormatStr {
    constexpr FormatStr(const char* str) noexcept : str(str) {}
    const char* str;
};

template <FormatStr F>
constexpr auto operator""_f() {
    return [=]<typename... T>(T... args) { return std::format(F.str, args...); };
}

namespace tuple {

template <int N>
class Int_ {};

namespace detail {

// 初始
template <std::size_t N, typename... Args, typename Iterator>
ENABLE_IF(N == 0)
iterate(std::tuple<Args...>&, Iterator&&) {
    return true;
}

// 递归
template <std::size_t N, typename... Args, typename Iterator>
ENABLE_IF(N >= 1)
iterate(std::tuple<Args...>& tup, Iterator&& it) {
    if (iterate<N - 1>(tup, it)) {
        auto&& val = std::get<N - 1>(tup);
        return it(Int_<N - 1>(), val);
    } else {
        return false;
    }
}
}  // namespace detail

// Func 要求
// template<int N, class T>
// bool operator()(Int_<N>, T)
template <typename Func, typename... Args>
bool for_each_tuple(std::tuple<Args...>& tup, Func&& func) {
    return detail::iterate<sizeof...(Args)>(tup, func);
}

template <std::size_t N>
struct TupleArrayRef {
    template <class Tuple>
    static auto ref_to_members(Tuple&& tup, int ix) {
        return std::tuple_cat(TupleArrayRef<N - 1>::ref_to_members(tup, ix), std::tie(std::get<N - 1>(tup)[ix]));
    }
};

template <>
struct TupleArrayRef<0> {
    template <class Tuple>
    static std::tuple<> ref_to_members(Tuple&&, int) {
        return std::tuple<>();
    }
};

}  // namespace tuple

template <int F, int L>
struct static_for_t {
    template <typename Functor>
    static inline constexpr void apply(const Functor& f) {
        if (F < L) {
            f(std::integral_constant<int, F>{});
            static_for_t<F + 1, L>::apply(f);
        }
    }

    template <typename Functor>
    inline constexpr void operator()(const Functor& f) const {
        apply(f);
    }
};

template <int N>
struct static_for_t<N, N> {
    template <typename Functor>
    static inline constexpr void apply([[maybe_unused]] const Functor& f) {}
};

template <int F, int L>
inline constexpr static_for_t<F, L> static_for = {};

template <typename T, typename... S>
constexpr bool static_find() {
    bool b = false;
    static_for<0, sizeof...(S)>([&b]([[maybe_unused]] auto i) constexpr {
        if constexpr (std::is_same<std::decay_t<decltype(std::get<i.value>(std::declval<std::tuple<S...>>()))>, T>::value) {
            b = true;
        }
    });
    return b;
}

#define neko_time_count(x) std::chrono::time_point_cast<std::chrono::microseconds>(x).time_since_epoch().count()

class timer {
public:
    inline void start() noexcept { startPos = std::chrono::high_resolution_clock::now(); }
    inline void stop() noexcept {
        auto endTime = std::chrono::high_resolution_clock::now();
        duration = static_cast<f64>(neko_time_count(endTime) - neko_time_count(startPos)) * 0.001;
    }
    [[nodiscard]] inline f64 get() const noexcept { return duration; }
    ~timer() noexcept { stop(); }

private:
    f64 duration = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> startPos;
};

template <typename T>
using initializer_list = std::initializer_list<T>;

template <typename T>
using function = std::function<T>;

template <typename T>
concept concept_is_pair = requires(T t) {
    t.first;
    t.second;
};

template <class T>
struct is_pair : public std::false_type {};

template <class T1, class T2>
struct is_pair<std::pair<T1, T2>> : public std::true_type {};

template <class>
inline constexpr bool always_false = false;

// std::function 合并方法

template <typename, typename...>
struct lastFnType;

template <typename F0, typename F1, typename... Fn>
struct lastFnType<F0, F1, Fn...> {
    using type = typename lastFnType<F1, Fn...>::type;
};

template <typename T1, typename T2>
struct lastFnType<function<T2(T1)>> {
    using type = T1;
};

template <typename T1, typename T2>
function<T1(T2)> func_combine(function<T1(T2)> conv) {
    return conv;
}

template <typename T1, typename T2, typename T3, typename... Fn>
auto func_combine(function<T1(T2)> conv1, function<T2(T3)> conv2, Fn... fn) -> function<T1(typename lastFnType<function<T2(T3)>, Fn...>::type)> {
    using In = typename lastFnType<function<T2(T3)>, Fn...>::type;

    return [=](In const& in) { return conv1(func_combine(conv2, fn...)(in)); };
}

template <typename T>
struct tuple_size;

template <typename... Args>
struct tuple_size<std::tuple<Args...>> {
    static constexpr std::size_t value = sizeof...(Args);
};

template <typename T>
constexpr std::size_t tuple_size_v = tuple_size<T>::value;

template <typename T, std::size_t N>
constexpr bool is_pointer_to_const_char(T (&)[N]) {
    return std::is_same_v<const char, T>;
}

template <typename T>
constexpr bool is_pointer_to_const_char(T&&) {
    return std::is_same_v<const char*, T>;
}

template <typename T>
struct is_vector : std::false_type {};

template <typename T, typename Alloc>
struct is_vector<std::vector<T, Alloc>> : std::true_type {};

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
struct is_mappish_impl<T, void_t<typename T::key_type, typename T::mapped_type, decltype(std::declval<T&>()[std::declval<const typename T::key_type&>()])>> : std::true_type {};
}  // namespace detail

template <typename T>
struct neko_is_mappish : detail::is_mappish_impl<T>::type {};

template <class... Ts>
struct neko_overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
neko_overloaded(Ts...) -> neko_overloaded<Ts...>;

}  // namespace util

}  // namespace Neko

#define neko_defer(code) auto JOIN_2(_defer_, __COUNTER__) = Neko::util::defer_func([&]() { code; })