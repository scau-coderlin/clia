#include <atomic>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "clia/log/async_logger.h"
#include "clia/log/file_appender.h"
#include "clia/log.h"

class TimeConsumingStatistics {
private:
    const std::time_t start_;
public:
    TimeConsumingStatistics()
        : start_(std::time(nullptr)) 
    {
        ;
    }
    ~TimeConsumingStatistics() {
        const auto end = std::time(nullptr);
        std::cout << "Time elapsed: " << (end - start_) << " seconds." << std::endl;
    }
};

TimeConsumingStatistics _;

int main(int argc, char *argv[]) {
    std::cout << "hello world" << std::endl;
    std::shared_ptr<clia::log::trait::Appender> appender(new clia::log::FileAppender("./", "testlog"));
    std::shared_ptr<clia::log::trait::Logger> logger(new clia::log::AsyncLogger(clia::log::Level::kInfo, appender));
    clia::log::LoggerManger::instance()->set_default(logger);

    bool running = false;
    std::atomic<unsigned long> count;
    count = 0;
    std::vector<std::thread> threads;
    for (int i = 0; i < std::atoi(argv[1]); ++i) {
        threads.emplace_back([&]() {
            while (!running) {
                ;
            }
            for (int i = 0; i < std::atoi(argv[2]); ++i) {
                CLIA_LOG_INFO  << "Hello, " << "world" << "! This is a test log message with value: " << count++;
            }
        });
    }

    const auto start = std::time(nullptr);
    running = true;
    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    const auto end = std::time(nullptr);
    std::cout << "Logging completed in " << (end - start) << " seconds." << std::endl;
    return 0;
}