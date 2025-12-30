#include "daemon/Daemon.h"
#include "kernel/SysCallsAPI.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace daemons {

Daemon::Daemon(sys::SysApi& sys, const std::string& name)
    : sysApi(sys), running(false), daemonName(name) {}

void Daemon::start() {
    if (running.load()) {
        logWarn("Daemon already running");
        return;
    }
    
    running.store(true);
    logInfo("Starting daemon...");
    
    thread = std::thread([this]() {
        this->run();
    });
}

void Daemon::stop() {
    if (!running.load()) {
        return;
    }
    
    logInfo("Stopping daemon...");
    running.store(false);
}

void Daemon::join() {
    if (thread.joinable()) {
        thread.join();
    }
}

void Daemon::run() {
    logInfo("Daemon started (PID " + std::to_string(pid) + ")");
    
    while (running.load()) {
        if (!suspended.load()) {
            if (!running.load()) break;
            
            int workCycles = getWorkCycles();
            sysApi.addCPUWork(pid, workCycles);
            
            doWork();
            
            // Check running flag frequently to make sure we can exit promptly
            int waitMs = getWaitIntervalMs();
            for (int i = 0; i < waitMs / 100 && running.load(); ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        } else {
            // If suspended, just sleep briefly to avoid busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    logInfo("Daemon stopped (PID " + std::to_string(pid) + ")");
}

void Daemon::handleSignal(int signal) {
    logInfo("Received signal " + std::to_string(signal));
    
    switch (signal) {
        case 9:  // SIGKILL
        case 15: // SIGTERM
            logInfo("Termination signal received, stopping daemon");
            stop();
            break;
        case 19: // SIGSTOP
            logInfo("Suspending daemon operations");
            suspended.store(true);
            break;
        case 18: // SIGCONT
            logInfo("Resuming daemon operations");
            suspended.store(false);
            break;
        default:
            logWarn("Unknown signal " + std::to_string(signal));
            break;
    }
}

} // namespace daemons
