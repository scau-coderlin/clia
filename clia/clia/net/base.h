#ifndef CLIA_NET_BASE_H_
#define CLIA_NET_BASE_H_

#include <memory>
#include <functional>

#include "clia/util/timestamp.h"

namespace clia {
    namespace net {
        class Buffer;
        class TcpConnection;

        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
        using ConnectionCallback = std::function<void (const TcpConnectionPtr&)>;
        using CloseCallback = std::function<void (const TcpConnectionPtr&)>;
        using WriteCompleteCallback = std::function<void (const TcpConnectionPtr&)>; 
        using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer*, clia::util::Timestamp)>;
    }
}

#endif