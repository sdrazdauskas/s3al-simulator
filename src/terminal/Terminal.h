#pragma once

#include <functional>
#include <thread>
#include <atomic>
#include <string>
#include <mutex>

namespace terminal {

class Terminal {
public:
    using sendCallback = std::function<void(const std::string&)>;
    using signalCallback = std::function<void(int)>;
    using promptCallback = std::function<std::string()>;
    using LogCallback = std::function<void(const std::string& level, 
                                           const std::string& module, 
                                           const std::string& message)>;

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

    void setLogCallback(LogCallback callback);

    // Start the terminal in a separate thread
    void start();
    
    // Stop the terminal thread
    void stop();
    
    // Wait for the terminal thread to finish
    void join();

    void runBlockingStdioLoop();

    // Called by shell to display output (writes directly to stdout)
    void print(const std::string& output);
    
    // Signal the terminal to stop
    void requestShutdown();

private:
    sendCallback sendCb;
    signalCallback sigCb;
    promptCallback promptCb;
    LogCallback log_callback;
    std::atomic<bool> should_shutdown{false};
    std::thread terminal_thread;
    
    // State for redrawing prompt after external output (e.g., logs)
    std::mutex input_mutex;
    std::string current_buffer;
    size_t current_cursor{0};
    std::atomic<bool> is_reading_input{false};
    
    void redrawPrompt();
    void clearCurrentLine();
    void updateInputState(const std::string& buffer, size_t cursor);
    void displayBuffer(const std::string& buffer, size_t cursor);

    void log(const std::string& level, const std::string& message);
};
        using LogCallback = std::function<void(const std::string& level,
                                               const std::string& module,
                                               const std::string& message)>;

} // namespace terminal
