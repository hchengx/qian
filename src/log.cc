#include "qian/log.h"
#include <functional>

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

void LogEvent::format(const char *fmt, va_list al)
{
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if(len != -1) {
        m_content << std::string(buf, len);
        free(buf);
    }
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
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getContent();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem
{
public:
    LevelFormatItem(const std::string& str=""){}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << LogLevel::ToString(level);
    }
};


// %xxx %xxx{xxx} %%
void LogFormatter::init()
{
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string nstr;
    for(size_t i = 0; i < m_pattern.size(); i++) {
        if(m_pattern[i] != '%') {
            nstr.append(1, m_pattern[i]);
            continue;
        }

        if((i + 1) < m_pattern.size()) {
            if(m_pattern[i+1] == '%') {
                nstr.append(1, '%');
                continue;
            }
        }

        size_t n = i + 1;
        int fmt_status = 0;
        size_t fmt_begin = 0;
        std::string str;
        std::string fmt;

        while (n < m_pattern.size()) {
            if(!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{' && m_pattern[n] != '}')) {
                str = m_pattern.substr(i+1, n-i-1);
                break;
            }
            if(fmt_status == 0) {
                if(m_pattern[n] == '{') {
                    str = m_pattern.substr(i+1, n-i-1);
                    fmt_status = 1;
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            } else if(fmt_status == 1) {
                if(m_pattern[n] == '}') {
                    fmt = m_pattern.substr(fmt_begin+1, n-fmt_begin-1);
                    fmt_status = 0;
                    ++n;
                    break;
                }
            }
            ++n;
            if(n == m_pattern.size()) {
                if(str.empty()) {
                    str = m_pattern.substr(i+1);
                }
            }
        }
        if(fmt_status == 0) {
            if(!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr,std::string(), 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt,1));
            i = n - 1;
        } else if(fmt_status == 1) {
            std::cout << "pattern parse error" << m_pattern << " - " << m_pattern.substr(i) << std::endl;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        }
    }
    if(!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }
    static std::map<std::string, std::function<FormatItem::ptr(const std::string & str)> > s_format_items = {
        // TODO:
    };

    for(auto& i :vec) {
        if(std::get<2>(i) == 0){
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        }
    }

}


}   // namespace qian