#include <gtest/gtest.h>
#include "Kernel.h"
#include "Storage.h"
#include "MemoryManager.h"
#include "ProcessManager.h"
#include "Scheduler.h"

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
    procMgr.send_signal(pid1, 15);
    procMgr.send_signal(pid2, 15);
    procMgr.send_signal(pid3, 15);
    
    // Submit and let scheduler handle execution
    int execPid1 = procMgr.submit("exec1", 10, 512, 5);
    int execPid2 = procMgr.submit("exec2", 20, 256, 10);
    
    EXPECT_GT(execPid1, 0);
    EXPECT_GT(execPid2, 0);
    
    // After executeProcess, snapshot should be empty
    snapshot = procMgr.snapshot();
    EXPECT_EQ(snapshot.size(), 0);
    
    // As well as memory (subject to change in future versions of project)
    EXPECT_EQ(memory.get_used_memory(), 0);
}
