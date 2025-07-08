#ifndef CLIA_UTIL_SPIN_LCK_H_
#define CLIA_UTIL_SPIN_LCK_H_

#include <atomic>

#include "clia/base/lock_guard.h"
#include "clia/base/noncopyable.h"

namespace clia {
    namespace util {
        class AtomLck final : public Noncopyable {
            friend clia::LockGuard<AtomLck>;
        public:
            AtomLck() noexcept;
            ~AtomLck() noexcept;
        private:
            void lock() noexcept;
            void unlock() noexcept;
        private:
            std::atomic_flag lck_;
        };
    }
}

#endif