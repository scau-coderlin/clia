#pragma once

#include <cstddef>
#include <ctime>

namespace clia {
    namespace util {
        namespace time {
            /// @brief 获取当前时间并格式化输出
            /// @param outbuf      用户提供的缓冲区
            /// @param size        缓冲区大小
            /// @param t           时间戳，默认为当前时间
            /// @param format      时间格式，默认是"%Y-%m-%d %H:%M:%S", 必须为strftime支持的格式
            /// @return std::size_t 返回格式化后的时间字符串长度
            extern const char* get_time_str(char* outbuf, size_t size, const std::time_t t = std::time(nullptr), const char *format = "%Y-%m-%d %H:%M:%S") noexcept;
        }
    }
}