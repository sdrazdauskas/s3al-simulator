#pragma once

#include <functional>
#include <thread>
#include <atomic>
#include <string>
#include "common/LoggingMixin.h"
#include "terminal/helper/Input.h"

namespace terminal {

// Forward declaration
class History;

class Terminal : public common::LoggingMixin {
public:
    using sendCallback = std::function<void(const std::string&)>;
    using signalCallback = std::function<void(int)>;
    using promptCallback = std::function<std::string()>;

    Terminal() = default;
    ~Terminal();
    
    // Prevent copying
    Terminal(const Terminal&) = delete;
    Terminal& operator=(const Terminal&) = delete;

    // Set the send callback which will receive lines read from stdin.
    void setSendCallback(sendCallback cb);

    // Set the signal callback which will be called when an interrupt
    void setSignalCallback(signalCallback cb);
    
    // Set the prompt callback which will provide the prompt string
    void setPromptCallback(promptCallback cb);

    // Start the terminal in a separate thread
    void start();
    
    // Stop the terminal thread
    void stop();
    
    // Wait for the terminal thread to finish
    void join();

    void runBlockingStdioLoop();

    // Called by shell to display output
    void print(const std::string& output);
    
    // Signal the terminal to stop
    void requestShutdown();

private:
    sendCallback sendCb;
    signalCallback sigCb;
    std::atomic<bool> shouldShutdown{false};
    std::thread terminalThread;
    Input input;

protected:
    std::string getModuleName() const override { return "TERMINAL"; }
};

} // namespace terminal
