#pragma once

#include "base/common/base.hpp"

namespace Neko {

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
    using BaseType = BaseT;

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

private:
    static std::unique_ptr<BaseT> instance_;
    std::thread::id main_thread_ = std::this_thread::get_id();
};

template <typename BaseT>
std::unique_ptr<BaseT> SingletonClass<BaseT>::instance_;
}  // namespace Neko

namespace Neko::modules {
template <typename T, typename... Args>
T& initialize(Args&&... args) {
    using BaseT = typename T::BaseType;
    return SingletonClass<BaseT>::template initialize<T>(std::forward<Args>(args)...);
}

template <typename... Ts>
void shutdown() noexcept {
    (..., SingletonClass<typename Ts::BaseType>::shutdown());
}

template <typename... Ts>
bool is_initialized() noexcept {
    return (... && SingletonClass<typename Ts::BaseType>::is_initialized());
}

template <typename T>
inline T& instance() {
    using BaseT = typename T::BaseType;
    return static_cast<T&>(SingletonClass<BaseT>::instance());
}

template <typename Tuple, std::size_t... Indices>
void initializeModulesHelper(Tuple&& tuple, std::index_sequence<Indices...>) {
    (initialize<std::tuple_element_t<Indices, std::decay_t<Tuple>>>(), ...);
}

}  // namespace Neko::modules

namespace Neko {
template <typename T>
inline T& the() {
    return modules::instance<T>();
}
}  // namespace Neko
