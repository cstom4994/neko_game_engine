#pragma once

#include "engine/base/base.hpp"

#define DeferLoop(start, end) for (int _i_ = ((start), 0); _i_ == 0; _i_ += 1, (end))

template <typename... Args>
auto println(const std::string &fmt, Args &&...args) {
    std::string str = std::vformat(fmt, std::make_format_args(args...));
    std::cout << str << std::endl;
}

namespace neko {

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
constexpr auto BitSet(const char *str) {
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
constexpr Scope<T> create_scope(Args &&...args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T>
using Ref = std::shared_ptr<T>;
template <typename T, typename... Args>
constexpr Ref<T> create_ref(Args &&...args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

}  // namespace util

}  // namespace neko

#define neko_defer(code) auto JOIN_2(_defer_, __COUNTER__) = neko::util::defer_func([&]() { code; })