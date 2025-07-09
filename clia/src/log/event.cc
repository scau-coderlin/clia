#include <cstring>

#include "clia/log/event.h"
#include "clia/util/timestamp.h"
#include "clia/util/process.h"

namespace {
    thread_local int kMicroSecPos = 0;
    thread_local char kTimeBuf[64] = {0};
    thread_local std::time_t kLastUpdateSec = 0;

    template <typename OStream>
    inline static void tips_message(OStream &out, const clia::log::Level level, const char *file, const int line, const char *func) noexcept {
        assert(file != nullptr && line != -1 && func != nullptr);
        constexpr char kSpacer = ' ';
        const auto now = clia::util::Timestamp::now();
        const auto sec = now.sec_since_epoch();
        if (sec != ::kLastUpdateSec) {
            ::kMicroSecPos = now.to_format_str(::kTimeBuf, sizeof(::kTimeBuf), false);
            ::kLastUpdateSec = sec;
        }
        std::sprintf(::kTimeBuf + ::kMicroSecPos, ".%06d", static_cast<int>(now.micro_sec_since_epoch() % clia::util::Timestamp::kMicroSecPerSec));

        const char *filename = std::strrchr(file, '/');
        if (!filename) {
            filename = file; // 如果没有找到斜杠，使用完整路径
        } else {
            ++filename; // 跳过斜杠
        }
        out << ::kTimeBuf << kSpacer 
            << clia::util::process::get_pid() << kSpacer
            << clia::util::process::get_tid() << kSpacer
            << clia::log::level_to_string(level) << kSpacer
            << '[' << filename << ':' << func << ':' << line << "]::" << kSpacer;
    }
}

clia::log::Event::Event(std::shared_ptr<trait::Logger> logger, const Level level, const char *file, const int line, const char *func) noexcept
    : logger_(logger)
    , level_(level)
{
    ::tips_message(stream_, level, file, line, func);
}

clia::log::Event::~Event() noexcept {
    stream_ << '\n'; 
    if (logger_) {
        logger_->log(level_, stream_.data(), stream_.size());
    }
}

clia::log::Event::Stream& clia::log::Event::stream() noexcept {
    return stream_;
}

void clia::log::Event::format(const char *fmt, ...) noexcept {
    std::va_list al;
    va_start(al, fmt);
    stream_.format(fmt, al);
    va_end(al);
}

clia::log::LargeEvent::LargeEvent(std::shared_ptr<trait::Logger> logger, const Level level, const char *file, const int line, const char *func) noexcept
    : logger_(logger)
    , level_(level)
{
    ::tips_message(stream_, level, file, line, func);
}

clia::log::LargeEvent::~LargeEvent() noexcept {
    stream_ << '\n'; 
    if (logger_) {
        const auto str = stream_.str();
        logger_->log(level_, str.data(), str.size());
    }
}

clia::log::LargeEvent::Stream& clia::log::LargeEvent::stream() noexcept {
    return stream_;
}

void clia::log::LargeEvent::format(const char *fmt, ...) noexcept {
    std::va_list al;
    va_start(al, fmt);
    char* buf = nullptr;
    const int len = ::vasprintf(&buf, fmt, al);
    if (len != -1) {
        stream_ << std::string(buf, len);
        std::free(buf);
    }
    va_end(al);
}