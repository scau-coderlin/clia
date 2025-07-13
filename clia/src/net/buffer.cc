#include <cassert>

#include <unistd.h>
#include <sys/uio.h>

#include "clia/net/buffer.h"
#include "clia/log.h"
#include "clia/util/process.h"

clia::net::Buffer::Buffer(const std::size_t inital_size)
    : buffer_(kCheapPrepend + inital_size)
    , reader_index_(kCheapPrepend)
    , writer_index_(kCheapPrepend) 
{
    assert(readable_bytes() == 0);
    assert(writable_bytes() == 0);
    assert(prependable_bytes() == kCheapPrepend);
}

clia::net::Buffer::~Buffer() noexcept {

}

clia::net::Buffer::Buffer(const Buffer &oth) 
    : buffer_(oth.buffer_)
    , reader_index_(oth.reader_index_)
    , writer_index_(oth.writer_index_)
{
    ;
}

clia::net::Buffer& clia::net::Buffer::operator=(const Buffer &oth) {
    if (&oth != this) {
        buffer_ = oth.buffer_;
        reader_index_ = oth.reader_index_;
        writer_index_ = oth.writer_index_;
    }
    return *this;
}

clia::net::Buffer::Buffer(Buffer &&oth) noexcept
    : buffer_(std::move(oth.buffer_))
    , reader_index_(oth.reader_index_)
    , writer_index_(oth.writer_index_)
{
    oth.reader_index_ = kCheapPrepend;
    oth.writer_index_ = kCheapPrepend;
}

clia::net::Buffer& clia::net::Buffer::operator=(Buffer &&oth) noexcept {
    if (&oth != this) {
        buffer_ = std::move(oth.buffer_);
        reader_index_ = oth.reader_index_;
        writer_index_ = oth.writer_index_;
        oth.reader_index_ = kCheapPrepend;
        oth.writer_index_ = kCheapPrepend;
    }
    return *this;
}

std::size_t clia::net::Buffer::readable_bytes() const noexcept {
    return writer_index_ - reader_index_;
}

std::size_t clia::net::Buffer::writable_bytes() const noexcept {
    return buffer_.size() - writer_index_;
}

std::size_t clia::net::Buffer::prependable_bytes() const noexcept {
    return reader_index_;
}

const unsigned char* clia::net::Buffer::peek() const noexcept {
    return this->begin() + reader_index_;
}

void clia::net::Buffer::retrieve(const std::size_t len) noexcept {
    assert(len <= readable_bytes());
    if (len < readable_bytes()) {
        reader_index_ += len;
    } else {
        retrieve_all();
    }
}

void clia::net::Buffer::retrieve_all() noexcept {
    reader_index_ = kCheapPrepend;
    writer_index_ = kCheapPrepend;
}

std::string clia::net::Buffer::retrieve_all_as_string() {
    return retrieve_as_string(readable_bytes());
}

std::string clia::net::Buffer::retrieve_as_string(const std::size_t len) {
    assert(len <= readable_bytes());
    std::string result(reinterpret_cast<const char*>(this->peek()), len);
    this->retrieve(len);
    return result;
}

void clia::net::Buffer::ensure_writable_bytes(const std::size_t len) {
    if (this->writable_bytes() < len) {
        this->make_space(len);
    }
    assert(writable_bytes() >= len);
}

void clia::net::Buffer::append(const void *data, const std::size_t len) {
    this->ensure_writable_bytes(len);
    const unsigned char *p = static_cast<const unsigned char*>(data);
    std::copy(p, p + len, begin_write());
    writer_index_ += len;
}

unsigned char* clia::net::Buffer::begin_write() noexcept {
    return this->begin() + writer_index_;
}

const unsigned char* clia::net::Buffer::begin_write() const noexcept {
    return this->begin() + writer_index_;
}

::ssize_t  clia::net::Buffer::read_fd(int fd) noexcept {
    unsigned char extrabuf[65536];
    ::iovec vec[2];
    const auto writable = this->writable_bytes();
    vec[0].iov_base = this->begin() + writer_index_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[0].iov_len = sizeof(extrabuf);

    const int iovcnt = 2;
    const auto n = ::readv(fd, vec, iovcnt);
    if (n < 0) {
        CLIA_FMT_LOG_ERROR("readv fail, errno = [%d][%s]", errno, clia::util::process::strerror(errno));
    } else if (n <= writable) {
        writer_index_ += n;
    } else {
        writer_index_ = buffer_.size();
        this->append(extrabuf, n - writable);
    }
    return n;
}

::ssize_t clia::net::Buffer::write_fd(int fd) noexcept {
    auto n = ::write(fd, this->peek(), readable_bytes());
    if (n < 0) {
        CLIA_FMT_LOG_ERROR("write fail, errno = [%d][%s]", errno, clia::util::process::strerror(errno));
    } else {
        this->retrieve(n);
    }
    return n;
}

unsigned char* clia::net::Buffer::begin() noexcept {
    return buffer_.data();
}

const unsigned char* clia::net::Buffer::begin() const noexcept {
    return buffer_.data();
}

void clia::net::Buffer::make_space(const std::size_t len) {
    if (this->writable_bytes() + prependable_bytes() < len + kCheapPrepend) {
        buffer_.resize(writer_index_ + len);
    } else {
        assert(kCheapPrepend < reader_index_);
        auto readable = this->readable_bytes();
        std::copy(this->begin() + reader_index_, 
            this->begin() + writer_index_, 
            this->begin() + kCheapPrepend
        );
        reader_index_ = kCheapPrepend;
        writer_index_ = reader_index_ + readable;
        assert(readable == readable_bytes());
    }
}