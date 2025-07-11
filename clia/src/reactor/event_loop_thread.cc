#include "clia/reactor/event_loop.h"
#include "clia/reactor/event_loop_thread.h"

clia::reactor::EventLoopThread::EventLoopThread(const ThreadInitCallBack &cb)
    : loop_(nullptr) 
    , exiting_(false)
    , callback_(cb)
{
    
}

clia::reactor::EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (loop_ != nullptr) {
        loop_->quit();
    }
    if (thread_.joinable()) {
        thread_.join();
    }
}

clia::reactor::EventLoop* clia::reactor::EventLoopThread::start_loop() {
    EventLoop *loop = nullptr;
    thread_ = std::thread(std::bind(&EventLoopThread::thread_func, this));
    {
        std::unique_lock<std::mutex> lck(mutex_);
        while (!loop_) {
            cond_.wait(lck);
        }
        loop = loop_;
    }
    return loop;
}

void clia::reactor::EventLoopThread::thread_func() {
    EventLoop loop;
    if (callback_) {
        callback_(&loop);
    }

    {
        std::lock_guard<std::mutex> lck(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();
    std::lock_guard<std::mutex> lck(mutex_);
    loop_ = nullptr;
}