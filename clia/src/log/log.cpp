#include "clia/log.hpp"
#include <mutex>

clia::log::LoggerManger::LoggerManger() noexcept = default;
clia::log::LoggerManger::~LoggerManger() noexcept = default;
clia::log::LoggerManger* clia::log::LoggerManger::instance() noexcept {
    static LoggerManger s_instance;
    return &s_instance;
}

std::shared_ptr<clia::log::LoggerTrait> clia::log::LoggerManger::get_logger(const std::string &name) noexcept {
    auto it = m_loggers.find(name);
    if (it != m_loggers.end()) {
        return it->second;
    }
    return m_default_logger; 
}
std::shared_ptr<clia::log::LoggerTrait> clia::log::LoggerManger::get_default() noexcept {
    return m_default_logger;
}
void clia::log::LoggerManger::set_default(std::shared_ptr<clia::log::LoggerTrait> logger) noexcept {
    std::lock_guard<std::mutex> lock(m_lck);
    if (!m_default_logger) {
        m_default_logger = std::move(logger);
    }
}

void clia::log::LoggerManger::register_logger(const std::string &name, std::shared_ptr<clia::log::LoggerTrait> logger) noexcept {
    std::lock_guard<std::mutex> lock(m_lck);
    m_loggers[name] = std::move(logger);
}