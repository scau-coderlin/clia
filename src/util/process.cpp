#include <unistd.h>

#include <sys/syscall.h>  // syscall

#include "util/process.hpp"

thread_local const int s_tid = static_cast<int>(syscall(SYS_gettid));
static const int s_pid = static_cast<int>(::getpid());

int clia::util::process::get_pid() noexcept {
    return s_pid;
}
int clia::util::process::get_tid() noexcept {
    return s_tid;
}