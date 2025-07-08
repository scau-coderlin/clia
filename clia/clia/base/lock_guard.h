#ifndef CLIA_BASE_LOCK_GUARD_H_
#define CLIA_BASE_LOCK_GUARD_H_

#include "clia/base/noncopyable.h"

namespace clia {
    template <typename Lock>
    class LockGuard : Noncopyable {
    public:
        LockGuard(Lock &lck) noexcept
            : lck_(lck) 
        {
            lck_.lock();
        }
        ~LockGuard() {
            lck_.unlock();
        }
    private:
        Lock &lck_;
    };
}

#endif