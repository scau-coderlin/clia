#ifndef CLIA_NET_INET_ADDR_H_
#define CLIA_NET_INET_ADDR_H_

#include <cstdint>
#include <netinet/in.h>

#include "clia/base/copyable.h"

namespace clia {
    namespace net {
        class InetAddress final : clia::Copyable {
        public:
            InetAddress() noexcept;
            InetAddress(const char *ip, std::uint16_t port, const bool is_ipv6 = false) noexcept;
            explicit InetAddress(const ::sockaddr_in &addr) noexcept;
            explicit InetAddress(const ::sockaddr_in6 &addr) noexcept;
            InetAddress(const InetAddress &oth) noexcept;
            InetAddress& operator=(const InetAddress &oth) noexcept;
            ~InetAddress() noexcept;
        public:
            ::sa_family_t family() const noexcept;
            int get_ipaddr(char *buf, const std::size_t sz) const noexcept;
            std::string get_addr() const noexcept;
            const ::sockaddr* get_sockaddr() const noexcept;
        private:
            union {
                ::sockaddr_in addr_;
                ::sockaddr_in6 addr6_;
            };
        };
    }
}

#endif