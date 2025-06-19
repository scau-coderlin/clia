#include <cassert>
#include <mutex>

#include "clia/util/countdown_latch.hpp"

clia::util::CountDownLatch::CountDownLatch(int count) noexcept
    : m_count(count) 
{
    assert(m_count > 0);
    // 初始化计数器和互斥锁
}

void clia::util::CountDownLatch::wait() {
    std::unique_lock<std::mutex> lck;
    while (m_count > 0) {
        m_condition.wait(lck);
    }
}

void clia::util::CountDownLatch::count_down() {
    std::lock_guard<std::mutex> lck(m_lck);
    --m_count;
    if (0 == m_count) {
        m_condition.notify_all();
    }
}

int clia::util::CountDownLatch::get_count() const noexcept {
    std::lock_guard<std::mutex> lck(m_lck);
    return m_count;
}
