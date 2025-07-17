#ifndef CLIA_CONTAINER_CIRCULAR_BUFFER_H_
#define CLIA_CONTAINER_CIRCULAR_BUFFER_H_

#include <cassert>
#include <algorithm>
#include <cstddef>
#include <vector>

#include "clia/base/noncopyable.h"

namespace clia {
    namespace container {
        template <typename Tp>
        class CircularBuffer final : Noncopyable {
        public:
            inline explicit CircularBuffer(const std::size_t inital_size = 1024) noexcept;
            inline ~CircularBuffer() noexcept;
        public:
            inline std::size_t size() const noexcept;
            inline std::size_t capacity() const noexcept;
            inline bool empty() const noexcept;
            inline bool full() const noexcept;
            inline void resize(const std::size_t new_size) noexcept;
            inline void push_back(const Tp &val);
            inline void push_back(Tp &&val);
            inline Tp& front() noexcept;
            inline const Tp& front() const noexcept;
            inline void pop_front() noexcept;
        private:
            std::size_t start_;
            std::size_t end_;
            std::size_t count_;
            std::vector<Tp> buffer_;
        };
    }
}

template <typename Tp>
inline clia::container::CircularBuffer<Tp>::CircularBuffer(const std::size_t inital_size) noexcept 
    : start_(0)
    , end_(0)
    , count_(0)
    , buffer_(inital_size)
{

}

template <typename Tp>
inline clia::container::CircularBuffer<Tp>::~CircularBuffer() noexcept = default;

template <typename Tp>
inline std::size_t clia::container::CircularBuffer<Tp>::size() const noexcept {
    return count_;
}

template <typename Tp>
inline std::size_t clia::container::CircularBuffer<Tp>::capacity() const noexcept {
    return buffer_.size();
}

template <typename Tp>
inline bool clia::container::CircularBuffer<Tp>::empty() const noexcept {
    return 0 == count_;
}

template <typename Tp>
inline bool clia::container::CircularBuffer<Tp>::full() const noexcept {
    return this->capacity() == count_;
}

template <typename Tp>
inline void clia::container::CircularBuffer<Tp>::resize(const std::size_t new_size) noexcept {
    buffer_.resize(new_size);
}

template <typename Tp>
inline void clia::container::CircularBuffer<Tp>::push_back(const Tp &val) {
    assert(!this->full());
    buffer_[end_] = val;
    ++end_;
    if (this->capacity() == end_) {
        end_ = 0;
    }
    ++count_;
}

template <typename Tp>
inline void clia::container::CircularBuffer<Tp>::push_back(Tp &&val) {
    assert(!this->full());
    buffer_[end_] = std::move(val);
    ++end_;
    if (this->capacity() == end_) {
        end_ = 0;
    }
    ++count_;
}

template <typename Tp>
inline Tp& clia::container::CircularBuffer<Tp>::front() noexcept {
    assert(!this->empty());
    return buffer_[start_];
}

template <typename Tp>
inline const Tp& clia::container::CircularBuffer<Tp>::front() const noexcept {
    assert(!this->empty());
    return buffer_[start_];
}

template <typename Tp>
inline void clia::container::CircularBuffer<Tp>::pop_front() noexcept {
    assert(!this->empty());
    ++start_;
    if (this->capacity() == start_) {
        start_ = 0;
    }
    --count_;
}

#endif