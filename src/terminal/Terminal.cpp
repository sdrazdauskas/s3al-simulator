// Terminal.cpp

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

void Terminal::print(const std::string& output) {
    std::cout << output << std::flush;
}

void Terminal::runBlockingStdioLoop() {
    std::string line;
    auto prev = std::signal(SIGINT, terminalSigintHandler);

    while (true) {
        if (sigintReceived.load()) {
            sigintReceived.store(false);
            if (sigCb) sigCb(SIGINT);
            else if (sendCb) sendCb("\x03");
            continue;
        }

        if (!std::getline(std::cin, line)) break; // EOF or error
        // std::getline removes the newline; add it back for the shell
        line.push_back('\n');
        if (sendCb) sendCb(line);
    }

    std::signal(SIGINT, prev);
}

} // namespace terminal

