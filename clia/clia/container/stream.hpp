#pragma once

#include <cstddef>
#include <cstdarg>
#include <cstdio>

#include <sys/stat.h>

#include "clia/container/fixed_buffer.hpp"
#include "clia/util/str.hpp"
#include "clia/base/copyable.hpp"

namespace clia {
    namespace container {
        template <int N>
        class Stream final : clia::Copyable {
            using Self = Stream<N>;
            static_assert(N > 0, "Stream size must be greater than 0");
        public:
            inline Stream() noexcept;
            inline ~Stream() noexcept;
            inline Stream(const Self &oth) noexcept;
            inline Stream& operator=(const Self &oth) noexcept;
        public:
            inline const char* data() const noexcept;
            inline std::size_t size() const noexcept;
            inline void resize(const std::size_t sz) noexcept;
            inline int modify(const int start_idx, const void *buf, const int len) noexcept;
        public:
            inline void format(const char *fmt, ...) noexcept;
            inline void format(const char *fmt, va_list al) noexcept;
        public:
            inline Stream& operator<<(const char *str) noexcept;
            inline Stream& operator<<(const char val) noexcept;
            inline Stream& operator<<(const int val) noexcept;
            inline Stream& operator<<(const unsigned int val) noexcept;
            inline Stream& operator<<(const short val) noexcept;
            inline Stream& operator<<(const unsigned short val) noexcept;
            inline Stream& operator<<(const long val) noexcept;
            inline Stream& operator<<(const unsigned long val) noexcept;
            inline Stream& operator<<(const float val) noexcept;
            inline Stream& operator<<(const double val) noexcept;
        private:
            clia::container::FixedBuffer<char, N> m_buffer;
        };
    }
}


template <int N>
inline clia::container::Stream<N>::Stream() noexcept {
    ;
}

template <int N>
inline clia::container::Stream<N>::~Stream() noexcept {
    ;
}

template <int N>
inline clia::container::Stream<N>::Stream(const Self &oth) noexcept
    : m_buffer(oth.m_buffer) {

}

template <int N>
inline clia::container::Stream<N>& clia::container::Stream<N>::operator=(const Self &oth) noexcept {
    if (this != &oth) {
        m_buffer = oth.m_buffer; // Use the assignment operator of FixedBuffer
    }
    return *this;
}

template <int N>
inline const char* clia::container::Stream<N>::data() const noexcept {
    return m_buffer.data();
}

template <int N>
inline std::size_t clia::container::Stream<N>::size() const noexcept {
    return m_buffer.size();
}

template <int N>
inline void clia::container::Stream<N>::format(const char *fmt, ...) noexcept {
    std::va_list args;
    va_start(args, fmt);
    format(fmt, args);
    va_end(args);
}

template <int N>
inline void clia::container::Stream<N>::resize(const std::size_t sz) noexcept {
    m_buffer.resize(sz);
}

template <int N>
inline int clia::container::Stream<N>::modify(const int start_idx, const void *buf, const int len) noexcept {
    assert(start_idx + len < m_buffer.max_size());
    assert(start_idx >= 0 && start_idx < m_buffer.max_size());
    assert(buf != nullptr);
    std::memmove(m_buffer.data() + start_idx, buf, len);
    if (size() < start_idx + len) {
        resize(start_idx + len);
    }
    return 0;
}

template <int N>
inline void clia::container::Stream<N>::format(const char *fmt, va_list args) noexcept {
    const auto n = std::vsnprintf(m_buffer.current(), m_buffer.avail(), fmt, args);
    m_buffer.add(n);
}

template <int N>
inline clia::container::Stream<N>& clia::container::Stream<N>::operator<<(const char *str) noexcept {
    if (str) {
        m_buffer.append_range(str, std::strlen(str));
    } else {
        constexpr const char *nullstr = "(null)";
        static const std::size_t nullstrlen = std::strlen(nullstr);
        m_buffer.append_range(nullstr, nullstrlen);
    }
    return *this;
}

template <int N>
inline clia::container::Stream<N>& clia::container::Stream<N>::operator<<(const char val) noexcept {
    m_buffer.append(val);
    return *this;
}

template <int N>
inline clia::container::Stream<N>& clia::container::Stream<N>::operator<<(const int val) noexcept {
    const auto n = clia::util::str::convert(m_buffer.current(), m_buffer.avail(), val);
    m_buffer.add(n);
    return *this;
}

template <int N>
inline clia::container::Stream<N>& clia::container::Stream<N>::operator<<(const unsigned int val) noexcept {
    const auto n = clia::util::str::convert(m_buffer.current(), m_buffer.avail(), val);
    m_buffer.add(n);
    return *this;
}

template <int N>
inline clia::container::Stream<N>& clia::container::Stream<N>::operator<<(const short val) noexcept {
    const auto n = clia::util::str::convert(m_buffer.current(), m_buffer.avail(), val);
    m_buffer.add(n);
    return *this;
}

template <int N>
inline clia::container::Stream<N>& clia::container::Stream<N>::operator<<(const unsigned short val) noexcept {
    const auto n = clia::util::str::convert(m_buffer.current(), m_buffer.avail(), val);
    m_buffer.add(n);
    return *this;
}

template <int N>
inline clia::container::Stream<N>& clia::container::Stream<N>::operator<<(const long val) noexcept {
    const auto n = clia::util::str::convert(m_buffer.current(), m_buffer.avail(), val);
    m_buffer.add(n);
    return *this;
}

template <int N>
inline clia::container::Stream<N>& clia::container::Stream<N>::operator<<(const unsigned long val) noexcept {
    const auto n = clia::util::str::convert(m_buffer.current(), m_buffer.avail(), val);
    m_buffer.add(n);
    return *this;
}

template <int N>
inline clia::container::Stream<N>& clia::container::Stream<N>::operator<<(const float val) noexcept {
    const auto n = std::snprintf(m_buffer.current(), m_buffer.avail(), "%.2f", val);
    m_buffer.add(n);
    return *this;
}

template <int N>
inline clia::container::Stream<N>& clia::container::Stream<N>::operator<<(const double val) noexcept {
    const auto n = std::snprintf(m_buffer.current(), m_buffer.avail(), "%.2lf", val);
    m_buffer.add(n);
    return *this;
}