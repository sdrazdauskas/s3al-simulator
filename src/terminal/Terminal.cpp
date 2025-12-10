#include "terminal/Terminal.h"
#include <algorithm>
#include <iostream>
#include <csignal>
#include <termios.h>
#include <unistd.h>
#include "terminal/helper/History.h"
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

void Terminal::setPromptCallback(promptCallback cb) { promptCb = std::move(cb); }

void Terminal::setLogCallback(LogCallback callback) {
    logCallback = callback;
}

void Terminal::log(const std::string& level, const std::string& message) {
    if (logCallback) {
        logCallback(level, "TERMINAL", message);
    }
}

void Terminal::print(const std::string& output) {
    std::cout << output << std::flush;
}

void Terminal::redrawPrompt() {
    if (!isReadingInput.load()) return;
    
    std::lock_guard<std::mutex> lock(inputMutex);
    // Redraw prompt and current input on a fresh line
    std::cout << "\r\33[2K";  // Clear current line
    if (promptCb) std::cout << promptCb();
    std::cout << currentBuffer;
    // Move cursor to correct position
    if (promptCb && currentCursor < currentBuffer.size()) {
        std::cout << "\r\33[" << (currentCursor + promptCb().size()) << "C";
    }
    std::cout << std::flush;
}

void Terminal::clearCurrentLine() {
    if (!isReadingInput.load()) return;
    
    // Clear the current line before log output
    std::cout << "\r\33[2K" << std::flush;
}

void Terminal::updateInputState(const std::string& buffer, size_t cursor) {
    std::lock_guard<std::mutex> lock(inputMutex);
    currentBuffer = buffer;
    currentCursor = cursor;
}

void Terminal::displayBuffer(const std::string& buffer, size_t cursor) {
    std::cout << "\r\33[2K";
    if (promptCb) std::cout << promptCb();
    std::cout << buffer;
    if (cursor < buffer.size() && promptCb) {
        std::cout << "\r\33[" << (cursor + promptCb().size()) << "C";
    }
    std::cout << std::flush;
}

bool Terminal::handleBackspace(std::string& buffer, size_t& cursor) {
    if (buffer.empty() || cursor == 0) return false;
    buffer.erase(cursor - 1, 1);
    --cursor;
    updateInputState(buffer, cursor);
    displayBuffer(buffer, cursor);
    return true;
}

bool Terminal::handleHistoryNavigation(char key, History& history, std::string& buffer, size_t& cursor) {
    std::string hist;
    
    switch (key) {
        case 'A': // UP
            if (!history.prev(hist)) return false;
            buffer = hist;
            cursor = buffer.size();
            break;
        case 'B': // DOWN
            if (history.next(hist)) {
                buffer = hist;
                cursor = buffer.size();
            } else {
                buffer.clear();
                cursor = 0;
            }
            break;
        default:
            return false;
    }
    
    updateInputState(buffer, cursor);
    displayBuffer(buffer, cursor);
    return true;
}

bool Terminal::handleCursorMovement(char key, size_t& cursor, size_t bufferSize) {
    switch (key) {
        case 'C': // RIGHT
            if (cursor >= bufferSize) return false;
            ++cursor;
            std::cout << "\033[C" << std::flush;
            break;
        case 'D': // LEFT
            if (cursor == 0) return false;
            --cursor;
            std::cout << "\033[D" << std::flush;
            break;
        default:
            return false;
    }
    
    updateInputState(currentBuffer, cursor);
    return true;
}

void Terminal::handleCharInput(char c, std::string& buffer, size_t& cursor) {
    buffer.insert(buffer.begin() + cursor, c);
    ++cursor;
    updateInputState(buffer, cursor);
    displayBuffer(buffer, cursor);
}

void Terminal::requestShutdown() {
    shouldShutdown.store(true);
}

void Terminal::start() {
    log("INFO", "Starting terminal thread");
    shouldShutdown.store(false);
    terminalThread = std::thread([this]() {
        this->runBlockingStdioLoop();
    });
}

void Terminal::stop() {
    log("INFO", "Stopping terminal thread");
    shouldShutdown.store(true);
}

void Terminal::join() {
    if (terminalThread.joinable()) {
        log("INFO", "Waiting for terminal thread to finish");
        terminalThread.join();
        log("INFO", "Terminal thread finished");
    }
}

void Terminal::runBlockingStdioLoop() {
    std::string line;
    auto prev = std::signal(SIGINT, terminalSigintHandler);

    log("INFO", "Terminal started, listening for input");

    // Register redraw callback with Logger
    activeTerminal = this;
    logging::Logger::getInstance().setConsoleOutputCallback([this](bool before) {
        if (before) {
            clearCurrentLine();  // Clear before log is printed
        } else {
            redrawPrompt();      // Redraw after log is printed
        }
    });

    // HISTORY + LEFT/RIGHT CURSOR
    History history;
    struct termios orig, raw;
    tcgetattr(STDIN_FILENO, &orig);
    raw = orig;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    while (!shouldShutdown.load()) {
        if (sigintReceived.load()) {
            sigintReceived.store(false);
            log("DEBUG", "Received SIGINT (Ctrl+C)");
            if (shouldShutdown.load()) break;
            continue;
        }

        if (promptCb) std::cout << promptCb() << std::flush;

        std::string buffer;
        size_t cursor = 0;
        
        // Mark that we're now reading input and clear previous state
        {
            std::lock_guard<std::mutex> lock(inputMutex);
            currentBuffer.clear();
            currentCursor = 0;
        }
        isReadingInput.store(true);

        while (true) {
            char c;
            if (read(STDIN_FILENO, &c, 1) <= 0) {
                shouldShutdown.store(true);
                break;
            }

            if (c == '\n' || c == '\r') {
                isReadingInput.store(false);
                std::cout << "\n";
                history.add(buffer);
                
                // Restore cooked mode before executing command
                // so commands can use std::getline() normally
                tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
                if (sendCb) sendCb(buffer);
                // Return to raw mode for next input
                tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
                break;
            }

            if (c == 127 || c == 8) {
                handleBackspace(buffer, cursor);
                continue;
            }

            if (c == '\x1b') {
                char seq[2];
                if (read(STDIN_FILENO, &seq[0], 1) <= 0) continue;
                if (read(STDIN_FILENO, &seq[1], 1) <= 0) continue;

                if (seq[0] == '[') {
                    handleHistoryNavigation(seq[1], history, buffer, cursor) ||
                    handleCursorMovement(seq[1], cursor, buffer.size());
                }
                continue;
            }

            handleCharInput(c, buffer, cursor);
        }

        if (shouldShutdown.load()) break;
    }

    // Cleanup
    isReadingInput.store(false);
    activeTerminal = nullptr;
    logging::Logger::getInstance().setConsoleOutputCallback(nullptr);
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
    std::signal(SIGINT, prev);
    log("INFO", "Terminal stopped");
}

} // namespace terminal
