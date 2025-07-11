#ifndef CLIA_REACTOR_EVENT_LOOP_THREAD_POOL_H_
#define CLIA_REACTOR_EVENT_LOOP_THREAD_POOL_H_

#include "clia/base/noncopyable.h"
#include "clia/reactor/base.h"
#include "clia/reactor/event_loop_thread.h"

namespace clia {
    namespace reactor {
        class EventLoopThreadPool : Noncopyable {
            using ThreadInitCallBack = EventLoopThread::ThreadInitCallBack;
        public:
            EventLoopThreadPool(EventLoop *base_loop) noexcept;
            ~EventLoopThreadPool() noexcept;
        public:
            void set_thread_num(int num) noexcept;
            void start(const ThreadInitCallBack &cb = ThreadInitCallBack());
            EventLoop* get_next_loop();
            std::vector<EventLoop*> get_all_loops();
            bool started() const noexcept;
        private:
            EventLoop *const base_loop_;
            bool started_;
            int num_threads_;
            int next_;
            std::vector<std::unique_ptr<EventLoopThread>> threads_;
            std::vector<EventLoop*> loops_;
        };
    }
}

#endif