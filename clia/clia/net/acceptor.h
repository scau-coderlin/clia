#ifndef CLIA_NET_ACCEPTOR_H_
#define CLIA_NET_ACCEPTOR_H_

#include <functional>
#include <memory>

#include "clia/base/noncopyable.h"
#include "clia/net/inet_address.h"
#include "clia/reactor/base.h"
#include "clia/reactor/channel.h"
#include "clia/net/socket.h"

namespace clia {
    namespace net {
        class Acceptor : Noncopyable {
        public:
            using NewConnectionCallback = std::function<void(int connfd, const InetAddress &peer_addr)>;
        public:
            Acceptor(clia::reactor::EventLoop *loop, const InetAddress &listen_addr, const bool reuseport = true);
            ~Acceptor() noexcept;
        public:
            void set_new_connection_callback(const NewConnectionCallback &cb);
            void listen() noexcept;
            bool listening() const noexcept;
        private:
            void handle_read();
        private:
            clia::reactor::EventLoop *const loop_;
            bool listening_;
            std::unique_ptr<Socket> accept_socket_;
            std::unique_ptr<clia::reactor::Channel> accept_channel_;
            NewConnectionCallback new_connection_callback_;
        };
    }
}

#endif