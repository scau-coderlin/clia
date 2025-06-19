#pragma once

#include "clia/base/nocopyable.hpp"
#include <cstddef>
#include <memory>

namespace clia {
    namespace log {
        enum class Level {
            TRACE = 0,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL,
        };
        extern const char* level_to_string(const Level level) noexcept;
        
        class AppenderTrait : Noncopyable {
        public:
            AppenderTrait() noexcept;
            virtual ~AppenderTrait() noexcept;
        public:
            virtual void append(const void* buf, const std::size_t size) noexcept = 0; // 添加日志行
            virtual void flush() noexcept; // 刷新日志
        };

        class LoggerTrait : Noncopyable {
        public:
            LoggerTrait(const Level level = Level::ERROR) noexcept;
            virtual ~LoggerTrait() noexcept;
        public:
            /// 日志记录函数，只有当日志级别大于等于当前设置的级别时才会记录
            /// @param level 日志级别
            /// @param message 日志消息内容
            /// @param len 日志消息长度 
            /// @note 该函数会调用具体的 log() 函数来处理日志记录, 该函数的实现要求是线程安全的
            void log(const Level level, const void *message, const std::size_t len) noexcept;
            /// 设置日志级别
            /// @param level 日志级别
            /// @note 该函数不会触发日志记录
            ///       仅仅是设置当前 LoggerTrait 的日志级别
            ///       该函数不是线程安全的
            ///       该函数会影响后续的 log() 函数调用
            void set_level(const Level level) noexcept;
            /// 设置日志追加器
            /// @param appender 日志追加器
            /// @note 该函数会替换当前的日志追加器
            ///       该函数不是线程安全的
            ///       该函数会影响后续的 log() 函数调用
            void set_appender(std::shared_ptr<AppenderTrait> appender) noexcept;
        protected:
            virtual void log(const void *message, const std::size_t len) noexcept = 0;
        protected:
            Level m_level = Level::ERROR; // 日志级别
            std::shared_ptr<AppenderTrait> m_appender; // 日志追加器
        };
    }
}