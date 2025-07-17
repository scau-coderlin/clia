#ifndef CLIA_CONTAINER_BOUNDED_BLOCKING_QUEUE_H_
#define CLIA_CONTAINER_BOUNDED_BLOCKING_QUEUE_H_

#include <mutex>
#include <condition_variable>

#include "clia/base/noncopyable.h"
#include "clia/container/circular_buffer.h"

namespace clia {
    namespace container {
        template <typename Tp>
        class BoundedBlockingQueue final : Noncopyable {
        public:
            explicit BoundedBlockingQueue(const std::size_t capacity);
            ~BoundedBlockingQueue();
        public:
            void put(Tp &&val);
            void put(const Tp &val);
            Tp take();
            bool empty() const noexcept;
            bool full() const noexcept;
            std::size_t size() const noexcept;
            std::size_t capacity() const noexcept;
        private:
            CircularBuffer<Tp> buffer_;
            std::mutex mutex_;
            std::condition_variable not_empty_;
            std::condition_variable not_full_;
        };
    }
}

template <typename Tp>
clia::container::BoundedBlockingQueue<Tp>::BoundedBlockingQueue(const std::size_t capacity) 
    : buffer_(capacity)
{
    ;
}

template <typename Tp>
clia::container::BoundedBlockingQueue<Tp>::~BoundedBlockingQueue() = default;

template <typename Tp>
void clia::container::BoundedBlockingQueue<Tp>::put(Tp &&val) {
    {
        std::unique_lock<std::mutex> lck(mutex_);
        while (buffer_.full()) {
            not_full_.wait(lck);
        }
        buffer_.push_back(std::move(val));
    }
    not_empty_.notify_one();
}

template <typename Tp>
void clia::container::BoundedBlockingQueue<Tp>::put(const Tp &val) {
    {
        std::unique_lock<std::mutex> lck(mutex_);
        while (buffer_.full()) {
            not_full_.wait(lck);
        }
        buffer_.push_back(val);
    }
    not_empty_.notify_one();
}
template <typename Tp>
Tp clia::container::BoundedBlockingQueue<Tp>::take() {
    std::unique_lock<std::mutex> lck(mutex_);
    while (buffer_.full()) {
        not_empty_.wait(lck);
    }
    const auto val = std::move(buffer_.front());
    buffer_.pop_front();
    not_full_.notify_one();
    return val;
}

bool empty() const noexcept;
bool full() const noexcept;
std::size_t size() const noexcept;
std::size_t capacity() const noexcept;

#endif