#pragma once

#include <memory>

#include "clia/log/trait.hpp"
#include "clia/container/stream.hpp"
#include "clia/base/nocopyable.hpp"

namespace clia {
    namespace log {
        class Event final : Noncopyable {
            using Stream = clia::container::Stream<1024>;
        public:
            Event(std::shared_ptr<LoggerTrait> logger, const Level level, const char *file, const int line, const char *func) noexcept;
            ~Event() noexcept;
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