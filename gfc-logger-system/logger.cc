#include "logger.hh"

#include <cassert>
#include <unordered_map>
#include <functional>

namespace gfc {

/* ------------ LogEvent ------------ */

LogEvent::LogEvent( LogLevel level, 
                    const char* file_name, int32_t line_num, 
                    uint32_t thread_id, uint32_t coroutine_id, 
                    uint32_t elapse, time_t time, 
                    const std::string& logger_name,
                    const std::string& content) 
                    : 
                    m_level(level), m_file_name(file_name), m_line_num(line_num),
                    m_thread_id(thread_id), m_coroutine_id(coroutine_id),
                    m_elapse(elapse), m_time(time), 
                    m_logger_name(logger_name), m_content(content) {}

std::string LogEvent::get_level_str() const {
    #define LOG_LEVEL_NAME_CASE(r) case LogLevel::r: return #r
    switch (m_level) {
        LOG_LEVEL_NAME_CASE(UNKNOW);
        LOG_LEVEL_NAME_CASE(DEBUG);
        LOG_LEVEL_NAME_CASE(INFO);
        LOG_LEVEL_NAME_CASE(WARN);
        LOG_LEVEL_NAME_CASE(ERROR);
        LOG_LEVEL_NAME_CASE(FATAL);
        default: return "UNKNOW";
    }
    #undef LOG_LEVEL_NAME_CASE
}


/* ------------ FormatItem ------------ */

class StringFormatItem :    public LogFormatter::FormatItem {
public:
    StringFormatItem(const std::string& str) : m_string(str) {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << m_string;
    }
private:
    std::string m_string;
};

class MessageFormatItem :   public LogFormatter::FormatItem {
public:
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->get_content();
    }
};

class LevelFormatItem :     public LogFormatter::FormatItem {
public:
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->get_level_str();
    }
};

class ElapseFormatItem :    public LogFormatter::FormatItem {
public:
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->get_elapse();
    }
};

class LoggerNameFormatItem: public LogFormatter::FormatItem {
public:
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->get_logger_name();
    }
};

class ThreadIdFormatItem :  public LogFormatter::FormatItem {
public:
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->get_thread_id();
    }
};

class CoroutineIdFormatItem:public LogFormatter::FormatItem {
public:
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->get_coroutine_id();
    }
};

class NewLineFormatItem :   public LogFormatter::FormatItem {
public:
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << std::endl;
    }
};

class DateTimeFormatItem :  public LogFormatter::FormatItem {
public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S") : m_format(format) {
        if(m_format.empty()) {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }
    void format(std::ostream& os, LogEvent::ptr event) override {
        struct tm tm;
        time_t time = event->get_time();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }
private:
    std::string m_format;
};

class FileNameFormatItem :  public LogFormatter::FormatItem {
public:
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->get_file_name();
    }
};

class LineFormatItem :      public LogFormatter::FormatItem {
public:
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->get_line_num();
    }
};

class TabFormatItem :       public LogFormatter::FormatItem {
public:
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << "\t";
    }
};

/* ------------ LogFormatter ------------ */

LogFormatter::LogFormatter(const std::string& pattern) : m_pattern(pattern) {
    init();
}

std::string LogFormatter::format(LogEvent::ptr event) {
    std::stringstream ss;
    for(auto& item : m_items) {
        item->format(ss, event);
    }
    return ss.str();
}

