
#ifndef NEKO_LOGGER_HPP
#define NEKO_LOGGER_HPP

#include <chrono>
#include <ctime>
#include <iostream>
#include <vector>

#include "engine/common/neko_types.h"
#include "engine/common/neko_util.h"
#include "engine/utility/enum.hpp"

namespace neko {

ENUM_HPP_CLASS_DECL(log_type, u8, (trace)(warning)(error)(info)(msg)(debug));

struct log_msg {
    std::string msg;
    log_type type;
    s64 time;
};

#define neko_define_logger_impl() \
    std::vector<::neko::log_msg> neko::logger::m_message_log {}

class logger {
private:
    friend class console;

    static void logger::writeline(const std::string &msg) {
        // OutputDebugStringA(msg.c_str());
        std::cout << msg;
    }

    template <typename... args>
    static void log_impl(const char *message, log_type type, args &&...argv) noexcept {

        constexpr uint8_t logTypeOffset = 6;

        std::string output = std::format("[{0}] {1}", log_type_traits::to_string(type).value(), message);

        std::stringstream ss;
        (ss << ... << argv);
        output += ss.str() + '\n';

        writeline(output);

        m_message_log.emplace_back(log_msg{output, type, neko_get_time()});
    }

    static std::vector<log_msg> m_message_log;

    static std::string logger::get_current_time() noexcept {
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::string realTime = std::ctime(&now);
        realTime.erase(24);
        return realTime;
    }

public:
    static const std::vector<log_msg> &message_log() noexcept { return m_message_log; }

    template <typename... args>
    static void log(log_type type, const char *message, args &&...argv) noexcept {
        log_impl(message, type, argv...);
    }

    template <typename... args>
    static void log(log_type type, const std::string& message, args &&...argv) noexcept {
        log_impl(message.c_str(), type, argv...);
    }
};

}  // namespace neko

#if defined(neko_debug_build)
#define neko_debug(...) ::neko::logger::log(::neko::log_type::debug, __VA_ARGS__, std::format(" (at {0}:{1})", __func__, __LINE__).c_str())
#else
#define neko_debug(...)
#endif

#define neko_trace(...) ::neko::logger::log(::neko::log_type::trace, __VA_ARGS__)
#define neko_info(...) ::neko::logger::log(::neko::log_type::info, __VA_ARGS__)
#define neko_warn(...) ::neko::logger::log(::neko::log_type::warning, __VA_ARGS__)
#define neko_error(...) ::neko::logger::log(::neko::log_type::error, __VA_ARGS__)

#endif