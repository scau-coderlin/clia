#ifndef CLIA_REACTOR_EPOLLER_H_
#define CLIA_REACTOR_EPOLLER_H_

#include <sys/epoll.h>

#include "clia/reactor/base.h"
#include "clia/util/timestamp.h"
#include "clia/base/noncopyable.h"

namespace clia {
    namespace reactor {
        class Epoller : Noncopyable {
        public:
            Epoller(EventLoop *loop);
            ~Epoller() noexcept;
        public:
            clia::util::Timestamp poll(int timeout_ms, ChannelList *active_channels);
            void update_channel(Channel *channel);
            void remove_channel(Channel *channel);
            bool has_channel(Channel *channel) const;
        private:
            void fill_active_channels(int num_events, ChannelList *active_channels) const;
            void update(int operation, Channel *channel);
        private:
            EventLoop *const owner_loop_;
            int epfd_;
            std::vector<::epoll_event> events_;
            ChannelMap channels_;
        };
    }
}

#endif