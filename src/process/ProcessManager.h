#pragma once

#include <string>
#include <optional>
#include <vector>
#include <functional>
#include "process/Process.h"

namespace memory { class MemoryManager; }
namespace scheduler { class CPUScheduler; }

namespace process {
    
class ProcessManager {
public:
    using LogCallback = std::function<void(const std::string& level, 
                                           const std::string& module, 
                                           const std::string& message)>;
    
    // Callback invoked when a process completes execution
    using ProcessCompleteCallback = std::function<void(int pid, int exitCode)>;

    ProcessManager(memory::MemoryManager& mem, scheduler::CPUScheduler& cpu);

    void setLogCallback(LogCallback callback) { logCallback = callback; }
    void setProcessCompleteCallback(ProcessCompleteCallback cb) { completeCallback = cb; }

    // Submit a new process with given CPU cost (cycles needed)
    // Returns PID on success, -1 on failure
    // Set persistent=true for long-running processes (init, daemons) that shouldn't terminate
    int submit(const std::string& name,
               int cpuCycles,
               int memoryNeeded,
               int priority = 0,
               bool persistent = false);

    // Query process existence
    bool processExists(int pid) const;
    
    // Check if process is persistent
    bool isProcessPersistent(int pid) const;
    
    // Process control - suspend/resume
    bool suspendProcess(int pid);
    bool resumeProcess(int pid);
    
    // Signal handling
    bool sendSignal(int pid, int signal);
    
    // Process exit (transitions to ZOMBIE state)
    bool exit(int pid, int exitCode = 0);
    
    // Reap a zombie process (remove from process table after completion)
    bool reapProcess(int pid);
    
    using SignalCallback = std::function<void(int pid, int signal)>;
    void setSignalCallback(SignalCallback callback) { signalCallback = callback; }

    // Read-only access for kernel/UI/tests
    std::vector<Process> snapshot() const;

private:
    int next_pid_{1};
    std::vector<Process> processTable;
    
    memory::MemoryManager& memManager;
    scheduler::CPUScheduler& cpuScheduler;
    
    LogCallback logCallback;
    SignalCallback signalCallback;
    ProcessCompleteCallback completeCallback;

    Process* find(int pid);
    void log(const std::string& level, const std::string& message);
    void onProcessComplete(int pid);
};

} // namespace process