#include <iostream>
#include <memory>

#include "log/event.hpp"
#include "log/stdout_logger.hpp"

int main() {
    std::cout << "hello world" << std::endl;
    std::shared_ptr<clia::log::LoggerTrait> logger(new clia::log::StdoutLogger(clia::log::Level::INFO));
    clia::log::Event(logger, clia::log::Level::INFO, __BASE_FILE__, __LINE__, __func__)
        .format("Hello, %s! This is a test log message with value: %d", "world", 42);
    clia::log::Event(logger, clia::log::Level::INFO, __FILE_NAME__, __LINE__, __func__)
        .stream() << "Hello, " << "world" << "! This is a test log message with value: " << 43;
    return 0;
}