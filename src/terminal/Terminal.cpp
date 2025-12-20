#include "terminal/Terminal.h"
#include <algorithm>
#include <iostream>
#include <csignal>
#include <unistd.h>
#include "terminal/helper/History.h"
#include "terminal/helper/Input.h"
#include "logger/Logger.h"

namespace terminal {

// Anonymous namespace for helpers
namespace {
    std::atomic<bool> sigintReceived{false};
    // Store the signal callback globally so signal handler can access it
    std::function<void(int)>* signalCallback = nullptr;
    // Store pointer to active terminal for redraw callback
    Terminal* activeTerminal = nullptr;

    extern "C" void terminalSigintHandler(int sig) {
        sigintReceived.store(true);
        // Call the callback immediately from signal handler
        // This is safe if the callback only sets atomic flags
        if (signalCallback && *signalCallback) {
            (*signalCallback)(sig);
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
    ::terminal::signalCallback = &sigCb;
}

void Terminal::setPromptCallback(promptCallback cb) { 
    input.setPromptCallback(std::move(cb)); 
}

void Terminal::print(const std::string& output) {
    std::cout << output << std::flush;
}

void Terminal::requestShutdown() {
    shouldShutdown.store(true);
}

void Terminal::start() {
    logInfo("Starting terminal thread");
    shouldShutdown.store(false);
    terminalThread = std::thread([this]() {
        this->runBlockingStdioLoop();
    });
}

void Terminal::stop() {
    logInfo("Stopping terminal thread");
    shouldShutdown.store(true);
}

void Terminal::join() {
    if (terminalThread.joinable()) {
        logInfo("Waiting for terminal thread to finish");
        terminalThread.join();
        logInfo("Terminal thread finished");
    }
}

void Terminal::runBlockingStdioLoop() {
    std::string line;
    auto prev = std::signal(SIGINT, terminalSigintHandler);

    logInfo("Terminal started, listening for input");

    // Register redraw callback with Logger
    activeTerminal = this;
    logging::Logger::getInstance().setConsoleOutputCallback([this](bool before) {
        if (before) {
            input.clearLine();
        } else {
            input.redraw();
        }
    });

    History history;
    history.enableRawMode();

    while (!shouldShutdown.load()) {
        if (sigintReceived.load()) {
            sigintReceived.store(false);
            logDebug("Received SIGINT (Ctrl+C)");
            if (shouldShutdown.load()) break;
            continue;
        }

        // Display prompt
        std::cout << "\r";
        input.display("", 0);

        std::string buffer;
        size_t cursor = 0;
        
        input.startReading();

        while (true) {
            char c;
            if (read(STDIN_FILENO, &c, 1) <= 0) {
                shouldShutdown.store(true);
                break;
            }

            if (c == '\n' || c == '\r') {
                input.stopReading();
                std::cout << "\n";
                history.add(buffer);
                
                // Restore cooked mode before executing command
                // so commands can use std::getline() normally
                history.temporarilyRestoreMode();
                if (sendCb) sendCb(buffer);
                // Return to raw mode for next input
                history.temporarilyEnableRawMode();
                break;
            }

            if (c == 127 || c == 8) {
                input.handleBackspace(buffer, cursor);
                continue;
            }

            if (c == '\x1b') {
                char seq[2];
                if (read(STDIN_FILENO, &seq[0], 1) <= 0) continue;
                if (read(STDIN_FILENO, &seq[1], 1) <= 0) continue;

                if (seq[0] == '[') {
                    if (history.navigate(seq[1], buffer, cursor)) {
                        input.update(buffer, cursor);
                        input.display(buffer, cursor);
                    } else {
                        input.handleCursorMovement(seq[1], cursor, buffer.size());
                    }
                }
                continue;
            }

            input.handleCharInput(c, buffer, cursor);
        }

        if (shouldShutdown.load()) break;
    }

    // Cleanup
    input.stopReading();
    activeTerminal = nullptr;
    logging::Logger::getInstance().setConsoleOutputCallback(nullptr);
    
    std::signal(SIGINT, prev);
    logInfo("Terminal stopped");
}

} // namespace terminal
