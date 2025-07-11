#ifndef CLIA_REACTOR_CHANNEL_H_
#define CLIA_REACTOR_CHANNEL_H_

#include <memory>

#include "clia/base/noncopyable.h"
#include "clia/reactor/base.h"

namespace clia {
    namespace reactor {
        class Channel final : Noncopyable{
        public:
            Channel(EventLoop *loop, const int fd) noexcept;
            ~Channel();
        public:
            void handle_event(clia::util::Timestamp receive_time);
            void set_read_callback(ReadEventCallback cb);
            void set_write_callback(EventCallback cb);
            void set_close_callback(EventCallback cb);
            void set_error_callback(EventCallback cb);

            void tie(const std::shared_ptr<void> &obj);

            int fd() const noexcept;
            int events() const noexcept;
            void set_revents(int revents) noexcept;

            void enable_reading() noexcept;
            void disable_reading() noexcept;
            void enable_writing() noexcept;
            void disable_writing() noexcept;
            void disable_all() noexcept;

            bool is_none_event() const noexcept;
            bool is_writing() const noexcept;
            bool is_reading() const noexcept;

            int index() const noexcept;
            void set_index(int index) noexcept;

            EventLoop* owner_loop() noexcept;
            void remove() noexcept;
        private:
            void update();
            void handle_event_with_guard(clia::util::Timestamp receive_time);
        private:
            EventLoop *const loop_;
            const int fd_;
            int events_;
            int revents_; // it's the received event types of epoll or poll
            int index_;
            bool event_handling_;
            bool added_to_loop_;
            bool tied_;
            std::weak_ptr<void> tie_;
            ReadEventCallback read_callback_;
            EventCallback write_callback_;
            EventCallback close_callback_;
            EventCallback error_callback_;
        };
    }
}

#endif