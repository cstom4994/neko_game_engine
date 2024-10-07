#pragma once

#include "engine/base/base.hpp"

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

#define neko_defer(code) auto JOIN_2(_defer_, __COUNTER__) = defer_func([&]() { code; })

#define DeferLoop(start, end) for (int _i_ = ((start), 0); _i_ == 0; _i_ += 1, (end))

template <typename... Args>
auto println(const std::string &fmt, Args &&...args) {
    std::string str = std::vformat(fmt, std::make_format_args(args...));
    std::cout << str << std::endl;
}