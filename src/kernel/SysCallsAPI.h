#pragma once
#include <string>
#include <vector>
 
namespace shell {

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
        size_t total_memory{0};
        size_t used_memory{0};
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
    
    // Interactive input - commands should use this instead of std::cin
    // Handles console logging suspension during input
    virtual std::string readLine() = 0;
    
    // Interactive mode control for full-screen applications (e.g., ncurses editors)
    // Call beginInteractiveMode() before taking over the terminal, endInteractiveMode() when done
    virtual void beginInteractiveMode() = 0;
    virtual void endInteractiveMode() = 0;

    // Submit a command to run through the scheduler with CPU cost cycles
    // Returns process ID, or -1 on failure
    virtual int submitCommand(const std::string& name, int cpuCycles, int priority = 0) = 0;
    
    // Wait for a submitted command to complete (blocks until done)
    // Returns true if completed normally, false if interrupted
    virtual bool waitForProcess(int pid) = 0;
    
    // Check if a process has completed
    virtual bool isProcessComplete(int pid) = 0;
    
    // Get remaining cycles for a process (-1 if not found)
    virtual int getProcessRemainingCycles(int pid) = 0;

    virtual ~SysApi() = default;
};

} // namespace shell
