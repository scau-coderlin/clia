#include <cassert>

#include "clia/reactor/event_loop.h"
#include "clia/net/acceptor.h"
#include "clia/net/tcp_server.h"
#include "clia/net/tcp_connection.h"
#include "clia/log.h"

clia::net::TcpServer::TcpServer(clia::reactor::EventLoop *loop, const InetAddress &listen_addr, const bool reuse_port) 
    : loop_(loop)
    , acceptor_(new Acceptor(loop, listen_addr, reuse_port))
    , threadpool_(new clia::reactor::EventLoopThreadPool(loop))
    , next_conn_id_(1)
    , started_(0)
{
    assert(loop_ != nullptr);
    acceptor_->set_new_connection_callback(
        std::bind(&TcpServer::new_connection, this, std::placeholders::_1, std::placeholders::_2));
}

clia::net::TcpServer::~TcpServer() {
    assert(loop_->is_in_loop_thread());
    for (auto &it : connections_) {
        TcpConnectionPtr conn(it.second);
        it.second.reset();
        conn->get_loop()->run_in_loop(std::bind(&TcpConnection::connect_destoryed, conn));
    }
}

void clia::net::TcpServer::set_thread_init_callback(clia::reactor::EventLoopThread::ThreadInitCallBack &cb) {
    thread_init_callback_ = cb;
}

void clia::net::TcpServer::set_connection_callback(const ConnectionCallback &cb) {
    connection_callback_ = cb;
}

void clia::net::TcpServer::set_message_callback(const MessageCallback &cb) {
    message_callback_ = cb;
}

void clia::net::TcpServer::set_write_complete_callback(const WriteCompleteCallback &cb) {
    write_complete_callback_ = cb;
}

void clia::net::TcpServer::set_thread_num(const int num) {
    threadpool_->set_thread_num(num);
}

void clia::net::TcpServer::start() {
    if (started_++ == 0) {
        threadpool_->start(thread_init_callback_);
        assert(!acceptor_->listening());
        loop_->run_in_loop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}
void clia::net::TcpServer::new_connection(int sockfd, const InetAddress &peer_addr) {
    assert(loop_->is_in_loop_thread());

    CLIA_LOG_DEBUG << "TcpServer::newConnection from " << peer_addr.get_addr();

    clia::reactor::EventLoop *io_loop = threadpool_->get_next_loop();
    ++next_conn_id_;

    TcpConnectionPtr conn(new TcpConnection(io_loop, sockfd, peer_addr));
    assert(connections_.find(sockfd) == connections_.end());
    connections_[sockfd] = conn;
    if (connection_callback_) {
        conn->set_connection_callback(connection_callback_);
    }
    if (message_callback_) {
        conn->set_message_callback(message_callback_);
    }
    if (write_complete_callback_) {
        conn->set_write_complete_callback(write_complete_callback_);
    }
    conn->set_close_callback(std::bind(&TcpServer::remove_connection, this, std::placeholders::_1));
    io_loop->run_in_loop(std::bind(&TcpConnection::connect_established, conn));
}

void clia::net::TcpServer::remove_connection(const TcpConnectionPtr &conn) {
    CLIA_LOG_DEBUG << "TcpServer::remove_connection " << conn->peer_addr().get_addr();
    loop_->run_in_loop(std::bind(&TcpServer::remove_connection_in_loop, this, conn));
}

void clia::net::TcpServer::remove_connection_in_loop(const TcpConnectionPtr &conn) {
    assert(loop_->is_in_loop_thread());
    CLIA_LOG_DEBUG << "TcpServer::remove_connection_in_loop [" << conn->peer_addr().get_addr();
    assert(connections_.find(conn->fd()) != connections_.end() && connections_[conn->fd()].get() == conn.get());
    const auto n = connections_.erase(conn->fd());
    assert(1 == n);
    clia::reactor::EventLoop *io_loop = conn->get_loop();
    io_loop->queue_in_loop(std::bind(&TcpConnection::connect_destoryed, conn));
}