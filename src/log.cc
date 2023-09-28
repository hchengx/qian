#include "qian/log.h"
#include <functional>
#include <stdarg.h>

namespace qian {

const char* LogLevel::ToString(LogLevel::Level level)
{
    switch (level) {
    case LogLevel::TRACE: return "TRACE";
    case LogLevel::DEBUG: return "DEBUG";
    case LogLevel::INFO: return "INFO";
    case LogLevel::WARN: return "WARN";
    case LogLevel::ERROR: return "ERROR";
    case LogLevel::FATAL: return "FATAL";
    default: return "Unknow";
    }
}


LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file,
                   int32_t line, uint32_t elapse, uint32_t threadId, uint32_t fiberId,
                   uint64_t time, const std::string& threadName)
    : m_file(file)
    , m_line(line)
    , m_elapse(elapse)
    , m_threadId(threadId)
    , m_fiberId(fiberId)
    , m_time(time)
    , m_logger(logger)
    , m_level(level)
{}

void LogEvent::format(const char* fmt, va_list al)
{
    char* buf = nullptr;
    int   len = vasprintf(&buf, fmt, al);
    if (len != -1) {
        m_content << std::string(buf, len);
        free(buf);
    }
}

void LogEvent::format(const char* fmt, ...)
{
    va_list al;
    va_start(al,fmt);
    format(fmt, al);
    va_end(al);
}

std::stringstream& LogEventWrap::getContentStream()
{
    return m_event->getContentStream();
}



LogEventWrap::~LogEventWrap()
{
    m_event->getLogger()->log(m_event->getLevel(), m_event);
}

LogFormatter::LogFormatter(const std::string& pattern)
    : m_pattern(pattern)
{
    init();
}

class MessageFormatItem : public LogFormatter::FormatItem
{
public:
    MessageFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level,
                LogEvent::ptr event) override
    {
        os << event->getContent();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem
{
public:
    LevelFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level,
                LogEvent::ptr event) override
    {
        os << LogLevel::ToString(level);
    }
};

class LineFormatItem : public LogFormatter::FormatItem
{
public:
    LineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level,
                LogEvent::ptr event) override
    {
        os << event->getLine();
    }
};

class StringFormatItem : public LogFormatter::FormatItem
{
public:
    StringFormatItem(const std::string& str)
        : m_string(str)
    {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level,
                LogEvent::ptr event) override
    {
        os << m_string;
    }

private:
    std::string m_string;
};

class TabFormatItem:public LogFormatter::FormatItem {
public:
    TabFormatItem(const std::string& str = "") {}
    void format(std::ostream & os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override 
    {
        os << "\t";
    }
};

class NewLineFormatItem: public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string& str = ""){}
    void format(std::ostream & os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override 
    {
        os << "\n";
    }
};

class ElapseFormatItem : public LogFormatter::FormatItem
{
public:
    ElapseFormatItem(const std::string& str = "") {}
    void format(std::ostream & os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override 
    {
        os << event->getElapse();
    }

};

class NameFormatItem : public LogFormatter::FormatItem
{
public:
    NameFormatItem(const std::string& str=""){}
    void format(std::ostream & os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override 
    {
        os << logger->getName();
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem
{
const static std::string default_format;
public:
    DateTimeFormatItem(const std::string& format = default_format )
        :m_format(format)
        {
            if(m_format.empty()) {
                m_format = default_format;
            }
        }
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }
private:
    std::string m_format;
};

const std::string DateTimeFormatItem::default_format = "%Y-%m-%d %H:%M:%S";

class FileFormatItem : public LogFormatter::FormatItem
{
public:
    FileFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level,
                LogEvent::ptr event) override
    {
        os << event->getFile();
    }
};

class FiberIdFormatItem: public LogFormatter::FormatItem
{
public:
    FiberIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->getFiberId();
    }

};

class ThreadIdFormatItem: public LogFormatter::FormatItem
{
public:
    ThreadIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->getThreadId();
    }

};



