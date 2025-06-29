#ifndef CLIA_LOG_EVENT_H_
#define CLIA_LOG_EVENT_H_

#include <memory>
#include <sstream>

#include "clia/log/trait.h"
#include "clia/container/fixed_ostream.h"
#include "clia/base/noncopyable.h"

namespace clia {
    namespace log {
        class Event final : Noncopyable {
            using Stream = clia::container::FixedOStream<1024>;
        public:
            Event(std::shared_ptr<trait::Logger> logger, const Level level, const char *file, const int line, const char *func) noexcept;
            ~Event() noexcept;
        public:
            Stream& stream() noexcept;
            void format(const char *fmt, ...) noexcept;
        private:
            std::shared_ptr<trait::Logger> logger_;
            Level level_ = Level::kError;
            Stream stream_;
        };

        class LargeEvent final : Noncopyable {
            using Stream = std::stringstream;
        public:
            LargeEvent(std::shared_ptr<trait::Logger> logger, const Level level, const char *file, const int line, const char *func) noexcept;
            ~LargeEvent() noexcept;
        public:
            Stream& stream() noexcept;
            void format(const char *fmt, ...) noexcept;
        private:
             std::shared_ptr<trait::Logger> logger_;
            Level level_ = Level::kError;
            Stream stream_;
        };
    }
}

#endif