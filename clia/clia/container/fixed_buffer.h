#ifndef CLIA_CONTAINER_FIXED_BUFFER_H_
#define CLIA_CONTAINER_FIXED_BUFFER_H_

#include <cassert>
#include <cstddef>
#include <cstring>
#include <type_traits>
#include <utility>

#include "clia/base/noncopyable.h"

namespace clia {
    namespace container {
        template <typename Tp, int Nm>
        class FixedBuffer final : Noncopyable {
            using Self = FixedBuffer<Tp, Nm>;
            static_assert(Nm > 0, "FixedBuffer size must be greater than 0");
        public:
            inline FixedBuffer() noexcept;
            inline ~FixedBuffer() noexcept;
        public:
            inline void append(const Tp& value) noexcept;
            inline void append(Tp&& value) noexcept;
            inline void append(const Tp *buf, const std::size_t len) noexcept;
            
            inline const Tp* data() const noexcept;
            inline Tp* data() noexcept;
            inline int size() const noexcept;
            inline int avail() const noexcept;
            inline void reset() noexcept;
            inline Tp* current() noexcept;
            inline void add(const std::size_t len) noexcept;
        public:
            static inline int max_size() noexcept;
        private:
            inline const Tp* end() const noexcept;
        private:
            Tp data_[Nm];
            Tp *cur_ = nullptr;
        };
    }
}

template <typename Tp, int Nm>
inline clia::container::FixedBuffer<Tp, Nm>::FixedBuffer() noexcept {
    this->reset();
    static_assert(Nm > 0, "FixedBuffer size must be greater than 0");
}

template <typename Tp, int Nm>
inline clia::container::FixedBuffer<Tp, Nm>::~FixedBuffer() noexcept = default;

template <typename Tp, int Nm>
inline const Tp* clia::container::FixedBuffer<Tp, Nm>::data() const noexcept {
    return data_;
}

template <typename Tp, int Nm>
inline Tp* clia::container::FixedBuffer<Tp, Nm>::data() noexcept {
    return data_;
}

template <typename Tp, int Nm>
inline int clia::container::FixedBuffer<Tp, Nm>::size() const noexcept {
    return static_cast<int>(cur_ - data());
}

template <typename Tp, int Nm>
inline int clia::container::FixedBuffer<Tp, Nm>::max_size() noexcept {
    return Nm;
}

template <typename Tp, int Nm>
inline int clia::container::FixedBuffer<Tp, Nm>::avail() const noexcept { 
    return static_cast<int>(end() - cur_); 
}

template <typename Tp, int Nm>
inline void clia::container::FixedBuffer<Tp, Nm>::reset() noexcept { 
    cur_ = data_; 
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
inline void clia::container::FixedBuffer<Tp, Nm>::append(const Tp* values, const std::size_t len) noexcept {
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
    return cur_;
}

template <typename Tp, int Nm>
inline void clia::container::FixedBuffer<Tp, Nm>::add(const std::size_t len) noexcept { 
    assert(avail() > len);
    cur_ += len; 
}

template <typename Tp, int Nm>
inline const Tp* clia::container::FixedBuffer<Tp, Nm>::end() const noexcept { 
    return data_ + Nm; 
}

#endif