// %xxx
// %xxx{xxx} xxx in {xxx} is format
// %% escape %
void LogFormatter::init()
{
    // 状态机定义
    enum class ParseState
    {
        Normal,    // 正常状态
        Percent,   // 遇到了%
        Str,       // 遇到了%xxx
        Fmt        // 遇到了{xxx}
    };

    // tuple: <str, format, type>
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string                                            nstr;      // 普通字符串
    std::string                                            fmt;       // 格式化字符串
    std::string                                            pattern;   // 格式化类型

    // 状态机解析
    ParseState state = ParseState::Normal;

    for (auto i : m_pattern) {
        switch (state) {
        case ParseState::Normal:
            if (i == '%') {
                state = ParseState::Percent;
            }
            else {
                nstr.append(1, i);
            }
            break;
        case ParseState::Percent:
            if (i == '%') {
                state = ParseState::Normal;
                nstr.append(1, i);
            }
            else {
                state = ParseState::Str;
                if (!nstr.empty()) {
                    vec.push_back(std::make_tuple(nstr, "", 0));
                    nstr.clear();
                }
                if (isalpha(i)) {
                    pattern.push_back(i);
                }
            }
            break;

        case ParseState::Str:
            if (isalpha(i)) {
                pattern.append(1, i);
            }
            else if (i == '{') {
                state = ParseState::Fmt;
            }
            else {
                if (!pattern.empty()) {
                    vec.push_back(std::make_tuple(pattern, "", 1));
                    pattern.clear();
                }
                else {
                    vec.push_back(std::make_tuple("<pattern_error>", "", 0));
                }
                if (i == '%')
                    state = ParseState::Percent;
                else {
                    state = ParseState::Normal;
                    nstr.append(1, i);
                }
            }
            break;

        case ParseState::Fmt:
            if (i == '}') {
                vec.push_back(std::make_tuple(pattern, fmt, 1));
                pattern.clear();
                fmt.clear();
                state = ParseState::Normal;
            }
            else {
                fmt.append(1, i);
            }
            break;
        }
    }
    if (state == ParseState::Normal) {
        if (!nstr.empty()) {
            vec.push_back(std::make_tuple(nstr, "", 0));
        }
    }
    else if (state == ParseState::Str) {
        if (!pattern.empty()) {
            vec.push_back(std::make_tuple(pattern, "", 1));
        }
        else {
            vec.push_back(std::make_tuple("<pattern_error>", "", 0));
        }
    }
    else {
        vec.push_back(std::make_tuple("<pattern_error>", fmt, 0));
    }
    // 测试
    // for (auto& i : vec) {
    //     std::cout << std::get<0>(i) << " - " << std::get<1>(i) << " - " << std::get<2>(i)
    //               << std::endl;
    // }

    static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)>>
        s_format_items = {
#define QIAN_XX(str, c)                                                          \
    {                                                                            \
        #str, [](const std::string& fmt) { return FormatItem::ptr(new c(fmt)); } \
    }

            QIAN_XX(m, MessageFormatItem),
            QIAN_XX(p, LevelFormatItem),
            QIAN_XX(r, ElapseFormatItem),
            QIAN_XX(c, NameFormatItem),
            QIAN_XX(t, ThreadIdFormatItem),
            QIAN_XX(n, NewLineFormatItem),
            QIAN_XX(d, DateTimeFormatItem),
            QIAN_XX(f, FileFormatItem),
            QIAN_XX(l, LineFormatItem),
            QIAN_XX(T, TabFormatItem),
            QIAN_XX(F, FiberIdFormatItem)};
#undef QIAN_XX

    for (auto& i : vec) {
        if (std::get<2>(i) == 0) {
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        }
        else {
            auto it = s_format_items.find(std::get<0>(i));
            if (it == s_format_items.end()) {
                m_items.push_back(FormatItem::ptr(
                    new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
            }
            else {
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }
    }
}

std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    std::stringstream ss;
    for(auto& i : m_items) {
        i->format(ss, logger, level, event);
    }
    return ss.str();
}

void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    if (level >= m_level) {
        std::cout << m_formatter->format(logger, level, event);
    }
}

FileLogAppender::FileLogAppender(const std::string& filename)
    :m_filename(filename)
    {
        reopen();
    }

void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    if(level >= m_level){
        m_filestream << m_formatter->format(logger, level, event);
    }
}

bool FileLogAppender::reopen()
{
    if(m_filestream) {
        m_filestream.close();
    }

    m_filestream.open(m_filename, std::ios::app);
    return !!m_filestream;
}


Logger::Logger(const std::string& name)
    : m_name(name)
    , m_level(LogLevel::DEBUG)
{
    // 默认格式字符串
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
}

void Logger::addAppender(LogAppender::ptr appender)
{
    // 如果appender没有格式字符串，则设置为默认
    if (!appender->getFormatter()) {
        appender->setFormatter(m_formatter);
    }
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender)
{
    for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it) {
        if (*it == appender) {
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event)
{
    if (level >= m_level) {
        auto self = shared_from_this();
        for (auto& i : m_appenders) {
            i->log(self, level, event);
        }
    }
}

void Logger::debug(LogEvent::ptr e)
{
    log(LogLevel::DEBUG, e);
}

void Logger::info(LogEvent::ptr e)
{
    log(LogLevel::INFO, e);
}

void Logger::warn(LogEvent::ptr e)
{
    log(LogLevel::WARN, e);
}

void Logger::error(LogEvent::ptr e)
{
    log(LogLevel::ERROR, e);
}

void Logger::fatal(LogEvent::ptr e)
{
    log(LogLevel::FATAL, e);
}

LoggerManager::LoggerManager()
{
    m_root.reset(new Logger);
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
}

Logger::ptr LoggerManager::getLogger(const std::string& name)
{
    auto it = m_loggers.find(name);
    return it == m_loggers.end() ? m_root : it->second;
}

}   // namespace qian