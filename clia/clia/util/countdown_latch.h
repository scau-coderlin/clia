#ifndef CLIA_UTIL_COUNTDOWN_LATCH_H_
#define CLIA_UTIL_COUNTDOWN_LATCH_H_

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
            int count_;        // 计数器
            mutable std::mutex lck_;  // 互斥锁
            std::condition_variable condition_; // 条件变量
        };
    };
};

#endif