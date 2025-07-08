#ifndef CLIA_UTIL_TIMESTAMP_H_
#define CLIA_UTIL_TIMESTAMP_H_

#include <cstdint>
#include <ctime>
#include <string>

#include "clia/base/copyable.h"

namespace clia {
    namespace util {
        class Timestamp : Copyable  {
        public:
            static constexpr int kMicroSecPerSec = 1000 * 1000; 
        public:
            Timestamp() noexcept;
            ~Timestamp() noexcept;
            explicit Timestamp(std::int64_t micro_sec_since_epoch) noexcept;
            Timestamp(const Timestamp &oth) noexcept;
            Timestamp& operator=(const Timestamp &oth) noexcept;
        public:
            std::int64_t micro_sec_since_epoch() const noexcept;
            std::time_t sec_since_epoch() const noexcept;
            std::string to_format_str(const bool show_micro_sec = true) const;
            int to_format_str(char *outbuf, std::size_t sz, const bool show_micro_sec = true) const noexcept;
        public:
            bool operator<(const Timestamp oth) const noexcept;
            bool operator<=(const Timestamp oth) const noexcept;
            bool operator>(const Timestamp oth) const noexcept;
            bool operator>=(const Timestamp oth) const noexcept;
            bool operator==(const Timestamp oth) const noexcept;
            bool operator!=(const Timestamp oth) const noexcept;
        public:
            static Timestamp now() noexcept;
        private:
            std::int64_t micro_sec_since_epoch_ = 0; // 微妙级别
        };
    }
}

#endif