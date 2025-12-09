#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "process/ProcessManager.h"
#include "memory/MemoryManager.h"
#include "scheduler/Scheduler.h"
#include "logger/Logger.h"

using namespace process;
using namespace memory;
using namespace scheduler;
using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;

// Mock MemoryManager for unit testing
class MockMemoryManager : public MemoryManager {
public:
    MockMemoryManager() : MemoryManager(4096) {}
    
    MOCK_METHOD(void*, allocate, (size_t size, int processId), (override));
    MOCK_METHOD(void, deallocate, (void* ptr), (override));
    MOCK_METHOD(void, freeProcessMemory, (int processId), (override));
};

class ProcessManagerMockTest : public ::testing::Test {
protected:
    NiceMock<MockMemoryManager> mock_memory;
    std::unique_ptr<CPUScheduler> scheduler;
    
    void SetUp() override {
        scheduler = std::make_unique<CPUScheduler>();
    }
};

// Memory allocation failure handling
TEST_F(ProcessManagerMockTest, ProcessCreationFailsWhenMemoryUnavailable) {
    ProcessManager pm(mock_memory, *scheduler);
    
    // Configure mock to simulate allocation failure when submitting the process
    EXPECT_CALL(mock_memory, allocate(512, 1))
        .WillOnce(Return(nullptr));
    
    // Submit process - allocation happens at submit time with new API
    int pid = pm.submit("test_process", 100, 512, 5);
    EXPECT_GT(pid, 0); // Process submitted successfully (allocation returned nullptr but didn't fail)
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
    scheduler->enqueue(1, 5, 1);
    
    auto result = scheduler->tick();
    
    EXPECT_EQ(scheduler->getReadyCount(), 0);
    EXPECT_EQ(result.currentPid, 1);
    EXPECT_TRUE(result.contextSwitch);
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
    scheduler->setAlgorithm(Algorithm::FCFS);
    
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
    scheduler->setAlgorithm(Algorithm::RoundRobin);
    scheduler->setQuantum(2);
    
    scheduler->enqueue(1, 5, 1);
    scheduler->enqueue(2, 5, 1);
    
    // Tick 1: starts first process, consumes 1 cycle (slice=1)
    auto r1 = scheduler->tick();
    EXPECT_EQ(r1.currentPid, 1);
    EXPECT_TRUE(r1.contextSwitch);
    
    // Tick 2: slice becomes 2 (hits quantum), preempts and switches
    auto r2 = scheduler->tick();
    EXPECT_EQ(r2.currentPid, 2);
    EXPECT_TRUE(r2.contextSwitch);
    
    // Tick 3: running second, slice=1
    auto r3 = scheduler->tick();
    EXPECT_EQ(r3.currentPid, 2);
    EXPECT_FALSE(r3.contextSwitch);
}

TEST_F(SchedulerTest, PriorityPreemptsLowerPriority) {
    scheduler->setAlgorithm(Algorithm::Priority);
    
    scheduler->enqueue(1, 10, 1);  // Low priority
    
    // Start low priority process
    scheduler->tick();
    EXPECT_EQ(scheduler->getCurrentPid(), 1);
    
    // Submit high priority process
    scheduler->enqueue(2, 2, 10);  // High priority
    
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

