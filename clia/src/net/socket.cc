#include <asm-generic/socket.h>
#include <cassert>
#include <cerrno>
#include <cstdlib>

#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

#include "clia/log.h"
#include "clia/util/process.h"
#include "clia/net/socket.h"
#include "clia/net/inet_address.h"

clia::net::Socket::Socket(const int sockfd) noexcept
    : sockfd_(sockfd) 
{
    assert(sockfd_ != -1);
}

clia::net::Socket::~Socket() noexcept {
    ::close(sockfd_);    
}

int clia::net::Socket::fd() const noexcept {
    return sockfd_;
}

void clia::net::Socket::bind_address(const InetAddress &local_addr) noexcept {
    int ret = 0;
    if (local_addr.family() == AF_INET) {
        ret = ::bind(sockfd_,local_addr.get_sockaddr(), static_cast<::socklen_t>(sizeof(::sockaddr_in)));
    } else if (local_addr.family() == AF_INET6) {
        ret = ::bind(sockfd_,local_addr.get_sockaddr(), static_cast<::socklen_t>(sizeof(::sockaddr_in6)));
    } else {
        CLIA_LOG_FATAL << "bind family err";
        std::abort();
    }

    if (ret < 0) {
        CLIA_FMT_LOG_FATAL("bind fail, errno = [%d][%s]", errno, clia::util::process::strerror(errno));
        std::abort();
    }
}

void clia::net::Socket::listen() noexcept {
    if (::listen(sockfd_, SOMAXCONN) < 0) {
        CLIA_FMT_LOG_FATAL("listen fail, errno = [%d][%s]", errno, clia::util::process::strerror(errno));
        std::abort();
    }
}

int clia::net::Socket::accept(InetAddress *peer_addr) noexcept {
    ::sockaddr_in6 addr;
    std::memset(&addr, 0, sizeof(addr));
    ::socklen_t addrlen = sizeof(addr);
    const int connfd = ::accept4(sockfd_, reinterpret_cast<::sockaddr*>(&addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0) {
        switch (errno) {
        case EAGAIN:
        case ECONNABORTED:
        case EINTR:
        case EPROTO: 
        case EPERM:
        case EMFILE: 
            CLIA_FMT_LOG_ERROR("accept fail, errno = [%d][%s]", errno, clia::util::process::strerror(errno));
            break;
        case EBADF:
        case EFAULT:
        case EINVAL:
        case ENFILE:
        case ENOBUFS:
        case ENOMEM:
        case ENOTSOCK:
        case EOPNOTSUPP:
            CLIA_FMT_LOG_FATAL("unexpected error of ::accept, errno = [%d][%s]", errno, clia::util::process::strerror(errno));
            break;
        default:
            CLIA_FMT_LOG_FATAL("accept fail, errno = [%d][%s]", errno, clia::util::process::strerror(errno));
            std::abort();
        }
    } else {
        if (peer_addr) {
            *peer_addr = InetAddress(addr);
        }
    }
    return connfd;
}

void clia::net::Socket::shutdown_write() noexcept {
    if (::shutdown(sockfd_, SHUT_WR) < 0) {
        CLIA_FMT_LOG_ERROR("shutdown fail, errno = [%d][%s]", errno, clia::util::process::strerror(errno));
    }
}

/*
功能：
    启用或禁用 Nagle 算法（TCP_NODELAY）。
作用：
    当开启 (on == true) 时，禁用 Nagle 算法，允许小数据包立即发送而不等待累积。
    默认情况下，TCP 会缓冲小的数据包，以减少网络中的小包数量，提高效率，但会增加延迟。
    设置 TCP_NODELAY=1 可以降低通信延迟，适用于需要快速响应的应用。
场景：
    实时通信（如游戏、聊天应用）
    RPC 框架
    需要低延迟的网络服务
*/
void clia::net::Socket::set_tcp_nodelay(const bool on) noexcept {
    const int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<::socklen_t>(sizeof(optval)));
}

/*
功能：
    启用或禁用地址复用（SO_REUSEADDR）。
作用：
    允许绑定到一个处于 TIME_WAIT 状态的端口。
    在服务器重启后可以立即重新绑定端口，而不是等待一段时间释放连接状态。
场景：
    开发调试阶段频繁重启的服务
    多进程或多线程监听同一个端口（结合 SO_REUSEPORT 更好）
*/
void clia::net::Socket::set_reuse_addr(const bool on) noexcept {
    const int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<::socklen_t>(sizeof(optval)));
}

/*
功能：
    启用或禁用端口复用（SO_REUSEPORT）。
作用：
    允许多个 socket 绑定到同一个 IP 和端口，内核负责负载均衡。
    提高并发性能，避免单线程瓶颈。
场景：
    高并发服务器（例如 Web 服务器）
    使用多进程或多线程处理请求的架构中
    Linux 特有功能（从 kernel 3.9 开始支持）
*/
void clia::net::Socket::set_reuse_port(const bool on) noexcept {
    const int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<::socklen_t>(sizeof(optval)));
}

/*
功能：
启用或禁用 TCP 保活机制（SO_KEEPALIVE）。
作用：
    如果连接在一段时间内没有数据交换，TCP 将自动发送探测包来检测对方是否仍然在线。
    防止连接“假死”状态，及时发现断开的连接。
场景：
    长时间保持连接的服务（如数据库连接池、聊天室）
    客户端可能异常退出而无法通知服务端的情况
    对连接可靠性要求高的系统
*/
void clia::net::Socket::set_keeyalive(const bool on) noexcept {
    const int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<::socklen_t>(sizeof(optval)));
}