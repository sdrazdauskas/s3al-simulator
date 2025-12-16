#include <gtest/gtest.h>
#include "kernel/Kernel.h"
#include "storage/Storage.h"
#include "memory/MemoryManager.h"
#include "process/ProcessManager.h"
#include "scheduler/Scheduler.h"

using namespace kernel;
using namespace storage;
using namespace memory;
using namespace process;
using namespace scheduler;


class IntegrationTest : public ::testing::Test {
protected:
};

// ProcessManager + MemoryManager + CPUScheduler
TEST_F(IntegrationTest, ProcessSchedulerIntegration) {
    MemoryManager memory(4096);
    CPUScheduler scheduler;
    ProcessManager procMgr(memory, scheduler);
    
    int pid1 = procMgr.submit("proc1", 10, 512, 5);
    int pid2 = procMgr.submit("proc2", 20, 256, 10);
    int pid3 = procMgr.submit("proc3", 15, 128, 3);
    
    EXPECT_GT(pid1, 0);
    EXPECT_GT(pid2, 0);
    EXPECT_GT(pid3, 0);
    
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
    
    // After reaping, snapshot should be empty
    snapshot = procMgr.snapshot();
    EXPECT_EQ(snapshot.size(), 0);
    
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
    
    // Memory should be freed
    EXPECT_EQ(memory.getUsedMemory(), 0);
}
