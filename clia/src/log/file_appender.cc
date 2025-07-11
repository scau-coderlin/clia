#include <cstddef>
#include <cstdio>
#include <cassert>
#include <ctime>
#include <cstring>


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
    , file_(nullptr)
{
    if (::access(path_.c_str(), F_OK) != 0) {
        ::mkdir(path_.c_str(), 0755);
    }
    buffer_.resize(kBufferSize);
    this->roll_file();
}

clia::log::FileAppender::~FileAppender() {
    if (file_ != nullptr && file_ != stdout && file_ != stderr) {
        std::fflush(file_);
        ::fsync(::fileno(file_));
        std::fclose(file_);
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
    std::fflush(file_);
}

void clia::log::FileAppender::roll_file() noexcept {
    const std::time_t now = std::time(nullptr);
    if (last_roll_time_ != now) { 
        const auto filename = this->get_logfilename();
        last_roll_time_ = now;
        this_roll_period_ = now / roll_period_sec_ * roll_period_sec_;
        if (file_ != nullptr && file_ != stdout && file_ != stderr) {
            std::fflush(file_);
            ::fsync(::fileno(file_));
            std::fclose(file_);
        }
        file_ = std::fopen(filename.c_str(), "a");
        if (file_ != nullptr) {
            if (std::setvbuf(file_, buffer_.data(), _IOFBF, buffer_.size()) != 0) {
                std::fprintf(stderr, "setvbuf buf err, errno = [%d][%s]\n", errno, clia::util::process::strerror(errno));
            }
        } else {
            std::fprintf(stderr, "fopen log [%s] err, errno = [%d][%s]\n", filename.c_str(), errno, clia::util::process::strerror(errno));
#ifdef NDEBUG
            file_ = stdout;
#else
            file_ = stderr;
#endif
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
    if (written_bytes_ > roll_size_byte_) {
        this->roll_file();
    } else {
        ++count_;
        if (count_ >= check_every_) {
            count_ = 0;
            const std::time_t this_period = std::time(nullptr) / roll_period_sec_ * roll_period_sec_;
            if (this_period != this_roll_period_) {
                roll_file();
            }
        }
    }

    std::size_t written = 0;
    while (written != size) {
        const std::size_t remain = size - written;
        const auto n = ::fwrite_unlocked(static_cast<const unsigned char*>(buf) + written, 1, remain, file_);
        if (n != remain) {
            const auto err = std::ferror(file_);
            if (err) {
                fprintf(stderr, "clia::log::FileAppender::append_unlocked failed %s\n", clia::util::process::strerror(err));
                break;
            }
        }        
        written += n;
    }
    written_bytes_ += written;
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