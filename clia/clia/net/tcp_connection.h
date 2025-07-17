#ifndef CLIA_NET_TCP_CONNECTION_H_
#define CLIA_NET_TCP_CONNECTION_H_

#include <atomic>
#include <memory>

#include "clia/net/inet_address.h"
#include "clia/reactor/base.h"
#include "clia/net/base.h"
#include "clia/net/buffer.h"
#include "clia/base/noncopyable.h"
#include "clia/net/socket.h"
#include "clia/reactor/channel.h"

namespace clia {
    namespace net {
        /**
         * TcpServer => Accept => 有一个新用户连接，通过accept函数拿到connfd
         * => TcpConnection设置回调 => 设置到 Channel => Poller => Channel 回调
         */
        class TcpConnection : Noncopyable, public std::enable_shared_from_this<TcpConnection> {
            enum class State {
                kDisconnected,  // 已断开连接
                kConnecting,    // 正在连接
                kConnected,     // 已连接
                kDisconnecting, // 正在断开连接
            };
        public:
            TcpConnection(clia::reactor::EventLoop *loop, const int sockfd, const InetAddress &peer_addr);
            ~TcpConnection();
        public:
            clia::reactor::EventLoop* get_loop() const noexcept;
            const InetAddress& peer_addr() const noexcept;
            int fd() const noexcept;
            bool connected() const noexcept;
            void send(const void *buf, const std::size_t len);
            void shutdown();
            void set_connection_callback(const ConnectionCallback &cb);
            void set_message_callback(const MessageCallback &cb);
            void set_write_complete_callback(const WriteCompleteCallback &cb);
            void set_close_callback(const CloseCallback &cb);

            // 连接建立
            void connect_established();
            // 连接销毁
            void connect_destoryed();
        private:
            void set_state(const State state);
            void handle_read(clia::util::Timestamp recvive_time);
            void handle_write();
            void handle_close();
            void handle_error();
            void send_in_loop(const void *data, const std::size_t len);
            void shutdown_in_loop();
        private:
            clia::reactor::EventLoop *const loop_;
            const int fd_;
            std::atomic<State> state_;
            bool reading_;

            Socket socket_;
            clia::reactor::Channel channel_;

            const InetAddress peer_addr_;

            //用户通过写入 TcpServer 注册，TcpServer再将注册的回调传递给 TcpConnection TcpConnection再将回调注册到 Channel 中
            ConnectionCallback connection_callback_;
            MessageCallback message_callback_;
            WriteCompleteCallback write_complete_callback_;
            CloseCallback close_callback_;

            Buffer input_buffer_;
            Buffer output_buffer_;
        };
    }
}

#endif