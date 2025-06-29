#ifndef CLIA_LOG_FILE_APPENDER_H_
#define CLIA_LOG_FILE_APPENDER_H_

#include <cstddef>
#include <cstdio>
#include <ctime>
#include <memory>

#include "clia/log/trait.h"

namespace clia {
    namespace log {
        class FileAppender : public trait::Appender {
        public:
            FileAppender(
                const char *path, 
                const char *logname, 
                const std::size_t roll_size_byte = 10 * 1024 * 1024, // 默认1MB
                const int roll_period_days = 1,
                const int retain_period_day = 6 * 30, // 保留周期6个月
                const int check_every = 1024, // 每1024次检查一次是否需要滚动
                const bool thread_safe = true
            );
            ~FileAppender();
        public:
            void append(const void *buf, const std::size_t size) noexcept override;
            void flush() noexcept override;
        private:
            class Impl;
            std::unique_ptr<Impl> impl_;
        };
    }
}

#endif