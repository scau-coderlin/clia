#ifndef CLIA_REACTOR_EVENT_LOOP_THREAD_H_
#define CLIA_REACTOR_EVENT_LOOP_THREAD_H_

#include <thread>
#include <mutex>
#include <condition_variable>

#include "clia/reactor/base.h"
#include "clia/base/noncopyable.h"

namespace clia {
    namespace reactor {
        class EventLoopThread final : Noncopyable {
        public:
            using ThreadInitCallBack = std::function<void(EventLoop*)>;
        public:
            EventLoopThread(const ThreadInitCallBack &cb = ThreadInitCallBack());
            ~EventLoopThread();
        public:
            EventLoop* start_loop();
        private:
            void thread_func();
        private:
            EventLoop *loop_;
            bool exiting_;
            ThreadInitCallBack callback_;
            std::thread thread_;
            std::mutex mutex_;
            std::condition_variable cond_;
        };
    }
}

#endif