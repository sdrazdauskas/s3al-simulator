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
    
    // Configure mock to simulate allocation failure during process creation
    EXPECT_CALL(mock_memory, allocate(512, testing::_))
        .WillOnce(Return(nullptr));
    
    // No memory gets freed, because allocation never succeeded
    EXPECT_CALL(mock_memory, free_process_memory(testing::_))
        .Times(0);
    
    // Attempt to create the process
    int pid = pm.create_process("test_process", 100, 512, 5);
    
    // Creation should fail due to memory allocation failure
    EXPECT_EQ(pid, -1);
}
