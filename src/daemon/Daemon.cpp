#include "daemon/Daemon.h"
#include "kernel/SysCallsAPI.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace daemons {

Daemon::Daemon(sys::SysApi& sys, const std::string& name)
    : sysApi(sys), running(false), daemonName(name) {}

void Daemon::log(const std::string& level, const std::string& message) {
    if (logCallback) {
        logCallback(level, daemonName, message);
    }
}

void Daemon::start() {
    if (running.load()) {
        log("WARNING", "Daemon already running");
        return;
    }
    
    running.store(true);
    log("INFO", "Starting daemon...");
    
    thread = std::thread([this]() {
        this->run();
    });
}

void Daemon::stop() {
    if (!running.load()) {
        return;
    }
    
    log("INFO", "Stopping daemon...");
    running.store(false);
}

void Daemon::join() {
    if (thread.joinable()) {
        thread.join();
    }
}

void Daemon::run() {
    log("INFO", "Daemon started (PID " + std::to_string(pid) + ")");
    
    while (running.load()) {
        // Only do work if not suspended
        if (!suspended.load()) {
            // Check if still running before doing work
            if (!running.load()) break;
            
            // Add CPU work to this daemon's process
            int workCycles = getWorkCycles();
            sysApi.addCPUWork(pid, workCycles);
            
            // Do the actual work - it will be scheduled through our own process
            doWork();
            
            // Sleep between work cycles, checking running flag frequently
            int waitMs = getWaitIntervalMs();
            for (int i = 0; i < waitMs / 100 && running.load(); ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        } else {
            // If suspended, just sleep briefly to avoid busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    log("INFO", "Daemon stopped (PID " + std::to_string(pid) + ")");
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
            log("INFO", "Suspending daemon operations");
            suspended.store(true);
            break;
        case 18: // SIGCONT
            log("INFO", "Resuming daemon operations");
            suspended.store(false);
            break;
        default:
            log("WARN", "Unknown signal " + std::to_string(signal));
            break;
    }
}

} // namespace daemons
