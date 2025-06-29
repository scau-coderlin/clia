#include <cassert>
#include <chrono>
#include <cstring>
#include <cstdio>
#include <vector>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "clia/log/async_logger.h"
#include "clia/util/timestamp.h"
#include "clia/container/fixed_buffer.h"

class clia::log::AsyncLogger::Impl {
public:
    explicit inline Impl(trait::Appender *const appender, const int flush_interval_sec) noexcept;
    inline ~Impl() noexcept;
public:
    inline void log(const void *message, const std::size_t len) noexcept;
private:
    inline void thread();
private:
    using Buffer = clia::container::FixedBuffer<char, 4 * 1024 * 1024>;
    using BufferVector = std::vector<std::unique_ptr<Buffer>>;
    using BufferPtr = BufferVector::value_type;
private:
    trait::Appender *const appender_;
    const int flush_interval_sec_;
    BufferPtr current_buffer_;
    BufferPtr next_buffer_;
    BufferVector buffers_;
    std::atomic<bool> running_;
    std::thread thread_;
    std::mutex lck_;
    std::condition_variable cond_;
};


clia::log::AsyncLogger::AsyncLogger(const Level level, std::shared_ptr<trait::Appender> appender, const int flush_interval_sec) noexcept
    : trait::Logger(level, appender)
    , impl_(new Impl(appender_.get(), flush_interval_sec))
{
    ;
}

clia::log::AsyncLogger::~AsyncLogger() noexcept = default;

void clia::log::AsyncLogger::log(const Level level, const void *message, const std::size_t len) noexcept {
    if (level < level_) {
        return; // Ignore messages below the current log level
    }
    impl_->log(message, len);
}


inline clia::log::AsyncLogger::Impl::Impl(trait::Appender *const appender, const int flush_interval_sec) noexcept 
        : appender_(appender)
        , flush_interval_sec_(flush_interval_sec)
        , current_buffer_(new Buffer)
        , next_buffer_(new Buffer)
{
    buffers_.reserve(16);
    running_ = true;
    thread_ = std::thread(&Impl::thread, this);
}

inline clia::log::AsyncLogger::Impl::~Impl() noexcept {
    running_ = false;
    if (thread_.joinable()) {
        thread_.join();
    }
}

inline void clia::log::AsyncLogger::Impl::log(const void *message, const std::size_t len) noexcept {
    if (!message || 0 == len || len > Buffer::max_size() || !running_) {
        return; // Ignore messages below the current log level
    }
    std::lock_guard<std::mutex> lck(lck_);
    if (current_buffer_->avail() > len) {
        current_buffer_->append(static_cast<const char*>(message), len);
    } else {
        buffers_.emplace_back(std::move(current_buffer_));
        if (next_buffer_) {
            current_buffer_ = std::move(next_buffer_);
        } else {
            current_buffer_.reset(new Buffer);
        }
        current_buffer_->append(static_cast<const char*>(message), len);
        cond_.notify_one();
    }
}

inline void clia::log::AsyncLogger::Impl::thread() {
    assert(appender_ != nullptr);
    BufferPtr new_buffer1(new Buffer);
    BufferPtr new_buffer2(new Buffer);
    BufferVector write_buffers;
    write_buffers.reserve(16);

    while (running_) {
        assert(new_buffer1 && new_buffer1->size() == 0);
        assert(new_buffer2 && new_buffer2->size() == 0);
        assert(write_buffers.empty());
        {
            std::unique_lock<std::mutex> lck(lck_);
            if (buffers_.empty()) {
                cond_.wait_for(lck, std::chrono::seconds(flush_interval_sec_));
            }
            buffers_.push_back(std::move(current_buffer_));
            if (new_buffer1) {
                current_buffer_ = std::move(new_buffer1);
            } else {
                current_buffer_.reset(new Buffer);
            }
            if (!next_buffer_) {
                if (new_buffer2) {
                    next_buffer_ = std::move(new_buffer2);
                } else {
                    next_buffer_.reset(new Buffer);
                }
            }
            write_buffers.swap(buffers_);
        }

        assert(!write_buffers.empty());

        if (write_buffers.size() > 25) {
            char buf[256] = {0};
            char timebuf[32] = {0};
            const auto now = clia::util::Timestamp::now();
            now.to_format_str(timebuf, sizeof(timebuf));
            std::snprintf(buf, sizeof(buf), "Dropped log messages at %s, %zd larger buffers\n",
                    timebuf, write_buffers.size() - 2);
            std::fputs(buf, stderr);
            appender_->append(buf, std::strlen(buf));
            write_buffers.erase(write_buffers.begin() + 2, write_buffers.end());
        }

        for (const auto &buffer : write_buffers) {
            if (buffer->size() <= 0) {
                continue; // Skip empty buffers
            }
            appender_->append(buffer->data(), buffer->size());
        }

        if (write_buffers.size() > 2) {
            // drop non-bzero-ed buffers, avoid trashing
            write_buffers.resize(2);
        }

        if (!new_buffer1) {
            assert(!write_buffers.empty());
            new_buffer1 = std::move(write_buffers.back());
            write_buffers.pop_back();
            new_buffer1->reset();
        }

        if (!new_buffer2) {
            assert(!write_buffers.empty());
            new_buffer2 = std::move(write_buffers.back());
            write_buffers.pop_back();
            new_buffer2->reset();
        }

        write_buffers.clear();
        appender_->flush();
    }
    appender_->flush();
}
