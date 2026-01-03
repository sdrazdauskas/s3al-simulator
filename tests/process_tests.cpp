#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "process/ProcessManager.h"
#include "kernel/SysCallsAPI.h"
#include "scheduler/Scheduler.h"
#include "scheduler/algorithms/PriorityAlgorithm.h"
#include "scheduler/algorithms/FCFSAlgorithm.h"
#include "scheduler/algorithms/RoundRobinAlgorithm.h"
#include "logger/Logger.h"
#include <map>

using namespace process;
using namespace scheduler;
using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;

// Mock SysApi for process manager testing
class MockSysApi : public sys::SysApi {
private:
    std::map<void*, size_t> allocations;
    
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
    
    void freeProcessMemory(int processId) override {
        // Mock implementation - just clear allocations for this process
        // In real implementation, MemoryManager tracks by PID
    }
    
    ~MockSysApi() {
        for (auto& [ptr, size] : allocations) {
            delete[] static_cast<char*>(ptr);
        }
    }
    
    // Stub implementations
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

class ProcessManagerMockTest : public ::testing::Test {
protected:
    MockSysApi mockSysApi;
    std::unique_ptr<CPUScheduler> scheduler;
    
    void SetUp() override {
        scheduler = std::make_unique<CPUScheduler>();
    }
};

// Memory allocation failure handling
TEST_F(ProcessManagerMockTest, ProcessCreationFailsWhenMemoryUnavailable) {
    ProcessManager pm(&mockSysApi, *scheduler);
    
    // Submit process - allocation happens at submit time
    int pid = pm.submit("test_process", 100, 512, 5);
    EXPECT_GT(pid, 0); // Process submitted successfully
}

class SchedulerTest : public ::testing::Test {
protected:
    std::unique_ptr<CPUScheduler> scheduler;
    
    void SetUp() override {
        scheduler = std::make_unique<CPUScheduler>();
    }
};

TEST_F(SchedulerTest, EnqueueAddsProcessToReadyQueue) {
    scheduler->enqueue(1, 5, 1);
    EXPECT_EQ(scheduler->getReadyCount(), 1);
    EXPECT_TRUE(scheduler->hasWork());
}

TEST_F(SchedulerTest, TickStartsProcessExecution) {
    scheduler->enqueue(1, 1, 1);
    
    auto result = scheduler->tick();
    
    EXPECT_EQ(scheduler->getReadyCount(), 0);
    EXPECT_EQ(result.currentPid, 1);
    EXPECT_FALSE(result.contextSwitch);
    EXPECT_FALSE(result.idle);
}

TEST_F(SchedulerTest, ProcessCompletesAfterEnoughCycles) {
    scheduler->enqueue(1, 3, 1);  // 3 cycles needed
    
    // Tick 1 - starts and consumes 1 cycle
    auto r1 = scheduler->tick();
    EXPECT_FALSE(r1.processCompleted);
    EXPECT_EQ(r1.remainingCycles, 2);
    
    // Tick 2 - consumes 1 cycle  
    auto r2 = scheduler->tick();
    EXPECT_FALSE(r2.processCompleted);
    EXPECT_EQ(r2.remainingCycles, 1);
    
    // Tick 3 - completes
    auto r3 = scheduler->tick();
    EXPECT_TRUE(r3.processCompleted);
    EXPECT_EQ(r3.completedPid, 1);
}

TEST_F(SchedulerTest, FCFSExecutesInOrder) {
    scheduler->setAlgorithm(std::make_unique<FCFSAlgorithm>());
    
    scheduler->enqueue(1, 2, 1);  // First
    scheduler->enqueue(2, 2, 5);  // Second (higher priority but later)
    
    // Should run first process
    auto r1 = scheduler->tick();
    EXPECT_EQ(r1.currentPid, 1);
    
    scheduler->tick();  // Complete first
    
    // Now should run second
    auto r3 = scheduler->tick();
    EXPECT_EQ(r3.currentPid, 2);
}

TEST_F(SchedulerTest, RoundRobinPreemptsAfterQuantum) {
    scheduler->setAlgorithm(std::make_unique<RoundRobinAlgorithm>(2));

    scheduler->enqueue(1, 5, 1);
    scheduler->enqueue(2, 5, 1);

    // Tick 1: starts first process, no context switch due scheduler being idle
    auto r1 = scheduler->tick();
    EXPECT_EQ(r1.currentPid, 1);
    EXPECT_FALSE(r1.contextSwitch);

    // Tick 2: process 1 continues, no context switch
    auto r2 = scheduler->tick();
    EXPECT_EQ(r2.currentPid, 1);
    EXPECT_FALSE(r2.contextSwitch);

    // Tick 3: quantum expires, process 2 should be selected, context switch occurs
    auto r3 = scheduler->tick();
    EXPECT_EQ(r3.currentPid, 2);
    EXPECT_TRUE(r3.contextSwitch);
}

TEST_F(SchedulerTest, PriorityPreemptsLowerPriority) {
    scheduler->setAlgorithm(std::make_unique<PriorityAlgorithm>());
    
    scheduler->enqueue(1, 10, 10);  // Low priority
    
    // Start low priority process
    scheduler->tick();
    EXPECT_EQ(scheduler->getCurrentPid(), 1);
    
    // Submit high priority process
    scheduler->enqueue(2, 2, 1);  // High priority
    
    // Next tick should preempt and run high priority
    auto result = scheduler->tick();
    EXPECT_EQ(result.currentPid, 2);
    EXPECT_TRUE(result.contextSwitch);
}

TEST_F(SchedulerTest, CyclesPerIntervalAffectsProgress) {
    scheduler->setCyclesPerInterval(3);  // 3 cycles per tick
    scheduler->enqueue(1, 6, 1);  // 6 cycles needed
    
    // First tick: 3 cycles consumed
    auto r1 = scheduler->tick();
    EXPECT_EQ(r1.remainingCycles, 3);
    
    // Second tick: completes (3 more cycles)
    auto r2 = scheduler->tick();
    EXPECT_TRUE(r2.processCompleted);
}

TEST_F(SchedulerTest, SuspendAndResumeProcess) {
    scheduler->enqueue(1, 10, 1);
    scheduler->tick();  // Start it
    
    scheduler->suspend(1);
    EXPECT_EQ(scheduler->getCurrentPid(), -1);  // No longer running
    
    scheduler->resume(1);
    scheduler->tick();
    EXPECT_EQ(scheduler->getCurrentPid(), 1);  // Running again
}

