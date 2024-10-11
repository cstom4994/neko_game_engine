#pragma once

#include "engine/base/base.hpp"

namespace neko {

class NonCopyable {
public:
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;

protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
};

template <typename BaseT>
class SingletonClass : private NonCopyable {
public:
    using base_type = BaseT;

public:
    SingletonClass() noexcept = default;
    virtual ~SingletonClass() noexcept = default;

    const std::thread::id& main_thread() const noexcept { return main_thread_; }

    bool is_in_main_thread() const noexcept { return std::this_thread::get_id() == main_thread_; }

public:
    template <typename T, typename... Args>
    static T& initialize(Args&&... args) {
        if (is_initialized()) {
            throw std::exception{"module_already_initialized"};
        }
        instance_ = std::make_unique<T>(std::forward<Args>(args)...);
        return static_cast<T&>(*instance_);
    }

    static void shutdown() noexcept { instance_.reset(); }

    static bool is_initialized() noexcept { return !!instance_; }

    static BaseT& instance() {
        if (!is_initialized()) {
            throw std::exception{"module_not_initialized"};
        }
        return *instance_;
    }

    virtual void init() {};
    virtual void fini() {};
    virtual void update() {};

private:
    static std::unique_ptr<BaseT> instance_;
    std::thread::id main_thread_ = std::this_thread::get_id();
};

template <typename BaseT>
std::unique_ptr<BaseT> SingletonClass<BaseT>::instance_;
}  // namespace neko

namespace neko::modules {
template <typename T, typename... Args>
T& initialize(Args&&... args) {
    using BaseT = typename T::base_type;
    return SingletonClass<BaseT>::template initialize<T>(std::forward<Args>(args)...);
}

template <typename... Ts>
void shutdown() noexcept {
    (..., SingletonClass<typename Ts::base_type>::shutdown());
}

template <typename... Ts>
bool is_initialized() noexcept {
    return (... && SingletonClass<typename Ts::base_type>::is_initialized());
}

template <typename T>
T& instance() {
    using BaseT = typename T::base_type;
    return static_cast<T&>(SingletonClass<BaseT>::instance());
}

}  // namespace neko::modules

namespace neko {
template <typename T>
T& the() {
    return modules::instance<T>();
}
}  // namespace neko
