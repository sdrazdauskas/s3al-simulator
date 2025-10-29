#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace logging {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static Logger& getInstance();
    
    void init(const std::string& filename, LogLevel min_level = LogLevel::INFO);
    void log(LogLevel level, const std::string& module, const std::string& message);
    void flush();
    
    // Disable copy/move, enforce existence of single instance
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
private:
    Logger() = default;
    ~Logger();
    
    std::ofstream file;
    LogLevel min_level = LogLevel::INFO;
    std::mutex mutex;
    
    std::string levelToString(LogLevel level);
    std::string getCurrentTime();
};

// Convenience macros
#define LOG_DEBUG(module, msg) logging::Logger::getInstance().log(logging::LogLevel::DEBUG, module, msg)
#define LOG_INFO(module, msg) logging::Logger::getInstance().log(logging::LogLevel::INFO, module, msg)
#define LOG_WARN(module, msg) logging::Logger::getInstance().log(logging::LogLevel::WARNING, module, msg)
#define LOG_ERROR(module, msg) logging::Logger::getInstance().log(logging::LogLevel::ERROR, module, msg)

} // namespace logging