#pragma once

namespace clia {
    namespace util {
        namespace process {
            /// @brief 获取当前进程的ID
            /// @return 返回当前进程的ID
            extern int get_pid() noexcept;

            /// @brief 获取当前线程的ID
            /// @return 返回当前线程的ID
            extern int get_tid() noexcept;
        }
    }
}