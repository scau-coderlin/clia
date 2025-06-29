#include <cstring>

#include <unistd.h>

#include <sys/syscall.h>  // syscall

#include "clia/util/process.h"

thread_local const int kThreadId = static_cast<int>(syscall(SYS_gettid));
static const int kProcessId = static_cast<int>(::getpid());

thread_local int kErrno = 0; // 线程局部存储的错误码
thread_local char kErrnoStrBuf[512] = {0};

const char* clia::util::process::strerror(int ec) noexcept {
    if (ec != kErrno) {
        kErrno = ec; // 更新线程局部存储的错误码
        ::strerror_r(kErrno, kErrnoStrBuf, sizeof(kErrnoStrBuf));
    }
    return kErrnoStrBuf;
}

int clia::util::process::get_pid() noexcept {
    return kProcessId;
}

int clia::util::process::get_tid() noexcept {
    return kThreadId;
}