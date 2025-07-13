#ifndef CLIA_NET_TCP_CONNECTION_H_
#define CLIA_NET_TCP_CONNECTION_H_

#include <atomic>
#include <memory>

#include "clia/net/inet_address.h"
#include "clia/reactor/base.h"

namespace clia {
    namespace net {
        class Socket;
        /**
         * TcpServer => Accept => 有一个新用户连接，通过accept函数拿到connfd
         * => TcpConnection设置回调 => 设置到 Channel => Poller => Channel 回调
         */
        class TcpConnection {
            enum class State {
                kDisconnected,  // 已断开连接
                kConnecting,    // 正在连接
                kConnected,     // 已连接
                kDisConnecting, // 正在断开连接
            };
        private:
            reactor::EventLoop *const loop_;
            std::atomic<State> state_;
            bool reading_;

            std::unique_ptr<Socket> socket_;
            std::unique_ptr<reactor::Channel> channel_;

            const InetAddress peer_addr;
            
        };
    }
}

#endif