#pragma once

#include <string>
#include <vector>
#include "scheduler/Scheduler.h"
#include "scheduler/algorithms/SchedulerAlgorithm.h"
 
namespace sys {

// TODO: error code system implementation?
enum class SysResult {
    OK,
    AlreadyExists,
    NotFound,
    AtRoot,
    InvalidArgument,
    Error
};

inline std::string toString(SysResult r) {
    switch(r) {
        case SysResult::OK: return "OK";
        case SysResult::AlreadyExists: return "AlreadyExists";
        case SysResult::NotFound: return "NotFound";
        case SysResult::AtRoot: return "AtRoot";
        case SysResult::InvalidArgument: return "InvalidArgument";
        default: return "Error";
    }
}

struct SysApi {
    struct SysInfo {
        size_t totalMemory{0};
        size_t usedMemory{0};
    };
    virtual SysResult fileExists(const std::string& name) = 0;
    virtual SysResult readFile(const std::string& name, std::string& out) = 0;
    virtual SysResult createFile(const std::string& name) = 0;
    virtual SysResult deleteFile(const std::string& name) = 0;
    virtual SysResult writeFile(const std::string& name, const std::string& content) = 0;
    virtual SysResult editFile(const std::string& name, const std::string& newContent) = 0;
    virtual SysResult copyFile(const std::string& src, const std::string& dest) = 0;
    virtual SysResult moveFile(const std::string& src, const std::string& dest) = 0;
    virtual SysResult appendFile(const std::string& name, const std::string& content) = 0;

    virtual std::string getWorkingDir() = 0;
    virtual SysResult listDir(const std::string& path, std::vector<std::string>& out) = 0;

    virtual SysResult makeDir(const std::string& name) = 0;
    virtual SysResult removeDir(const std::string& name) = 0;
    virtual SysResult changeDir(const std::string& name) = 0;
    virtual SysResult copyDir(const std::string& src, const std::string& dest) = 0;
    virtual SysResult moveDir(const std::string& src, const std::string& dest) = 0;

    virtual SysResult saveToDisk(const std::string& fileName) = 0;
    virtual SysResult loadFromDisk(const std::string& fileName) = 0;
    virtual SysResult resetStorage() = 0;
    virtual SysResult listDataFiles(std::vector<std::string>& out) = 0;

    virtual SysInfo getSysInfo() = 0;
    
    // Memory allocation syscalls for storage
    virtual void* allocateMemory(size_t size, int processId = 0) = 0;
    virtual SysResult deallocateMemory(void* ptr) = 0;

    virtual void requestShutdown() = 0;
    
    virtual void sendSignal(int signal) = 0;
    
    // Process control
    virtual SysResult sendSignalToProcess(int pid, int signal) = 0;
    
    // Process creation - returns PID of new process or -1 on failure
    virtual int fork(const std::string& name, int cpuTimeNeeded, int memoryNeeded, int priority = 0, bool persistent = false) = 0;
    
    // Process information
    struct ProcessInfo {
        int pid;
        std::string name;
        std::string state;
        int priority;
    };
    virtual std::vector<ProcessInfo> getProcessList() = 0;
    
    // Check if a process exists
    virtual bool processExists(int pid) = 0;
    
    // Interactive input - commands should use this instead of std::cin
    // Handles console logging suspension during input
    virtual std::string readLine() = 0;
    
    // Interactive mode control for full-screen applications (e.g., ncurses editors)
    // Call beginInteractiveMode() before taking over the terminal, endInteractiveMode() when done
    virtual void beginInteractiveMode() = 0;
    virtual void endInteractiveMode() = 0;
    
    // Add CPU work to an existing process (for daemons doing periodic work)
    // Returns true if successful, false if process not found
    virtual bool addCPUWork(int pid, int cpuCycles) = 0;
    
    // Wait for a process to complete (blocks until all CPU cycles consumed)
    // Returns true if completed normally, false if interrupted
    virtual bool waitForProcess(int pid) = 0;
    
    // Exit syscall - process terminates and becomes zombie
    virtual bool exit(int pid, int exitCode = 0) = 0;
    
    // Reap a zombie process (clean up after completion)
    virtual bool reapProcess(int pid) = 0;
    
    // Check if a process has completed
    virtual bool isProcessComplete(int pid) = 0;
    
    // Get remaining cycles for a process (-1 if not found)
    virtual int getProcessRemainingCycles(int pid) = 0;

    // Scheduler configuration
    virtual bool setSchedulingAlgorithm(scheduler::SchedulerAlgorithm algo, int quantum = 0) = 0;
    virtual bool setSchedulerCyclesPerInterval(int cycles) = 0;
    virtual bool setSchedulerTickIntervalMs(int ms) = 0;

    virtual ~SysApi() = default;
};

} // namespace sys
