#ifndef CLIA_NET_SOCKET_H_
#define CLIA_NET_SOCKET_H_

#include "clia/base/noncopyable.h"

namespace clia {
    namespace net {
        class InetAddress;

        class Socket : Noncopyable {
        public:
            explicit Socket(const int sockfd) noexcept;
            virtual ~Socket() noexcept;
        public:
            int fd() const noexcept;
            void bind_address(const InetAddress &local_addr) noexcept;
            void listen() noexcept;
            int accept(InetAddress *peer_addr) noexcept;
            void shutdown_write() noexcept;
        public:
            void set_tcp_nodelay(const bool on) noexcept;
            void set_reuse_addr(const bool on) noexcept;
            void set_reuse_port(const bool on) noexcept;
            void set_keeyalive(const bool on) noexcept;
        private:
            const int sockfd_;
        };
    }
}

#endif