#include "process/Process.h"

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
        logError("Cannot transition to READY from " + stateToString(state));
        return false;
    }
    state = ProcessState::READY;
    logDebug("State: " + stateToString(state));
    return true;
}

bool Process::start() {
    if (state != ProcessState::READY) {
        logError("Cannot start process from " + stateToString(state) + " state");
        return false;
    }
    state = ProcessState::RUNNING;
    logDebug("State: " + stateToString(state));
    return true;
}

bool Process::suspend() {
    if (state != ProcessState::RUNNING && state != ProcessState::READY) {
        logError("Cannot suspend process from " + stateToString(state) + " state");
        return false;
    }
    ProcessState prev = state;
    state = ProcessState::STOPPED;
    logInfo("Suspended from " + stateToString(prev));
    return true;
}

bool Process::resume() {
    if (state != ProcessState::STOPPED) {
        logError("Cannot resume process - not in STOPPED state");
        return false;
    }
    state = ProcessState::READY;
    logInfo("Resumed to READY");
    return true;
}

bool Process::wait() {
    if (state != ProcessState::RUNNING) {
        logError("Cannot wait - not in RUNNING state");
        return false;
    }
    state = ProcessState::WAITING;
    logDebug("State: " + stateToString(state));
    return true;
}

bool Process::makeZombie() {
    // Can't become zombie if already a zombie
    if (state == ProcessState::ZOMBIE) {
        logWarn("Process already a zombie");
        return false;
    }
    
    // Allow zombie transition from any state (killed by signal or normal exit)
    state = ProcessState::ZOMBIE;
    logDebug("State: " + stateToString(state));
    return true;
}

bool Process::consumeCycle() {
    if (remainingCycles > 0) {
        --remainingCycles;
        logDebug("Consumed cycle, remaining: " + std::to_string(remainingCycles));
    }
    return remainingCycles == 0;
}

void Process::onComplete(int exitCode) {
    if (execCallback) {
        execCallback(pid, exitCode);
    }
}

} // namespace process
