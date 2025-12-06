#include "logger/Logger.h"
#include <iostream>
#include <map>

namespace logging {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::init(const std::string& filename, LogLevel min_level) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (file.is_open()) {
        file.close();
    }

    file.open(filename, std::ios::out | std::ios::app);
    this->min_level = min_level;

    if (file.is_open()) {
        file << getCurrentTime() << " "
             << "[INFO] [LOGGER] Logger initialized: " << filename << "\n";
        file.flush();
    } else {
        std::cerr << "ERROR: Failed to open log file: " << filename << std::endl;
    }
}

Logger::~Logger() {
    if (file.is_open()) {
        file.flush();
        file.close();
    }
}

void Logger::log(LogLevel level, const std::string& module, const std::string& message) {
    if (level < min_level) return;

    std::lock_guard<std::mutex> lock(mutex);

    std::string log_entry = getCurrentTime() + " " +
                            "[" + levelToString(level) + "] " +
                            "[" + module + "] " +
                            message;

    if (file.is_open()) {
        file << log_entry << "\n";
        file.flush();
    }

    if (log_to_console) {
        // Call callback before output to clear the current input line
        if (console_output_callback) {
            console_output_callback(true);  // true = before log
        }
        std::cerr << log_entry << std::endl;
        // Call callback after output to redraw the prompt
        if (console_output_callback) {
            console_output_callback(false);  // false = after log
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