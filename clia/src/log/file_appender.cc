#include <cstddef>
#include <cstdio>
#include <mutex>
#include <cassert>
#include <ctime>
#include <cstring>

#include <string>
#include <vector>

#include <unistd.h> // access
#include <sys/stat.h> // stat
#include <dirent.h>

#include "clia/log/file_appender.h"
#include "clia/util/process.h"

class clia::log::FileAppender::Impl {
    class AppendFile;
public:
    Impl(
        const char *path, 
        const char *logname, 
        const std::size_t roll_size_byte,   // 默认1MB
        const int roll_period_days,
        const int retain_period_day,        // 保留周期6个月
        const int check_every,              // 每1024次检查一次是否需要滚动
        const bool thread_safe
    );
    ~Impl();
public:
    void append(const void *buf, const std::size_t size) noexcept;
    void flush() noexcept;
private:
    std::size_t get_date_prefix(char *outbuf, const std::size_t size, const std::time_t now);
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
    std::unique_ptr<AppendFile> file_;
};

clia::log::FileAppender::FileAppender(
    const char *path, 
    const char *logname, 
    const std::size_t roll_size_bytes, // 默认1MB
    const int roll_period_days,
    const int retain_period_day, 
    const int check_every, // 每1024次检查一次是否需要滚动
    const bool thread_safe 
)   : impl_(new Impl(path, logname, roll_size_bytes, roll_period_days, retain_period_day, check_every, thread_safe))
{
   ;
}

clia::log::FileAppender::~FileAppender() {
    ;
}

void clia::log::FileAppender::append(const void *buf, const std::size_t size) noexcept {
    return impl_->append(buf, size);
}

void clia::log::FileAppender::flush() noexcept {
    return impl_->flush();
}

class clia::log::FileAppender::Impl::AppendFile final {
public:
    explicit AppendFile(const char *filename) noexcept;
    ~AppendFile() noexcept;
    void append(const void *buf, const std::size_t size) noexcept;
public:
    void flush() noexcept;
    std::size_t written_bytes() const noexcept;
private:
    std::FILE* fp_ = nullptr;
    std::size_t written_bytes_ = 0;
    char buffer_[64 * 1024];
};

clia::log::FileAppender::Impl::Impl(
    const char *path, 
    const char *logname, 
    const std::size_t roll_size_byte,   // 默认1MB
    const int roll_period_days,
    const int retain_period_day,        // 保留周期6个月
    const int check_every,              // 每1024次检查一次是否需要滚动
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
{
    if (::access(path_.c_str(), F_OK) != 0) {
        ::mkdir(path_.c_str(), 0755);
    }
    roll_file();
}

clia::log::FileAppender::Impl::~Impl() {

}

void clia::log::FileAppender::Impl::append(const void *buf, const std::size_t size) noexcept {
    if (!buf || size <= 0) {
        return;
    }
    if (lck_) {
        std::lock_guard<std::mutex> lock(*lck_);
        append_unlocked(buf, size);
    } else {
        append_unlocked(buf, size);
    }
}

void clia::log::FileAppender::Impl::flush() noexcept {
    if (lck_) {
        std::lock_guard<std::mutex> lock(*lck_);
        file_->flush();
    } else {
        file_->flush();
    }
}

std::size_t clia::log::FileAppender::Impl::get_date_prefix(char *outbuf, const std::size_t size, const std::time_t t) {
    struct tm tm_info;
    localtime_r(&t, &tm_info);
    // 格式化时间到缓冲区（不含毫秒）
    return std::strftime(outbuf, size, "%Y%m%d%H%M%S", &tm_info);
}

void clia::log::FileAppender::Impl::roll_file() noexcept {
    const std::time_t now = std::time(nullptr);
    if (last_roll_time_ != now) {
        const auto filename = get_logfilename();
        last_roll_time_ = now;
        this_roll_period_ = now / roll_period_sec_ * roll_period_sec_;
        file_.reset(new AppendFile(filename.c_str()));
        del_old_files();
    }
}

std::string clia::log::FileAppender::Impl::get_logfilename() noexcept {
    std::string filename;
    filename.reserve(path_.size() + logname_.size() + 64);

    const auto t = std::time(nullptr);
    char timebuf[32] = {0};
    get_date_prefix(timebuf, sizeof(timebuf), t);
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

void clia::log::FileAppender::Impl::append_unlocked(const void *buf, const std::size_t size) noexcept {
    file_->append(buf, size);

    if (file_->written_bytes() >= roll_size_byte_) {
        roll_file();
    } else {
        ++count_;
        if (count_ >= check_every_) {
            count_ = 0;
            const time_t this_period = std::time(nullptr) / roll_period_sec_ * roll_period_sec_;
            if (this_period != this_roll_period_) {
                roll_file();
            } 
        }
    }
}

void clia::log::FileAppender::Impl::del_old_files() noexcept {
    // 删除过期的日志文件
    const std::time_t old = std::time(nullptr) - retain_period_sec_;
    char timebuf[32] = {0};
    const auto timebuflen = get_date_prefix(timebuf, sizeof(timebuf), old);
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


clia::log::FileAppender::Impl::AppendFile::AppendFile(const char *filename) noexcept
    : fp_(std::fopen(filename, "ae")) // 打开文件以追加模式
{
    assert(fp_ != nullptr); // 确保文件成功打开
    ::setbuffer(fp_, buffer_, sizeof(buffer_));
}

clia::log::FileAppender::Impl::AppendFile::~AppendFile() noexcept {
    std::fflush(fp_);
    ::fsync(::fileno(fp_));
    std::fclose(fp_);
    fp_ = nullptr;
}

void clia::log::FileAppender::Impl::AppendFile::append(const void *buf, const std::size_t size) noexcept {
    std::size_t written = 0;
    while (written != size) {
        const size_t remain = size - written;
        const auto n = ::fwrite_unlocked(static_cast<const unsigned char*>(buf) + written, 1, remain, fp_);
        if (n != remain) {
            const int err = std::ferror(fp_);
            if (err) {
                fprintf(stderr, "AppendFile::append() failed %s\n", clia::util::process::strerror(err));
                break;
            }
        }
        written += n;
    } 
    written_bytes_ += written;
}

void clia::log::FileAppender::Impl::AppendFile::flush() noexcept {
    std::fflush(fp_);
}

std::size_t clia::log::FileAppender::Impl::AppendFile::written_bytes() const noexcept { 
    return written_bytes_; 
}