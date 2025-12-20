#pragma once

#include <functional>
#include <thread>
#include <atomic>
#include <string>
#include <mutex>
#include "common/LoggingMixin.h"

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

    // Called by shell to display output (writes directly to stdout)
    void print(const std::string& output);
    
    // Signal the terminal to stop
    void requestShutdown();

private:
    sendCallback sendCb;
    signalCallback sigCb;
    promptCallback promptCb;
    std::atomic<bool> shouldShutdown{false};
    std::thread terminalThread;
    
    // State for redrawing prompt after external output (e.g., logs)
    std::mutex inputMutex;
    std::string currentBuffer;
    size_t currentCursor{0};
    std::atomic<bool> isReadingInput{false};
    
    void redrawPrompt();
    void clearCurrentLine();
    void updateInputState(const std::string& buffer, size_t cursor);
    void displayBuffer(const std::string& buffer, size_t cursor);
    
    // Input handlers
    bool handleBackspace(std::string& buffer, size_t& cursor);
    bool handleHistoryNavigation(char key, History& history, std::string& buffer, size_t& cursor);
    bool handleCursorMovement(char key, size_t& cursor, size_t bufferSize);
    void handleCharInput(char c, std::string& buffer, size_t& cursor);

protected:
    std::string getModuleName() const override { return "TERMINAL"; }
};

} // namespace terminal
