#pragma once

#include <string>
#include <optional>
#include <vector>
#include <functional>
#include "process/Process.h"
#include "common/LoggingMixin.h"

namespace sys { struct SysApi; }

namespace process {
    
class ProcessManager : public common::LoggingMixin {
public:
    // Callback invoked when a process completes execution
    using ProcessCompleteCallback = std::function<void(int pid, int exitCode)>;

    ProcessManager(sys::SysApi* sysApi);

    void setProcessCompleteCallback(ProcessCompleteCallback cb) { completeCallback = cb; }
    
    // Set system API (must be called before operations that need memory)
    void setSysApi(sys::SysApi* api) { sysApi = api; }

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
    
    // Called by scheduler when a process completes its CPU cycles
    void onProcessComplete(int pid);
    
    using SignalCallback = std::function<void(int pid, int signal)>;
    void setSignalCallback(SignalCallback callback) { signalCallback = callback; }

    // Read-only access for kernel/UI/tests
    std::vector<Process> snapshot() const;
    
    // Get the next PID that will be assigned
    int getNextPid() const { return nextPid; }

private:
    int nextPid{1};  // 0 is reserved for kernel
    std::vector<Process> processTable;
    
    sys::SysApi* sysApi;
    
    SignalCallback signalCallback;
    ProcessCompleteCallback completeCallback;

    Process* find(int pid);

protected:
    std::string getModuleName() const override { return "PROCESS_MGR"; }
};

} // namespace process