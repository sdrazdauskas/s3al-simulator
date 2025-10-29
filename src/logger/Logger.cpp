#include "Logger.h"
#include <iostream>

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

    if (!file.is_open()) return;

    file << getCurrentTime() << " "
         << "[" << levelToString(level) << "] "
         << "[" << module << "] "
         << message << "\n";
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