#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ProcessManager.h"
#include "MemoryManager.h"
#include "Scheduler.h"
#include "Logger.h"

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
    
    MOCK_METHOD(void*, allocate, (size_t size, int process_id), (override));
    MOCK_METHOD(void, deallocate, (void* ptr), (override));
    MOCK_METHOD(void, free_process_memory, (int process_id), (override));
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
    
    // Create process (preparation step - doesn't allocate yet)
    int pid = pm.create_process("test_process", 100, 512, 5);
    EXPECT_GT(pid, 0); // Process created successfully
    
    // Configure mock to simulate allocation failure when running the process
    EXPECT_CALL(mock_memory, allocate(512, pid))
        .WillOnce(Return(nullptr));
    
    // Configure mock for cleanup (called after execution)
    EXPECT_CALL(mock_memory, free_process_memory(pid))
        .Times(1);
    
    // Attempt to run process (this triggers memory allocation)
    bool result = pm.run_process(pid);
}
