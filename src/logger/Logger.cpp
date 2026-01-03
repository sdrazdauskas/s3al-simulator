#include "logger/Logger.h"
#include "common/ColorUtils.h"
#include <iostream>
#include <map>

namespace {
    using namespace common;
    
    std::string getLevelColor(logging::LogLevel level) {
        static const std::map<logging::LogLevel, const char*> colorMap = {
            {logging::LogLevel::DEBUG,   ColorUtils::CYAN},
            {logging::LogLevel::INFO,    ColorUtils::GREEN},
            {logging::LogLevel::WARNING, ColorUtils::YELLOW},
            {logging::LogLevel::ERROR,   ColorUtils::RED}
        };
        
        auto it = colorMap.find(level);
        return (it != colorMap.end()) ? it->second : ColorUtils::RESET;
    }
}

namespace logging {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::init(const std::string& fileName, LogLevel minLevel) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (file.is_open()) {
        file.close();
    }

    file.open(fileName, std::ios::out | std::ios::app);
    this->minLevel = minLevel;

    if (file.is_open()) {
        file << getCurrentTime() << " "
             << "[INFO] [LOGGER] Logger initialized: " << fileName << "\n";
        file.flush();
    } else {
        std::cerr << "ERROR: Failed to open log file: " << fileName << std::endl;
    }
}

Logger::~Logger() {
    if (file.is_open()) {
        file.flush();
        file.close();
    }
}

void Logger::log(LogLevel level, const std::string& module, const std::string& message) {
    if (level < minLevel) return;

    std::lock_guard<std::mutex> lock(mutex);

    std::string log_entry = getCurrentTime() + " " +
                            "[" + levelToString(level) + "] " +
                            "[" + module + "] " +
                            message;

    if (file.is_open()) {
        file << log_entry << "\n";
        file.flush();
    }

    if (logToConsole) {
        // Call callback before output to clear the current input line
        if (consoleOutputCallback) {
            consoleOutputCallback(true);  // true = before log
        }
        
        // Colored console output
        std::string levelColor = getLevelColor(level);
        std::string moduleColor = common::ColorUtils::getColorForString(module);
        std::string coloredModule = moduleColor + "[" + module + "]" + common::ColorUtils::RESET;
        
        std::cerr << getCurrentTime() << " "
                  << levelColor << common::ColorUtils::BOLD << "[" << levelToString(level) << "]" 
                  << common::ColorUtils::RESET << " "
                  << coloredModule << " "
                  << message << std::endl;
        
        // Call callback after output to redraw the prompt
        if (consoleOutputCallback) {
            consoleOutputCallback(false);  // false = after log
        }
    }
}

void Logger::log(const std::string& level, const std::string& module, const std::string& message) {
    static const std::map<std::string, LogLevel> level_map = {
        {"DEBUG", LogLevel::DEBUG},
        {"INFO", LogLevel::INFO},
        {"WARNING", LogLevel::WARNING},
        {"WARN", LogLevel::WARNING},
        {"ERROR", LogLevel::ERROR}
    };
    
    auto it = level_map.find(level);
    LogLevel log_level = (it != level_map.end()) ? it->second : LogLevel::INFO;
    
    log(log_level, module, message);
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(mutex);
    if (file.is_open()) {
        file.flush();
    }
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

std::string Logger::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

} // namespace logging