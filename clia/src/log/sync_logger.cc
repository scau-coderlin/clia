#include "clia/log/sync_logger.h"

clia::log::SyncLogger::SyncLogger(const Level level, std::shared_ptr<trait::Appender> appender) noexcept 
    : Logger(level, appender) 
{
    ;
}

clia::log::SyncLogger::~SyncLogger() noexcept = default;

void clia::log::SyncLogger::log(const Level level, const void *message, const std::size_t len) noexcept {
    if (level < level_ || nullptr == message || 0 == len) {
        return; // Ignore messages below the current log level
    }
    appender_->append(message, len);
    appender_->flush();
}