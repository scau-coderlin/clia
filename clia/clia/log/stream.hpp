#pragma once

#include <cstddef>
#include <cstdarg>

#include "clia/container/fixed_buffer.hpp"

namespace clia {
    namespace log {
        class Stream final {
        public:
            Stream() noexcept;
            ~Stream() noexcept;
            Stream(const Stream&) = delete;
            Stream& operator=(const Stream&) = delete;
            Stream(Stream&&) noexcept = delete;
            Stream& operator=(Stream&&) noexcept = delete;
        public:
            const char* data() const noexcept;
            std::size_t size() const noexcept;
            void resize(const std::size_t sz) noexcept;
            int modify(const int start_idx, const void *buf, const int len) noexcept;
        public:
            void format(const char *fmt, ...) noexcept;
            void format(const char *fmt, va_list al) noexcept;
        public:
            Stream& operator<<(const char *str) noexcept;
            Stream& operator<<(const char val) noexcept;
            Stream& operator<<(const int val) noexcept;
            Stream& operator<<(const unsigned int val) noexcept;
            Stream& operator<<(const short val) noexcept;
            Stream& operator<<(const unsigned short val) noexcept;
            Stream& operator<<(const long val) noexcept;
            Stream& operator<<(const unsigned long val) noexcept;
            Stream& operator<<(const float val) noexcept;
            Stream& operator<<(const double val) noexcept;
        private:
            clia::container::FixedBuffer<char, 1024> m_buffer;
        };
    }
}