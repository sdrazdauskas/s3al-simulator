#include <gtest/gtest.h>
#include "kernel/Kernel.h"
#include "storage/Storage.h"
#include "memory/MemoryManager.h"
#include "process/ProcessManager.h"
#include "scheduler/Scheduler.h"
#include "scheduler/algorithms/SchedulerAlgorithm.h"
#include "kernel/SysCallsAPI.h"
#include "logger/Logger.h"
#include "config/Config.h"
#include <filesystem>
#include <map>

using namespace kernel;
using namespace storage;
using namespace memory;
using namespace process;
using namespace scheduler;

// Mock SysApi for acceptance tests
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
    
    void freeProcessMemory(int processId) override {}
    
    void scheduleProcess(int, int, int) override {}
    void unscheduleProcess(int) override {}
    void suspendScheduledProcess(int) override {}
    void resumeScheduledProcess(int) override {}
    
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

class AcceptanceTest : public ::testing::Test {
protected:
    MockSysApi mockSysApi;
};

// User CREATES and MAANGES files
TEST_F(AcceptanceTest, UserManagesFilesScenario) {
    StorageManager storage;
    storage.setSysApi(&mockSysApi);
    
    ASSERT_EQ(storage.createFile("notes.txt"), StorageManager::StorageResponse::OK);
    
    ASSERT_EQ(storage.writeFile("notes.txt", "Meeting at 3pm"), StorageManager::StorageResponse::OK);
    
    std::string content;
    ASSERT_EQ(storage.readFile("notes.txt", content), StorageManager::StorageResponse::OK);
    EXPECT_TRUE(content.find("Meeting at 3pm") != std::string::npos);
    
    ASSERT_EQ(storage.editFile("notes.txt", "\nBring documents"), StorageManager::StorageResponse::OK);
    
    ASSERT_EQ(storage.readFile("notes.txt", content), StorageManager::StorageResponse::OK);
    EXPECT_TRUE(content.find("Bring documents") != std::string::npos);
    
    ASSERT_EQ(storage.deleteFile("notes.txt"), StorageManager::StorageResponse::OK);
    EXPECT_EQ(storage.fileExists("notes.txt"), StorageManager::StorageResponse::NotFound);
}

// User ORGANIZES directories and files
TEST_F(AcceptanceTest, UserOrganizesDirectoriesScenario) {
    StorageManager storage;
    storage.setSysApi(&mockSysApi);
    
    ASSERT_EQ(storage.makeDir("work"), StorageManager::StorageResponse::OK);
    
    ASSERT_EQ(storage.changeDir("work"), StorageManager::StorageResponse::OK);
    EXPECT_TRUE(storage.getWorkingDir().find("work") != std::string::npos);
    
    ASSERT_EQ(storage.makeDir("documents"), StorageManager::StorageResponse::OK);
    ASSERT_EQ(storage.makeDir("reports"), StorageManager::StorageResponse::OK);
    
    ASSERT_EQ(storage.changeDir("documents"), StorageManager::StorageResponse::OK);
    ASSERT_EQ(storage.createFile("doc1.txt"), StorageManager::StorageResponse::OK);
    ASSERT_EQ(storage.createFile("doc2.txt"), StorageManager::StorageResponse::OK);
    
    std::vector<std::string> entries;
    ASSERT_EQ(storage.listDir(".", entries), StorageManager::StorageResponse::OK);
    EXPECT_EQ(entries.size(), 2);
    
    ASSERT_EQ(storage.changeDir("/"), StorageManager::StorageResponse::OK);
    EXPECT_EQ(storage.getWorkingDir(), "/");
    
    ASSERT_EQ(storage.removeDir("work"), StorageManager::StorageResponse::OK);
}

// User SAVES and RESTORES session
TEST_F(AcceptanceTest, UserSavesAndRestoresSessionScenario) {
    std::string session_file = "user_session.json";
    
    {
        StorageManager storage;
        storage.setSysApi(&mockSysApi);
        ASSERT_EQ(storage.makeDir("my_project"), StorageManager::StorageResponse::OK);
        ASSERT_EQ(storage.changeDir("my_project"), StorageManager::StorageResponse::OK);
        ASSERT_EQ(storage.createFile("code.cpp"), StorageManager::StorageResponse::OK);
        ASSERT_EQ(storage.writeFile("code.cpp", "#include <iostream>"), StorageManager::StorageResponse::OK);
        
        ASSERT_EQ(storage.saveToDisk(session_file), StorageManager::StorageResponse::OK);
    }
    
    {
        StorageManager storage;
        storage.setSysApi(&mockSysApi);
        
        ASSERT_EQ(storage.loadFromDisk(session_file), StorageManager::StorageResponse::OK);
        
        ASSERT_EQ(storage.changeDir("my_project"), StorageManager::StorageResponse::OK);
        EXPECT_EQ(storage.fileExists("code.cpp"), StorageManager::StorageResponse::OK);
        
        std::string content;
        ASSERT_EQ(storage.readFile("code.cpp", content), StorageManager::StorageResponse::OK);
        EXPECT_TRUE(content.find("#include <iostream>") != std::string::npos);
    }
    
    std::filesystem::remove(session_file);
}