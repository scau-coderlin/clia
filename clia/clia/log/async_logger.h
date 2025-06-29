#ifndef CLIA_LOG_ASYNC_LOGGER_H_
#define CLIA_LOG_ASYNC_LOGGER_H_

#include <cstddef>
#include <memory>

#include "clia/log/trait.h"

namespace clia {
    namespace log {
        class AsyncLogger final : public trait::Logger {
        public:
            explicit AsyncLogger(const Level level, std::shared_ptr<trait::Appender> appender, const int flush_interval_sec = 3) noexcept;
            ~AsyncLogger() noexcept;
        public:
            void log(const Level level, const void *message, const std::size_t len) noexcept override; // 异步日志记录方法
        private:
            class Impl;
            std::unique_ptr<Impl> impl_;
        };
        // 其他成员函数和数据成员可以根据需要添加
    } // namespace log
} // namespace clia

#endif