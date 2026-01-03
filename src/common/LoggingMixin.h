#pragma once

#include <string>
#include <functional>
#include "logger/Logger.h"
#include "common/ColorUtils.h"

namespace common {

// Base class providing common logging functionality
// Classes that need logging can inherit from this mixin
class LoggingMixin {
public:
    using LogCallback = std::function<void(const std::string& level, 
                                           const std::string& module, 
                                           const std::string& message)>;

    void setLogCallback(LogCallback callback) { 
        logCallback = std::move(callback); 
    }

protected:
    // Log a message with the module name from derived class
    void log(const std::string& level, const std::string& message) {
        if (logCallback) {
            logCallback(level, getModuleName(), message);
        } else {
            // Default to logging directly to Logger if no callback is set
            // Use logColored with cached colored module name for performance
            logging::Logger::getInstance().log(level, getModuleName(), message);
        }
    }

    // Convenience methods for specific log levels
    void logDebug(const std::string& message) { log("DEBUG", message); }
    void logInfo(const std::string& message) { log("INFO", message); }
    void logWarn(const std::string& message) { log("WARN", message); }
    void logError(const std::string& message) { log("ERROR", message); }

    // Derived classes must implement this to provide their module name
    virtual std::string getModuleName() const = 0;

    virtual ~LoggingMixin() = default;

private:
    LogCallback logCallback;
};

} // namespace common
