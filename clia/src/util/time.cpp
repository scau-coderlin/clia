#include <ctime>

#include <sys/time.h>

#include "clia/util/time.hpp"

const char* clia::util::time::get_time_str(char* outbuf, size_t size, const std::time_t t, const char *format) noexcept {
    // 将秒转换为本地时间（线程安全）
    struct tm tm_info;
    localtime_r(&t, &tm_info);

    // 格式化时间到缓冲区（不含毫秒）
    return std::strftime(outbuf, size, format, &tm_info) == 0 ? nullptr : outbuf;
}