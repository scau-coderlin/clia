#include <unistd.h>
#include <cassert>

#include "clia/reactor/base.h"
#include "clia/util/process.h"
#include "clia/reactor/channel.h"
#include "clia/reactor/epoller.h"
#include "clia/log.h"

namespace {
    constexpr int kNew = -1;
    constexpr int kAdded = 1;
    constexpr int kDeleted = 2;
}

clia::reactor::Epoller::Epoller(EventLoop *loop)
    : owner_loop_(loop)
    , epfd_(::epoll_create1(EPOLL_CLOEXEC)) 
    , events_(kInitEventListSize)
{
    if (epfd_ < 0) {
        CLIA_FMT_LOG_FATAL("epoll_create1 err, errno = [%d][%s]\n", errno, clia::util::process::strerror(errno));
    }
}
clia::reactor::Epoller::~Epoller() noexcept {
    ::close(epfd_);
}

clia::util::Timestamp clia::reactor::Epoller::poll(int timeout_ms, ChannelList *active_channels) {
    const auto num_events = ::epoll_wait(epfd_, events_.data(), static_cast<int>(events_.size()), timeout_ms);
    clia::util::Timestamp now(clia::util::Timestamp::now());
    if (num_events > 0) {
        CLIA_LOG_TRACE << num_events << " events happened";
        fill_active_channels(num_events, active_channels);
        if (static_cast<std::size_t>(num_events) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    } else if (0 == num_events) {
        CLIA_LOG_TRACE << "Nothing happened";
    } else {
        if (errno != EINTR) {
            CLIA_FMT_LOG_ERROR("epoll_wait err, errno = [%d][%s]\n", errno, clia::util::process::strerror(errno));
        }
    }
    return now;
}

void clia::reactor::Epoller::update_channel(Channel *channel) {
    const int fd = channel->fd();
    const int index = channel->index();
    CLIA_LOG_TRACE << "fd = " << fd
        << " events = " << channel->events() << " index = " << index;
    if (kNew == index || kDeleted == index) {
        if (kNew == index) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        } else {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);
        if (channel->is_none_event()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void clia::reactor::Epoller::remove_channel(Channel *channel) {
    const int fd = channel->fd();
    CLIA_LOG_TRACE << "fd = " << fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->is_none_event());
    const int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    const auto n = channels_.erase(fd);
    assert(n == 1);
    if (kAdded == index) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}
void clia::reactor::Epoller::fill_active_channels(int num_events, ChannelList *active_channels) const {
    assert(static_cast<std::size_t>(num_events) <= events_.size());
    active_channels->reserve(active_channels->size() + num_events);
    for (int i = 0; i < num_events; ++i) {
        Channel *channel = static_cast<Channel*>(events_[i].data.ptr);
#ifndef NDEBUG
        const int fd = channel->fd();
        ChannelMap::const_iterator it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == channel);
#endif
        channel->set_revents(events_[i].events);
        active_channels->push_back(channel);
    }
}
void clia::reactor::Epoller::update(int operation, Channel *channel) {
    ::epoll_event event;
    std::memset(&event, 0, sizeof(event));
    event.events = channel->events();
    event.data.ptr = channel;
    const int fd = channel->fd();

    CLIA_LOG_TRACE << "epoll_ctl op = " << operation
        << " fd = " << fd << " event = { " << channel->events() << " }";

    if (::epoll_ctl(epfd_, operation, fd, &event) < 0) {
        if (EPOLL_CTL_DEL == operation) {
            CLIA_LOG_ERROR << "epoll_ctl op =" << operation << " fd =" << fd;
        } else {
            CLIA_LOG_FATAL << "epoll_ctl op =" << operation << " fd =" << fd;
        }
    }
}