#pragma once

#include <cstddef>

namespace clia {
    namespace log {
        enum class Level {
            TRACE = 0,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL,
        };
        extern const char* level_to_string(const Level level) noexcept;
        class LoggerTrait {
        public:
            LoggerTrait(const Level level) noexcept;
            virtual ~LoggerTrait() noexcept;
            void log(const Level level, const void *message, const std::size_t len) noexcept;
        protected:
            virtual void log(const void *message, const std::size_t len) noexcept = 0;
        private:
            Level m_level = Level::ERROR;
        };
    }
}