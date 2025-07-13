#include <cassert>
#include <cstdlib>

#include <unistd.h>
#include <sys/socket.h>

#include "clia/net/acceptor.h"
#include "clia/log.h"
#include "clia/net/inet_address.h"
#include "clia/net/socket.h"
#include "clia/reactor/channel.h"
#include "clia/util/process.h"
#include "clia/reactor/event_loop.h"

clia::net::Acceptor::Acceptor(clia::reactor::EventLoop *loop, const InetAddress &listen_addr, const bool reuseport) 
    : loop_(loop)
    , listening_(false)
{
    const int listenfd = ::socket(listen_addr.family(), SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (-1 == listenfd) {
        CLIA_FMT_LOG_FATAL("socket fail, errno = [%d][%s]", errno, clia::util::process::strerror((errno)));
        std::abort();
    }
    
    accept_socket_.reset(new Socket(listenfd));
    accept_socket_->set_reuse_addr(true);
    accept_socket_->set_reuse_port(reuseport);
    accept_socket_->bind_address(listen_addr);

    accept_channel_.reset(new clia::reactor::Channel(loop_, accept_socket_->fd()));
    accept_channel_->set_read_callback(std::bind(&Acceptor::handle_read, this));
}

clia::net::Acceptor::~Acceptor() noexcept {
    accept_channel_->disable_all();
    accept_channel_->remove();
}

void clia::net::Acceptor::set_new_connection_callback(const NewConnectionCallback &cb) {
    new_connection_callback_ = cb;
}

void clia::net::Acceptor::listen() noexcept {
    assert(loop_->is_in_loop_thread());
    listening_ = true;
    accept_socket_->listen();
    accept_channel_->enable_reading();
}

bool clia::net::Acceptor::listening() const noexcept {
    return listening_;
}

void clia::net::Acceptor::handle_read() {
    assert(loop_->is_in_loop_thread());
    InetAddress peer_addr;
    const int connfd = accept_socket_->accept(&peer_addr);
    if (connfd >= 0) {
        if (new_connection_callback_) {
            new_connection_callback_(connfd, peer_addr);
        } else {
            ::close(connfd);
        }
    } else {
        CLIA_FMT_LOG_ERROR("accept err: errno = [%d][%s]", errno, clia::util::process::strerror((errno)));
    }
}