#include "Terminal.h"
#include <algorithm>
#include <iostream>
#include <csignal>
#include <termios.h>
#include <unistd.h>
#include "helper/History.h"
#include "Logger.h"

namespace terminal {

// Anonymous namespace for helpers
namespace {
    std::atomic<bool> sigintReceived{false};
    // Store the signal callback globally so signal handler can access it
    std::function<void(int)>* g_signalCallback = nullptr;
    // Store pointer to active terminal for redraw callback
    Terminal* g_activeTerminal = nullptr;

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

void Terminal::redrawPrompt() {
    if (!is_reading_input.load()) return;
    
    std::lock_guard<std::mutex> lock(input_mutex);
    // Redraw prompt and current input on a fresh line
    std::cout << "\r\33[2K";  // Clear current line
    if (promptCb) std::cout << promptCb();
    std::cout << current_buffer;
    // Move cursor to correct position
    if (promptCb && current_cursor < current_buffer.size()) {
        std::cout << "\r\33[" << (current_cursor + promptCb().size()) << "C";
    }
    std::cout << std::flush;
}

void Terminal::clearCurrentLine() {
    if (!is_reading_input.load()) return;
    
    // Clear the current line before log output
    std::cout << "\r\33[2K" << std::flush;
}

void Terminal::updateInputState(const std::string& buffer, size_t cursor) {
    std::lock_guard<std::mutex> lock(input_mutex);
    current_buffer = buffer;
    current_cursor = cursor;
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

    // Register redraw callback with Logger
    g_activeTerminal = this;
    logging::Logger::getInstance().setConsoleOutputCallback([this](bool before) {
        if (before) {
            clearCurrentLine();  // Clear before log is printed
        } else {
            redrawPrompt();      // Redraw after log is printed
        }
    });

    // =================== HISTORY + LEFT/RIGHT CURSOR =====================
    History history;
    struct termios orig, raw;
    tcgetattr(STDIN_FILENO, &orig);
    raw = orig;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    while (!should_shutdown.load()) {
        if (sigintReceived.load()) {
            sigintReceived.store(false);
            log("DEBUG", "Received SIGINT (Ctrl+C) - already handled in signal handler");
            if (should_shutdown.load()) break;
            continue;
        }

        if (promptCb) std::cout << promptCb() << std::flush;

        std::string buffer;
        size_t cursor = 0;
        
        // Mark that we're now reading input and clear previous state
        {
            std::lock_guard<std::mutex> lock(input_mutex);
            current_buffer.clear();
            current_cursor = 0;
        }
        is_reading_input.store(true);

        while (true) {
            char c;
            if (read(STDIN_FILENO, &c, 1) <= 0) {
                should_shutdown.store(true);
                break;
            }

            if (c == '\n' || c == '\r') {
                is_reading_input.store(false);
                std::cout << "\n";
                history.add(buffer);
                if (sendCb) sendCb(buffer);
                break;
            }

            if (c == 127 || c == 8) {
                if (!buffer.empty() && cursor > 0) {
                    buffer.erase(cursor - 1, 1);
                    --cursor;
                    updateInputState(buffer, cursor);
                    displayBuffer(buffer, cursor);
                }
                continue;
            }

            if (c == '\x1b') {
                char seq[2];
                if (read(STDIN_FILENO, &seq[0], 1) <= 0) continue;
                if (read(STDIN_FILENO, &seq[1], 1) <= 0) continue;

                if (seq[0] == '[') {
                    std::string hist;
                    switch (seq[1]) {
                        case 'A': // UP
                            if (history.prev(hist)) {
                                buffer = hist;
                                cursor = buffer.size();
                                updateInputState(buffer, cursor);
                                displayBuffer(buffer, cursor);
                            }
                            break;
                        case 'B': // DOWN
                            if (history.next(hist)) {
                                buffer = hist;
                                cursor = buffer.size();
                            } else {
                                buffer.clear();
                                cursor = 0;
                            }
                            updateInputState(buffer, cursor);
                            displayBuffer(buffer, cursor);
                            break;
                        case 'C': // RIGHT
                            if (cursor < buffer.size()) {
                                ++cursor;
                                updateInputState(buffer, cursor);
                                std::cout << "\033[C" << std::flush;
                            }
                            break;
                        case 'D': // LEFT
                            if (cursor > 0) {
                                --cursor;
                                updateInputState(buffer, cursor);
                                std::cout << "\033[D" << std::flush;
                            }
                            break;
                    }
                }
                continue;
            }

            buffer.insert(buffer.begin() + cursor, c);
            ++cursor;
            updateInputState(buffer, cursor);
            displayBuffer(buffer, cursor);
        }

        if (should_shutdown.load()) break;
    }

    // Cleanup
    is_reading_input.store(false);
    g_activeTerminal = nullptr;
    logging::Logger::getInstance().setConsoleOutputCallback(nullptr);
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
    std::signal(SIGINT, prev);
    log("INFO", "Terminal stopped");
    // =====================================================================
}

} // namespace terminal
