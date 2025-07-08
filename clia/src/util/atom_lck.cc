#include "clia/util/atom_lck.h"

clia::util::AtomLck::AtomLck() noexcept {
    lck_.clear();
}

clia::util::AtomLck::~AtomLck() noexcept = default;

void clia::util::AtomLck::lock() noexcept {
    while (std::atomic_flag_test_and_set_explicit(&lck_, std::memory_order_acquire)) {
        ;
    }
}
void clia::util::AtomLck::unlock() noexcept {
    std::atomic_flag_clear_explicit(&lck_, std::memory_order_release);
}