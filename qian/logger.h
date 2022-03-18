#pragma once
#include "noncopyable.h"
#include <string>

#define QIAN_LOG_INFO(log_msg_format, ...)                         \
    do {                                                           \
        Logger& logger = Logger::getInstance();                    \
        logger.setLogLevel(LogLevel::INFO);                        \
        char buf[1024] = { 0 };                                    \
        snprintf(buf, sizeof(buf), log_msg_format, ##__VA_ARGS__); \
        logger.log(buf);                                           \
    } while (0)

#define QIAN_LOG_DEBUG(log_msg_format, ...)                        \
    do {                                                           \
        Logger& logger = Logger::getInstance();                    \
        logger.setLogLevel(LogLevel::DEBUG);                       \
        char buf[1024] = { 0 };                                    \
        snprintf(buf, sizeof(buf), log_msg_format, ##__VA_ARGS__); \
        logger.log(buf);                                           \
    } while (0)

namespace qian {
enum class LogLevel {
    DEBUG,
    INFO,
    ERROR,
    FATAL
};

class Logger : Noncopyable {
public:
    static Logger& getInstance();
    void setLogLevel(LogLevel level)
    {
        loglevel_ = level;
    };
    void log(std::string message);

private:
    LogLevel loglevel_;
};

} // namespace qian