#ifndef CLIA_LOG_SYNC_LOGGER_H_
#define CLIA_LOG_SYNC_LOGGER_H_

#include "clia/log/trait.h"

namespace clia {
    namespace log {
        class SyncLogger final : public clia::log::trait::Logger {
        public:
            SyncLogger(const Level level, std::shared_ptr<trait::Appender> appender) noexcept;
            ~SyncLogger() noexcept;
        public:
            void log(const Level level, const void *message, const std::size_t len) noexcept override;
        };
    }
}

#endif