#include <cassert>
#include <chrono>
#include <mutex>
#include <cstring>
#include <cstdio>

#include "clia/log/async_logger.hpp"
#include "clia/util/time.hpp"

clia::log::AsyncLogger::AsyncLogger(clia::log::Level level, int flush_interval_sec) noexcept
    : LoggerTrait(level)
    , m_flush_interval_sec(flush_interval_sec)
    , m_current_buffer(new Buffer)
    , m_next_buffer(new Buffer)
{
    m_running = false;
    m_buffers.reserve(16);
}

clia::log::AsyncLogger::~AsyncLogger() noexcept {
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void clia::log::AsyncLogger::log(const void *message, const std::size_t len) noexcept {
    std::lock_guard<std::mutex> lck(m_lck);
    if (m_current_buffer->avail() > len) {
        m_current_buffer->append_range(static_cast<const char*>(message), len);
    } else {
        m_buffers.emplace_back(std::move(m_current_buffer));
        if (m_next_buffer) {
            m_current_buffer = std::move(m_next_buffer);
        } else {
            m_current_buffer.reset(new Buffer);
        }
        m_current_buffer->append_range(static_cast<const char*>(message), len);
        m_cond.notify_one();
    }
}

void clia::log::AsyncLogger::start() noexcept {
    assert(m_appender.get() != nullptr);
    m_running = true;
    m_thread = std::thread(&AsyncLogger::thread, this);
}

void clia::log::AsyncLogger::thread() {
    assert(m_running == true);
    assert(m_appender.get() != nullptr);
    BufferPtr new_buffer1(new Buffer);
    BufferPtr new_buffer2(new Buffer);
    BufferVector write_buffers;
    write_buffers.reserve(16);

    while (m_running) {
        assert(new_buffer1 && new_buffer1->size() == 0);
        assert(new_buffer2 && new_buffer2->size() == 0);
        assert(write_buffers.empty());

        {
            std::unique_lock<std::mutex> lck(m_lck);
            if (m_buffers.empty()) {
                m_cond.wait_for(lck, std::chrono::seconds(m_flush_interval_sec));
            }
            m_buffers.push_back(std::move(m_current_buffer));
            m_current_buffer = std::move(new_buffer1);
            write_buffers.swap(m_buffers);
            if (!m_next_buffer) {
                m_next_buffer = std::move(new_buffer2);
            }
        }

        assert(!write_buffers.empty());

        if (write_buffers.size() > 25) {
            char buf[256] = {0};
            char timebuf[32] = {0};
            std::snprintf(buf, sizeof(buf), "Dropped log messages at %s, %zd larger buffers\n",
                    clia::util::time::get_time_str(timebuf, sizeof(timebuf)),
                    write_buffers.size() - 2);
            std::fputs(buf, stderr);
            m_appender->append(buf, std::strlen(buf));
            write_buffers.erase(write_buffers.begin() + 2, write_buffers.end());
        }

        for (const auto& buffer : write_buffers) {
            m_appender->append(buffer->data(), buffer->size());
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
        m_appender->flush();
    }
    m_appender->flush();
}
