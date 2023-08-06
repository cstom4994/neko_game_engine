
#ifndef NEKO_DEFER_HPP
#define NEKO_DEFER_HPP

#include <exception>
#include <tuple>
#include <type_traits>
#include <utility>

namespace neko {
namespace cpp::defer {
template <typename F, typename... Args>
class defer_impl {
public:
    defer_impl() = delete;
    defer_impl(defer_impl&&) = delete;
    defer_impl(const defer_impl&) = delete;
    defer_impl& operator=(defer_impl&&) = delete;
    defer_impl& operator=(const defer_impl&) = delete;

    template <typename UF>
    explicit defer_impl(UF&& f, std::tuple<Args...>&& args) : f_(std::forward<UF>(f)), args_(std::move(args)) {}

    virtual ~defer_impl() noexcept {
        if (!dismissed_) {
            std::apply(std::move(f_), std::move(args_));
        }
    }

    void dismiss() noexcept { dismissed_ = true; }

private:
    F f_;
    std::tuple<Args...> args_;
    bool dismissed_{};
};

template <typename F, typename... Args>
class error_defer_impl final : public defer_impl<F, Args...> {
public:
    error_defer_impl() = delete;
    error_defer_impl(error_defer_impl&&) = delete;
    error_defer_impl(const error_defer_impl&) = delete;
    error_defer_impl& operator=(error_defer_impl&&) = delete;
    error_defer_impl& operator=(const error_defer_impl&) = delete;

    template <typename UF>
    explicit error_defer_impl(UF&& f, std::tuple<Args...>&& args) : defer_impl<F, Args...>(std::forward<UF>(f), std::move(args)), exceptions_(std::uncaught_exceptions()) {}

    ~error_defer_impl() noexcept final {
        if (exceptions_ == std::uncaught_exceptions()) {
            this->dismiss();
        }
    }

private:
    int exceptions_{};
};

template <typename F, typename... Args>
class return_defer_impl final : public defer_impl<F, Args...> {
public:
    return_defer_impl() = delete;
    return_defer_impl(return_defer_impl&&) = delete;
    return_defer_impl(const return_defer_impl&) = delete;
    return_defer_impl& operator=(return_defer_impl&&) = delete;
    return_defer_impl& operator=(const return_defer_impl&) = delete;

    template <typename UF>
    explicit return_defer_impl(UF&& f, std::tuple<Args...>&& args) : defer_impl<F, Args...>(std::forward<UF>(f), std::move(args)), exceptions_(std::uncaught_exceptions()) {}

    ~return_defer_impl() noexcept final {
        if (exceptions_ != std::uncaught_exceptions()) {
            this->dismiss();
        }
    }

private:
    int exceptions_{};
};
}  // namespace cpp::defer

template <typename F, typename... Args>
auto make_defer(F&& f, Args&&... args) {
    using defer_t = cpp::defer::defer_impl<std::decay_t<F>, std::decay_t<Args>...>;
    return defer_t(std::forward<F>(f), std::make_tuple(std::forward<Args>(args)...));
}

template <typename F, typename... Args>
auto make_error_defer(F&& f, Args&&... args) {
    using defer_t = cpp::defer::error_defer_impl<std::decay_t<F>, std::decay_t<Args>...>;
    return defer_t(std::forward<F>(f), std::make_tuple(std::forward<Args>(args)...));
}

template <typename F, typename... Args>
auto make_return_defer(F&& f, Args&&... args) {
    using defer_t = cpp::defer::return_defer_impl<std::decay_t<F>, std::decay_t<Args>...>;
    return defer_t(std::forward<F>(f), std::make_tuple(std::forward<Args>(args)...));
}
}  // namespace neko

#define DEFER_HPP_IMPL_PP_CAT(x, y) DEFER_HPP_IMPL_PP_CAT_I(x, y)
#define DEFER_HPP_IMPL_PP_CAT_I(x, y) x##y

#define neko_defer(...) auto DEFER_HPP_IMPL_PP_CAT(generated_defer_, __COUNTER__) = ::neko::make_defer(__VA_ARGS__)
#define neko_defer_error(...) auto DEFER_HPP_IMPL_PP_CAT(generated_error_defer_, __COUNTER__) = ::neko::make_error_defer(__VA_ARGS__)
#define neko_defer_return(...) auto DEFER_HPP_IMPL_PP_CAT(generated_return_defer_, __COUNTER__) = ::neko::make_return_defer(__VA_ARGS__)

#endif