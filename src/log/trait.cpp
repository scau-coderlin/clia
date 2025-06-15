#include "log/trait.hpp"

const char* clia::log::level_to_string(const Level level) noexcept {
    switch (level) {
    case Level::TRACE:
        return "TRACE";
    case Level::DEBUG:
        return "DEBUG";
    case Level::INFO:
        return "INFO";
    case Level::WARN:
        return "WARN";
    case Level::ERROR:
        return "ERROR";
    case Level::FATAL:
        return "FATAL";
    default:
        return "UNKNOWN";
    }
}

clia::log::LoggerTrait::LoggerTrait(const Level level) noexcept 
    : m_level(level) 
{
    ;
}

clia::log::LoggerTrait::~LoggerTrait() noexcept {
    ;
}

void clia::log::LoggerTrait::log(const Level level, const void *message, const std::size_t len) noexcept {
    if (level >= m_level) {
        log(message, len);
    }
}