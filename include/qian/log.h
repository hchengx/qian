#ifndef __QIAN_LOG_H__
#define __QIAN_LOG_H__

#include "qian/singleton.h"
#include "qian/util.h"

#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>

#define QIAN_LOG_LEVEL(logger, level)                                              \
    if (logger->getLevel() <= level)                                               \
    qian::LogEventWrap(qian::LogEvent::ptr(new qian::LogEvent(logger,              \
                                                              level,               \
                                                              __FILE__,            \
                                                              __LINE__,            \
                                                              0,                   \
                                                              qian::GetThreadId(), \
                                                              qian::GetFiberId(),  \
                                                              time(0),             \
                                                              "")))                \
        .getContentStream()

#define QIAN_LOG_DEBUG(logger) QIAN_LOG_LEVEL(logger, qian::LogLevel::DEBUG)
#define QIAN_LOG_INFO(logger) QIAN_LOG_LEVEL(logger, qian::LogLevel::INFO)
#define QIAN_LOG_WARN(logger) QIAN_LOG_LEVEL(logger, qian::LogLevel::WARN)
#define QIAN_LOG_ERROR(logger) QIAN_LOG_LEVEL(logger, qian::LogLevel::ERROR)
#define QIAN_LOG_FATAL(logger) QIAN_LOG_LEVEL(logger, qian::LogLevel::FATAL)

#define QIAN_LOG_ROOT() qian::LoggerMgr::GetInstance()->getRoot()

namespace qian {

class Logger;

class LogLevel
{
public:
    enum Level
    {
        TRACE = 0,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS,
    };

    static const char* ToString(LogLevel::Level level);
};

class LogEvent
{
public:
    typedef std::shared_ptr<LogEvent> ptr;

public:
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, int32_t line,
             uint32_t elapse, uint32_t threadId, uint32_t fiberId, uint64_t time,
             const std::string& threadName = "");

public:
    const char*             getFile() const { return m_file; }
    int32_t                 getLine() const { return m_line; }
    u_int32_t               getElapse() const { return m_elapse; }
    u_int32_t               getThreadId() const { return m_threadId; }
    u_int32_t               getFiberId() const { return m_fiberId; }
    u_int64_t               getTime() const { return m_time; }
    std::string             getContent() const { return m_content.str(); }
    std::shared_ptr<Logger> getLogger() const { return m_logger; }
    LogLevel::Level         getLevel() const { return m_level; }

    std::stringstream& getContentStream() { return m_content; }
    void               format(const char* fmt, ...);
    void               format(const char* fmt, va_list al);

private:
    const char*       m_file     = nullptr;   // 文件名
    int32_t           m_line     = 0;         // 行号
    uint32_t          m_elapse   = 0;         // 程序启动开始到现在的毫秒数
    uint32_t          m_threadId = 0;         // 线程id
    uint32_t          m_fiberId  = 0;         // 协程id
    uint64_t          m_time     = 0;         // 时间戳
    std::stringstream m_content;              // 日志内容

    std::shared_ptr<Logger> m_logger;   // 日志器
    LogLevel::Level         m_level;    // 日志等级
};

// 用于包装LogEvent，通过析构函数将LogEvent写入日志器
class LogEventWrap
{
public:
    LogEventWrap(LogEvent::ptr e);
    ~LogEventWrap();   // 析构函数将LogEvent写入日志器
    LogEvent::ptr      getEvent() const { return m_event; }
    std::stringstream& getContentStream();

private:
    LogEvent::ptr m_event;
};

class LogFormatter
{
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string& pattern);
    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);

public:
    class FormatItem
    {
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem() {}
        virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level,
                            LogEvent::ptr event) = 0;
    };
    void init();

private:
    std::string                  m_pattern;
    std::vector<FormatItem::ptr> m_items;
};

class LogAppender
{
public:
    typedef std::shared_ptr<LogAppender> ptr;
    virtual ~LogAppender() {}

    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
                     LogEvent::ptr event) = 0;

    void              setFormatter(LogFormatter::ptr val) { m_formatter = val; }
    LogFormatter::ptr getFormatter() const { return m_formatter; }

    LogLevel::Level getLevel() const { return m_level; }
    void            setLevel(LogLevel::Level level) { m_level = level; }

protected:
    LogLevel::Level   m_level = LogLevel::DEBUG;
    LogFormatter::ptr m_formatter;
};

class StdoutLogAppender : public LogAppender
{
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;
};

class FileLogAppender : public LogAppender
{
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;

    bool reopen();

private:
    std::string   m_filename;
    std::ofstream m_filestream;
};



// 日志器
class Logger : public std::enable_shared_from_this<Logger>
{
public:
    typedef std::shared_ptr<Logger> ptr;

public:
    Logger(const std::string& name = "root");
    void log(LogLevel::Level level, const LogEvent::ptr event);

    LogLevel::Level    getLevel() const { return m_level; }
    void               setLevel(LogLevel::Level level) { m_level = level; }
    const std::string& getName() const { return m_name; }


private:
    std::string                 m_name;
    LogLevel::Level             m_level;
    std::list<LogAppender::ptr> m_appenders;
};

class LoggerManager
{
public:
    LoggerManager();
    Logger::ptr getLogger(const std::string& name);
    void        init();
    Logger::ptr getRoot() const { return m_root; }

private:
    std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr                        m_root;
};

typedef qian::SingletonPtr<LoggerManager> LoggerMgr;

}   // namespace qian


#endif