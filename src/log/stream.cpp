#include <cstdio>
#include <cstring>
#include <cassert>

#include "log/stream.hpp"
#include "util/str.hpp"

clia::log::Stream::Stream() noexcept {
    ;
}

clia::log::Stream::~Stream() noexcept {
    ;
}

const char* clia::log::Stream::data() const noexcept {
    return m_buffer.data();
}

std::size_t clia::log::Stream::size() const noexcept {
    return m_buffer.size();
}

void clia::log::Stream::format(const char *fmt, ...) noexcept {
    std::va_list args;
    va_start(args, fmt);
    format(fmt, args);
    va_end(args);
}

void clia::log::Stream::resize(const std::size_t sz) noexcept {
    m_buffer.resize(sz);
}

int clia::log::Stream::modify(const int start_idx, const void *buf, const int len) noexcept {
    assert(start_idx + len < m_buffer.max_size());
    assert(start_idx >= 0 && start_idx < m_buffer.max_size());
    assert(buf != nullptr);
    std::memmove(m_buffer.data() + start_idx, buf, len);
    if (size() < start_idx + len) {
        resize(start_idx + len);
    }
    return 0;
}

void clia::log::Stream::format(const char *fmt, va_list args) noexcept {
    const auto n = std::vsnprintf(m_buffer.current(), m_buffer.avail(), fmt, args);
    m_buffer.add(n);
}

clia::log::Stream& clia::log::Stream::operator<<(const char *str) noexcept {
    if (str) {
        m_buffer.append_range(str, std::strlen(str));
    } else {
        constexpr const char *nullstr = "(null)";
        static const std::size_t nullstrlen = std::strlen(nullstr);
        m_buffer.append_range(nullstr, nullstrlen);
    }
    return *this;
}

clia::log::Stream& clia::log::Stream::operator<<(const char val) noexcept {
    m_buffer.append(val);
    return *this;
}

clia::log::Stream& clia::log::Stream::operator<<(const int val) noexcept {
    const auto n = clia::util::str::convert(m_buffer.current(), m_buffer.avail(), val);
    m_buffer.add(n);
    return *this;
}

clia::log::Stream& clia::log::Stream::operator<<(const unsigned int val) noexcept {
    const auto n = clia::util::str::convert(m_buffer.current(), m_buffer.avail(), val);
    m_buffer.add(n);
    return *this;
}

clia::log::Stream& clia::log::Stream::operator<<(const short val) noexcept {
    const auto n = clia::util::str::convert(m_buffer.current(), m_buffer.avail(), val);
    m_buffer.add(n);
    return *this;
}

clia::log::Stream& clia::log::Stream::operator<<(const unsigned short val) noexcept {
    const auto n = clia::util::str::convert(m_buffer.current(), m_buffer.avail(), val);
    m_buffer.add(n);
    return *this;
}

clia::log::Stream& clia::log::Stream::operator<<(const long val) noexcept {
    const auto n = clia::util::str::convert(m_buffer.current(), m_buffer.avail(), val);
    m_buffer.add(n);
    return *this;
}

clia::log::Stream& clia::log::Stream::operator<<(const unsigned long val) noexcept {
    const auto n = clia::util::str::convert(m_buffer.current(), m_buffer.avail(), val);
    m_buffer.add(n);
    return *this;
}

clia::log::Stream& clia::log::Stream::operator<<(const float val) noexcept {
    const auto n = std::snprintf(m_buffer.current(), m_buffer.avail(), "%.2f", val);
    m_buffer.add(n);
    return *this;
}

clia::log::Stream& clia::log::Stream::operator<<(const double val) noexcept {
    const auto n = std::snprintf(m_buffer.current(), m_buffer.avail(), "%.2lf", val);
    m_buffer.add(n);
    return *this;
}
