#pragma once

#include "base/common/base.hpp"

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

}  // namespace util

}  // namespace Neko

#define neko_defer(code) auto JOIN_2(_defer_, __COUNTER__) = Neko::util::defer_func([&]() { code; })