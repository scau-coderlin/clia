#include <cstring>

#include <unistd.h>

#include <sys/syscall.h>  // syscall

#include "util/process.hpp"

thread_local const int s_tid = static_cast<int>(syscall(SYS_gettid));
static const int s_pid = static_cast<int>(::getpid());

thread_local int t_errno = 0; // 线程局部存储的错误码
thread_local char t_errnobuf[512];

const char* clia::util::process::strerror(int ec) noexcept {
    if (ec != t_errno) {
        t_errno = ec; // 更新线程局部存储的错误码
        strerror_r(t_errno, t_errnobuf, sizeof(t_errnobuf));
    }
    return t_errnobuf;
}

int clia::util::process::get_pid() noexcept {
    return s_pid;
}

int clia::util::process::get_tid() noexcept {
    return s_tid;
}