#include "clia/log/trait.hpp"

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

clia::log::AppenderTrait::AppenderTrait() noexcept {
    ;
}

clia::log::AppenderTrait::~AppenderTrait() noexcept {
    ;
}

void clia::log::AppenderTrait::flush() noexcept {
    ;
}

clia::log::LoggerTrait::LoggerTrait(const Level level) noexcept 
    : m_level(level) 
{
    ;
}

clia::log::LoggerTrait::~LoggerTrait() noexcept {
    ;
}

void clia::log::LoggerTrait::set_level(const Level level) noexcept {
    m_level = level;
}

void clia::log::LoggerTrait::set_appender(std::shared_ptr<AppenderTrait> appender) noexcept {
    m_appender = std::move(appender);
}

void clia::log::LoggerTrait::log(const Level level, const void *message, const std::size_t len) noexcept {
    if (level >= m_level) {
        log(message, len);
    }
}