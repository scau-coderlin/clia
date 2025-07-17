#include <cstdio>

#include "clia/log/stdout_appender.h"

clia::log::StdoutAppender::StdoutAppender() noexcept {

}

clia::log::StdoutAppender::~StdoutAppender() noexcept {

}

void clia::log::StdoutAppender::append(const void *buf, const std::size_t size) noexcept {
    std::fwrite(buf, 1, size, stdout);
}

void clia::log::StdoutAppender::flush() noexcept {
    std::fflush(stdout);
}