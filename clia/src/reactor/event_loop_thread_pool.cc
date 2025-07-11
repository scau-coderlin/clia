#include <cassert>

#include "clia/log.h"
#include "clia/reactor/event_loop.h"
#include "clia/reactor/event_loop_thread_pool.h"

clia::reactor::EventLoopThreadPool::EventLoopThreadPool(EventLoop *base_loop) noexcept
    : base_loop_(base_loop)
    , started_(false)
    , num_threads_(0)
    , next_(0)
{
    assert(base_loop_ != nullptr);
}

clia::reactor::EventLoopThreadPool::~EventLoopThreadPool() noexcept {

}

void clia::reactor::EventLoopThreadPool::set_thread_num(int num) noexcept {
    num_threads_ = num;
}

bool clia::reactor::EventLoopThreadPool::started() const noexcept {
    return started_;
}

void clia::reactor::EventLoopThreadPool::start(const ThreadInitCallBack &cb) {
    assert(!started_);
    assert(base_loop_->is_in_loop_thread());

    started_ = true;
    for (int i = 0; i < num_threads_; ++i) {
        EventLoopThread *t = new EventLoopThread(cb);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->start_loop());
    }
    if (0 == num_threads_ && cb) {
        cb(base_loop_);
    }
}

clia::reactor::EventLoop* clia::reactor::EventLoopThreadPool::get_next_loop() {
    assert(base_loop_->is_in_loop_thread() && started_);

    EventLoop *loop = base_loop_;
    if (!loops_.empty()) {
        loop = loops_[next_];
        ++next_;
        if (static_cast<std::size_t>(next_) >= loops_.size()) {
            next_ = 0;
        }
    }
    return loop;
}

std::vector<clia::reactor::EventLoop*> clia::reactor::EventLoopThreadPool::get_all_loops() {
    assert(base_loop_->is_in_loop_thread() && started_);

    if (loops_.empty()) {
        return std::vector<clia::reactor::EventLoop*>(1, base_loop_);
    } else {
        return loops_;
    }
}
