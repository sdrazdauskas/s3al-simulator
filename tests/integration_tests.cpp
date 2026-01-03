#include <gtest/gtest.h>
#include "kernel/Kernel.h"
#include "storage/Storage.h"
#include "kernel/SysCallsAPI.h"
#include "memory/MemoryManager.h"
#include "process/ProcessManager.h"
#include "scheduler/Scheduler.h"
#include "logger/Logger.h"

using namespace kernel;
using namespace storage;
using namespace memory;
using namespace process;
using namespace scheduler;

// SysApi wrapper that uses real MemoryManager for integration testing
class IntegrationSysApi : public sys::SysApi {
private:
    MemoryManager& memManager;
    
public:
    explicit IntegrationSysApi(MemoryManager& mem) : memManager(mem) {}
    
    void* allocateMemory(size_t size, int processId = 0) override {
        return memManager.allocate(size, processId);
    }
    
    sys::SysResult deallocateMemory(void* ptr) override {
        return memManager.deallocate(ptr) ? sys::SysResult::OK : sys::SysResult::Error;
    }
    
    void freeProcessMemory(int processId) override {
        memManager.freeProcessMemory(processId);
    }
    
    // Stub implementations for other methods
    sys::SysResult fileExists(const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult readFile(const std::string&, std::string&) override { return sys::SysResult::OK; }
    sys::SysResult createFile(const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult deleteFile(const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult writeFile(const std::string&, const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult editFile(const std::string&, const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult copyFile(const std::string&, const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult moveFile(const std::string&, const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult appendFile(const std::string&, const std::string&) override { return sys::SysResult::OK; }
    std::string getWorkingDir() override { return "/"; }
    sys::SysResult listDir(const std::string&, std::vector<std::string>&) override { return sys::SysResult::OK; }
    sys::SysResult makeDir(const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult removeDir(const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult changeDir(const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult copyDir(const std::string&, const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult moveDir(const std::string&, const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult saveToDisk(const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult loadFromDisk(const std::string&) override { return sys::SysResult::OK; }
    sys::SysResult resetStorage() override { return sys::SysResult::OK; }
    sys::SysResult listDataFiles(std::vector<std::string>&) override { return sys::SysResult::OK; }
    sys::SysApi::SysInfo getSysInfo() override { return {}; }
    void requestShutdown() override {}
    void sendSignal(int) override {}
    sys::SysResult sendSignalToProcess(int, int) override { return sys::SysResult::OK; }
    int fork(const std::string&, int, int, int, bool) override { return 0; }
    std::vector<ProcessInfo> getProcessList() override { return {}; }
    bool processExists(int) override { return false; }
    std::string readLine() override { return ""; }
    void beginInteractiveMode() override {}
    void endInteractiveMode() override {}
    bool addCPUWork(int, int) override { return false; }
    bool waitForProcess(int) override { return false; }
    bool exit(int, int) override { return false; }
    bool reapProcess(int) override { return false; }
    bool isProcessComplete(int) override { return false; }
    int getProcessRemainingCycles(int) override { return -1; }
    bool setSchedulingAlgorithm(scheduler::SchedulerAlgorithm, int) override { return false; }
    bool setSchedulerCyclesPerInterval(int) override { return false; }
    bool setSchedulerTickIntervalMs(int) override { return false; }
    bool getConsoleOutput() const override { return false; }
    void setConsoleOutput(bool) override {}
    std::string getLogLevel() const override { return "INFO"; }
    void setLogLevel(logging::LogLevel) override {}
};


class IntegrationTest : public ::testing::Test {
protected:
};

// ProcessManager + MemoryManager + CPUScheduler integration
TEST_F(IntegrationTest, ProcessSchedulerIntegration) {
    MemoryManager memory(4096);
    CPUScheduler scheduler;
    IntegrationSysApi sysApi(memory);
    ProcessManager procMgr(&sysApi, scheduler);
    
public:
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

    bool getConsoleOutput() const override { return false; }
    void setConsoleOutput(bool) override {}
    std::string getLogLevel() const override { return "INFO"; }
    void setLogLevel(logging::LogLevel) override {}
};


class IntegrationTest : public ::testing::Test {
protected:
};

// ProcessManager + MemoryManager + CPUScheduler integration
TEST_F(IntegrationTest, ProcessSchedulerIntegration) {
    MemoryManager memory(4096);
    CPUScheduler scheduler;
    IntegrationSysApi sysApi(memory);
    ProcessManager procMgr(&sysApi, scheduler);
    
    int pid1 = procMgr.submit("proc1", 10, 512, 5);
    int pid2 = procMgr.submit("proc2", 20, 256, 10);
    int pid3 = procMgr.submit("proc3", 15, 128, 3);
    
    EXPECT_GT(pid1, 0);
    EXPECT_GT(pid2, 0);
    EXPECT_GT(pid3, 0);
    
    // Verify memory was allocated for processes
    EXPECT_GT(memory.getUsedMemory(), 0);
    
    auto snapshot = procMgr.snapshot();
    EXPECT_EQ(snapshot.size(), 3);
    
    // Terminate processes using SIGTERM
    EXPECT_TRUE(procMgr.sendSignal(pid1, 15));
    EXPECT_TRUE(procMgr.sendSignal(pid2, 15));
    EXPECT_TRUE(procMgr.sendSignal(pid3, 15));
    
    // Verify processes are now zombies (still in snapshot)
    snapshot = procMgr.snapshot();
    EXPECT_EQ(snapshot.size(), 3);
    
    // Reap the zombie processes
    EXPECT_TRUE(procMgr.reapProcess(pid1));
    EXPECT_TRUE(procMgr.reapProcess(pid2));
    EXPECT_TRUE(procMgr.reapProcess(pid3));
    
    // After reaping, snapshot should be empty and memory should be freed
    snapshot = procMgr.snapshot();
    EXPECT_EQ(snapshot.size(), 0);
    EXPECT_EQ(memory.getUsedMemory(), 0);
    
    // Submit new processes
    int execPid1 = procMgr.submit("exec1", 10, 512, 5);
    int execPid2 = procMgr.submit("exec2", 20, 256, 10);
    
    EXPECT_GT(execPid1, 0);
    EXPECT_GT(execPid2, 0);
    
    // Processes are queued but not yet completed, verify they exist
    snapshot = procMgr.snapshot();
    EXPECT_EQ(snapshot.size(), 2);
    
    // Terminate the new processes
    procMgr.sendSignal(execPid1, 15);
    procMgr.sendSignal(execPid2, 15);
    
    // Verify processes are zombies
    snapshot = procMgr.snapshot();
    EXPECT_EQ(snapshot.size(), 2);
    
    // Reap the zombies
    EXPECT_TRUE(procMgr.reapProcess(execPid1));
    EXPECT_TRUE(procMgr.reapProcess(execPid2));
    
    // After reaping, snapshot should be empty
    snapshot = procMgr.snapshot();
    EXPECT_EQ(snapshot.size(), 0);
}
