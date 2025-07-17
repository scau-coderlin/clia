#include <cassert>

#include <unistd.h>

#include "clia/log.h"
#include "clia/net/socket.h"
#include "clia/net/tcp_connection.h"
#include "clia/reactor/event_loop.h"
#include "clia/util/process.h"

clia::net::TcpConnection::TcpConnection(clia::reactor::EventLoop *loop, const int sockfd, const InetAddress &peer_addr) 
    : loop_(loop)
    , fd_(sockfd)
    , state_(State::kConnecting)
    , reading_(true)
    , socket_(sockfd)
    , channel_(loop, sockfd)
    , peer_addr_(peer_addr)
{
    channel_.set_read_callback(std::bind(&TcpConnection::handle_read, this, std::placeholders::_1));
    channel_.set_write_callback(std::bind(&TcpConnection::handle_write, this));
    channel_.set_close_callback(std::bind(&TcpConnection::handle_close, this));
    channel_.set_error_callback(std::bind(&TcpConnection::handle_error, this));
    socket_.set_keeyalive(true);
}

clia::net::TcpConnection::~TcpConnection() {
    assert(State::kDisconnected == state_);
}

clia::reactor::EventLoop* clia::net::TcpConnection::get_loop() const noexcept {
    return loop_;
}

const clia::net::InetAddress& clia::net::TcpConnection::peer_addr() const noexcept {
    return peer_addr_;
}

int clia::net::TcpConnection::fd() const noexcept {
    return fd_;
}

bool clia::net::TcpConnection::connected() const noexcept {
    return State::kConnected == state_;
}

void clia::net::TcpConnection::send(const void *buf, const std::size_t len) {
    if (State::kConnected == state_) {
        if (loop_->is_in_loop_thread()) {
            this->send_in_loop(buf, len);
        } else {
            loop_->run_in_loop(std::bind(&TcpConnection::send_in_loop, this, buf, len));
        }
    }
}

void clia::net::TcpConnection::shutdown() {
    if (State::kConnected == state_) {
        this->set_state(State::kDisconnecting);
        loop_->run_in_loop(std::bind(&TcpConnection::shutdown_in_loop, this));
    }
}

void clia::net::TcpConnection::set_connection_callback(const ConnectionCallback &cb) {
    connection_callback_ = cb;
}

void clia::net::TcpConnection::set_message_callback(const MessageCallback &cb) {
    message_callback_ = cb;
}

void clia::net::TcpConnection::set_write_complete_callback(const WriteCompleteCallback &cb) {
    write_complete_callback_ = cb;
}

void clia::net::TcpConnection::set_close_callback(const CloseCallback &cb) {
    close_callback_ = cb;
}

// 连接建立
void clia::net::TcpConnection::connect_established() {
    assert(loop_->is_in_loop_thread());
    assert(State::kConnecting == state_);
    this->set_state(State::kConnected);
    channel_.tie(this->shared_from_this());
    channel_.enable_reading();
    if (connection_callback_) {
        connection_callback_(this->shared_from_this());
    }
}

// 连接销毁
void clia::net::TcpConnection::connect_destoryed() {
    CLIA_LOG_DEBUG << "TcpConnection::connect_destoryed [" << this->peer_addr().get_addr();
    assert(loop_->is_in_loop_thread());
    if (State::kConnected == state_) {
        this->set_state(State::kDisconnected);
        channel_.disable_all();
        if (connection_callback_) {
            connection_callback_(this->shared_from_this());
        }
    }
    CLIA_LOG_DEBUG << "TcpConnection::connect_destoryed [" << this->peer_addr().get_addr();
    channel_.remove();
    CLIA_LOG_DEBUG << "TcpConnection::connect_destoryed [" << this->peer_addr().get_addr();
}

void clia::net::TcpConnection::set_state(const State state) {
    state_ = state;
}

void clia::net::TcpConnection::handle_read(clia::util::Timestamp recvive_time) {
    assert(loop_->is_in_loop_thread());
    const auto n = input_buffer_.read_fd(channel_.fd());
    if (n > 0) {
        if (message_callback_) {
            message_callback_(this->shared_from_this(), &input_buffer_, recvive_time);
        }
    } else if (0 == n) {
        this->handle_close();
    } else {
        CLIA_FMT_LOG_ERROR("read err, errno = [%d][%s]", errno, clia::util::process::strerror(errno));
        this->handle_error();
    }
}

void clia::net::TcpConnection::handle_write() {
    assert(loop_->is_in_loop_thread());
    if (channel_.is_writing()) {
        const auto n = output_buffer_.write_fd(channel_.fd());
        if (n > 0) {
            if (output_buffer_.readable_bytes() == 0) {
                channel_.disable_writing();
                if (write_complete_callback_) {
                    loop_->queue_in_loop(std::bind(write_complete_callback_, this->shared_from_this()));
                }
                if (State::kDisconnecting == state_) {
                    this->shutdown_in_loop();
                }
            }
        } else {
            CLIA_FMT_LOG_ERROR("TcpConnection::handleWrite, errno = [%d][%s]", errno, clia::util::process::strerror(errno));
        }
    }
}

void clia::net::TcpConnection::handle_close() {
    assert(loop_->is_in_loop_thread());
    assert(State::kConnected == state_ || State::kDisconnecting == state_);
    this->set_state(State::kDisconnected);
    channel_.disable_all();
    TcpConnectionPtr guard(this->shared_from_this());
    if (connection_callback_) {
        connection_callback_(guard);
    }
    if (close_callback_) {
        close_callback_(guard);
    }
}

void clia::net::TcpConnection::handle_error() {
    int optval = 0;
    ::socklen_t optlen = static_cast<::socklen_t>(sizeof(optval));
    int err = 0;
    if (::getsockopt(channel_.fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        err = errno;
    } else {
        err = optval;
    }
    CLIA_LOG_ERROR << "TcpConnection::handleError - SO_ERROR = " << err << " " << clia::util::process::strerror(err);
}

void clia::net::TcpConnection::send_in_loop(const void *data, const std::size_t len) {
    assert(loop_->is_in_loop_thread());
    if (State::kDisconnected == state_) {
        CLIA_LOG_WARN << "disconnected, give up writing";
        return;
    }

    ::ssize_t nwrote = 0;
    ::ssize_t remaining = len;

    bool fault_error = false;
    if (!channel_.is_writing() && output_buffer_.readable_bytes() == 0) {
        nwrote = ::write(channel_.fd(), static_cast<const unsigned char*>(data) + nwrote, remaining);
        if (nwrote >= 0) {
            remaining -= nwrote;
            if (0 == remaining && write_complete_callback_) {
                loop_->queue_in_loop(std::bind(write_complete_callback_, this->shared_from_this()));
            }
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                CLIA_FMT_LOG_ERROR("TcpConnection::sendInLoop, errno = [%d][%s]", errno, clia::util::process::strerror(errno));
                if (EPIPE == errno || ECONNRESET) {
                    fault_error = true;
                }
            }
        }
    }
    assert(remaining <= len);
    if (!fault_error && remaining > 0) {
        output_buffer_.append(static_cast<const unsigned char*>(data) + nwrote, remaining);
        if (!channel_.is_writing()) {
            channel_.enable_writing();
        }
    }
}

void clia::net::TcpConnection::shutdown_in_loop() {
    assert(loop_->is_in_loop_thread());
    if (!channel_.is_writing()) {
        socket_.shutdown_write();
    }
}