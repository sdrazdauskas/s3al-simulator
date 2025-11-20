#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Shell.h"
#include "CommandsInit.h"
#include "SysCallsAPI.h"
#include "Logger.h"
#include <sstream>

using namespace shell;
using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;
using ::testing::DoAll;
using ::testing::SetArgReferee;

class MockSysApi : public SysApi {
public:
    MOCK_METHOD(SysResult, fileExists, (const std::string& name), (override));
    MOCK_METHOD(SysResult, readFile, (const std::string& name, std::string& out), (override));
    MOCK_METHOD(SysResult, createFile, (const std::string& name), (override));
    MOCK_METHOD(SysResult, deleteFile, (const std::string& name), (override));
    MOCK_METHOD(SysResult, writeFile, (const std::string& name, const std::string& content), (override));
    MOCK_METHOD(SysResult, editFile, (const std::string& name, const std::string& newContent), (override));
    MOCK_METHOD(SysResult, copyFile, (const std::string& src, const std::string& dest), (override));
    MOCK_METHOD(SysResult, moveFile, (const std::string& src, const std::string& dest), (override));
    MOCK_METHOD(SysResult, appendFile, (const std::string& name, const std::string& content), (override));
    MOCK_METHOD(std::string, getWorkingDir, (), (override));
    MOCK_METHOD(SysResult, listDir, (const std::string& path, std::vector<std::string>& out), (override));
    MOCK_METHOD(SysResult, makeDir, (const std::string& name), (override));
    MOCK_METHOD(SysResult, removeDir, (const std::string& name), (override));
    MOCK_METHOD(SysResult, changeDir, (const std::string& name), (override));
    MOCK_METHOD(SysResult, copyDir, (const std::string& src, const std::string& dest), (override));
    MOCK_METHOD(SysResult, moveDir, (const std::string& src, const std::string& dest), (override));
    MOCK_METHOD(SysResult, saveToDisk, (const std::string& fileName), (override));
    MOCK_METHOD(SysResult, loadFromDisk, (const std::string& fileName), (override));
    MOCK_METHOD(SysResult, resetStorage, (), (override));
    MOCK_METHOD(SysResult, listDataFiles, (std::vector<std::string>& out), (override));
    MOCK_METHOD(SysInfo, get_sysinfo, (), (override));
    MOCK_METHOD(void, requestShutdown, (), (override));
};

class ShellTest : public ::testing::Test {
protected:
    NiceMock<MockSysApi> mock_sys;
    std::unique_ptr<CommandRegistry> registry;
    
    void SetUp() override {
        registry = std::make_unique<CommandRegistry>();
        init_commands(*registry);
        
        // Setup default return values
        ON_CALL(mock_sys, getWorkingDir()).WillByDefault(Return("/"));
    }
};

TEST_F(ShellTest, TouchCommandFileExists) {
    Shell shell(mock_sys, *registry);
    std::ostringstream output;
    
    EXPECT_CALL(mock_sys, createFile("test.txt")).WillOnce(Return(SysResult::AlreadyExists));
    
    shell.setOutputCallback([&output](const std::string& str) {
        output << str;
    });
    
    shell.processCommandLine("touch test.txt");
    
    // Test behavior: Shell should display appropriate error message
    std::string out = output.str();
    EXPECT_TRUE(out.find("exist") != std::string::npos || 
                out.find("Exist") != std::string::npos ||
                out.find("already") != std::string::npos ||
                out.find("Already") != std::string::npos);
}

TEST_F(ShellTest, LsCommandDisplaysFiles) {
    Shell shell(mock_sys, *registry);
    std::ostringstream output;
    std::vector<std::string> files = {"file1.txt", "file2.txt", "dir1"};
    
    EXPECT_CALL(mock_sys, listDir(_, _))
        .WillOnce(DoAll(
            SetArgReferee<1>(files),
            Return(SysResult::OK)
        ));
    
    shell.setOutputCallback([&output](const std::string& str) {
        output << str;
    });
    
    shell.processCommandLine("ls");
    
    // Test behavior: Shell should display all files in output
    std::string out = output.str();
    EXPECT_TRUE(out.find("file1.txt") != std::string::npos);
    EXPECT_TRUE(out.find("file2.txt") != std::string::npos);
    EXPECT_TRUE(out.find("dir1") != std::string::npos);
}

TEST_F(ShellTest, EmptyCommand) {
    Shell shell(mock_sys, *registry);
    
    // Test behavior: Should handle empty command gracefully without crashing
    EXPECT_NO_THROW(shell.processCommandLine(""));
    EXPECT_NO_THROW(shell.processCommandLine("   "));
}

TEST_F(ShellTest, MemInfoCommand) {
    Shell shell(mock_sys, *registry);
    SysApi::SysInfo info;
    info.total_memory = 4096;  // 4 KB
    info.used_memory = 2048;   // 2 KB
    
    EXPECT_CALL(mock_sys, get_sysinfo()).WillOnce(Return(info));
    
    std::ostringstream output;
    shell.setOutputCallback([&output](const std::string& str) {
        output << str;
    });
    
    shell.processCommandLine("meminfo");
    
    // Test behavior: Shell should display memory information
    std::string out = output.str();
    EXPECT_FALSE(out.empty());
    // Values are displayed in KB (divided by 1024), so 4096 -> 4, 2048 -> 2
    EXPECT_TRUE(out.find("Total") != std::string::npos);
    EXPECT_TRUE(out.find("4 KB") != std::string::npos);  // 4096 / 1024 = 4
    EXPECT_TRUE(out.find("2 KB") != std::string::npos);  // 2048 / 1024 = 2 (used or free)
}
