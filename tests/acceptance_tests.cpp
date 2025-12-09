#include <gtest/gtest.h>
#include "kernel/Kernel.h"
#include "storage/Storage.h"
#include "memory/MemoryManager.h"
#include "process/ProcessManager.h"
#include "scheduler/Scheduler.h"
#include "config/Config.h"
#include <filesystem>

using namespace kernel;
using namespace storage;
using namespace memory;
using namespace process;
using namespace scheduler;

class AcceptanceTest : public ::testing::Test {
protected:
};

// User CREATES and MAANGES files
TEST_F(AcceptanceTest, UserManagesFilesScenario) {
    StorageManager storage;
    
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
        ASSERT_EQ(storage.makeDir("my_project"), StorageManager::StorageResponse::OK);
        ASSERT_EQ(storage.changeDir("my_project"), StorageManager::StorageResponse::OK);
        ASSERT_EQ(storage.createFile("code.cpp"), StorageManager::StorageResponse::OK);
        ASSERT_EQ(storage.writeFile("code.cpp", "#include <iostream>"), StorageManager::StorageResponse::OK);
        
        ASSERT_EQ(storage.saveToDisk(session_file), StorageManager::StorageResponse::OK);
    }
    
    {
        StorageManager storage;
        
        ASSERT_EQ(storage.loadFromDisk(session_file), StorageManager::StorageResponse::OK);
        
        ASSERT_EQ(storage.changeDir("my_project"), StorageManager::StorageResponse::OK);
        EXPECT_EQ(storage.fileExists("code.cpp"), StorageManager::StorageResponse::OK);
        
        std::string content;
        ASSERT_EQ(storage.readFile("code.cpp", content), StorageManager::StorageResponse::OK);
        EXPECT_TRUE(content.find("#include <iostream>") != std::string::npos);
    }
    
    std::filesystem::remove(session_file);
}

// Sys init
TEST_F(AcceptanceTest, SystemInitializationScenario) {
    config::Config cfg;
    cfg.memorySize = 4096;
    Kernel kernel(cfg);
    
    EXPECT_TRUE(kernel.isKernelRunning());
    
    auto sysinfo = kernel.getSysInfo();
    EXPECT_EQ(sysinfo.totalMemory, 4096);
    EXPECT_EQ(sysinfo.usedMemory, 0);
    
    std::string result = kernel.executeCommand("ls");
    EXPECT_FALSE(result.empty());
    
    EXPECT_TRUE(kernel.isKernelRunning());
}
