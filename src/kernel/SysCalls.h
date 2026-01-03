#pragma once

#include "kernel/SysCallsAPI.h"
#include "storage/Storage.h"
#include "kernel/Kernel.h"
#include "logger/Logger.h"
#include "scheduler/algorithms/SchedulerAlgorithm.h"
#include "process/Process.h"
#include <string>
#include <iostream>

namespace kernel {

struct SysApiKernel : ::sys::SysApi {
    storage::StorageManager& storageManager;
    memory::MemoryManager& memoryManager;
    process::ProcessManager& processManager;
    scheduler::CPUScheduler& scheduler;
    Kernel* kernelOwner{nullptr};
    
    explicit SysApiKernel(
        storage::StorageManager& sm,
        memory::MemoryManager& mm,
        process::ProcessManager& pm,
        scheduler::CPUScheduler& sched,
        Kernel* owner = nullptr)
        : storageManager(sm), memoryManager(mm), processManager(pm), scheduler(sched), kernelOwner(owner) {}

    Kernel* getKernel() { return kernelOwner; }

    ::sys::SysResult readFile(const std::string& name, std::string& out) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.readFile(name, out);
        switch(res) {
            case Resp::OK: return ::sys::SysResult::OK;
            case Resp::NotFound: return ::sys::SysResult::NotFound;
            case Resp::InvalidArgument: return ::sys::SysResult::InvalidArgument;
            default: return ::sys::SysResult::Error;
        }
    }

