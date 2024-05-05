#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <vector>
#include <cinttypes>
#include <unordered_map>
#include <memory>

namespace gfc {

#define GFC_LOG_LEVEL(logger, level) \
    if (level >= logger->get_level()) \
    gfc::LogEventWrap(logger, std::make_shared<gfc::LogEvent>( \
        level, __FILE__, __LINE__, 0, 0, 0, time(nullptr), logger->get_name(), "" \
    )).get_ss()

#define GFC_LOG_DEBUG(logger)   GFC_LOG_LEVEL(logger, gfc::LogLevel::DEBUG)
#define GFC_LOG_INFO(logger)    GFC_LOG_LEVEL(logger, gfc::LogLevel::INFO)
#define GFC_LOG_WARN(logger)    GFC_LOG_LEVEL(logger, gfc::LogLevel::WARN)
#define GFC_LOG_ERROR(logger)   GFC_LOG_LEVEL(logger, gfc::LogLevel::ERROR)
#define GFC_LOG_FATAL(logger)   GFC_LOG_LEVEL(logger, gfc::LogLevel::FATAL)

enum class LogLevel {
    UNKNOW  = 0,
    DEBUG   = 1,
    INFO    = 2,
    WARN    = 3,
    ERROR   = 4,
    FATAL   = 5
};

class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr;
public:
    LogEvent();
    LogEvent(   LogLevel level, 
                const char* file_name, int32_t line_num, 
                uint32_t thread_id, uint32_t coroutine_id, 
                uint32_t elapse, time_t time, 
                const std::string& logger_name,
                const std::string& content);
    
    const char* get_file_name()     const { return m_file_name; }
    int32_t     get_line_num()      const { return m_line_num; }
    uint32_t    get_thread_id()     const { return m_thread_id; }
    uint32_t    get_coroutine_id()  const { return m_coroutine_id; }
    uint32_t    get_elapse()        const { return m_elapse; }
    time_t      get_time()          const { return m_time; }
    LogLevel    get_level()         const { return m_level; }
    std::string get_logger_name()   const { return m_logger_name; }
    std::string get_content()       const { return m_content; }
    std::string get_level_str() const;

    void        set_content(const std::string& content) { m_content = content; }
private:
    LogLevel    m_level;                // log level
    const char* m_file_name = nullptr;  // file name
    int32_t     m_line_num = 0;         // line number
    uint32_t    m_thread_id = 0;        // thread id
    uint32_t    m_coroutine_id = 0;     // coroutine id
    uint32_t    m_elapse = 0;           // elipse time
    time_t      m_time;                 // UTC time
    std::string m_logger_name;          // logger name
    std::string m_content;              // log content

};

/* ------------ LogFormatter ------------ */

class LogFormatter {
public:
    typedef std::shared_ptr<LogFormatter> ptr;
public:
    LogFormatter(const std::string& pattern = "%d{%Y-%m-%d %H:%M:%S} [%p] [%c] [%t] [%f:%l] %m%n"); 
    std::string format(LogEvent::ptr event);
    void        init();
public:
    class FormatItem {
    public:
        typedef std::shared_ptr<FormatItem> ptr;
    public:
        virtual ~FormatItem() {}
        virtual void format(std::ostream& os, LogEvent::ptr event) = 0;
    };

private:
    std::string m_pattern;                  // log pattern
    bool m_error = false;                   // parse error
    std::vector<FormatItem::ptr> m_items;   // format items, store the parsed pattern
};

/* ------------ LogAppender ------------ */

class LogAppender {
public:
    typedef std::shared_ptr<LogAppender> ptr;

public:
    virtual ~LogAppender() {}
    
    virtual void        log(LogEvent::ptr event) = 0;
    LogFormatter::ptr   get_formatter() const { return m_formatter; }
    void                set_formatter(LogFormatter::ptr formatter) { m_formatter = formatter; }

protected:
    LogFormatter::ptr   m_formatter;
};

class StdoutLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;

public:
    virtual void log(LogEvent::ptr event) override;

private:

};

class FileLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<FileLogAppender> ptr;

public:
    FileLogAppender(const std::string& filename);
    virtual void log(LogEvent::ptr event) override;
    bool reopen();

private:
    std::string     m_filename;
    std::ofstream   m_filestream;
};


/* ------------ Logger ------------ */

class Logger {
public:
    typedef std::shared_ptr<Logger> ptr;
    
public:
    Logger(const std::string& name = "root");

    void log(LogLevel level, LogEvent::ptr event);

    void debug  (LogEvent::ptr event);
    void info   (LogEvent::ptr event);
    void warn   (LogEvent::ptr event);
    void error  (LogEvent::ptr event);
    void fatal  (LogEvent::ptr event);

    void add_appender(LogAppender::ptr appender);
    void del_appender(LogAppender::ptr appender);

    const std::string&  get_name()  const           { return m_name; }
    LogLevel            get_level() const           { return m_level; }
    void                set_level(LogLevel level)   { m_level = level; }
    void                set_formatter(LogFormatter::ptr formatter) { m_formatter = formatter; }

private:
    std::string                 m_name;         // logger name
    LogLevel                    m_level;        // logger level
    std::list<LogAppender::ptr> m_appenders;    // appenders
    LogFormatter::ptr           m_formatter;    // formatter
};

/* ------------ LoggerManager ------------ */

// Use Singleton pattern
class LoggerManager {
public:
    typedef std::shared_ptr<LoggerManager> ptr;
public:
    static LoggerManager&   get_instance();
    Logger::ptr             get_logger(const std::string& name) { return m_loggers[name];}
    Logger::ptr             get_root_logger()                   { return m_root_logger;  }
    void                    add_logger(const std::string& name, Logger::ptr logger);
private:
    LoggerManager();
private:
    Logger::ptr m_root_logger;
    std::unordered_map<std::string, Logger::ptr> m_loggers;
};

/* ------------ LogEventWrap ------------ */

class LogEventWrap {
public:
    LogEventWrap() = delete;
    LogEventWrap(Logger::ptr logger, LogEvent::ptr event);
    ~LogEventWrap();
    std::stringstream& get_ss() { return m_ss; }
private:
    Logger::ptr         m_logger;
    LogEvent::ptr       m_event;
    std::stringstream   m_ss;
};

} // namespace gfc

