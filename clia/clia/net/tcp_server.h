#ifndef CLIA_NET_TCP_SERVER_H_
#define CLIA_NET_TCP_SERVER_H_

#include <atomic>

#include "clia/base/noncopyable.h"
#include "clia/net/base.h"
#include "clia/net/inet_address.h"
#include "clia/reactor/base.h"
#include "clia/reactor/event_loop_thread_pool.h"

namespace clia {
    namespace net {
        class Acceptor;
        class TcpServer : Noncopyable {
        public:
            TcpServer(clia::reactor::EventLoop *loop, const InetAddress &listen_addr, const bool reuse_port = false);
            ~TcpServer();
        public:
            void set_thread_init_callback(clia::reactor::EventLoopThread::ThreadInitCallBack &cb);
            void set_connection_callback(const ConnectionCallback &cb);
            void set_message_callback(const MessageCallback &cb);
            void set_write_complete_callback(const WriteCompleteCallback &cb);
            void set_thread_num(const int num = std::thread::hardware_concurrency());
            void start();
        private:
            void new_connection(int sockfd, const InetAddress &peer_addr);
            void remove_connection(const TcpConnectionPtr &conn);
            void remove_connection_in_loop(const TcpConnectionPtr &conn);
        private:
            using ConnectionMap = std::map<int, TcpConnectionPtr>;
            clia::reactor::EventLoop *const loop_;
            std::unique_ptr<Acceptor> acceptor_;
            std::shared_ptr<clia::reactor::EventLoopThreadPool> threadpool_;
            int next_conn_id_;
            std::atomic_int started_;
            ConnectionCallback connection_callback_;
            MessageCallback message_callback_;
            WriteCompleteCallback write_complete_callback_;
            clia::reactor::EventLoopThread::ThreadInitCallBack thread_init_callback_;
            ConnectionMap connections_;
        };
    }
}

#endif