void LogFormatter::init() {

    // 按顺序存储解析到的pattern项
    // 每个pattern包括一个整数类型和一个字符串，类型为0表示该pattern是常规字符串，为1表示该pattern需要转义
    // 日期格式单独用下面的dataformat存储
    std::vector<std::pair<bool, std::string>> patterns;
    std::string tmp;            // 临时存储常规字符串
    std::string dateformat;     // 日期格式字符串，默认把位于%d后面的大括号对里的全部字符都当作格式字符，不校验格式是否合法
    bool error = false;         // 是否解析出错
    bool parsing_string = true; // 是否正在解析常规字符，初始时为true

    size_t i = 0;
    while(i < m_pattern.size()) {
        std::string c = std::string(1, m_pattern[i]);
        if(c == "%") {
           if(parsing_string) {
                if(!tmp.empty()) {
                    patterns.push_back(std::make_pair(false, tmp));
                    tmp.clear();
                }
                parsing_string = false; // 在解析常规字符时遇到%，表示开始解析模板字符
                i++;
                continue;
            } else {
                patterns.push_back(std::make_pair(true, c));
                parsing_string = true; // 在解析模板字符时遇到%，表示这里是一个%转义
                // parsing_pattern = false;
                i++;
                continue;
            }
        } else { // not %
            if(parsing_string) { // 持续解析常规字符直到遇到%，解析出的字符串作为一个常规字符串加入patterns
                tmp += c;
                i++;
                continue;
            } else { // 模板字符，直接添加到patterns中，添加完成后，状态变为解析常规字符，%d特殊处理
                patterns.push_back(std::make_pair(true, c));
                parsing_string = true; 
                i++;

                // 后面是对%d的特殊处理，如果%d后面直接跟了一对大括号，那么把大括号里面的内容提取出来作为dateformat
                if(c != "d") {
                    continue;
                }
                else {
                    if(i < m_pattern.size() && m_pattern[i] != '{') {
                        continue;
                    }
                    i++;
                    while( i < m_pattern.size() && m_pattern[i] != '}') {
                        dateformat.push_back(m_pattern[i]);
                        i++;
                    }
                    if(m_pattern[i] != '}') {
                        // %d后面的大括号没有闭合，直接报错
                        std::cout << "[ERROR] LogFormatter::init() " << "pattern: [" << m_pattern << "] '{' not closed" << std::endl;
                        error = true;
                        break;
                    }
                    i++;
                    continue;
                }
            }
        }
    } // end while(i < m_pattern.size())

    if(error) {
        m_error = true;
        return;
    }

    // 模板解析结束之后剩余的常规字符也要算进去
    if(!tmp.empty()) {
        patterns.push_back(std::make_pair(false, tmp));
        tmp.clear();
    }

    /*
        Log Format Parse:
        %m - message
        %p - level
        %r - program elapse time
        %c - class name
        %t - thread id
        %n - new line
        %d - time yyyy-mm-dd HH:MM:SS [ISO 8601]
        %f - file name
        %l - line number
        extends:
        %T - tab
    */

    auto createFormatItem = [](const std::string& str) -> FormatItem::ptr {

        #define CREATE_FORMAT_ITEM(format, FormatItemClass) \
            if (str == format) { \
                return std::make_shared<FormatItemClass>(); \
            }

        CREATE_FORMAT_ITEM("m", MessageFormatItem)
        CREATE_FORMAT_ITEM("p", LevelFormatItem)
        CREATE_FORMAT_ITEM("r", ElapseFormatItem)
        CREATE_FORMAT_ITEM("c", LoggerNameFormatItem)
        CREATE_FORMAT_ITEM("t", ThreadIdFormatItem)
        CREATE_FORMAT_ITEM("n", NewLineFormatItem)
        CREATE_FORMAT_ITEM("f", FileNameFormatItem)
        CREATE_FORMAT_ITEM("l", LineFormatItem) 
        CREATE_FORMAT_ITEM("T", TabFormatItem)

        return nullptr;

        #undef CREATE_FORMAT_ITEM

    };

    for(auto &[is_pattern, fmt_str] : patterns) {
        // common string
        if(is_pattern == false) {
            m_items.push_back(std::make_shared<StringFormatItem>(fmt_str));
        } 
        // pattern string
        else {
            if( fmt_str =="d") {
                m_items.push_back(std::make_shared<DateTimeFormatItem>(dateformat));
            } else {
                auto item = createFormatItem(fmt_str);
                if(item) {
                    m_items.push_back(item);
                } else {
                    std::cout <<"[ERROR] LogFormatter::init() " 
                                "pattern: [" << m_pattern << "] " << 
                                "unknown format item: " << fmt_str << std::endl;
                    error = true;
                    break;
                }
            }
        }
    } // end for loop

    if(error) {
        m_error = true;
        return;
    }

}


/* ------------ LogAppender ------------ */

void StdoutLogAppender::log(LogEvent::ptr event) {
    std::cout << m_formatter->format(event);
}

FileLogAppender::FileLogAppender(const std::string& filename) : m_filename(filename) {
    m_filestream.open(filename);
}

bool FileLogAppender::reopen() {
    if(m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename);
    return static_cast<bool>(m_filestream);
}

void FileLogAppender::log(LogEvent::ptr event) {
    m_filestream << m_formatter->format(event);
}

/* ------------ Logger ------------ */

Logger::Logger(const std::string& name) : m_name(name), m_level(LogLevel::DEBUG) {
    m_formatter = std::make_shared<LogFormatter>("%d{%Y-%m-%d %H:%M:%S} [%p] [%c] [%t] [%f:%l] %m%n");
}

void Logger::log(LogLevel level, LogEvent::ptr event) {
    if(level >= m_level) {
        for(auto& appender : m_appenders) {
            appender->log(event);
        }
    }
}
void Logger::debug(LogEvent::ptr event) {
    log(LogLevel::DEBUG, event);
}
void Logger::info(LogEvent::ptr event) {
    log(LogLevel::INFO, event);
}
void Logger::warn(LogEvent::ptr event) {
    log(LogLevel::WARN, event);
}
void Logger::error(LogEvent::ptr event) {
    log(LogLevel::ERROR, event);
}
void Logger::fatal(LogEvent::ptr event) {
    log(LogLevel::FATAL, event);
}

void Logger::add_appender(LogAppender::ptr appender) {
    appender->set_formatter(m_formatter);
    m_appenders.push_back(appender); 
}
void Logger::del_appender(LogAppender::ptr appender) {
    for(auto it = m_appenders.begin(); it != m_appenders.end(); ++it) {
        if(*it == appender) {
            m_appenders.erase(it);
            break;
        }
    }
}

/* ------------ LoggerManager ------------ */

LoggerManager::LoggerManager() {
    if(m_root_logger == nullptr) {
        m_root_logger = std::make_shared<Logger>();
    }
    m_loggers[m_root_logger->get_name()] = m_root_logger;
}

LoggerManager& LoggerManager::get_instance() {
    static LoggerManager instance;
    return instance;
}

void LoggerManager::add_logger(const std::string& name, Logger::ptr logger) {
    m_loggers[name] = logger;
}


/* ------------ LogEventWrap ------------ */

LogEventWrap::LogEventWrap(Logger::ptr logger, LogEvent::ptr event) : m_logger(logger), m_event(event) {
}
LogEventWrap::~LogEventWrap() {
    if(m_logger != nullptr && m_event != nullptr) {
        m_event->set_content(m_ss.str());
        m_logger->log(m_event->get_level(), m_event);
    }
}

} // namespace gfc