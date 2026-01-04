#pragma once

#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace common {

class TimeUtils {
public:
    enum class Format {
        DateTimeSeconds,      // "YYYY-MM-DD HH:MM:SS"
        DateTimeMilliseconds  // "YYYY-MM-DD HH:MM:SS.mmm"
    };
    
    /**
     * Format a time point as a string
     * @param tp The time point to format
     * @param format The desired output format
     * @return Formatted time string
     */
    static std::string format(const std::chrono::system_clock::time_point& tp, 
                             Format format = Format::DateTimeSeconds) {
        std::time_t t = std::chrono::system_clock::to_time_t(tp);
        std::tm local{};
        
#ifdef _WIN32
        localtime_s(&local, &t);
#else
        localtime_r(&t, &local);
#endif
        
        std::ostringstream oss;
        oss << std::put_time(&local, "%Y-%m-%d %H:%M:%S");
        
        if (format == Format::DateTimeMilliseconds) {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                tp.time_since_epoch()) % 1000;
            oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        }
        
        return oss.str();
    }
    
    /**
     * Get current time formatted as a string
     * @param format The desired output format
     * @return Formatted current time string
     */
    static std::string now(Format format = Format::DateTimeSeconds) {
        return TimeUtils::format(std::chrono::system_clock::now(), format);
    }
};

} // namespace common
