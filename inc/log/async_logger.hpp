#pragma once

#include <cstddef>
#include <memory>
#include <vector>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "log/trait.hpp"
#include "container/fixed_buffer.hpp"

namespace clia {
    namespace log {
        class AsyncLogger final : public LoggerTrait {
        public:
            explicit AsyncLogger(clia::log::Level level, int flush_interval_sec = 3) noexcept;
            ~AsyncLogger() noexcept;

            AsyncLogger(const AsyncLogger&) = delete;
            AsyncLogger& operator=(const AsyncLogger&) = delete;
            AsyncLogger(AsyncLogger&&) noexcept = delete;
            AsyncLogger& operator=(AsyncLogger&&) noexcept = delete;

        public:
            void log(const void *message, const std::size_t len) noexcept override; // 异步日志记录方法
            void start(std::shared_ptr<clia::log::AppenderTrait> appender) noexcept;
        private:
            void thread();
        private:
            using Buffer = clia::container::FixedBuffer<char, 4 * 1024 * 1024>;
            using BufferVector = std::vector<std::unique_ptr<Buffer>>;
            using BufferPtr = BufferVector::value_type;
        private:
            const int m_flush_interval_sec;
            BufferPtr m_current_buffer;
            BufferPtr m_next_buffer;
            BufferVector m_buffers;
            std::atomic<bool> m_running;
            std::thread m_thread;
            std::mutex m_lck;
            std::condition_variable m_cond;
        };
        // 其他成员函数和数据成员可以根据需要添加
    } // namespace log
} // namespace clia