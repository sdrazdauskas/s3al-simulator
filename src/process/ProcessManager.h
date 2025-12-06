#pragma once

#include <string>
#include <optional>
#include <vector>
#include <functional>
#include "Process.h"

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

    void setLogCallback(LogCallback callback) { log_callback_ = callback; }
    void setProcessCompleteCallback(ProcessCompleteCallback cb) { complete_callback_ = cb; }

    // ============= Process Submission =============
    // Submit a new process with given CPU cost (cycles needed)
    // Returns PID on success, -1 on failure
    // Set persistent=true for long-running processes (init, daemons) that shouldn't terminate
    int submit(const std::string& name,
               int cpuCycles,
               int memoryNeeded,
               int priority = 0,
               bool persistent = false);

    // ============= Process Lifecycle =============
    // Query process existence
    bool process_exists(int pid) const;
    
    // Process control - suspend/resume
    bool suspend_process(int pid);
    bool resume_process(int pid);
    
    // Signal handling
    bool send_signal(int pid, int signal);
    
    using SignalCallback = std::function<void(int pid, int signal)>;
    void setSignalCallback(SignalCallback callback) { signal_callback_ = callback; }

    // Read-only access for kernel/UI/tests
    std::vector<Process> snapshot() const;

private:
    // ============= Core State =============
    int next_pid_{1};
    std::vector<Process> table_;         // All processes (for metadata)
    
    // ============= Dependencies =============
    memory::MemoryManager& mem_;
    scheduler::CPUScheduler& cpu_;
    
    // ============= Callbacks =============
    LogCallback log_callback_;
    SignalCallback signal_callback_;
    ProcessCompleteCallback complete_callback_;

    // ============= Internal Methods =============
    Process* find(int pid);
    const Process* find(int pid) const;
    void log(const std::string& level, const std::string& message);
    void onProcessComplete(int pid);
};

} // namespace process