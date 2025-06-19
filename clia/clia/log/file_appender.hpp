#pragma once

#include <cstddef>
#include <cstdio>
#include <ctime>
#include <mutex>
#include <string>
#include <memory>

#include "clia/base/nocopyable.hpp"
#include "clia/log/trait.hpp"

namespace clia {
    namespace log {
        class FileAppender : public AppenderTrait, Noncopyable {
        public:
        FileAppender(
            const char *path, 
            const char *logname, 
            const std::size_t roll_size_bytes = 10 * 1024 * 1024, // 默认1MB
            const int roll_period_days = 1,
            const int retain_period_day = 6 * 30, // 保留周期6个月
            const int check_every = 1024, // 每1024次检查一次是否需要滚动
            const bool thread_safe = true
        );
        ~FileAppender();
    public:
        void append(const void *buf, const std::size_t size) noexcept override;
        void flush() noexcept override;
    private:
        void roll_file() noexcept;
        static std::string get_logfilename(const std::string &path, const std::string &logname) noexcept;
        void append_unlocked(const void *buf, const std::size_t size) noexcept;
        void del_old_files() noexcept; 
    private:
        const std::string m_path;
        const std::string m_logname;
        const std::size_t m_roll_size;
        const std::time_t m_roll_period_sec;
        const int m_retain_period_sec;
        const int m_check_every;
        int m_count;
        std::time_t m_last_roll_time;
        std::time_t m_start_roll_period;
        std::unique_ptr<std::mutex> m_mutex;
        class AppendFile;
        std::unique_ptr<AppendFile> m_file;
        };
    }
}