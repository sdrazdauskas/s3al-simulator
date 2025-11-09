#include "Terminal.h"
#include <algorithm>
#include <iostream>
#include <csignal>

namespace terminal {

namespace {
    std::atomic<bool> sigintReceived{false};
    // Store the signal callback globally so signal handler can access it
    std::function<void(int)>* g_signalCallback = nullptr;

    extern "C" void terminalSigintHandler(int sig) {
        sigintReceived.store(true);
        // Call the callback immediately from signal handler
        // This is safe if the callback only sets atomic flags
        if (g_signalCallback && *g_signalCallback) {
            (*g_signalCallback)(sig);
        }
    }
}

Terminal::~Terminal() {
    stop();
    join();
}

void Terminal::setSendCallback(sendCallback cb) { sendCb = std::move(cb); }

void Terminal::setSignalCallback(signalCallback cb) { 
    sigCb = std::move(cb);
    // Store pointer to callback for signal handler
    g_signalCallback = &sigCb;
}

void Terminal::setPromptCallback(promptCallback cb) { promptCb = std::move(cb); }

void Terminal::setLogCallback(LogCallback callback) {
    log_callback = callback;
}

void Terminal::log(const std::string& level, const std::string& message) {
    if (log_callback) {
        log_callback(level, "TERMINAL", message);
    }
}

void Terminal::print(const std::string& output) {
    std::cout << output << std::flush;
}

void Terminal::requestShutdown() {
    should_shutdown.store(true);
}

void Terminal::start() {
    log("INFO", "Starting terminal thread");
    should_shutdown.store(false);
    terminal_thread = std::thread([this]() {
        this->runBlockingStdioLoop();
    });
}

void Terminal::stop() {
    log("INFO", "Stopping terminal thread");
    should_shutdown.store(true);
}

void Terminal::join() {
    if (terminal_thread.joinable()) {
        log("INFO", "Waiting for terminal thread to finish");
        terminal_thread.join();
        log("INFO", "Terminal thread finished");
    }
}

void Terminal::runBlockingStdioLoop() {
    std::string line;
    auto prev = std::signal(SIGINT, terminalSigintHandler);
    
    log("INFO", "Terminal started, listening for input");

    while (!should_shutdown.load()) {
        if (sigintReceived.load()) {
            sigintReceived.store(false);
            log("DEBUG", "Received SIGINT (Ctrl+C) - already handled in signal handler");
            
            // Check if shutdown was requested by the callback
            if (should_shutdown.load()) {
                log("INFO", "Shutdown requested via signal");
                break;
            }
            continue;
        }

        // Print prompt if callback is set
        if (promptCb) {
            std::cout << promptCb() << std::flush;
        }
        
        if (!std::getline(std::cin, line)) {
            log("INFO", "Terminal input stream closed (EOF)");
            break; // EOF or error
        }
        
        log("DEBUG", "Received input line: " + line);
        if (sendCb) sendCb(line);
        
        // Check if shutdown was requested after processing command
        if (should_shutdown.load()) {
            log("INFO", "Shutdown requested via command");
            break;
        }
    }

    std::signal(SIGINT, prev);
    log("INFO", "Terminal stopped");
}

} // namespace terminal