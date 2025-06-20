#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <type_traits>
#include <utility>

#include "clia/base/copyable.hpp"

namespace clia {
    namespace container {
        template <typename Tp, int Nm>
        class FixedBuffer final : Copyable {
            using Self = FixedBuffer<Tp, Nm>;
            static_assert(Nm > 0, "FixedBuffer size must be greater than 0");
        public:
            inline FixedBuffer() noexcept;
            inline ~FixedBuffer() noexcept;
            inline FixedBuffer(const Self &oth) noexcept;
            inline FixedBuffer& operator=(const Self &oth) noexcept;
        public:
            inline const Tp* data() const noexcept ;
            inline Tp* data() noexcept;
            inline int size() const noexcept;
            inline int max_size() const noexcept;
            inline void resize(const std::size_t sz) noexcept;
            inline int avail() const noexcept;
            inline void reset() noexcept;
            inline void append(const Tp& value) noexcept;
            inline void append(Tp&& value) noexcept;
            inline void append_range(const Tp* values, const std::size_t len) noexcept;
            inline Tp* current() noexcept;
            inline void add(const std::size_t len) noexcept;
        private:
            inline const Tp* end() const noexcept;
        private:
            Tp *m_current = nullptr;
            Tp m_data[Nm];
        };
    }
}

template <typename Tp, int Nm>
inline clia::container::FixedBuffer<Tp, Nm>::FixedBuffer() noexcept
    : m_current(m_data) {
    static_assert(Nm > 0, "FixedBuffer size must be greater than 0");
}

template <typename Tp, int Nm>
inline clia::container::FixedBuffer<Tp, Nm>::~FixedBuffer() noexcept {
    // No dynamic memory to release, so nothing to do here.
}

template <typename Tp, int Nm>
inline clia::container::FixedBuffer<Tp, Nm>::FixedBuffer(const Self& oth) noexcept
    : m_current(m_data) {
    static_assert(Nm > 0, "FixedBuffer size must be greater than 0");
    if (std::is_arithmetic<Tp>::value) {
        std::memcpy(current(), oth.data(), oth.size() * sizeof(Tp));
        add(oth.size());
    }  else {
        for (int i = 0; i < oth.size(); ++i) {
            append(oth.data()[i]);
        }
    }
}

template <typename Tp, int Nm>
inline clia::container::FixedBuffer<Tp, Nm>& ::clia::container::FixedBuffer<Tp, Nm>::operator=(const Self& oth) noexcept {
    if (this != &oth) {
        this->reset();
        if (std::is_arithmetic<Tp>::value) {
            std::memcpy(current(), oth.data(), oth.size() * sizeof(Tp));
            add(oth.size());
        }  else {
            for (int i = 0; i < oth.size(); ++i) {
                append(oth.data()[i]);
            }
        }
    }
    return *this;
}

template <typename Tp, int Nm>
inline const Tp* clia::container::FixedBuffer<Tp, Nm>::data() const noexcept {
    return m_data;
}

template <typename Tp, int Nm>
inline Tp* clia::container::FixedBuffer<Tp, Nm>::data() noexcept {
    return m_data;
}

template <typename Tp, int Nm>
inline int clia::container::FixedBuffer<Tp, Nm>::size() const noexcept {
    return static_cast<int>(m_current - data());
}

template <typename Tp, int Nm>
inline int clia::container::FixedBuffer<Tp, Nm>::max_size() const noexcept {
    return Nm;
}

template <typename Tp, int Nm>
inline void clia::container::FixedBuffer<Tp, Nm>::resize(const std::size_t sz) noexcept {
    assert(sz < Nm);
    m_current = m_data + sz;
}

template <typename Tp, int Nm>
inline int clia::container::FixedBuffer<Tp, Nm>::avail() const noexcept { 
    return static_cast<int>(end() - m_current); 
}

template <typename Tp, int Nm>
inline void clia::container::FixedBuffer<Tp, Nm>::reset() noexcept { 
    m_current = m_data; 
}

template <typename Tp, int Nm>
inline void clia::container::FixedBuffer<Tp, Nm>::append(const Tp& value) noexcept {
    assert(avail() > 0);
    *current() = value;
    add(1);
}

template <typename Tp, int Nm>
inline void clia::container::FixedBuffer<Tp, Nm>::append(Tp&& value) noexcept {
    assert(avail() > 0);
    *current() = std::move(value);
    add(1);
}

template <typename Tp, int Nm>
inline void clia::container::FixedBuffer<Tp, Nm>::append_range(const Tp* values, const std::size_t len) noexcept {
    assert(len < avail());
    if (std::is_arithmetic<Tp>::value) {
        std::memcpy(current(), values, len * sizeof(Tp));
        add(len);
    } else {
        for (std::size_t i = 0; i < len; ++i) {
            append(values[i]);
        }
    }
}

template <typename Tp, int Nm>
inline Tp* clia::container::FixedBuffer<Tp, Nm>::current() noexcept {
    return m_current;
}

template <typename Tp, int Nm>
inline void clia::container::FixedBuffer<Tp, Nm>::add(const std::size_t len) noexcept { 
    assert(avail() > len);
    m_current += len; 
}

template <typename Tp, int Nm>
inline const Tp* clia::container::FixedBuffer<Tp, Nm>::end() const noexcept { 
    return m_data + Nm; 
}