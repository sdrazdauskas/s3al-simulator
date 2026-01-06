#pragma once

#include "kernel/SysCallsAPI.h"
#include <map>
#include <vector>
#include <string>

namespace testHelpers {

// Base mock implementation with sensible defaults for all SysApi methods
// Tests can inherit and override only what they need
class MockSysApi : public sys::SysApi {
protected:
    std::map<void*, size_t> allocations;
    
public:
    virtual ~MockSysApi() {
        // Cleanup any remaining allocations
        for (auto& [ptr, size] : allocations) {
            delete[] static_cast<char*>(ptr);
        }
    }
    
    // Memory management - actual implementation for testing
    void* allocateMemory(size_t size, int processId = 0) override {
        void* ptr = new char[size];
        allocations[ptr] = size;
        return ptr;
    }
    
    sys::SysResult deallocateMemory(void* ptr) override {
        auto it = allocations.find(ptr);
        if (it == allocations.end()) {
            return sys::SysResult::Error;
        }
        delete[] static_cast<char*>(ptr);
        allocations.erase(it);
        return sys::SysResult::OK;
    }
    
    void freeProcessMemory(int processId) override {}
    
    // Scheduler operations - stubs
    void scheduleProcess(int, int, int) override {}
    void unscheduleProcess(int) override {}
    void suspendScheduledProcess(int) override {}
    void resumeScheduledProcess(int) override {}
    
    // File operations - stubs
    sys::SysResult fileExists(const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult readFile(const std::string&, std::string&) override { return sys::SysResult::OK; }
    sys::SysResult createFile(const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult deleteFile(const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult writeFile(const std::string&, const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult editFile(const std::string&, const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult copyFile(const std::string&, const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult moveFile(const std::string&, const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult appendFile(const std::string&, const std::string&) override { return sys::SysResult::OK; }
    
    // Directory operations - stubs
    std::string getWorkingDir() override { return "/"; }
    sys::SysResult listDir(const std::string&, std::vector<std::string>&) override { return sys::SysResult::OK; }
    sys::SysResult makeDir(const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult removeDir(const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult changeDir(const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult copyDir(const std::string&, const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult moveDir(const std::string&, const std::string&) override { return sys::SysResult::OK; }
    
    // Storage persistence - stubs
    sys::SysResult saveToDisk(const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult loadFromDisk(const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult readFileFromHost(const std::string&, std::string&) override { return sys::SysResult::OK; }
    sys::SysResult resetStorage() override { return sys::SysResult::OK; }
    sys::SysResult listDataFiles(std::vector<std::string>&) override { return sys::SysResult::OK; }
    
    // System info - stubs
    sys::SysApi::SysInfo getSysInfo() override { return {}; }
    void requestShutdown() override {}
    
    // Process operations - stubs
    void sendSignal(int) override {}
    sys::SysResult sendSignalToProcess(int, int) override { return sys::SysResult::OK; }
    int fork(const std::string&, int, int, int, bool) override { return 0; }
    std::vector<ProcessInfo> getProcessList() override { return {}; }
    bool processExists(int) override { return false; }
    bool addCPUWork(int, int) override { return false; }
    bool waitForProcess(int) override { return false; }
    bool exit(int, int) override { return false; }
    bool reapProcess(int) override { return false; }
    bool isProcessComplete(int) override { return false; }
    int getProcessRemainingCycles(int) override { return -1; }
    
    // Terminal/Shell - stubs
    std::string readLine() override { return ""; }
    void beginInteractiveMode() override {}
    void endInteractiveMode() override {}
    
    // Scheduler configuration - stubs
    bool setSchedulingAlgorithm(scheduler::SchedulerAlgorithm, int) override { return false; }
    bool setSchedulerCyclesPerInterval(int) override { return false; }
    bool setSchedulerTickIntervalMs(int) override { return false; }
    
    // Logging - stubs
    bool getConsoleOutput() const override { return false; }
    void setConsoleOutput(bool) override {}
    std::string getLogLevel() const override { return "INFO"; }
    void setLogLevel(logging::LogLevel) override {}
};

} // namespace testHelpers
