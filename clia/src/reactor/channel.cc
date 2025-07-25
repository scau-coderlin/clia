#include <cassert>

#include <sys/epoll.h>

#include "clia/reactor/channel.h"
#include "clia/reactor/event_loop.h"

namespace {
    constexpr int kNoneEvent = 0;
    constexpr int kReadEvent = EPOLLIN | EPOLLPRI;
    constexpr int kWriteEvent = EPOLLOUT;
}

clia::reactor::Channel::Channel(EventLoop *loop, const int fd) noexcept
    : loop_(loop)
    , fd_(fd)
    , events_(0)
    , revents_(0)
    , index_(-1) 
    , event_handling_(false)
    , added_to_loop_(false)
    , tied_(false)
{
    assert(fd != -1);
    assert(loop_ != nullptr);
}

clia::reactor::Channel::~Channel() {
    assert(!event_handling_);
    assert(!added_to_loop_);
    if (loop_->is_in_loop_thread()) {
        assert(!loop_->has_channel(this));
    }
}

void clia::reactor::Channel::handle_event(clia::util::Timestamp receive_time) {
    if (tied_) {
        const auto guard = tie_.lock();
        if (guard) {
            this->handle_event_with_guard(receive_time);
        }
    } else {
        this->handle_event_with_guard(receive_time);
    }
}
void clia::reactor::Channel::set_read_callback(ReadEventCallback cb) {
    read_callback_ = std::move(cb);
}

void clia::reactor::Channel::set_write_callback(EventCallback cb) {
    write_callback_ = std::move(cb);
}

void clia::reactor::Channel::set_close_callback(EventCallback cb) {
    close_callback_ = std::move(cb);
}

void clia::reactor::Channel::set_error_callback(EventCallback cb) {
    error_callback_ = std::move(cb);
}

void clia::reactor::Channel::tie(const std::shared_ptr<void> &obj) {
    tie_ = obj;
    tied_ = true;
}

int clia::reactor::Channel::fd() const noexcept {
    return fd_;
}

int clia::reactor::Channel::events() const noexcept {
    return events_;
}

void clia::reactor::Channel::set_revents(int revents) noexcept {
    revents_ = revents;
}

void clia::reactor::Channel::enable_reading() noexcept {
    events_ |= kReadEvent;
    this->update();
}

void clia::reactor::Channel::disable_reading() noexcept {
    events_ &= ~kReadEvent;
    this->update();
}

void clia::reactor::Channel::enable_writing() noexcept {
    events_ |= kWriteEvent;
    this->update();
}

void clia::reactor::Channel::disable_writing() noexcept {
    events_ &= ~kWriteEvent;
    this->update();
}

void clia::reactor::Channel::disable_all() noexcept {
    events_ = kNoneEvent;
    this->update(); 
}

bool clia::reactor::Channel::is_none_event() const noexcept {
    return kNoneEvent == events_;
}

bool clia::reactor::Channel::is_writing() const noexcept {
    return events_ & kWriteEvent;
}

bool clia::reactor::Channel::is_reading() const noexcept {
    return events_ & kReadEvent;
}

int clia::reactor::Channel::index() const noexcept {
    return index_;
}

void clia::reactor::Channel::set_index(int index) noexcept {
    index_ = index;
}

clia::reactor::EventLoop* clia::reactor::Channel::owner_loop() noexcept {
    return loop_;
}

void clia::reactor::Channel::remove() noexcept {
    assert(this->is_none_event());
    added_to_loop_ = false;
    loop_->remove_channel(this);
}

void clia::reactor::Channel::update() {
    added_to_loop_ = true;
    loop_->update_channel(this);
}

void clia::reactor::Channel::handle_event_with_guard(clia::util::Timestamp receive_time) {
    event_handling_ = true;
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        if (close_callback_) {
            close_callback_();
        }
    }

    if (revents_ & EPOLLERR) {
        if (error_callback_) {
            error_callback_();
        }
    }
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (read_callback_) {
            read_callback_(receive_time);
        }
    }
    if (revents_ & EPOLLOUT) {
        if (write_callback_) {
            write_callback_();
        }
    }
    event_handling_ = false;
}