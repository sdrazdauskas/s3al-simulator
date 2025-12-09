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

    Process(const std::string& processName, 
            int pid,
            int cpuTimeNeeded,
            int memoryNeeded,
            int priority = 0,
            int parent_pid = 0);

    const std::string& getName() const { return processName; }
    int getPid() const { return pid; }
    int getCpuTimeNeeded() const { return cpuTimeNeeded; }
    int getRemainingCycles() const { return remainingCycles; }
    int getMemoryNeeded() const { return memoryNeeded; }
    int getPriority() const { return priority; }
    int getParentPid() const { return parentPid; }
    ProcessState getState() const { return state; }
    bool isPersistent() const { return persistent; }
    
    // Set process as persistent (won't terminate when cycles reach 0)
    void setPersistent(bool persistent) { this->persistent = persistent; }
    
    // CPU cycle management
    void setRemainingCycles(int cycles) { remainingCycles = cycles; }
    bool consumeCycle();  // Returns true if process completed (remaining == 0)
    
    // Execution callback - called when process finishes
    void setExecutionCallback(ExecutionCallback cb) { execCallback = cb; }
    void onComplete(int exitCode);
    
    void setLogCallback(LogCallback callback) { logCallback = callback; }

    // State transitions - these validate and enforce valid state changes
    bool makeReady();
    bool start();
    bool suspend();
    bool resume();
    bool wait();
    bool terminate();
    bool makeZombie();

private:
    std::string processName;
    int pid;
    int cpuTimeNeeded;
    int remainingCycles{0};
    int memoryNeeded;
    int priority;
    int parentPid;
    ProcessState state;
    bool persistent{false};  // If true, process won't terminate when cycles reach 0
    LogCallback logCallback;
    ExecutionCallback execCallback;

    void log(const std::string& level, const std::string& message);
};

} // namespace process
