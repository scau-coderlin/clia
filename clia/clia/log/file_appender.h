#ifndef CLIA_LOG_FILE_APPENDER_H_
#define CLIA_LOG_FILE_APPENDER_H_

#include <cstddef>
#include <ctime>
#include <fstream>
#include <string>

#include "clia/log/trait.h"

namespace clia {
    namespace log {
        class FileAppender : public trait::Appender {
        public:
            FileAppender(
                const char *path, 
                const char *logname, 
                const std::size_t roll_size_byte = 10 * 1024 * 1024, // 默认1MB
                const int roll_period_days = 1,
                const int retain_period_day = 6 * 30, // 保留周期6个月
                const int check_every = 1024, // 每1024次检查一次是否需要滚动
                const bool thread_safe = false
            );
            ~FileAppender();
        public:
            void append(const void *buf, const std::size_t size) noexcept override;
            void flush() noexcept override;
        private:
            void roll_file() noexcept;
            std::string get_logfilename() noexcept;
            void append_unlocked(const void *buf, const std::size_t size) noexcept;
            void del_old_files() noexcept; 
        private:
            const std::string path_;
            const std::string logname_;
            const std::size_t roll_size_byte_;
            const std::time_t roll_period_sec_;
            const int retain_period_sec_;
            const int check_every_;
            int count_;
            std::time_t last_roll_time_;
            std::time_t this_roll_period_;
            std::unique_ptr<std::mutex> lck_;
            std::size_t written_bytes_;
            std::ofstream file_;            
        };
    }
}

#endif