#pragma once

#include <variant>

#include "base/common/array.hpp"
#include "base/common/base.hpp"
#include "base/common/hashmap.hpp"
#include "base/common/string.hpp"

#include <source_location>
#include <array>
#include <format>

namespace Neko {
class Logger {
public:
    enum class Level { TRACE, INFO, WARNING, ERR, FATAL };

    static constexpr std::array<std::string_view, 5> LEVEL_STRINGS = {"TRACE", "INFO", "WARN", "ERROR", "FATAL"};

    Logger();
    ~Logger();

private:
    struct Context {
        std::string moduleName;
        std::string functionName;
        std::string fileName;
        int line;
        std::thread::id threadId;

        Context(const std::source_location &loc = std::source_location::current())
            : moduleName(getThreadLocalModuleName()), functionName(loc.function_name()), fileName(loc.file_name()), line(loc.line()), threadId(std::this_thread::get_id()) {}

    private:
        static std::string &getThreadLocalModuleName() {
            static thread_local std::string currentModule = "Neko Default Module";
            return currentModule;
        }

        friend class Logger;
    };

    struct alignas(64) LogMessage {
        std::string message;
        Level level;
        Context context;
        std::chrono::system_clock::time_point timestamp;

        LogMessage() = default;

        LogMessage(const LogMessage &) = delete;
        LogMessage &operator=(const LogMessage &) = delete;

        LogMessage(LogMessage &&) noexcept = default;
        LogMessage &operator=(LogMessage &&) noexcept = default;

        LogMessage(std::string msg, Level lvl, Context ctx) : message(std::move(msg)), level(lvl), context(std::move(ctx)), timestamp(std::chrono::system_clock::now()) {}

        ~LogMessage() = default;
    };

    static constexpr size_t FULL_THRESHOLD = 0.75f;

    struct alignas(64) RingBuffer {
        static constexpr size_t SHARD_SIZE = 1 << 18;  // 256kb
        alignas(64) std::array<LogMessage, SHARD_SIZE> messages;
        alignas(64) std::atomic<size_t> head{0};
        alignas(64) std::atomic<size_t> tail{0};

        bool push(LogMessage &&msg) noexcept {
            auto current_tail = tail.load(std::memory_order_relaxed);
            auto next_tail = (current_tail + 1) & (SHARD_SIZE - 1);

            if (next_tail == head.load(std::memory_order_relaxed) && next_tail == head.load(std::memory_order_acquire)) {
                return false;
            }

            new (&messages[current_tail]) LogMessage(std::move(msg));
            tail.store(next_tail, std::memory_order_release);
            return true;
        }

        bool pop(LogMessage &msg) noexcept {
            auto current_head = head.load(std::memory_order_relaxed);

            if (current_head == tail.load(std::memory_order_relaxed) && current_head == tail.load(std::memory_order_acquire)) {
                return false;
            }

            msg = std::move(messages[current_head]);
            head.store((current_head + 1) & (SHARD_SIZE - 1), std::memory_order_release);
            return true;
        }

        bool isFull() const noexcept {
            auto current_tail = tail.load(std::memory_order_relaxed);
            auto current_head = head.load(std::memory_order_relaxed);

            size_t used;
            if (current_tail >= current_head) {
                used = current_tail - current_head;
            } else {
                used = SHARD_SIZE - (current_head - current_tail);
            }

            return (static_cast<float>(used) / SHARD_SIZE) >= FULL_THRESHOLD;
        }
    };

private:
    static inline std::unique_ptr<Logger> instance;
    static inline std::once_flag init_flag;

    RingBuffer buffer;

    std::jthread loggerThread;

    std::atomic<bool> running{true};

public:
    static Logger *getInstance();
    static void init();
    static void shutdown();

private:
    void processLogs(std::stop_token st);
    void formatLogMessage(const LogMessage &msg, std::vector<char> &buffer) noexcept;

public:
    template <typename... Args>
    void log(const std::source_location &loc, Level level, std::format_string<Args...> fmt, Args &&...args) {
        try {
            LogMessage msg{std::format(fmt, std::forward<Args>(args)...), level, Context(loc)};
            bool messageLogged = false;
            while (!messageLogged) {
                if (buffer.push(std::move(msg))) {
                    messageLogged = true;
                    break;
                }
                if (!messageLogged) {
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                }
            }
        } catch (const std::exception &e) {
            std::cerr << "Logging error: " << e.what() << std::endl;
        }
    }

    template <typename... Args>
    void info(const std::source_location &loc, std::format_string<Args...> fmt, Args &&...args) {
        log(loc, Level::INFO, fmt, std::forward<Args>(args)...);
    }
};

#define LOG_INFO(...) ::Neko::Logger::getInstance()->info(std::source_location::current(), __VA_ARGS__)

}  // namespace Neko