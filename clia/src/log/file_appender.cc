#include <cstddef>
#include <cstdio>
#include <mutex>
#include <cassert>
#include <ctime>
#include <cstring>
#include <iostream>

#include <string>
#include <vector>

#include <unistd.h> // access
#include <sys/stat.h> // stat
#include <dirent.h>
#include <fcntl.h>

#include "clia/log/file_appender.h"
#include "clia/util/process.h"

namespace {
    static inline std::size_t get_date_prefix(char *outbuf, const std::size_t size, const std::time_t t) {
        struct tm tm_info;
        ::localtime_r(&t, &tm_info);
        // 格式化时间到缓冲区（不含毫秒）
        return std::strftime(outbuf, size, "%Y%m%d%H%M%S", &tm_info);
    }
}

clia::log::FileAppender::FileAppender(
    const char *path, 
    const char *logname, 
    const std::size_t roll_size_byte, // 默认1MB
    const int roll_period_days,
    const int retain_period_day, 
    const int check_every, // 每1024次检查一次是否需要滚动
    const bool thread_safe 
)   : path_(path)
    , logname_(logname)
    , roll_size_byte_(roll_size_byte)
    , roll_period_sec_(roll_period_days * 24 * 60 * 60)
    , retain_period_sec_(retain_period_day * 24 * 60 * 60) // 保留周期转换为秒
    , check_every_(check_every)
    , count_(0)
    , last_roll_time_(0)
    , this_roll_period_(0)
    , lck_(thread_safe ? new std::mutex : nullptr) 
    , written_bytes_(0)
{
    if (::access(path_.c_str(), F_OK) != 0) {
        ::mkdir(path_.c_str(), 0755);
    }
    this->roll_file();
}

clia::log::FileAppender::~FileAppender() {
    if (file_.is_open()) {
        file_.close();
    }
}

void clia::log::FileAppender::append(const void *buf, const std::size_t size) noexcept {
    if (!buf || size <= 0) {
        return;
    }
    if (lck_) {
        std::lock_guard<std::mutex> lock(*lck_);
        this->append_unlocked(buf, size);
    } else {
        this->append_unlocked(buf, size);
    }
}

void clia::log::FileAppender::flush() noexcept {
    file_.flush();
}

void clia::log::FileAppender::roll_file() noexcept {
    const std::time_t now = std::time(nullptr);
    if (last_roll_time_ != now) {
        bool check_period = false;
        if (written_bytes_ < roll_size_byte_) {
            if (count_ < check_every_) {
                return;
            } 
            count_ = 0;
            check_period = true;
        } 
        const std::time_t this_period = now / roll_period_sec_ * roll_period_sec_;
        if (check_period && this_period == this_roll_period_) {
            return;
        } 

        const auto filename = this->get_logfilename();
        last_roll_time_ = now;
        this_roll_period_ = this_period;
        if (file_) {
            file_.close();
        }
        file_ .open(filename, std::ios::app);
        if (!file_.is_open()) {
            std::fprintf(stderr, "fopen log [%s] err, errno = [%d][%s]\n", filename.c_str(), errno, clia::util::process::strerror(errno));
            std::abort();
        }
        written_bytes_ = 0;
        this->del_old_files();
    }
}

std::string clia::log::FileAppender::get_logfilename() noexcept {
    std::string filename;
    filename.reserve(path_.size() + logname_.size() + 64);

    const auto t = std::time(nullptr);
    char timebuf[32] = {0};
    ::get_date_prefix(timebuf, sizeof(timebuf), t);
    filename += path_;
    if (!filename.empty() && filename.back() != '/') {
        filename += '/';
    }
    filename += timebuf;
    filename += '.';

    char pidbuf[32];
    std::snprintf(pidbuf, sizeof pidbuf, "%d", clia::util::process::get_pid());
    filename += pidbuf;
    filename += '.';

    constexpr char kSuffix[] = ".log";
    filename += logname_;
    if (logname_.find(kSuffix) == std::string::npos) {
        filename += kSuffix;
    }

    return filename;
}

void clia::log::FileAppender::append_unlocked(const void *buf, const std::size_t size) noexcept {
    ++count_;
    this->roll_file();
    file_.write(static_cast<const char*>(buf), size);
    written_bytes_ += size;
}

void clia::log::FileAppender::del_old_files() noexcept {
    // 删除过期的日志文件
    const std::time_t old = std::time(nullptr) - retain_period_sec_;
    char timebuf[32] = {0};
    const auto timebuflen = ::get_date_prefix(timebuf, sizeof(timebuf), old);
    std::vector<std::string> old_files;
    ::DIR *dir = ::opendir(path_.c_str());
    if (!dir) {
        return;
    }
    ::dirent *entry;
    while ((entry = ::readdir(dir)) != nullptr) {
        if (entry->d_type != DT_REG) {
            continue;
        }
        if (std::strncmp(entry->d_name, timebuf, timebuflen) >= 0) {
            continue; // 跳过隐藏文件
        }
        old_files.emplace_back(path_ + "/" + entry->d_name);
    }
    ::closedir(dir);

    for (const auto &oldfile : old_files) {
        std::remove(oldfile.c_str());
    }
} 