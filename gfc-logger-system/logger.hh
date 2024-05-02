#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <cinttypes>
#include <memory>

namespace gfc {


enum class LogLevel {
    DEBUG   = 1,
    INFO    = 2,
    WARN    = 3,
    ERROR   = 4,
    FATAL   = 5
};

// 日志事件
class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr;
public:
    LogEvent();
private:
    const char* m_file_name = nullptr;  // file name
    int32_t     m_line_num = 0;         // line number
    uint32_t    m_thread_id = 0;        // thread id
    uint32_t    m_coroutine_id = 0;     // coroutine id
    uint32_t    m_elipse = 0;           // elipse time
    uint64_t    m_time;                 // time
    std::string m_content;              // log content
};


// 日志格式器
class LogFormatter {
public:
    typedef std::shared_ptr<LogFormatter> ptr;
public:
    LogFormatter(const std::string& pattern); 
    std::string format(const LogEvent& event);
private:
    std::string m_pattern;

};


// 日志输出
class LogAppender {
public:
    typedef std::shared_ptr<LogAppender> ptr;

public:
   
    virtual ~LogAppender() {}

    virtual void log(LogLevel level, const LogEvent& event) = 0;

    LogLevel get_level() const { return m_level; }
    void set_level(LogLevel level) { m_level = level; }
    
    LogFormatter::ptr get_formatter() const { return m_formatter; }
    void set_formatter(LogFormatter::ptr formatter) { m_formatter = formatter; }

protected:
    LogLevel            m_level;
    LogFormatter::ptr   m_formatter;
};


// 日志器
class Logger {
public:
    typedef std::shared_ptr<Logger> ptr;
    
public:
    Logger(const std::string& name = "root");

    void log(LogLevel level, const LogEvent& event);

    void debug(const LogEvent& event);
    void info(const LogEvent& event);
    void warn(const LogEvent& event);
    void error(const LogEvent& event);
    void fatal(const LogEvent& event);

    void add_appender(LogAppender::ptr appender);
    void del_appender(LogAppender::ptr appender);

    LogLevel get_level() const { return m_level; }
    void set_level(LogLevel level) { m_level = level; }

private:
    std::string                 m_name;         // logger name
    LogLevel                    m_level;        // logger level
    std::list<LogAppender::ptr> m_appenders;    // appenders
};

class StdoutLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;

public:
    virtual void log(LogLevel level, const LogEvent& event) override;

private:

};

class FileLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<FileLogAppender> ptr;

public:
    FileLogAppender(const std::string& filename);
    virtual void log(LogLevel level, const LogEvent& event) override;
    bool reopen();

private:
    std::string     m_filename;
    std::ofstream   m_filestream;
};







} // namespace gfc

