#ifndef CLIA_NET_BUFFER_H_
#define CLIA_NET_BUFFER_H_

#include <cstddef>
#include <sys/types.h>
#include <vector>
#include <string>

#include "clia/base/copyable.h"

namespace clia {
    namespace net {
        class Buffer : Copyable {
        public:
            static constexpr std::size_t kCheapPrepend = 8;
            static constexpr std::size_t kInitialSize = 1024;
        public:
            explicit Buffer(const std::size_t inital_size = kInitialSize);
            Buffer(const Buffer &oth);
            Buffer& operator=(const Buffer &oth);
            Buffer(Buffer &&oth) noexcept;
            Buffer& operator=(Buffer &&oth) noexcept;
            ~Buffer() noexcept;
        public:
            std::size_t readable_bytes() const noexcept;
            std::size_t writable_bytes() const noexcept;
            std::size_t prependable_bytes() const noexcept;
        public:
            const unsigned char* peek() const noexcept;
            void retrieve(const std::size_t len) noexcept;
            void retrieve_all() noexcept;
            std::string retrieve_all_as_string();
            std::string retrieve_as_string(const std::size_t len);
            void ensure_writable_bytes(const std::size_t len);
            void append(const void *data, const std::size_t len);
            unsigned char* begin_write() noexcept;
            const unsigned char* begin_write() const noexcept;
            ::ssize_t read_fd(int fd) noexcept;
            ::ssize_t write_fd(int fd) noexcept;
        private:
            unsigned char* begin() noexcept;
            const unsigned char* begin() const noexcept;
            void make_space(const std::size_t len);
        private:
            std::vector<unsigned char> buffer_;
            std::size_t reader_index_;
            std::size_t writer_index_;
        };
    }
}

#endif