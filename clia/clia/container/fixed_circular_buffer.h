#ifndef CLIA_CONTAINER_FIXED_CIRCULAR_BUFFER_H_
#define CLIA_CONTAINER_FIXED_CIRCULAR_BUFFER_H_

#include <cassert>
#include <cstdint>

#include "clia/base/noncopyable.h"

namespace clia {
    namespace container {
        template <typename Tp, int Nm>
        class FixedCircularBuffer final : Noncopyable {
        public:
            inline FixedCircularBuffer() noexcept;
            inline ~FixedCircularBuffer() noexcept;
        public:
            inline std::size_t size() const noexcept;
            inline constexpr std::size_t capacity() const noexcept;
            inline bool empty() const noexcept;
            inline bool full() const noexcept;
            inline void push_back(const Tp &val);
            inline void push_back(Tp &&val);
            inline Tp& front() noexcept;
            inline const Tp& front() const noexcept;
            inline void pop_front() noexcept;
        private:
            std::size_t start_;
            std::size_t end_;
            std::size_t count_;
            Tp buffer_[Nm];
        };
    }
}

template <typename Tp, int Nm>
inline clia::container::FixedCircularBuffer<Tp, Nm>::FixedCircularBuffer() noexcept 
    : start_(0)
    , end_(0)
    , count_(0)
{

}

template <typename Tp, int Nm>
inline clia::container::FixedCircularBuffer<Tp, Nm>::~FixedCircularBuffer() noexcept = default;

template <typename Tp, int Nm>
inline std::size_t clia::container::FixedCircularBuffer<Tp, Nm>::size() const noexcept {
    return count_;
}

template <typename Tp, int Nm>
inline constexpr std::size_t clia::container::FixedCircularBuffer<Tp, Nm>::capacity() const noexcept {
    return Nm;
}

template <typename Tp, int Nm>
inline bool clia::container::FixedCircularBuffer<Tp, Nm>::empty() const noexcept {
    return 0 == count_;
}

template <typename Tp, int Nm>
inline bool clia::container::FixedCircularBuffer<Tp, Nm>::full() const noexcept {
    return Nm == count_;
}

template <typename Tp, int Nm>
inline void clia::container::FixedCircularBuffer<Tp, Nm>::push_back(const Tp &val) {
    assert(!this->full());
    buffer_[end_] = val;
    ++end_;
    if (Nm == end_) {
        end_ = 0;
    }
    ++count_;
}

template <typename Tp, int Nm>
inline void clia::container::FixedCircularBuffer<Tp, Nm>::push_back(Tp &&val) {
    assert(!this->full());
    buffer_[end_] = std::move(val);
    ++end_;
    if (this->capacity() == end_) {
        end_ = 0;
    }
    ++count_;
}

template <typename Tp, int Nm>
inline Tp& clia::container::FixedCircularBuffer<Tp, Nm>::front() noexcept {
    assert(!this->empty());
    return buffer_[start_];
}

template <typename Tp, int Nm>
inline const Tp& clia::container::FixedCircularBuffer<Tp, Nm>::front() const noexcept {
    assert(!this->empty());
    return buffer_[start_];
}

template <typename Tp, int Nm>
inline void clia::container::FixedCircularBuffer<Tp, Nm>::pop_front() noexcept {
    assert(!this->empty());
    ++start_;
    if (this->capacity() == start_) {
        start_ = 0;
    }
    --count_;
}

#endif