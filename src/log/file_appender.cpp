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

#include "log/file_appender.hpp"
#include "util/process.hpp"
#include "util/time.hpp"

constexpr std::time_t ROLL_PER_SECONDS = 60 * 60 * 24; // 每天滚动一次

class clia::log::FileAppender::AppendFile final {
public:
    explicit AppendFile(const char *filename)
        : m_fp(std::fopen(filename, "ab")) // 打开文件以追加模式
    {
        assert(m_fp != nullptr); // 确保文件成功打开
        ::setbuffer(m_fp, m_buffer, sizeof(m_buffer));
    }
    ~AppendFile() {
        std::fflush(m_fp);
        ::fsync(::fileno(m_fp));
        std::fseek(m_fp, 0, SEEK_END); // 确保文件指针在文件末尾
        const auto fz = std::ftell(m_fp);
        std::fclose(m_fp);
        m_fp = nullptr;
    }
    void append(const void *buf, const std::size_t size) noexcept {
        std::size_t written = 0;
        while (written != size) {
            const size_t remain = size - written;
            const auto n = this->write(static_cast<const unsigned char*>(buf) + written, remain);
            if (n != remain) {
                const int err = std::ferror(m_fp);
                if (err) {
                    fprintf(stderr, "AppendFile::append() failed %s\n", clia::util::process::strerror(err));
                    break;
                }
            }
            written += n;
        } 
        m_written_bytes += written;
    }

    void flush() noexcept {
        std::fflush(m_fp);
    }

    std::size_t written_bytes() const noexcept { 
        return m_written_bytes; 
    }

private:
    size_t write(const void *buf, std::size_t len) noexcept {
        return ::fwrite_unlocked(buf, 1, len, m_fp);
    }
private:
    std::FILE* m_fp = nullptr;
    std::size_t m_written_bytes = 0;
    char m_buffer[64*1024];
};

clia::log::FileAppender::FileAppender(
    const char *path, 
    const char *logname, 
    const std::size_t roll_size_bytes, // 默认1MB
    const int roll_period_days,
    const int retain_period_day, 
    const int check_every, // 每1024次检查一次是否需要滚动
    const bool thread_safe 
)   : m_path(path)
    , m_logname(logname)
    , m_roll_size(roll_size_bytes)
    , m_roll_period_sec(roll_period_days * 24 * 1024 * 1024)
    , m_retain_period_sec(retain_period_day * 24 * 60 * 60) // 保留周期转换为秒
    , m_check_every(check_every)
    , m_count(0)
    , m_last_roll_time(0)
    , m_start_roll_period(0)
    , m_mutex(thread_safe ? new std::mutex : nullptr)
{
    if (::access(m_path.c_str(), F_OK) != 0) {
        ::mkdir(m_path.c_str(), 0755);
    }
    roll_file();
}

clia::log::FileAppender::~FileAppender() {
    ;
}

void clia::log::FileAppender::append(const void *buf, const std::size_t size) noexcept {
    if (m_mutex) {
        std::lock_guard<std::mutex> lock(*m_mutex);
        append_unlocked(buf, size);
    } else {
        append_unlocked(buf, size);
    }
}

void clia::log::FileAppender::flush() noexcept {
    if (m_mutex) {
        std::lock_guard<std::mutex> lock(*m_mutex);
        m_file->flush();
    } else {
        m_file->flush();
    }
}

void clia::log::FileAppender::append_unlocked(const void *buf, const std::size_t size) noexcept {
    m_file->append(buf, size);

    if (m_file->written_bytes() >= m_roll_size) {
        roll_file();
    } else {
        ++m_count;
        if (m_count >= m_check_every) {
            m_count = 0;
            const time_t now = ::time(nullptr);
            const time_t this_period = now / m_roll_period_sec * m_roll_period_sec;
            if (this_period != m_start_roll_period) {
                roll_file();
            } 
        }
    }
}

std::string clia::log::FileAppender::get_logfilename(const std::string &path, const std::string &logname) noexcept {
    std::string filename;
    filename.reserve(path.size() + logname.size() + 64);

    char timebuf[32] = {0};
    clia::util::time::get_time_str(timebuf, sizeof(timebuf), std::time(nullptr), "%Y%m%d%H%M%S");
    filename += path;
    if (!filename.empty() && filename.back() != '/') {
        filename += '/';
    }
    filename += timebuf;
    filename += '.';

    char pidbuf[32];
    std::snprintf(pidbuf, sizeof pidbuf, "%d", clia::util::process::get_pid());
    filename += pidbuf;
    filename += '.';

    filename += logname;
    filename += ".log";

    return filename;
}

void clia::log::FileAppender::roll_file() noexcept {
    const std::time_t now = std::time(nullptr);
    if (m_last_roll_time != now) {
        const auto filename = FileAppender::get_logfilename(m_path, m_logname);
        m_last_roll_time = now;
        m_start_roll_period = now / m_roll_period_sec * m_roll_period_sec;
        m_file.reset(new AppendFile(filename.c_str()));
        del_old_files();
    }
}

void clia::log::FileAppender::del_old_files() noexcept {
    // 删除过期的日志文件
    const std::time_t old = std::time(nullptr) - m_retain_period_sec;
    char timebuf[32] = {0};
    clia::util::time::get_time_str(timebuf, sizeof(timebuf), old, "%Y%m%d%H%M%S");
    const auto timebuflen = std::strlen(timebuf);
    std::vector<std::string> old_files;
    ::DIR *dir = ::opendir(m_path.c_str());
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
        old_files.emplace_back(m_path + "/" + entry->d_name);
    }
    ::closedir(dir);

    for (const auto &oldfile : old_files) {
        std::remove(oldfile.c_str());
    }
}
