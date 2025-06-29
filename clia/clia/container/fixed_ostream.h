#ifndef CLIA_CONTAINER_FIXED_OSTREAM_H_
#define CLIA_CONTAINER_FIXED_OSTREAM_H_

#include <cstddef>
#include <cstdarg>
#include <cstdio>

#include <sys/stat.h>

#include "clia/util/str_func.h"
#include "clia/base/noncopyable.h"
#include "clia/container/fixed_buffer.h"

namespace clia {
    namespace container {
        template <int Nm>
        class FixedOStream final : clia::Noncopyable {
            using Self = FixedOStream<Nm>;
            static_assert(Nm > 0, "Stream size must be greater than 0");
        public:
            inline FixedOStream() noexcept;
            inline ~FixedOStream() noexcept;
        public:
            inline const char* data() const noexcept;
            inline std::size_t size() const noexcept;
        public:
            inline void format(const char *fmt, ...) noexcept;
            inline void format(const char *fmt, va_list al) noexcept;
        public:
            inline Self& operator<<(const char *str) noexcept;
            inline Self& operator<<(const char val) noexcept;
            inline Self& operator<<(const float val) noexcept;
            inline Self& operator<<(const double val) noexcept;
            inline Self& operator<<(const unsigned char val) noexcept;
            inline Self& operator<<(const short val) noexcept;
            inline Self& operator<<(const unsigned short val) noexcept;
            inline Self& operator<<(const int val) noexcept;
            inline Self& operator<<(const unsigned int val) noexcept;
            inline Self& operator<<(const long val) noexcept;
            inline Self& operator<<(const unsigned long val) noexcept;
            inline Self& operator<<(const unsigned long long val) noexcept;
            inline Self& operator<<(const long long val) noexcept;
        private:
            clia::container::FixedBuffer<char, Nm> buffer_;
        };
    }
}


template <int Nm>
inline clia::container::FixedOStream<Nm>::FixedOStream() noexcept = default;

template <int Nm>
inline clia::container::FixedOStream<Nm>::~FixedOStream() noexcept = default;


template <int Nm>
inline const char* clia::container::FixedOStream<Nm>::data() const noexcept {
    return buffer_.data();
}

template <int Nm>
inline std::size_t clia::container::FixedOStream<Nm>::size() const noexcept {
    return buffer_.size();
}

template <int Nm>
inline void clia::container::FixedOStream<Nm>::format(const char *fmt, ...) noexcept {
    std::va_list args;
    va_start(args, fmt);
    this->format(fmt, args);
    va_end(args);
}

template <int Nm>
inline void clia::container::FixedOStream<Nm>::format(const char *fmt, std::va_list args) noexcept {
    const auto n = std::vsnprintf(buffer_.current(), buffer_.avail(), fmt, args);
    buffer_.add(n);
}

template <int Nm>
inline clia::container::FixedOStream<Nm>& clia::container::FixedOStream<Nm>::operator<<(const char *str) noexcept {
    if (str) {
        buffer_.append(str, std::strlen(str));
    } else {
        constexpr const char kNullStr[] = "(null)";
        buffer_.append(kNullStr, sizeof(kNullStr) - 1);
    }
    return *this;
}

template <int Nm>
inline clia::container::FixedOStream<Nm>& clia::container::FixedOStream<Nm>::operator<<(const char val) noexcept {
    buffer_.append(val);
    return *this;
}


template <int Nm>
inline clia::container::FixedOStream<Nm>& clia::container::FixedOStream<Nm>::operator<<(const float val) noexcept {
    const auto n = std::snprintf(buffer_.current(), buffer_.avail(), "%.2f", val);
    buffer_.add(n);
    return *this;
}

template <int Nm>
inline clia::container::FixedOStream<Nm>& clia::container::FixedOStream<Nm>::operator<<(const double val) noexcept {
    const auto n = std::snprintf(buffer_.current(), buffer_.avail(), "%.2lf", val);
    buffer_.add(n);
    return *this;
}

template <int Nm>
inline clia::container::FixedOStream<Nm>& clia::container::FixedOStream<Nm>::operator<<(const unsigned char val) noexcept {
    const auto n = clia::util::str_func::convert(buffer_.current(), buffer_.avail(), val);
    buffer_.add(n);
    return *this;
}

template <int Nm>
inline clia::container::FixedOStream<Nm>& clia::container::FixedOStream<Nm>::operator<<(const short val) noexcept {
    const auto n = clia::util::str_func::convert(buffer_.current(), buffer_.avail(), val);
    buffer_.add(n);
    return *this;
}

template <int Nm>
inline clia::container::FixedOStream<Nm>& clia::container::FixedOStream<Nm>::operator<<(const unsigned short val) noexcept {
    const auto n = clia::util::str_func::convert(buffer_.current(), buffer_.avail(), val);
    buffer_.add(n);
    return *this;
}

template <int Nm>
inline clia::container::FixedOStream<Nm>& clia::container::FixedOStream<Nm>::operator<<(const int val) noexcept {
    const auto n = clia::util::str_func::convert(buffer_.current(), buffer_.avail(), val);
    buffer_.add(n);
    return *this;
}

template <int Nm>
inline clia::container::FixedOStream<Nm>& clia::container::FixedOStream<Nm>::operator<<(const unsigned int val) noexcept {
    const auto n = clia::util::str_func::convert(buffer_.current(), buffer_.avail(), val);
    buffer_.add(n);
    return *this;
}

template <int Nm>
inline clia::container::FixedOStream<Nm>& clia::container::FixedOStream<Nm>::operator<<(const long val) noexcept {
    const auto n = clia::util::str_func::convert(buffer_.current(), buffer_.avail(), val);
    buffer_.add(n);
    return *this;
}

template <int Nm>
inline clia::container::FixedOStream<Nm>& clia::container::FixedOStream<Nm>::operator<<(const unsigned long val) noexcept {
    const auto n = clia::util::str_func::convert(buffer_.current(), buffer_.avail(), val);
    buffer_.add(n);
    return *this;
}

template <int Nm>
inline clia::container::FixedOStream<Nm>& clia::container::FixedOStream<Nm>::operator<<(const unsigned long long val) noexcept {
    const auto n = clia::util::str_func::convert(buffer_.current(), buffer_.avail(), val);
    buffer_.add(n);
    return *this;
}

template <int Nm>
inline clia::container::FixedOStream<Nm>& clia::container::FixedOStream<Nm>::operator<<(const long long val) noexcept {
    const auto n = clia::util::str_func::convert(buffer_.current(), buffer_.avail(), val);
    buffer_.add(n);
    return *this;
}

#endif