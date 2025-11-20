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
    ProcessManager proc_mgr(memory, scheduler);
    
    int pid1 = proc_mgr.create_process("proc1", 10, 512, 5);
    int pid2 = proc_mgr.create_process("proc2", 20, 256, 10);
    int pid3 = proc_mgr.create_process("proc3", 15, 128, 3);
    
    EXPECT_GT(pid1, 0);
    EXPECT_GT(pid2, 0);
    EXPECT_GT(pid3, 0);
    
    auto snapshot = proc_mgr.snapshot();
    EXPECT_EQ(snapshot.size(), 3);
    
    proc_mgr.stop_process(pid1);
    proc_mgr.stop_process(pid2);
    proc_mgr.stop_process(pid3);
    
    int exec_pid1 = proc_mgr.execute_process("exec1", 10, 512, 5);
    int exec_pid2 = proc_mgr.execute_process("exec2", 20, 256, 10);
    
    EXPECT_GT(exec_pid1, 0);
    EXPECT_GT(exec_pid2, 0);
    
    // After execute_process, snapshot should be empty
    snapshot = proc_mgr.snapshot();
    EXPECT_EQ(snapshot.size(), 0);
    
    // As well as memory (subject to change in future versions of project)
    EXPECT_EQ(memory.get_used_memory(), 0);
}
