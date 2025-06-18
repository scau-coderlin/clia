#include <atomic>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "log/event.hpp"
#include "log/async_logger.hpp"
#include "log/file_appender.hpp"
#include "log/trait.hpp"

class TimeS {
private:
    std::time_t m_start;
public:
    TimeS() {
        m_start = std::time(nullptr);
    }
    ~TimeS() {
        const auto end = std::time(nullptr);
        std::cout << "Time elapsed: " << (end - m_start) << " seconds." << std::endl;
    }
};

TimeS _;

int main(int argc, char *argv[]) {
    std::cout << "hello world" << std::endl;
    std::shared_ptr<clia::log::AsyncLogger> logger(new clia::log::AsyncLogger(clia::log::Level::INFO));
    std::shared_ptr<clia::log::FileAppender> appender(new clia::log::FileAppender("./", "testlog"));
    logger->set_appender(appender);
    logger->start();

    bool m_start = false;
    std::atomic<unsigned long> count;
    count = 0;
    std::vector<std::thread> threads;
    for (int i = 0; i < std::atoi(argv[1]); ++i) {
        threads.emplace_back([&]() {
            while (!m_start) {
                ;
            }
            for (int i = 0; i < std::atoi(argv[2]); ++i) {
                clia::log::Event(logger, clia::log::Level::INFO, __FILE_NAME__, __LINE__, __func__)
                    .stream() << "Hello, " << "world" << "! This is a test log message with value: " << count++;
            }
        });
    }

    const auto start = std::time(nullptr);
    m_start = true;
    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    const auto end = std::time(nullptr);
    std::cout << "Logging completed in " << (end - start) << " seconds." << std::endl;
    return 0;
}