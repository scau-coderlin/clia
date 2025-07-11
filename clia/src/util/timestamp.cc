#include <cstdint>
#include <ctime>
#include <sys/time.h>

#include "clia/util/timestamp.h"

clia::util::Timestamp::Timestamp() noexcept
    : micro_sec_since_epoch_(0)
{
    ;
}
clia::util::Timestamp::~Timestamp() noexcept = default;

clia::util::Timestamp::Timestamp(std::int64_t micro_sec_since_epoch) noexcept
    : micro_sec_since_epoch_(micro_sec_since_epoch)
{
    ;
}

clia::util::Timestamp::Timestamp(const Timestamp &oth) noexcept 
    : micro_sec_since_epoch_(oth.micro_sec_since_epoch()) {

}
clia::util::Timestamp& clia::util::Timestamp::operator=(const Timestamp &oth) noexcept {
    if (this != &oth) {
        micro_sec_since_epoch_ = oth.micro_sec_since_epoch();
    }
    return *this;
}

std::int64_t clia::util::Timestamp::micro_sec_since_epoch() const noexcept {
    return micro_sec_since_epoch_;
}
            
std::time_t clia::util::Timestamp::sec_since_epoch() const noexcept {
    return static_cast<std::time_t>(micro_sec_since_epoch() / kMicroSecPerSec);
}

std::string clia::util::Timestamp::to_format_str(const bool show_micro_sec) const {
    char buf[64] = {0};
    const int n = to_format_str(buf, sizeof(buf), show_micro_sec);
    return std::string(buf, n);
}

int clia::util::Timestamp::to_format_str(char *outbuf, std::size_t sz, const bool show_micro_sec) const noexcept {
    const auto sec = sec_since_epoch();
    ::tm tm_time;
    ::localtime_r(&sec, &tm_time);
    if (show_micro_sec) {
        const int microsec = static_cast<int>(micro_sec_since_epoch() % kMicroSecPerSec);
        return std::snprintf(outbuf, sz, "%04d-%02d-%02d %02d:%02d:%02d.%06d", 
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
                microsec);
    } else {
        return std::snprintf(outbuf, sz, "%4d%02d%02d %02d:%02d:%02d",
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
}

bool clia::util::Timestamp::operator<(const Timestamp oth) const noexcept {
    return micro_sec_since_epoch() < oth.micro_sec_since_epoch();
}
bool clia::util::Timestamp::operator<=(const Timestamp oth) const noexcept {
    return micro_sec_since_epoch() <= oth.micro_sec_since_epoch();
}

bool clia::util::Timestamp::operator>(const Timestamp oth) const noexcept {
    return micro_sec_since_epoch() > oth.micro_sec_since_epoch();
}
bool clia::util::Timestamp::operator>=(const Timestamp oth) const noexcept {
    return micro_sec_since_epoch() >= oth.micro_sec_since_epoch();
}
bool clia::util::Timestamp::operator==(const Timestamp oth) const noexcept {
    return micro_sec_since_epoch() == oth.micro_sec_since_epoch();
}
bool clia::util::Timestamp::operator!=(const Timestamp oth) const noexcept {
    return micro_sec_since_epoch() != oth.micro_sec_since_epoch();
}

clia::util::Timestamp clia::util::Timestamp::now() noexcept {
    ::timeval tv;
    ::gettimeofday(&tv, nullptr);
    const std::int64_t sec = tv.tv_sec;
    return Timestamp(sec * kMicroSecPerSec + tv.tv_usec);
}