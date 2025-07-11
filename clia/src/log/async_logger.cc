#include <cassert>
#include <chrono>
#include <cstring>
#include <cstdio>
#include <vector>
#include <atomic>
#include <thread>
#include <condition_variable>

#include "clia/log/async_logger.h"
#include "clia/util/timestamp.h"
#include "clia/container/fixed_buffer.h"

clia::log::AsyncLogger::AsyncLogger(const Level level, std::shared_ptr<trait::Appender> appender, const int flush_interval_sec) noexcept
    : trait::Logger(level, appender)
    , flush_interval_sec_(flush_interval_sec)
    , current_buffer_(new Buffer)
    , next_buffer_(new Buffer)
{
    buffers_.reserve(16);
    running_ = true;
    thread_ = std::thread(&AsyncLogger::sync_thread, this);
}

clia::log::AsyncLogger::~AsyncLogger() noexcept {
    running_ = false;
    cond_.notify_one();
    if (thread_.joinable()) {
        thread_.join();
    }
}

void clia::log::AsyncLogger::log(const Level level, const void *message, const std::size_t len) noexcept {
    if (level < level_ || nullptr == message || 0 == len || len > Buffer::max_size() || !running_) {
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

inline void clia::log::AsyncLogger::sync_thread() {
    assert(appender_ != nullptr);
    BufferPtr new_buffer1(new Buffer);
    BufferPtr new_buffer2(new Buffer);
    BufferVector write_buffers;
    write_buffers.reserve(16);

    while (running_ || current_buffer_->size() != 0) {
        assert(new_buffer1 && new_buffer1->size() == 0);
        assert(new_buffer2 && new_buffer2->size() == 0);
        assert(write_buffers.empty());
        {
            std::unique_lock<std::mutex> lck(lck_);
            if (buffers_.empty()) {
                cond_.wait_for(lck, std::chrono::seconds(flush_interval_sec_));
            }
            buffers_.push_back(std::move(current_buffer_));
            current_buffer_ = std::move(new_buffer1);
            if (!next_buffer_) {
                next_buffer_ = std::move(new_buffer2);
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
