#include <cassert>
#include <cstring>

#include <endian.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "clia/log.h"
#include "clia/util/process.h"
#include "clia/net/inet_address.h"

clia::net::InetAddress::InetAddress() noexcept {
    std::memset(&addr6_, 0, sizeof(addr6_));
}

clia::net::InetAddress::InetAddress(const char *ip, std::uint16_t port, const bool is_ipv6) noexcept {
    if (is_ipv6 || std::strchr(ip, ':')) {
        std::memset(&addr6_, 0, sizeof(addr6_));
        addr6_.sin6_family = AF_INET6;
        addr6_.sin6_port = htobe16(port);
        if (::inet_pton(AF_INET6, ip, &addr6_.sin6_addr) <= 0) {
            CLIA_FMT_LOG_ERROR("inet_pton fail, errno = [%d][%s]", errno, clia::util::process::strerror(errno));
        }
    } else {
        std::memset(&addr_, 0, sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_port = htobe16(port);
        if (::inet_pton(AF_INET, ip, &addr_.sin_addr) <= 0) {
            CLIA_FMT_LOG_ERROR("inet_pton fail, errno = [%d][%s]", errno, clia::util::process::strerror(errno));
        }
    }
}

clia::net::InetAddress::~InetAddress() noexcept {

}

clia::net::InetAddress::InetAddress(const ::sockaddr_in &addr) noexcept {
    addr_ = addr;
}

clia::net::InetAddress::InetAddress(const ::sockaddr_in6 &addr) noexcept {
    addr6_ = addr;
}

clia::net::InetAddress::InetAddress(const InetAddress &oth) noexcept = default;

clia::net::InetAddress& clia::net::InetAddress::operator=(const InetAddress &oth) noexcept = default;

::sa_family_t clia::net::InetAddress::family() const noexcept {
    return addr_.sin_family;
}

int clia::net::InetAddress::get_ipaddr(char *buf, const std::size_t sz) const noexcept {
    const char *p = nullptr;
    switch (this->family()) {
    case AF_INET:
        assert(sz > INET_ADDRSTRLEN);
        p = ::inet_ntop(AF_INET, &addr_.sin_addr, buf, static_cast<::socklen_t>(sz));
        break;
    case AF_INET6:
        assert(sz > INET6_ADDRSTRLEN);
        p = ::inet_ntop(AF_INET6, &addr6_.sin6_addr, buf, static_cast<::socklen_t>(sz));
    default:
        return -1;
    }
    return (p != nullptr ? 0 : -1);
}

const ::sockaddr* clia::net::InetAddress::get_sockaddr() const noexcept {
    return reinterpret_cast<const ::sockaddr*>(&addr6_);
}

std::string clia::net::InetAddress::get_addr() const noexcept {
    char buf[64] = {0};
    std::string address;
    address.reserve(64);
    if (this->family() == AF_INET6) {
        ::inet_ntop(AF_INET6, &addr6_.sin6_addr, buf, static_cast<::socklen_t>(sizeof(buf)));
        address += '[';
        address += buf;
        address += ']';
        address += ':';
        address += std::to_string(be16toh(addr6_.sin6_port));
    } else {
        ::inet_ntop(AF_INET, &addr_.sin_addr, buf, static_cast<::socklen_t>(sizeof(buf)));
        address += buf;
        address += ':';
        address = std::string(buf) + ':' + std::to_string(be16toh(addr_.sin_port));
    }
    return address;
}