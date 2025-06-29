#ifndef CLIA_LOG_STDOUT_LOGGER_H_
#define CLIA_LOG_STDOUT_LOGGER_H_

#include <cstdio>

#include "clia/log/trait.h"

namespace clia {
    namespace log {
        class StdoutLogger final : public trait::Logger {
        public:
            using trait::Logger::Logger;
            ~StdoutLogger() = default;
        public:
            void log(Level level, const void *message, const std::size_t len) noexcept override {
                if (level >= level_) {
                    std::fwrite(message, 1, len, stdout);
                }
            }
        };
    }
}

#endif