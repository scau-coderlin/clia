#pragma once

#include <mutex>
#include <condition_variable>

namespace clia {
    namespace util {
        class CountDownLatch final {
        public:
            explicit CountDownLatch(int count) noexcept;
            void wait();
            void count_down();
            int get_count() const noexcept;
        private:
            int m_count;        // 计数器
            mutable std::mutex m_lck;  // 互斥锁
            std::condition_variable m_condition; // 条件变量
        };
    };
};