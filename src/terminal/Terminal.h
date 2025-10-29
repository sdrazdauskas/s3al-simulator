#pragma once

#include <memory>
#include <string>
#include <utility>

#include <functional>
#include <string>

namespace terminal {

class Terminal {
public:
    using sendCallback = std::function<void(const std::string&)>;
    using signalCallback = std::function<void(int)>;
    using LogCallback = std::function<void(const std::string& level, 
                                           const std::string& module, 
                                           const std::string& message)>;

    Terminal() = default;
    ~Terminal() = default;

    // Set the send callback which will receive lines read from stdin.
    void setSendCallback(sendCallback cb);

    // Set the signal callback which will be called when an interrupt
    void setSignalCallback(signalCallback cb);

    void setLogCallback(LogCallback callback);

    void runBlockingStdioLoop();

    // Called by shell to display output (writes directly to stdout)
    void print(const std::string& output);
    
    // Signal the terminal to stop
    void requestShutdown();

private:
    sendCallback sendCb;
    signalCallback sigCb;
    LogCallback log_callback;
    bool should_shutdown = false;

    void log(const std::string& level, const std::string& message);
};

} // namespace terminal
