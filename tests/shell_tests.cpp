#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "shell/Shell.h"
#include "shell/CommandsInit.h"
#include "kernel/SysCallsAPI.h"
#include "logger/Logger.h"
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
    MOCK_METHOD(SysInfo, getSysInfo, (), (override));
    MOCK_METHOD(void, requestShutdown, (), (override));
    MOCK_METHOD(void, sendSignal, (int signal), (override));
    MOCK_METHOD(SysResult, sendSignalToProcess, (int pid, int signal), (override));
    MOCK_METHOD(int, fork, (const std::string& name, int cpuTimeNeeded, int memoryNeeded, int priority, bool persistent), (override));
    MOCK_METHOD(std::vector<ProcessInfo>, getProcessList, (), (override));
    MOCK_METHOD(bool, processExists, (int pid), (override));
    MOCK_METHOD(std::string, readLine, (), (override));
    MOCK_METHOD(void, beginInteractiveMode, (), (override));
    MOCK_METHOD(void, endInteractiveMode, (), (override));
    MOCK_METHOD(bool, changeSchedulingAlgorithm, (scheduler::Algorithm algo, int quantum), (override));
    
    // Async command execution
    MOCK_METHOD(int, submitCommand, (const std::string& name, int cpuCycles, int priority), (override));
    MOCK_METHOD(bool, addCPUWork, (int pid, int cpuCycles), (override));
    MOCK_METHOD(bool, waitForProcess, (int pid), (override));
    MOCK_METHOD(bool, exit, (int pid, int exitCode), (override));
    MOCK_METHOD(bool, reapProcess, (int pid), (override));
    MOCK_METHOD(bool, isProcessComplete, (int pid), (override));
    MOCK_METHOD(int, getProcessRemainingCycles, (int pid), (override));
    MOCK_METHOD(void*, allocateMemory, (size_t, int), (override));
    MOCK_METHOD(void, deallocateMemory, (void*), (override));
};

class ShellTest : public ::testing::Test {
protected:
    NiceMock<MockSysApi> mock_sys;
    std::unique_ptr<CommandRegistry> registry;
    
    void SetUp() override {
        registry = std::make_unique<CommandRegistry>();
        initCommands(*registry);
        
        // Setup default return values
        ON_CALL(mock_sys, getWorkingDir()).WillByDefault(Return("/"));
        
        // Default async execution: instant completion
        ON_CALL(mock_sys, submitCommand(_, _, _)).WillByDefault(Return(100)); // Return a fake PID
        ON_CALL(mock_sys, waitForProcess(_)).WillByDefault(Return(true));     // Completes immediately
        ON_CALL(mock_sys, exit(_, _)).WillByDefault(Return(true));            // Exits successfully
        ON_CALL(mock_sys, reapProcess(_)).WillByDefault(Return(true));        // Reaps successfully
        ON_CALL(mock_sys, isProcessComplete(_)).WillByDefault(Return(true));
        ON_CALL(mock_sys, getProcessRemainingCycles(_)).WillByDefault(Return(-1));
        ON_CALL(mock_sys, processExists(_)).WillByDefault(Return(true));
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
    info.totalMemory = 4096;  // 4 KB
    info.usedMemory = 2048;   // 2 KB
    
    EXPECT_CALL(mock_sys, getSysInfo()).WillOnce(Return(info));
    
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
    EXPECT_TRUE(out.find("4.00") != std::string::npos);
    EXPECT_TRUE(out.find("2.00") != std::string::npos);
    EXPECT_TRUE(out.find("KB") != std::string::npos);
}

TEST_F(ShellTest, PsCommandDisplaysProcesses) {
    Shell shell(mock_sys, *registry);
    std::ostringstream output;
    
    std::vector<SysApi::ProcessInfo> processes = {
        {1, "init", "running", 0},
        {2, "test_proc", "ready", 5}
    };
    
    EXPECT_CALL(mock_sys, getProcessList()).WillOnce(Return(processes));
    
    shell.setOutputCallback([&output](const std::string& str) {
        output << str;
    });
    
    shell.processCommandLine("ps");
    
    // Test behavior: Shell should display process list
    std::string out = output.str();
    EXPECT_TRUE(out.find("init") != std::string::npos);
    EXPECT_TRUE(out.find("test_proc") != std::string::npos);
}

TEST_F(ShellTest, KillCommandSendsSignal) {
    Shell shell(mock_sys, *registry);
    std::ostringstream output;
    
    EXPECT_CALL(mock_sys, sendSignalToProcess(123, 9)).WillOnce(Return(SysResult::OK));
    
    shell.setOutputCallback([&output](const std::string& str) {
        output << str;
    });
    
    shell.processCommandLine("kill -9 123");
    
    // Test behavior: Should successfully send signal to process
    std::string out = output.str();
    EXPECT_FALSE(out.empty());
}
