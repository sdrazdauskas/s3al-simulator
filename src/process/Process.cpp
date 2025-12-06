#include "Process.h"

namespace process {

Process::Process(const std::string& processName, 
                 int pid,
                 int cpuTimeNeeded,
                 int memoryNeeded,
                 int priority,
                 int parent_pid)
    : processName(processName),
            pid(pid),
            cpuTimeNeeded(cpuTimeNeeded),
            memoryNeeded(memoryNeeded),
            priority(priority),
            parentPid(parent_pid),
            state(ProcessState::NEW)
{}

bool Process::makeReady() {
    if (state != ProcessState::NEW && state != ProcessState::WAITING) {
        log("ERROR", "Cannot transition to READY from " + stateToString(state));
        return false;
    }
    state = ProcessState::READY;
    log("DEBUG", "State: " + stateToString(state));
    return true;
}

bool Process::start() {
    if (state != ProcessState::READY) {
        log("ERROR", "Cannot start process from " + stateToString(state) + " state");
        return false;
    }
    state = ProcessState::RUNNING;
    log("DEBUG", "State: " + stateToString(state));
    return true;
}

bool Process::suspend() {
    if (state != ProcessState::RUNNING && state != ProcessState::READY) {
        log("ERROR", "Cannot suspend process from " + stateToString(state) + " state");
        return false;
    }
    ProcessState prev = state;
    state = ProcessState::STOPPED;
    log("INFO", "Suspended from " + stateToString(prev));
    return true;
}

bool Process::resume() {
    if (state != ProcessState::STOPPED) {
        log("ERROR", "Cannot resume process - not in STOPPED state");
        return false;
    }
    state = ProcessState::READY;
    log("INFO", "Resumed to READY");
    return true;
}

bool Process::wait() {
    if (state != ProcessState::RUNNING) {
        log("ERROR", "Cannot wait - not in RUNNING state");
        return false;
    }
    state = ProcessState::WAITING;
    log("DEBUG", "State: " + stateToString(state));
    return true;
}

bool Process::terminate() {
    // Can terminate from any state except already terminated
    if (state == ProcessState::TERMINATED) {
        log("WARN", "Process already terminated");
        return false;
    }
    state = ProcessState::TERMINATED;
    log("INFO", "Terminated");
    return true;
}

bool Process::makeZombie() {
    if (state != ProcessState::RUNNING && state != ProcessState::WAITING) {
        log("ERROR", "Cannot become zombie from " + stateToString(state));
        return false;
    }
    state = ProcessState::ZOMBIE;
    log("DEBUG", "State: " + stateToString(state));
    return true;
}

bool Process::consumeCycle() {
    if (remainingCycles > 0) {
        --remainingCycles;
        log("DEBUG", "Consumed cycle, remaining: " + std::to_string(remainingCycles));
    }
    return remainingCycles == 0;
}

void Process::onComplete(int exitCode) {
    if (execCallback) {
        execCallback(pid, exitCode);
    }
}

void Process::log(const std::string& level, const std::string& message) {
    if (logCallback) {
        logCallback(level, "PID=" + std::to_string(pid) + " '" + processName + "': " + message);
    }
}

} // namespace process
