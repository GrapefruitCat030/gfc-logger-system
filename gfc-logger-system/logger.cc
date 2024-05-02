#include "logger.hh"

namespace gfc {

Logger::Logger(const std::string& name = "root") :
    m_name(name),
    m_level(LogLevel::DEBUG) {

}

void Logger::log(LogLevel level, const LogEvent& event) {
    if(level >= m_level) {
        for(auto& appender : m_appenders) {
            appender->log(level, event);
        }
    }
}
void Logger::debug(const LogEvent& event) {

}
void Logger::info(const LogEvent& event) {

}
void Logger::warn(const LogEvent& event) {

}
void Logger::error(const LogEvent& event) {

}
void Logger::fatal(const LogEvent& event) {

}

void Logger::add_appender(LogAppender::ptr appender) {
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


void StdoutLogAppender::log(LogLevel level, const LogEvent& event) {
    if(level >= m_level) {
        std::cout << m_formatter->format(event);
    }
}


FileLogAppender::FileLogAppender(const std::string& filename) :
    m_filename(filename) 
{
    m_filestream.open(filename);
}

bool FileLogAppender::reopen() {
    if(m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename);
    return static_cast<bool>(m_filestream);
}

void FileLogAppender::log(LogLevel level, const LogEvent& event) {
    if(level >= m_level) {
        m_filestream << m_formatter->format(event);
    }
}













} // namespace gfc