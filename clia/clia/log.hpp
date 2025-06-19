#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "clia/log/trait.hpp"
#include "clia/log/event.hpp"


namespace clia {
    namespace log { 
        class LoggerManger final : Noncopyable {
        public:
            LoggerManger() noexcept;
            ~LoggerManger() noexcept;
        public:
            static LoggerManger* instance() noexcept;
            std::shared_ptr<LoggerTrait> get_logger(const std::string &name) noexcept;
            std::shared_ptr<LoggerTrait> get_default() noexcept;
            void set_default(std::shared_ptr<LoggerTrait> logger) noexcept;
            void register_logger(const std::string &name, std::shared_ptr<LoggerTrait> logger) noexcept;
        private:
            std::mutex m_lck;
            std::shared_ptr<LoggerTrait> m_default_logger; // 默认日志记录器
            std::unordered_map<std::string, std::shared_ptr<LoggerTrait>> m_loggers; // 日志记录器映射
        };
    }
}