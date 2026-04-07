#include "logger.h"

#include <format>
#include <plog/Log.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#ifdef _WIN32
#include <plog/Appenders/DebugOutputAppender.h>
#endif

namespace rdp_ext::logging {

namespace {

plog::Severity fromLogLevel(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:
            return plog::Severity::debug;
        case LogLevel::Info:
            return plog::Severity::info;
        case LogLevel::Warning:
            return plog::Severity::warning;
        case LogLevel::Error:
            return plog::Severity::error;
        default:
            return plog::Severity::none;
    }
}

std::unique_ptr<plog::IAppender> makeAppender(LogOutput output) {
    switch (output) {
        case LogOutput::Console:
            return std::make_unique<plog::ColorConsoleAppender<plog::TxtFormatter>>(plog::streamStdOut);
        case LogOutput::WindowsDebug:
#ifdef _WIN32
            return std::make_unique<plog::DebugOutputAppender<plog::TxtFormatter>>();
#else
            throw std::logic_error("Windows debug logging is not available");
#endif
        default:
            throw std::logic_error("Unknown log output type");
    }
}

} // namespace

LogSink::LogSink(LogLevel max_level, LogOutput output)
    : appender_{makeAppender(output)}
    , logger_{std::make_unique<plog::Logger<LOG_INSTANCE_ID>>(fromLogLevel(max_level))}
{
    logger_->addAppender(appender_.get());
}

LogSink::~LogSink() = default;

void LogSink::log(LogLevel level, std::string_view prefix, std::string_view message) {
    *logger_ += plog::Record(fromLogLevel(level), PLOG_GET_FUNC(), __LINE__, PLOG_GET_FILE(), PLOG_GET_THIS(), LOG_INSTANCE_ID).ref() << prefix << message;
}

Logger::Logger(LogSink& sink, std::string_view prefix)
    : sink_(sink), prefix_(std::format("[{}] ", prefix)) {}

void Logger::debug(std::string_view message) const {
    sink_.log(LogLevel::Debug, prefix_, message);
}

void Logger::info(std::string_view message) const {
    sink_.log(LogLevel::Info, prefix_, message);
}

void Logger::warning(std::string_view message) const {
    sink_.log(LogLevel::Warning, prefix_, message);
}

void Logger::error(std::string_view message) const {
    sink_.log(LogLevel::Error, prefix_, message);
}

} // namespace rdp_ext::logging