    ::sys::SysResult createFile(const std::string& name) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.touchFile(name);
        switch(res) {
            case Resp::OK: return ::sys::SysResult::OK;
            case Resp::AlreadyExists: return ::sys::SysResult::AlreadyExists;
            case Resp::NotFound: return ::sys::SysResult::NotFound;
            case Resp::AtRoot: return ::sys::SysResult::AtRoot;
            case Resp::InvalidArgument: return ::sys::SysResult::InvalidArgument;
            default: return ::sys::SysResult::Error;
        }
    }

    ::sys::SysResult editFile(const std::string& name, const std::string& newContent) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.editFile(name, newContent);
        switch (res) {
            case Resp::OK: return ::sys::SysResult::OK;
            case Resp::NotFound: return ::sys::SysResult::NotFound;
            case Resp::InvalidArgument: return ::sys::SysResult::InvalidArgument;
            default: return ::sys::SysResult::Error;
        }
    }
    
    ::sys::SysResult deleteFile(const std::string& name) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.deleteFile(name);
        switch(res) {
            case Resp::OK: return ::sys::SysResult::OK;
            case Resp::NotFound: return ::sys::SysResult::NotFound;
            case Resp::InvalidArgument: return ::sys::SysResult::InvalidArgument;
            default: return ::sys::SysResult::Error;
        }
    }

    sys::SysResult appendFile(const std::string& name, const std::string& content) override {
        std::string existing;
        auto readRes = readFile(name, existing);
        if (readRes != sys::SysResult::OK)
            return readRes;

        existing += content;
        return writeFile(name, existing);
    }

    ::sys::SysResult writeFile(const std::string& name, const std::string& content) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.writeFile(name, content);
        switch(res) {
            case Resp::OK: return ::sys::SysResult::OK;
            case Resp::NotFound: return ::sys::SysResult::NotFound;
            case Resp::InvalidArgument: return ::sys::SysResult::InvalidArgument;
            default: return ::sys::SysResult::Error;
        }
    }

    ::sys::SysResult makeDir(const std::string& name) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.makeDir(name);
        switch(res) {
            case Resp::OK: return ::sys::SysResult::OK;
            case Resp::NotFound: return sys::SysResult::NotFound;
            case Resp::AlreadyExists: return ::sys::SysResult::AlreadyExists;
            case Resp::InvalidArgument: return ::sys::SysResult::InvalidArgument;
            default: return ::sys::SysResult::Error;
        }
    }

    ::sys::SysResult removeDir(const std::string& name) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.removeDir(name);
        switch(res) {
            case Resp::OK: return ::sys::SysResult::OK;
            case Resp::NotFound: return ::sys::SysResult::NotFound;
            case Resp::InvalidArgument: return ::sys::SysResult::InvalidArgument;
            default: return ::sys::SysResult::Error;
        }
    }

    ::sys::SysResult changeDir(const std::string& name) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.changeDir(name);
        switch(res) {
            case Resp::OK: return ::sys::SysResult::OK;
            case Resp::AtRoot: return sys::SysResult::AtRoot;
            case Resp::NotFound: return ::sys::SysResult::NotFound;
            case Resp::InvalidArgument: return ::sys::SysResult::InvalidArgument;
            default: return ::sys::SysResult::Error;
        }
    }

    ::sys::SysResult saveToDisk(const std::string& fileName) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.saveToDisk(fileName);
        switch (res) {
            case Resp::OK: return ::sys::SysResult::OK;
            case Resp::InvalidArgument: return ::sys::SysResult::InvalidArgument;
            default: return ::sys::SysResult::Error;
        }
    }

    ::sys::SysResult loadFromDisk(const std::string& fileName) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.loadFromDisk(fileName);
        switch (res) {
            case Resp::OK: return ::sys::SysResult::OK;
            case Resp::NotFound: return ::sys::SysResult::NotFound;
            case Resp::InvalidArgument: return ::sys::SysResult::InvalidArgument;
            default: return ::sys::SysResult::Error;
        }
    }

    ::sys::SysResult listDataFiles(std::vector<std::string>& out) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.listDataFiles(out);
        switch (res) {
            case Resp::OK: return sys::SysResult::OK;
            case Resp::NotFound: return sys::SysResult::NotFound;
            default: return sys::SysResult::Error;
        }
    }

    ::sys::SysResult resetStorage() override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.reset();
        switch (res) {
            case Resp::OK: return ::sys::SysResult::OK;
            default: return ::sys::SysResult::Error;
        }
    }

    ::sys::SysResult copyFile(const std::string& src, const std::string& dest) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.copyFile(src, dest);
        switch (res) {
            case Resp::OK: return sys::SysResult::OK;
            case Resp::AlreadyExists: return sys::SysResult::AlreadyExists;
            case Resp::NotFound: return sys::SysResult::NotFound;
            case Resp::InvalidArgument: return sys::SysResult::InvalidArgument;
            default: return sys::SysResult::Error;
        }
    }

    ::sys::SysResult copyDir(const std::string& src, const std::string& dest) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.copyDir(src, dest);
        switch (res) {
            case Resp::OK: return sys::SysResult::OK;
            case Resp::AlreadyExists: return sys::SysResult::AlreadyExists;
            case Resp::NotFound: return sys::SysResult::NotFound;
            case Resp::InvalidArgument: return sys::SysResult::InvalidArgument;
            default: return sys::SysResult::Error;
        }
    }

    ::sys::SysResult moveFile(const std::string& src, const std::string& dest) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.moveFile(src, dest);
        switch (res) {
            case Resp::OK: return sys::SysResult::OK;
            case Resp::AlreadyExists: return sys::SysResult::AlreadyExists;
            case Resp::NotFound: return sys::SysResult::NotFound;
            case Resp::InvalidArgument: return sys::SysResult::InvalidArgument;
            default: return sys::SysResult::Error;
        }
    }

    ::sys::SysResult moveDir(const std::string& src, const std::string& dest) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.moveDir(src, dest);
        switch (res) {
            case Resp::OK: return sys::SysResult::OK;
            case Resp::AlreadyExists: return sys::SysResult::AlreadyExists;
            case Resp::NotFound: return sys::SysResult::NotFound;
            case Resp::InvalidArgument: return sys::SysResult::InvalidArgument;
            default: return sys::SysResult::Error;
        }
    }

    ::sys::SysResult listDir(const std::string& path, std::vector<std::string>& out) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.listDir(path, out);
        switch (res) {
            case Resp::OK: return sys::SysResult::OK;
            case Resp::NotFound: return sys::SysResult::NotFound;
            default: return sys::SysResult::Error;
        }
    }

    std::string getWorkingDir() override {
        return storageManager.getWorkingDir();
    }

    ::sys::SysResult fileExists(const std::string& name) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.fileExists(name);
        switch(res) {
            case Resp::OK: return ::sys::SysResult::OK;
            case Resp::NotFound: return ::sys::SysResult::NotFound;
            default: return ::sys::SysResult::Error;
        }
    }

    ::sys::SysApi::SysInfo getSysInfo() override {
        ::sys::SysApi::SysInfo info;
        info.totalMemory = memoryManager.getTotalMemory();
        info.usedMemory = memoryManager.getUsedMemory();
        return info;
    }
    
    void* allocateMemory(size_t size, int processId) override {
        return memoryManager.allocate(size, processId);
    }
    
    ::sys::SysResult deallocateMemory(void* ptr) override {
        return memoryManager.deallocate(ptr) ? ::sys::SysResult::OK : ::sys::SysResult::Error;
    }
    
    void freeProcessMemory(int processId) override {
        memoryManager.freeProcessMemory(processId);
    }
    
    void scheduleProcess(int pid, int cpuCycles, int priority) override {
        scheduler.enqueue(pid, cpuCycles, priority);
    }
    
    void unscheduleProcess(int pid) override {
        scheduler.remove(pid);
    }
    
    void suspendScheduledProcess(int pid) override {
        scheduler.suspend(pid);
    }
    
    void resumeScheduledProcess(int pid) override {
        scheduler.resume(pid);
    }

    void requestShutdown() override {
        if (kernelOwner) kernelOwner->handleQuit(std::vector<std::string>());
    }

    void sendSignal(int signal) override {
        if (kernelOwner) kernelOwner->handleInterruptSignal(signal);
    }
    
    ::sys::SysResult sendSignalToProcess(int pid, int signal) override {
        return processManager.sendSignal(pid, signal) ? ::sys::SysResult::OK : ::sys::SysResult::Error;
    }
    
    int fork(const std::string& name, int cpuTimeNeeded, int memoryNeeded, int priority = 0, bool persistent = false) override {
        return processManager.submit(name, cpuTimeNeeded, memoryNeeded, priority, persistent);
    }
    
    std::vector<::sys::SysApi::ProcessInfo> getProcessList() override {
        auto processes = processManager.snapshot();
        std::vector<::sys::SysApi::ProcessInfo> result;
        for (const auto& p : processes) {
            result.push_back({
                p.getPid(),
                p.getName(),
                process::stateToString(p.getState()),
                p.getPriority()
            });
        }
        return result;
    }
    
    bool processExists(int pid) override {
        return processManager.processExists(pid);
    }
    
    std::string readLine() override {
        // Disable console logging during interactive input
        bool wasConsoleLogging = logging::Logger::getInstance().getConsoleOutput();
        logging::Logger::getInstance().setConsoleOutput(false);
        
        std::string line;
        std::getline(std::cin, line);
        
        // Restore console logging
        logging::Logger::getInstance().setConsoleOutput(wasConsoleLogging);
        return line;
    }
    
    void beginInteractiveMode() override {
        // Save and disable console logging for full-screen interactive applications
        savedConsoleLogging = logging::Logger::getInstance().getConsoleOutput();
        logging::Logger::getInstance().setConsoleOutput(false);
    }
    
    void endInteractiveMode() override {
        // Restore console logging state
        logging::Logger::getInstance().setConsoleOutput(savedConsoleLogging);
    }
    
    bool addCPUWork(int pid, int cpuCycles) override {
        return kernelOwner ? kernelOwner->addCPUWork(pid, cpuCycles) : false;
    }
    
    bool waitForProcess(int pid) override {
        return kernelOwner ? kernelOwner->waitForProcess(pid) : false;
    }
    
    bool exit(int pid, int exitCode = 0) override {
        return processManager.exit(pid, exitCode);
    }
    
    bool reapProcess(int pid) override {
        return processManager.reapProcess(pid);
    }
    
    bool isProcessComplete(int pid) override {
        // A process is complete when it has no more CPU cycles scheduled
        return scheduler.getRemainingCycles(pid) < 0;
    }
    
    int getProcessRemainingCycles(int pid) override {
        return scheduler.getRemainingCycles(pid);
    }

    bool setSchedulingAlgorithm(scheduler::SchedulerAlgorithm algo, int quantum = 0) override {
        if (kernelOwner) {
            return kernelOwner->setSchedulingAlgorithm(algo, quantum);
        }
        return false;
    }

    bool setSchedulerCyclesPerInterval(int cycles) override {
        if (kernelOwner) {
            return kernelOwner->setSchedulerCyclesPerInterval(cycles);
        }
        return false;
    }

    bool setSchedulerTickIntervalMs(int ms) override {
        if (kernelOwner) {
            return kernelOwner->setSchedulerTickIntervalMs(ms);
        }
        return false;
    }

    bool getConsoleOutput() const override {
        return logging::Logger::getInstance().getConsoleOutput();
    }
    
    void setConsoleOutput(bool enabled) override {
        logging::Logger::getInstance().setConsoleOutput(enabled);
    }
    
    std::string getLogLevel() const override {
        using LL = logging::LogLevel;
        LL lvl = logging::Logger::getInstance().getMinLevel();
        switch (lvl) {
            case LL::DEBUG: return "debug";
            case LL::INFO: return "info";
            case LL::WARNING: return "warn";
            case LL::ERROR: return "error";
            default: return "unknown";
        }
    }
    
    void setLogLevel(logging::LogLevel level) override {
        logging::Logger::getInstance().setMinLevel(level);
    }
    
private:
    bool savedConsoleLogging = false;
};

} // namespace kernel
