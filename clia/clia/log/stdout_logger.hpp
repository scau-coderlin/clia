#pragma once

#include <cstdio>

#include "clia/base/nocopyable.hpp"
#include "clia/log/trait.hpp"

namespace clia {
    namespace log {
        class StdoutLogger final : public LoggerTrait, Noncopyable {
        public:
            using LoggerTrait::LoggerTrait;
            ~StdoutLogger() = default;
        public:
            void log(const void *message, const std::size_t len) noexcept override {
                std::fwrite(message, 1, len, stdout);
            }
        };
    }
}