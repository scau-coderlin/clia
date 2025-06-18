#include <cstring>

#include "log/event.hpp"
#include "util/time.hpp"
#include "util/process.hpp"

clia::log::Event::Event(std::shared_ptr<LoggerTrait> logger, const Level level, const char *file, const int line, const char *func) noexcept
    : m_logger(logger)
    , m_level(level)
    , m_file(file)
    , m_line(line)
    , m_func(func)
{
    constexpr char SPACER = ' ';
    char timebuf[32] = {0};
    clia::util::time::get_time_str(timebuf, sizeof(timebuf), std::time(nullptr), "%Y%m%d%H%M%S");
    const char *filename = std::strrchr(m_file, '/');
    if (!filename) {
        filename = m_file; // 如果没有找到斜杠，使用完整路径
    } else {
        ++filename; // 跳过斜杠
    }
    m_stream << timebuf << SPACER 
            << clia::util::process::get_pid() << SPACER
            << clia::util::process::get_tid() << SPACER
            << level_to_string(m_level) << SPACER
            << '[' << filename << ':' << m_func << ':' << m_line << "]::" << SPACER;
}

clia::log::Event::~Event() noexcept {
    m_stream << '\n'; 
    if (m_logger) {
        m_logger->log(m_level, m_stream.data(), m_stream.size());
    }
}

clia::log::Stream& clia::log::Event::stream() noexcept {
    return m_stream;
}

void clia::log::Event::format(const char *fmt, ...) noexcept {
    std::va_list al;
    va_start(al, fmt);
    m_stream.format(fmt, al);
    va_end(al);
}