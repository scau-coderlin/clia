#ifndef CLIA_NET_TRAIT_H_
#define CLIA_NET_TRAIT_H_

#include "clia/base/noncopyable.h"

namespace clia {
    namespace net {
        namespace trait {
            class Socket : Noncopyable {
            public:
                Socket() noexcept = default;
                virtual ~Socket() noexcept = default;
            };
        }
    }
}

#endif