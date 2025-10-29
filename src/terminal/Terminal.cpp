#include "Terminal.h"
#include <algorithm>
#include <iostream>
#include <csignal>
#include <atomic>

namespace terminal {

namespace {
    std::atomic<bool> sigintReceived{false};

    extern "C" void terminalSigintHandler(int) {
        sigintReceived.store(true);
    }
}

void Terminal::setSendCallback(sendCallback cb) { sendCb = std::move(cb); }

void Terminal::setSignalCallback(signalCallback cb) { sigCb = std::move(cb); }

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
    should_shutdown = true;
}

void Terminal::runBlockingStdioLoop() {
    std::string line;
    auto prev = std::signal(SIGINT, terminalSigintHandler);
    
    log("INFO", "Terminal started, listening for input");

    while (true) {
        if (sigintReceived.load()) {
            sigintReceived.store(false);
            log("DEBUG", "Received SIGINT (Ctrl+C)");
            if (sigCb) sigCb(SIGINT);
            else if (sendCb) sendCb("\x03");
            
            // Check if shutdown was requested by the callback
            if (should_shutdown) {
                log("INFO", "Shutdown requested via signal");
                break;
            }
            continue;
        }

        if (!std::getline(std::cin, line)) {
            log("INFO", "Terminal input stream closed (EOF)");
            break; // EOF or error
        }
        
        log("DEBUG", "Received input line: " + line);
        if (sendCb) sendCb(line);
        
        // Check if shutdown was requested after processing command
        if (should_shutdown) {
            log("INFO", "Shutdown requested via command");
            break;
        }
    }

    std::signal(SIGINT, prev);
    log("INFO", "Terminal stopped");
}

} // namespace terminal