#pragma once

#include <memory>

#include "log/trait.hpp"
#include "log/stream.hpp"

namespace clia {
    namespace log {
        class Event final {
        public:
            Event(std::shared_ptr<LoggerTrait> logger, const Level level, const char *file, const int line, const char *func) noexcept;
            ~Event() noexcept;
            Event(const Event&) = delete;
            Event& operator=(const Event&) = delete;
            Event(Event&&) noexcept = delete;
            Event& operator=(Event&&) noexcept = delete;
        public:
            Stream& stream() noexcept;
            void format(const char *fmt, ...) noexcept;
        private:
            std::shared_ptr<LoggerTrait> m_logger;
            Level m_level = Level::ERROR;
            const char *m_file = nullptr;
            const int m_line = -1;
            const char *m_func = nullptr;
            Stream m_stream;
        };
    }
}