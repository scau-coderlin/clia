#include <iostream>

#include "clia/log.h"
#include "clia/net/tcp_server.h"
#include "clia/net/tcp_connection.h"
#include "clia/log/async_logger.h"
#include "clia/log/file_appender.h"
#include "clia/reactor/event_loop.h"
#include "clia/log/sync_logger.h"
#include "clia/log/stdout_appender.h"

class EchoServer {
public:
    EchoServer(clia::reactor::EventLoop *loop, const clia::net::InetAddress &addr)
        : loop_(loop)
        , server_(loop, addr)
    {
        server_.set_connection_callback(std::bind(&EchoServer::handle_connect, this, std::placeholders::_1));
        server_.set_message_callback(std::bind(&EchoServer::handle_message, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        server_.set_thread_num();
    }
    ~EchoServer() {
        ;
    }
    void start() {
        server_.start();
    }
private:
    void handle_connect(const clia::net::TcpConnectionPtr &conn) {
        if (conn->connected()) {
            CLIA_LOG_DEBUG << "Connection Up : " << conn->peer_addr().get_addr();
        } else {
            CLIA_LOG_DEBUG << "Connection Down : " << conn->peer_addr().get_addr();
        }
    }
    void handle_message(const clia::net::TcpConnectionPtr &conn, clia::net::Buffer *buf, clia::util::Timestamp time) {
        std::string msg = buf->retrieve_all_as_string();
        CLIA_LOG_DEBUG << "Recv from [" << conn->peer_addr().get_addr() << "]:: " << msg;
        conn->send(msg.c_str(), msg.size());
    }
private:
    clia::reactor::EventLoop *loop_;
    clia::net::TcpServer server_;
};

int main() {
#ifdef NDEBUG    
    std::shared_ptr<clia::log::trait::Appender> appender(new clia::log::FileAppender("./", "test_server"));
    std::shared_ptr<clia::log::trait::Logger> logger(new clia::log::AsyncLogger(clia::log::Level::kWarn, appender));
#else
    std::shared_ptr<clia::log::trait::Appender> appender(new clia::log::StdoutAppender);
    std::shared_ptr<clia::log::trait::Logger> logger(new clia::log::SyncLogger(clia::log::Level::kDebug, appender));
#endif
    clia::log::LoggerManger::instance()->set_default(logger);

    clia::reactor::EventLoop loop;
    clia::net::InetAddress addr("0.0.0.0", 1818);
    EchoServer server(&loop, addr);
    server.start();
    loop.loop();
    return 0;
}