#ifndef CLIA_LOG_TRAIT_H_
#define CLIA_LOG_TRAIT_H_

#include <cstddef>
#include <memory>

#include "clia/base/noncopyable.h"

namespace clia {
    namespace log {
        enum class Level {
            kTrace = 0,
            kDebug,
            kInfo,
            kWarn,
            kError,
            kFatal,
        };
        extern const char* level_to_string(const Level level) noexcept;
        
        namespace trait {
            class Appender : Noncopyable {
            public:
                Appender() noexcept = default;
                virtual ~Appender() noexcept = default;
            public:
                /// append() 函数用于将日志行追加到日志记录器中
                /// @param buf 日志行内容
                /// @param size 日志行内容长度
                /// @note 该函数是纯虚函数，具体的实现需要在子类中实现
                ///       该函数不要求是线程安全的
                ///       该函数会被 Logger 调用来记录日志
                ///       该函数不需要处理日志级别和日志时间戳等信息
                ///       该函数只需要处理日志行内容的追加
                virtual void append(const void* buf, const std::size_t size) noexcept = 0; // 添加日志行
                /// flush() 函数用于刷新日志记录器
                /// @note 该函数是纯虚函数，具体的实现需要在子类中实现
                ///       该函数不要求是线程安全的
                ///       该函数会被 Logger 调用来刷新日志记录器
                ///       该函数通常用于将日志行写入到文件或其他存储
                virtual void flush() noexcept = 0; // 刷新日志
            };

            class Logger : Noncopyable {
            public:
                /// 构造函数，接受日志级别和日志追加器
                /// @param level 日志级别
                /// @param appender 日志追加器
                Logger(const Level level, std::shared_ptr<Appender> appender) noexcept;
                virtual ~Logger() noexcept = default;
            public:
                Level level() const noexcept;
                /// 日志记录函数，只有当日志级别大于等于当前设置的级别时才会记录
                /// @param level 日志级别
                /// @param message 日志消息内容
                /// @param len 日志消息长度 
                /// @note 该函数会调用具体的 log() 函数来处理日志记录, 该函数的实现要求是线程安全的
                virtual void log(const Level level, const void *message, const std::size_t len) noexcept = 0;
            protected:
                const Level level_ = Level::kError;          // 日志级别
                const std::shared_ptr<Appender> appender_;  // 日志追加器
            };
        }
    }
}

#endif