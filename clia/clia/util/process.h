#ifndef CLIA_UTIL_PROCESS_H_
#define CLIA_UTIL_PROCESS_H_

namespace clia {
    namespace util {
        namespace process {
            /// @brief 获取当前进程的ID
            /// @return 返回当前进程的ID
            extern int get_pid() noexcept;

            /// @brief 获取当前线程的ID
            /// @return 返回当前线程的ID
            extern int get_tid() noexcept;

            /// @brief 将errno转为字符串
            /// @param ec 错误码
            /// @return 返回错误码对应的字符串描述
            /// @note 该函数是线程安全的，且不会使用动态内存分配
            extern const char* strerror(const int ec) noexcept;
        }
    }
}

#endif