#ifndef CLIA_LOG_STDOUT_APPENDER_H_
#define CLIA_LOG_STDOUT_APPENDER_H_

#include "clia/log/trait.h"

namespace clia {
    namespace log {
        class StdoutAppender final : public trait::Appender {
        public:
            StdoutAppender() noexcept;;
            ~StdoutAppender() noexcept;
        public:
            virtual void append(const void *buf, const std::size_t size) noexcept override; // 添加日志行
            virtual void flush() noexcept override; // 刷新日志
        };
    }
}

#endif