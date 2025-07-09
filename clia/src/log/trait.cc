#include <memory>

#include "clia/log/trait.h"

const char* clia::log::level_to_string(const Level level) noexcept {
    switch (level) {
    case Level::kTrace:
        return "TRACE";
    case Level::kDebug:
        return "DEBUG";
    case Level::kInfo:
        return "INFO";
    case Level::kWarn:
        return "WARN";
    case Level::kError:
        return "ERROR";
    case Level::kFatal:
        return "FATAL";
    default:
        return "UNKNOWN";
    }
}

clia::log::trait::Logger::Logger(const Level level, std::shared_ptr<Appender> appender) noexcept 
    : level_(level), appender_(std::move(appender))
{
    ;
}

clia::log::Level clia::log::trait::Logger::level() const noexcept {
    return level_;
}