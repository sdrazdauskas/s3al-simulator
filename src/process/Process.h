#pragma once

#include <string>
#include <functional>

namespace process {

enum class ProcessState {
    NEW = 0,        // Just created, not yet ready
    READY = 1,      // Ready to run
    RUNNING = 2,    // Currently executing
    WAITING = 3,    // Waiting for I/O or event
    STOPPED = 4,    // Suspended (SIGSTOP)
    ZOMBIE = 5,     // Finished but not yet reaped
    TERMINATED = 6  // Fully terminated and removed
};

inline std::string stateToString(ProcessState s) {
    switch(s) {
        case ProcessState::NEW: return "NEW";
        case ProcessState::READY: return "READY";
        case ProcessState::RUNNING: return "RUNNING";
        case ProcessState::WAITING: return "WAITING";
        case ProcessState::STOPPED: return "STOPPED";
        case ProcessState::ZOMBIE: return "ZOMBIE";
        case ProcessState::TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
}

// Callback type for when a process completes execution
using ExecutionCallback = std::function<void(int pid, int exitCode)>;

class Process {
public:
    using LogCallback = std::function<void(const std::string& level, const std::string& message)>;

    Process(const std::string& name, 
            int pid,
            int cpuTimeNeeded,
            int memoryNeeded,
            int priority = 0,
            int parent_pid = 0);

    // Getters
    const std::string& name() const { return m_name; }
    int pid() const { return m_pid; }
    int cpuTimeNeeded() const { return m_cpuTimeNeeded; }
    int remainingCycles() const { return m_remainingCycles; }
    int memoryNeeded() const { return m_memoryNeeded; }
    int priority() const { return m_priority; }
    int parentPid() const { return m_parent_pid; }
    ProcessState state() const { return m_state; }
    bool isPersistent() const { return m_persistent; }
    
    // Set process as persistent (won't terminate when cycles reach 0)
    void setPersistent(bool persistent) { m_persistent = persistent; }
    
    // CPU cycle management
    void setRemainingCycles(int cycles) { m_remainingCycles = cycles; }
    bool consumeCycle();  // Returns true if process completed (remaining == 0)
    
    // Execution callback - called when process finishes
    void setExecutionCallback(ExecutionCallback cb) { m_execCallback = cb; }
    void onComplete(int exitCode);
    
    void setLogCallback(LogCallback callback) { m_log_callback = callback; }

    // State transitions - these validate and enforce valid state changes
    bool makeReady();
    bool start();
    bool suspend();
    bool resume();
    bool wait();
    bool terminate();
    bool makeZombie();

private:
    std::string m_name;
    int m_pid;
    int m_cpuTimeNeeded;
    int m_remainingCycles{0};
    int m_memoryNeeded;
    int m_priority;
    int m_parent_pid;
    ProcessState m_state;
    bool m_persistent{false};  // If true, process won't terminate when cycles reach 0
    LogCallback m_log_callback;
    ExecutionCallback m_execCallback;

    void log(const std::string& level, const std::string& message);
};

} // namespace process
