#include "../gfc-logger-system/logger.hh"

#include <memory>
#include <thread>
#include <iostream>

int main() {
    std::cout << "Hello, World!" << std::endl;

    
    // gfc::LogFormatter::ptr fmtter = std::make_shared<gfc::LogFormatter>(
    //     "[TEST-LOGGER] %d{%Y-%m-%d %H:%M:%S} [%p] [%c] [%t] [%f:%l] %m%n");



    auto logger = std::make_shared<gfc::Logger>("test_logger");
    logger->set_formatter(std::make_shared<gfc::LogFormatter>("[TEST-LOGGER] %d{%Y-%m-%d %H:%M:%S} [%p] [%c] [%t] [%f:%l] %m%n"));
    logger->add_appender(std::make_shared<gfc::StdoutLogAppender>());
    logger->set_level(gfc::LogLevel::FATAL);
    gfc::LoggerManager::get_instance().add_logger("test_logger", logger);

    auto root_logger = gfc::LoggerManager::get_instance().get_root_logger();
    root_logger->set_formatter(std::make_shared<gfc::LogFormatter>("[ROOT-LOGGER] %d{%Y-%m-%d %H:%M:%S} [%p] [%c] [%t] [%f:%l] %m%n"));
    root_logger->add_appender(std::make_shared<gfc::StdoutLogAppender>());

    // auto output_test = [&](gfc::LogLevel level) {
    //     gfc::LogEvent::ptr event = std::make_shared<gfc::LogEvent>(
    //         level, __FILE__, __LINE__, 0, 0, 0, time(nullptr), logger.get_name(), "test message"
    //     );
    //     root_logger->log(level, event);   
    //     logger.log(level, event);
    // };
        
    // output_test(gfc::LogLevel::DEBUG);
    // output_test(gfc::LogLevel::INFO);

    GFC_LOG_DEBUG(gfc::LoggerManager::get_instance().get_logger("test_logger")) << "hello, world";
    GFC_LOG_INFO(gfc::LoggerManager::get_instance().get_logger("test_logger")) << "hello, world";
    GFC_LOG_WARN(gfc::LoggerManager::get_instance().get_logger("test_logger")) << "hello, world";
    GFC_LOG_ERROR(gfc::LoggerManager::get_instance().get_logger("test_logger")) << "hello, world";
    GFC_LOG_FATAL(gfc::LoggerManager::get_instance().get_logger("test_logger")) << "hello, world";

    GFC_LOG_DEBUG(root_logger) << "this is root logger";
    GFC_LOG_INFO(root_logger);
    GFC_LOG_WARN(root_logger);
    GFC_LOG_ERROR(root_logger);
    GFC_LOG_FATAL(root_logger);

    return 0;
}