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

// Convenience inline functions
inline void logDebug(const std::string& module, const std::string& msg) {
    Logger::getInstance().log(LogLevel::DEBUG, module, msg);
}

inline void logInfo(const std::string& module, const std::string& msg) {
    Logger::getInstance().log(LogLevel::INFO, module, msg);
}

inline void logWarn(const std::string& module, const std::string& msg) {
    Logger::getInstance().log(LogLevel::WARNING, module, msg);
}

inline void logError(const std::string& module, const std::string& msg) {
    Logger::getInstance().log(LogLevel::ERROR, module, msg);
}

} // namespace logging