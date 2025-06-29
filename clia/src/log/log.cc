#include "clia/log.h"

clia::log::LoggerManger::LoggerManger() noexcept = default;
clia::log::LoggerManger::~LoggerManger() noexcept = default;

clia::log::LoggerManger* clia::log::LoggerManger::instance() noexcept {
    static LoggerManger kInstance;
    return &kInstance;
}

std::shared_ptr<clia::log::trait::Logger> clia::log::LoggerManger::logger(const std::string &name) noexcept {
    auto it = loggers_.find(name);
    if (it != loggers_.end()) {
        return it->second;
    }
    return default_logger_; 
}
std::shared_ptr<clia::log::trait::Logger> clia::log::LoggerManger::default_logger() noexcept {
    return default_logger_;
}
void clia::log::LoggerManger::set_default(std::shared_ptr<clia::log::trait::Logger> logger) noexcept {
    std::lock_guard<std::mutex> lock(lck_);
    if (!default_logger_) {
        default_logger_ = std::move(logger);
    }
}

void clia::log::LoggerManger::register_logger(const std::string &name, std::shared_ptr<clia::log::trait::Logger> logger) noexcept {
    std::lock_guard<std::mutex> lock(lck_);
    loggers_[name] = std::move(logger);
}