#include <cassert>
#include <cstring>

#include <arpa/inet.h>

#include "clia/log.h"
#include "clia/util/process.h"
#include "clia/net/inet_address.h"

clia::net::InetAddress::InetAddress(const char *ip, std::uint16_t port, const bool is_ipv6) noexcept {
    std::memset(&addr_, 0, sizeof(addr_));
    if (is_ipv6 || std::strchr(ip, ':')) {
        addr_.ipv6.sin6_family = AF_INET6;
        addr_.ipv6.sin6_port = htobe16(port);
        if (::inet_pton(AF_INET6, ip, &addr_.ipv6.sin6_addr) <= 0) {
            CLIA_FMT_LOG_ERROR("inet_pton fail, errno = [%d][%s]", errno, clia::util::process::strerror(errno));
        }
    } else {
        addr_.ipv4.sin_family = AF_INET;
        addr_.ipv4.sin_port = htobe16(port);
        if (::inet_pton(AF_INET, ip, &addr_.ipv4.sin_addr) <= 0) {
            CLIA_FMT_LOG_ERROR("inet_pton fail, errno = [%d][%s]", errno, clia::util::process::strerror(errno));
        }
    }
}

clia::net::InetAddress::~InetAddress() noexcept {

}

clia::net::InetAddress::InetAddress(const ::sockaddr_in &addr) noexcept {
    addr_.ipv4 = addr;
}

clia::net::InetAddress::InetAddress(const ::sockaddr_in6 &addr) noexcept {
    addr_.ipv6 = addr;
}

clia::net::InetAddress::InetAddress(const InetAddress &oth) noexcept 
    : addr_(oth.addr_) {

}

clia::net::InetAddress& clia::net::InetAddress::operator=(const InetAddress &oth) noexcept {
    if (this != &oth) {
        addr_ = oth.addr_;
    }
    return *this;
}

::sa_family_t clia::net::InetAddress::family() const noexcept {
    return addr_.addr.sa_family;
}

int clia::net::InetAddress::get_ipaddr(char *buf, const std::size_t sz) const noexcept {
    const char *p = nullptr;
    switch (this->family()) {
    case AF_INET:
        assert(sz > INET_ADDRSTRLEN);
        p = ::inet_ntop(AF_INET, &addr_.ipv4.sin_addr, buf, static_cast<::socklen_t>(sz));
        break;
    case AF_INET6:
        assert(sz > INET6_ADDRSTRLEN);
        p = ::inet_ntop(AF_INET6, &addr_.ipv6.sin6_addr, buf, static_cast<::socklen_t>(sz));
    default:
        return -1;
    }
    return (p != nullptr ? 0 : -1);
}

const ::sockaddr* clia::net::InetAddress::get_sockaddr() const noexcept {
    return &addr_.addr;
}