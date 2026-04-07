#ifndef RDP_EXT_LOGGER_H
#define RDP_EXT_LOGGER_H

#include <memory>
#include <string>

namespace plog {
    template<int instanceId>
    class Logger;
    class IAppender;
}

namespace rdp_ext::logging {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
};

enum class LogOutput {
    Console,
    WindowsDebug,
};

constexpr auto LOG_INSTANCE_ID = 0;

class LogSink {
public:
    explicit LogSink(LogLevel max_level, LogOutput output);
    ~LogSink();

    LogSink(const LogSink&) = delete;
    LogSink& operator=(const LogSink&) = delete;
    LogSink(LogSink&&) noexcept = default;
    LogSink& operator=(LogSink&&) noexcept = default;

    void log(LogLevel level, std::string_view prefix, std::string_view message);

private:

    std::unique_ptr<plog::IAppender> appender_;
    std::unique_ptr<plog::Logger<LOG_INSTANCE_ID>> logger_;
};

class Logger {
public:
    explicit Logger(LogSink& sink, std::string_view prefix);

    void debug(std::string_view message) const;
    void info(std::string_view message) const;
    void warning(std::string_view message) const;
    void error(std::string_view message) const;

private:
    LogSink& sink_;
    std::string prefix_;
};

} // namespace rdp_ext::logging

#endif //RDP_EXT_LOGGER_H