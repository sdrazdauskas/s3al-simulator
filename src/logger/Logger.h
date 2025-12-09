#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <functional>

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
    
    void init(const std::string& fileName, LogLevel minLevel = LogLevel::INFO);
    void log(LogLevel level, const std::string& module, const std::string& message);
    void log(const std::string& level, const std::string& module, const std::string& message);
    void flush();
    
    void setConsoleOutput(bool enabled) { logToConsole = enabled; }
    bool getConsoleOutput() const { return logToConsole; }
    
    // Callback called before/after console output, allowing terminal to redraw prompt
    // Parameter: true = before log output (clear line), false = after log output (redraw prompt)
    using ConsoleOutputCallback = std::function<void(bool)>;
    void setConsoleOutputCallback(ConsoleOutputCallback cb) { consoleOutputCallback = std::move(cb); }
    
    // Disable copy/move, enforce existence of single instance
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
private:
    Logger() = default;
    ~Logger();
    
    std::ofstream file;
    LogLevel minLevel = LogLevel::INFO;
    std::mutex mutex;
    bool logToConsole = false;
    ConsoleOutputCallback consoleOutputCallback;
    
    std::string levelToString(LogLevel level);
    std::string getCurrentTime();
};

// Convenience macros
#define LOG_DEBUG(module, msg) logging::Logger::getInstance().log(logging::LogLevel::DEBUG, module, msg)
#define LOG_INFO(module, msg) logging::Logger::getInstance().log(logging::LogLevel::INFO, module, msg)
#define LOG_WARN(module, msg) logging::Logger::getInstance().log(logging::LogLevel::WARNING, module, msg)
#define LOG_ERROR(module, msg) logging::Logger::getInstance().log(logging::LogLevel::ERROR, module, msg)

} // namespace logging