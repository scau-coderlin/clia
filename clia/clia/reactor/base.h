#ifndef CLIA_REACTOR_BASE_H_
#define CLIA_REACTOR_BASE_H_

#include <functional>
#include <map>
#include <vector>

#include "clia/util/timestamp.h"

namespace clia {
    namespace reactor {
        class EventLoop;
        class Channel;
        class Epoller;

        using ChannelList = std::vector<Channel*>;
        using ChannelMap = std::map<int, Channel*>;
        using Functor = std::function<void()>;
        using EventCallback = std::function<void()>;
        using ReadEventCallback = std::function<void(clia::util::Timestamp)>;
    }
}

#endif