#include "Process.h"

namespace process {

Process::Process(const std::string& name, 
                 int pid,
                 int cpuTimeNeeded,
                 int memoryNeeded,
                 int priority,
                 int parent_pid)
    : m_name(name),
      m_pid(pid),
      m_cpuTimeNeeded(cpuTimeNeeded),
      m_memoryNeeded(memoryNeeded),
      m_priority(priority),
      m_parent_pid(parent_pid),
      m_state(ProcessState::NEW)
{}

bool Process::makeReady() {
    if (m_state != ProcessState::NEW && m_state != ProcessState::WAITING) {
        log("ERROR", "Cannot transition to READY from " + stateToString(m_state));
        return false;
    }
    m_state = ProcessState::READY;
    log("DEBUG", "State: " + stateToString(m_state));
    return true;
}

bool Process::start() {
    if (m_state != ProcessState::READY) {
        log("ERROR", "Cannot start process from " + stateToString(m_state) + " state");
        return false;
    }
    m_state = ProcessState::RUNNING;
    log("DEBUG", "State: " + stateToString(m_state));
    return true;
}

bool Process::suspend() {
    if (m_state != ProcessState::RUNNING && m_state != ProcessState::READY) {
        log("ERROR", "Cannot suspend process from " + stateToString(m_state) + " state");
        return false;
    }
    ProcessState prev = m_state;
    m_state = ProcessState::STOPPED;
    log("INFO", "Suspended from " + stateToString(prev));
    return true;
}

bool Process::resume() {
    if (m_state != ProcessState::STOPPED) {
        log("ERROR", "Cannot resume process - not in STOPPED state");
        return false;
    }
    m_state = ProcessState::READY;
    log("INFO", "Resumed to READY");
    return true;
}

bool Process::wait() {
    if (m_state != ProcessState::RUNNING) {
        log("ERROR", "Cannot wait - not in RUNNING state");
        return false;
    }
    m_state = ProcessState::WAITING;
    log("DEBUG", "State: " + stateToString(m_state));
    return true;
}

bool Process::terminate() {
    // Can terminate from any state except already terminated
    if (m_state == ProcessState::TERMINATED) {
        log("WARN", "Process already terminated");
        return false;
    }
    m_state = ProcessState::TERMINATED;
    log("INFO", "Terminated");
    return true;
}

bool Process::makeZombie() {
    if (m_state != ProcessState::RUNNING && m_state != ProcessState::WAITING) {
        log("ERROR", "Cannot become zombie from " + stateToString(m_state));
        return false;
    }
    m_state = ProcessState::ZOMBIE;
    log("DEBUG", "State: " + stateToString(m_state));
    return true;
}

bool Process::consumeCycle() {
    if (m_remainingCycles > 0) {
        --m_remainingCycles;
        log("DEBUG", "Consumed cycle, remaining: " + std::to_string(m_remainingCycles));
    }
    return m_remainingCycles == 0;
}

void Process::onComplete(int exitCode) {
    if (m_execCallback) {
        m_execCallback(m_pid, exitCode);
    }
}

void Process::log(const std::string& level, const std::string& message) {
    if (m_log_callback) {
        m_log_callback(level, "PID=" + std::to_string(m_pid) + " '" + m_name + "': " + message);
    }
}

} // namespace process
