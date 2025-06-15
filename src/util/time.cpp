#include <ctime>
#include <sys/time.h>

#include "util/time.hpp"

/// @brief 获取当前时间并格式化输出
/// @param buffer      用户提供的缓冲区
/// @param size        缓冲区大小
/// @param format      时间格式，默认是"%Y-%m-%d %H:%M:%S", 必须为strftime支持的格式
/// @return const char* 返回格式化后的时间字符串
const char* clia::util::time::get_current_time_str(char* outbuf, size_t size, const char *format) noexcept {
    // 获取当前时间（秒和微秒）
    const auto t = std::time(nullptr); 

    // 将秒转换为本地时间（线程安全）
    struct tm tm_info;
    localtime_r(&t, &tm_info);

    // 格式化时间到缓冲区（不含毫秒）
    return std::strftime(outbuf, size, format, &tm_info) == 0 ? nullptr : outbuf;
}