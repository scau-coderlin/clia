#include <cassert>

#include "clia/util/countdown_latch.h"

clia::util::CountDownLatch::CountDownLatch(int count) noexcept
    : count_(count) 
{
    assert(count_ > 0);
    // 初始化计数器和互斥锁
}

void clia::util::CountDownLatch::wait() {
    std::unique_lock<std::mutex> lck;
    while (count_ > 0) {
        condition_.wait(lck);
    }
}

void clia::util::CountDownLatch::count_down() {
    std::lock_guard<std::mutex> lck(lck_);
    --count_;
    if (0 == count_) {
        condition_.notify_all();
    }
}

int clia::util::CountDownLatch::get_count() const noexcept {
    std::lock_guard<std::mutex> lck(lck_);
    return count_;
}
