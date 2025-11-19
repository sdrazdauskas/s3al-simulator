#include "Daemon.h"
#include <iostream>

namespace daemons {

Daemon::Daemon(shell::SysApi& sys, const std::string& name)
    : m_sys(sys), m_running(false), m_name(name) {}

void Daemon::log(const std::string& level, const std::string& message) {
    if (log_callback) {
        log_callback(level, m_name, message);
    }
}

void Daemon::start() {
    if (m_running.load()) {
        log("WARNING", "Daemon already running");
        return;
    }
    
    m_running.store(true);
    log("INFO", "Starting daemon...");
    
    m_thread = std::thread([this]() {
        this->run();
    });
}

void Daemon::stop() {
    if (!m_running.load()) {
        return;
    }
    
    log("INFO", "Stopping daemon...");
    m_running.store(false);
}

void Daemon::join() {
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void Daemon::handleSignal(int signal) {
    log("INFO", "Received signal " + std::to_string(signal));
    
    switch (signal) {
        case 9:  // SIGKILL
        case 15: // SIGTERM
            log("INFO", "Termination signal received, stopping daemon");
            stop();
            break;
        case 19: // SIGSTOP
            log("INFO", "Stop signal received (SIGSTOP)");
            // Could pause operations here if needed
            break;
        case 18: // SIGCONT
            log("INFO", "Continue signal received (SIGCONT)");
            // Could resume operations here if needed
            break;
        default:
            log("WARN", "Unknown signal " + std::to_string(signal));
            break;
    }
}

} // namespace daemons
