#ifndef CLIA_REACTOR_EVENT_LOOP_H_
#define CLIA_REACTOR_EVENT_LOOP_H_

#include <atomic>
#include <mutex>
#include <memory>

#include "clia/base/noncopyable.h"
#include "clia/reactor/base.h"
#include "clia/util/timestamp.h"

namespace clia {
    namespace reactor {
        class EventLoop final : Noncopyable {
        public:
            EventLoop();
            ~EventLoop();
        public:
            void loop();
            void quit();
            clia::util::Timestamp poll_return_time() const noexcept;
            // 在当前loop中执行
            void run_in_loop(Functor cb);
            // 把上层注册的回调函数cb放到队列中，唤醒loop所在的线程执行cb 
            void queue_in_loop(Functor cb);
            // 通过eventfd唤醒loop所在线程
            void wakeup();
            // 判断eventloop对象是否在自己的线程
            bool is_in_loop_thread() const;
        public:
            void remove_channel(Channel *channel);
            void update_channel(Channel *channel);
            bool has_channel(Channel *channel) const;
        private:
            // 给eventfd返回的文件描述符wakeup绑定的事件回调，唤醒epoll_wait
            void handle_read();
            // 执行上层回调
            void do_pending_functors(); 
        private:
            std::atomic_bool looping_;
            std::atomic_bool quit_;
            std::atomic_bool event_handing_; 
            std::atomic_bool calling_pending_functors_; // 标识当前loop是否有需要执行的回调操作
            const int tid_;
            const int wakeup_fd_;
            Channel *current_active_channel_;
            clia::util::Timestamp poll_return_time_;
            std::unique_ptr<clia::reactor::Epoller> poller_;
            std::unique_ptr<Channel> wakeup_channel_;
            ChannelList active_channels_;
            std::vector<Functor> pending_functors_;     // 存储loop需要执行的所有回调操作
            std::mutex mutex_;
        };
    }
}

#endif