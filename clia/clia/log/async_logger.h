#ifndef CLIA_LOG_ASYNC_LOGGER_H_
#define CLIA_LOG_ASYNC_LOGGER_H_

#include <cstddef>
#include <memory>
#include <mutex>
#include <vector>
#include <atomic>
#include <thread>
#include <condition_variable>

#include "clia/log/trait.h"
#include "clia/container/fixed_buffer.h"

namespace clia {
    namespace log {
        class AsyncLogger final : public trait::Logger {
        public:
            explicit AsyncLogger(const Level level, std::shared_ptr<trait::Appender> appender, const int flush_interval_sec = 1) noexcept;
            ~AsyncLogger() noexcept;
        public:
            void log(const Level level, const void *message, const std::size_t len) noexcept override; // 异步日志记录方法
        private:
            inline void sync_thread();
        private:
            static constexpr int kBufferSize = 4 * 1024 * 1024;
            using Buffer = clia::container::FixedBuffer<char, kBufferSize>;
            using BufferVector = std::vector<std::unique_ptr<Buffer>>;
            using BufferPtr = BufferVector::value_type;
        private:
            const int flush_interval_sec_;
            BufferPtr current_buffer_;
            BufferPtr next_buffer_;
            BufferVector buffers_;
            std::atomic<bool> running_;
            std::thread thread_;
            std::mutex lck_;
            std::condition_variable cond_;
        };
        // 其他成员函数和数据成员可以根据需要添加
    } // namespace log
} // namespace clia

#endif