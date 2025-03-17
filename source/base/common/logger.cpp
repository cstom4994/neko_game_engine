#include "logger.hpp"

namespace Neko {

Logger *Logger::getInstance() {
    if (!instance) {
        throw std::runtime_error("Logger not initialized");
    }
    return instance.get();
}

void Logger::init() {
    std::call_once(init_flag, [&]() {
        instance = std::make_unique<Logger>();

        instance->loggerThread = std::jthread([instance = instance.get()](std::stop_token st) { instance->processLogs(st); });

        instance->log(std::source_location::current(), Level::INFO, "Logger initialized");
    });
}

void Logger::shutdown() { instance.reset(); }

Logger::Logger() : running(true) {}

Logger::~Logger() {
    try {
        running = false;
        if (loggerThread.joinable()) {
            loggerThread.request_stop();
            loggerThread.join();
        }
    } catch (...) {
    }
}

void Logger::processLogs(std::stop_token st) {
    constexpr size_t BATCH_SIZE = 16 * 1024;
    std::vector<LogMessage> batchBuffer;
    batchBuffer.reserve(BATCH_SIZE);

    std::vector<char> consoleBuffer;
    consoleBuffer.reserve(1024 * 1024);

    auto processMessageBatch = [this](std::vector<LogMessage> &batchBuffer, std::vector<char> &consoleBuffer) {
        for (const auto &msg : batchBuffer) {
            formatLogMessage(msg, consoleBuffer);
            std::back_inserter(consoleBuffer)++ = '\n';
        }

        if (!consoleBuffer.empty()) {
            std::cout.write(consoleBuffer.data(), consoleBuffer.size());
            std::cout.flush();

            if (console_printf) {
                console_printf(std::string(consoleBuffer.data(), consoleBuffer.size()));
            }
        }
    };

    while (!st.stop_requested()) {
        bool messagesProcessed = false;
        LogMessage msg;

        while (batchBuffer.size() < BATCH_SIZE && buffer.pop(msg)) {
            batchBuffer.push_back(std::move(msg));
            messagesProcessed = true;
        }

        if (!batchBuffer.empty()) {
            processMessageBatch(batchBuffer, consoleBuffer);

            batchBuffer.clear();
            consoleBuffer.clear();
        }
    }

    // 处理剩余消息
    LogMessage msg;
    while (buffer.pop(msg)) {
        batchBuffer.push_back(std::move(msg));
        if (batchBuffer.size() >= 4096) {
            processMessageBatch(batchBuffer, consoleBuffer);
            batchBuffer.clear();
            consoleBuffer.clear();
        }
    }

    if (!batchBuffer.empty()) {
        processMessageBatch(batchBuffer, consoleBuffer);
    }
}

void Logger::formatLogMessage(const LogMessage &msg, std::vector<char> &buffer) noexcept {

    bool showTimestamp{false};
    bool showThreadId{false};
    bool showSourceLocation{true};
    bool showModuleName{false};

    size_t required_size = 256 + msg.message.size();

    if (showTimestamp) required_size += 32;
    if (showThreadId) required_size += 32;
    if (showModuleName && !msg.context.moduleName.empty()) required_size += msg.context.moduleName.size() + 3;
    if (showSourceLocation) required_size += msg.context.fileName.size() + 10;

    size_t original_size = buffer.size();
    buffer.reserve(original_size + required_size);

    auto inserter = std::back_inserter(buffer);

    if (showTimestamp) [[likely]] {
        auto time = std::chrono::system_clock::to_time_t(msg.timestamp);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(msg.timestamp.time_since_epoch()) % 1000;

        char time_buffer[64];
        size_t time_len = std::strftime(time_buffer, sizeof(time_buffer), "[%Y-%m-%d %H:%M:%S.", std::localtime(&time));

        int ms_len = std::snprintf(time_buffer + time_len, sizeof(time_buffer) - time_len, "%03d] ", static_cast<int>(ms.count()));

        std::copy_n(time_buffer, time_len + ms_len, inserter);
    }

    const auto &level_str = LEVEL_STRINGS[static_cast<size_t>(msg.level)];
    *inserter++ = '[';
    std::copy(level_str.begin(), level_str.end(), inserter);
    *inserter++ = ']';
    *inserter++ = ' ';

    if (showThreadId) [[likely]] {
        char thread_buffer[32];
        int thread_len = std::snprintf(thread_buffer, sizeof(thread_buffer), "[T-%zu] ", std::hash<std::thread::id>{}(msg.context.threadId));
        std::copy_n(thread_buffer, thread_len, inserter);
    }

    if (showModuleName && !msg.context.moduleName.empty()) [[likely]] {
        *inserter++ = '[';
        std::copy(msg.context.moduleName.begin(), msg.context.moduleName.end(), inserter);
        *inserter++ = ']';
        *inserter++ = ' ';
    }

    if (showSourceLocation) [[likely]] {
        std::string_view file(msg.context.fileName);
        if (!file.ends_with(".lua\"]")) [[likely]] {
            if (auto pos = file.find_last_of("/\\"); pos != std::string_view::npos) file = file.substr(pos + 1);
        }

        *inserter++ = '[';
        std::copy(file.begin(), file.end(), inserter);

        char line_buffer[16];
        int line_len = std::snprintf(line_buffer, sizeof(line_buffer), ":%d] ", msg.context.line);
        std::copy_n(line_buffer, line_len, inserter);
    }

    std::copy(msg.message.begin(), msg.message.end(), inserter);
}

}  // namespace Neko