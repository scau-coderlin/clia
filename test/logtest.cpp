#include <iostream>
#include <memory>

#include "log/event.hpp"
#include "log/async_logger.hpp"
#include "log/file_appender.hpp"
#include "log/trait.hpp"

int main(int argc, char *argv[]) {
    std::cout << "hello world" << std::endl;
    std::shared_ptr<clia::log::LoggerTrait> logger(new clia::log::AsyncLogger(clia::log::Level::INFO));
    std::shared_ptr<clia::log::AppenderTrait> appender(new clia::log::FileAppender("./", "testlog"));
    logger->set_appender(appender);

    const auto start = std::time(nullptr);
#if 1
    for (int i = 0; i < std::atoi(argv[1]); ++i) {
        clia::log::Event(logger, clia::log::Level::INFO, __FILE_NAME__, __LINE__, __func__)
            .stream() << "Hello, " << "world" << "! This is a test log message with value: " << i;
    }
#else
    clia::log::Event(logger, clia::log::Level::INFO, __FILE_NAME__, __LINE__, __func__)
            .stream() << "Hello, " << "world" << "! This is a test log message with value: " << 0;
    
#endif
    const auto end = std::time(nullptr);
    std::cout << "Logging completed in " << (end - start) << " seconds." << std::endl;
    return 0;
}