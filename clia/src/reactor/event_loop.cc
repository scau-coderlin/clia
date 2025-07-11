#include <unistd.h>
#include <sys/eventfd.h>

#include "clia/reactor/event_loop.h"
#include "clia/util/process.h"
#include "clia/reactor/epoller.h"
#include "clia/reactor/channel.h"
#include "clia/log.h"

namespace {
    constexpr int kPollTimeMs = 10000;
    thread_local clia::reactor::EventLoop *kLoopInThisThread = nullptr;
}

// 判断eventloop对象是否在自己的线程
bool clia::reactor::EventLoop::is_in_loop_thread() const {
    return clia::util::process::get_tid() == tid_;
}

clia::reactor::EventLoop::EventLoop()
    : looping_(false)
    , quit_(false)
    , event_handing_(false)
    , calling_pending_functors_(false)
    , tid_(clia::util::process::get_tid())
    , wakeup_fd_(::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)) 
    , current_active_channel_(nullptr)
{
    assert(wakeup_fd_ != -1 && nullptr == kLoopInThisThread);
#ifdef NDEBUG
    if (-1 == wakeup_fd_) {
        CLIA_LOG_ERROR << "Failed in eventfd";
        std::abort();
    }
    if (kLoopInThisThread) {
        CLIA_LOG_FATAL << "Another EventLoop " << kLoopInThisThread
            << " exists in this thread " << tid_;
        std::abort();
    } 
#endif
    kLoopInThisThread = this;
    poller_.reset(new Epoller(this));
    wakeup_channel_.reset(new Channel(this, wakeup_fd_));
    wakeup_channel_->set_read_callback(std::bind(&EventLoop::handle_read, this));
    wakeup_channel_->enable_reading();
}

clia::reactor::EventLoop::~EventLoop() {
    wakeup_channel_->disable_all();
    wakeup_channel_->remove();
    ::close(wakeup_fd_);
    kLoopInThisThread = nullptr;
}

void clia::reactor::EventLoop::loop() {
    assert(!looping_ && this->is_in_loop_thread());

    looping_ = true;
    quit_ = false;

    while (!quit_) {
        active_channels_.clear();
        poll_return_time_ = poller_->poll(kPollTimeMs, &active_channels_);
        event_handing_ = true;
        for (Channel *channel : active_channels_) {
            current_active_channel_ = channel;
            current_active_channel_->handle_event(poll_return_time_);
        }
        current_active_channel_ = nullptr;
        event_handing_ = false;
        this->do_pending_functors();
    }
    looping_ = false;
}

void clia::reactor::EventLoop::quit() {
    quit_ = true;
    if (!this->is_in_loop_thread()) {
        wakeup();
    }
}

clia::util::Timestamp clia::reactor::EventLoop::poll_return_time() const noexcept {
    return poll_return_time_;
}

// 在当前loop中执行
void clia::reactor::EventLoop::run_in_loop(Functor cb) {
    if (this->is_in_loop_thread()) {
        cb();
    } else {
        this->queue_in_loop(std::move(cb));
    }
}

// 把上层注册的回调函数cb放到队列中，唤醒loop所在的线程执行cb 
void clia::reactor::EventLoop::queue_in_loop(Functor cb) {
    {
        std::lock_guard<std::mutex> lck(mutex_);
        pending_functors_.push_back(std::move(cb));
    }

    if (!this->is_in_loop_thread() || calling_pending_functors_) {
        wakeup();
    }
}

void clia::reactor::EventLoop::update_channel(Channel *channel) {
    assert(channel->owner_loop() == this && this->is_in_loop_thread());
    poller_->update_channel(channel);
}

void clia::reactor::EventLoop::remove_channel(Channel *channel) {
    assert(channel->owner_loop() == this && this->is_in_loop_thread());
    if (event_handing_) {
        assert(current_active_channel_ == channel || 
            std::find(active_channels_.begin(), active_channels_.end(), channel) == active_channels_.end());
    }
    poller_->remove_channel(channel);
}


bool clia::reactor::EventLoop::has_channel(Channel *channel) const {
    assert(channel->owner_loop() == this && this->is_in_loop_thread());
    return poller_->has_channel(channel);
}

// 通过eventfd唤醒loop所在线程
void clia::reactor::EventLoop::wakeup() {
    std::uint64_t one = 1;
    const auto n = ::write(wakeup_fd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        CLIA_LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

// 给eventfd返回的文件描述符wakeup绑定的事件回调，唤醒epoll_wait
void clia::reactor::EventLoop::handle_read() {
    std::uint64_t one = 1;
    const auto n = ::read(wakeup_fd_, &one, sizeof(one)); 
    if (n != sizeof(one)) {
        CLIA_LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

// 执行上层回调
void clia::reactor::EventLoop::do_pending_functors() {
    std::vector<Functor> functors;
    calling_pending_functors_ = true;

    {
        std::lock_guard<std::mutex> lck(mutex_);
        functors.swap(pending_functors_);
    }

    for (const Functor &functor : functors) {
        functor();
    }
    calling_pending_functors_ = false;
}
