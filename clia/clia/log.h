#ifndef CLIA_LOG_H_
#define CLIA_LOG_H_

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "clia/log/trait.h"
#include "clia/log/event.h"

#define CLIA_LOG(LOGGER, LEVEL) \
    if (LOGGER && LOGGER->level() <= LEVEL) \
        clia::log::Event(LOGGER, LEVEL, __FILE__, __LINE__, __FUNCTION__).stream()

#define CLIA_LOG_LEVEL(LEVEL) CLIA_LOG(clia::log::LoggerManger::instance()->default_logger(), LEVEL)

#define CLIA_LOG_TRACE  CLIA_LOG_LEVEL(clia::log::Level::kTrace)
#define CLIA_LOG_DEBUG  CLIA_LOG_LEVEL(clia::log::Level::kDebug)
#define CLIA_LOG_INFO   CLIA_LOG_LEVEL(clia::log::Level::kInfo)
#define CLIA_LOG_WARN   CLIA_LOG_LEVEL(clia::log::Level::kWarn)
#define CLIA_LOG_ERROR  CLIA_LOG_LEVEL(clia::log::Level::kError)
#define CLIA_LOG_FATAL  CLIA_LOG_LEVEL(clia::log::Level::kFatal)

#define CLIA_LARGE_LOG(LOGGER, LEVEL) \
    if (LOGGER && LOGGER->level() <= LEVEL) \
        clia::log::LargeEvent(LOGGER, LEVEL, __FILE__, __LINE__, __FUNCTION__).stream()

#define CLIA_LARGE_LOG_LEVEL(LEVEL) CLIA_LARGE_LOG(clia::log::LoggerManger::instance()->default_logger(), LEVEL)

#define CLIA_LARGE_LOG_TRACE  CLIA_LARGE_LOG_LEVEL(clia::log::Level::kTrace)
#define CLIA_LARGE_LOG_DEBUG  CLIA_LARGE_LOG_LEVEL(clia::log::Level::kDebug)
#define CLIA_LARGE_LOG_INFO   CLIA_LARGE_LOG_LEVEL(clia::log::Level::kInfo)
#define CLIA_LARGE_LOG_WARN   CLIA_LARGE_LOG_LEVEL(clia::log::Level::kWarn)
#define CLIA_LARGE_LOG_ERROR  CLIA_LARGE_LOG_LEVEL(clia::log::Level::kError)
#define CLIA_LARGE_LOG_FATAL  CLIA_LARGE_LOG_LEVEL(clia::log::Level::kFatal)

#define CLIA_FMT_LOG(LOGGER, LEVEL, ...) do { \
    if (LOGGER && LOGGER->level() <= LEVEL) { \
        clia::log::Event(LOGGER, LEVEL, __FILE__, __LINE__, __FUNCTION__).format(__VA_ARGS__); \
    } \
} while (0);

#define CLIA_FMT_LOG_LEVEL(LEVEL, ...) CLIA_FMT_LOG(clia::log::LoggerManger::instance()->default_logger(), LEVEL, __VA_ARGS__)

#define CLIA_FMT_LOG_TRACE(...)  CLIA_FMT_LOG_LEVEL(clia::log::Level::kTrace, __VA_ARGS__)
#define CLIA_FMT_LOG_DEBUG(...)  CLIA_FMT_LOG_LEVEL(clia::log::Level::kDebug, __VA_ARGS__)
#define CLIA_FMT_LOG_INFO(...)   CLIA_FMT_LOG_LEVEL(clia::log::Level::kInfo, __VA_ARGS__)
#define CLIA_FMT_LOG_WARN(...)   CLIA_FMT_LOG_LEVEL(clia::log::Level::kWarn, __VA_ARGS__)
#define CLIA_FMT_LOG_ERROR(...)  CLIA_FMT_LOG_LEVEL(clia::log::Level::kError, __VA_ARGS__)
#define CLIA_FMT_LOG_FATAL(...)  CLIA_FMT_LOG_LEVEL(clia::log::Level::kFatal, __VA_ARGS__)

#define CLIA_LARGE_FMT_LOG(LOGGER, LEVEL, ...) do { \
    if (LOGGER && LOGGER->level() <= LEVEL) { \
        clia::log::LargeEvent(LOGGER, LEVEL, __FILE__, __LINE__, __FUNCTION__).format(__VA_ARGS__); \
    } \
} while (0);

#define CLIA_LARGE_FMT_LOG_LEVEL(LEVEL, ...) CLIA_LARGE_FMT_LOG(clia::log::LoggerManger::instance()->default_logger(), LEVEL, __VA_ARGS__)

#define CLIA_LARGE_FMT_LOG_TRACE(...)  CLIA_LARGE_FMT_LOG_LEVEL(clia::log::Level::kTrace, __VA_ARGS__)
#define CLIA_LARGE_FMT_LOG_DEBUG(...)  CLIA_LARGE_FMT_LOG_LEVEL(clia::log::Level::kDebug, __VA_ARGS__)
#define CLIA_LARGE_FMT_LOG_INFO(...)   CLIA_LARGE_FMT_LOG_LEVEL(clia::log::Level::kInfo, __VA_ARGS__)
#define CLIA_LARGE_FMT_LOG_WARN(...)   CLIA_LARGE_FMT_LOG_LEVEL(clia::log::Level::kWarn, __VA_ARGS__)
#define CLIA_LARGE_FMT_LOG_ERROR(...)  CLIA_LARGE_FMT_LOG_LEVEL(clia::log::Level::kError, __VA_ARGS__)
#define CLIA_LARGE_FMT_LOG_FATAL(...)  CLIA_LARGE_FMT_LOG_LEVEL(clia::log::Level::kFatal, __VA_ARGS__)

namespace clia {
    namespace log { 
        class LoggerManger final : Noncopyable {
        public:
            static LoggerManger* instance() noexcept;
            std::shared_ptr<trait::Logger> logger(const std::string &name) noexcept;
            std::shared_ptr<trait::Logger> default_logger() noexcept;
            void set_default(std::shared_ptr<trait::Logger> logger) noexcept;
            void register_logger(const std::string &name, std::shared_ptr<trait::Logger> logger) noexcept;
        private:
            LoggerManger() noexcept;
            ~LoggerManger() noexcept;
        private:
            std::mutex lck_;
            std::shared_ptr<trait::Logger> default_logger_;                            // 默认日志记录器
            std::unordered_map<std::string, std::shared_ptr<trait::Logger>> loggers_;  // 日志记录器映射
        };
    }
}

#endif