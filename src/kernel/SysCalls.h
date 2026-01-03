#pragma once

#include "kernel/SysCallsAPI.h"
#include "storage/Storage.h"
#include "kernel/Kernel.h"
#include "logger/Logger.h"
#include "scheduler/algorithms/SchedulerAlgorithm.h"
#include <string>
#include <iostream>

namespace kernel {

struct SysApiKernel : ::sys::SysApi {
    storage::StorageManager& storageManager;
    Kernel* kernelOwner{nullptr};
    explicit SysApiKernel(storage::StorageManager& sm, Kernel* owner = nullptr)
        : storageManager(sm), kernelOwner(owner) {}

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
        if (kernelOwner) {
            info = kernelOwner->getSysInfo();
        }
        return info;
    }
    
    void* allocateMemory(size_t size, int processId) override {
        if (kernelOwner) {
            return kernelOwner->allocateMemory(size, processId);
        }
        return nullptr;
    }
    
    ::sys::SysResult deallocateMemory(void* ptr) override {
        if (kernelOwner) {
            return kernelOwner->deallocateMemory(ptr);
        }
        return ::sys::SysResult::Error;
    }
    
    void freeProcessMemory(int processId) override {
        if (kernelOwner) {
            kernelOwner->freeProcessMemory(processId);
        }
    }
    
    void scheduleProcess(int pid, int cpuCycles, int priority) override {
        if (kernelOwner) {
            kernelOwner->scheduleProcess(pid, cpuCycles, priority);
        }
    }
    
    void unscheduleProcess(int pid) override {
        if (kernelOwner) {
            kernelOwner->unscheduleProcess(pid);
        }
    }
    
    void suspendScheduledProcess(int pid) override {
        if (kernelOwner) {
            kernelOwner->suspendScheduledProcess(pid);
        }
    }
    
    void resumeScheduledProcess(int pid) override {
        if (kernelOwner) {
            kernelOwner->resumeScheduledProcess(pid);
        }
    }

    void requestShutdown() override {
        if (kernelOwner) kernelOwner->handleQuit(std::vector<std::string>());
    }

    void sendSignal(int signal) override {
        if (kernelOwner) kernelOwner->handleInterruptSignal(signal);
    }
    
    ::sys::SysResult sendSignalToProcess(int pid, int signal) override {
        if (kernelOwner) {
            bool success = kernelOwner->sendSignalToProcess(pid, signal);
            return success ? ::sys::SysResult::OK : ::sys::SysResult::Error;
        }
        return ::sys::SysResult::Error;
    }
    
    int fork(const std::string& name, int cpuTimeNeeded, int memoryNeeded, int priority = 0, bool persistent = false) override {
        if (kernelOwner) {
            return kernelOwner->forkProcess(name, cpuTimeNeeded, memoryNeeded, priority, persistent);
        }
        return -1;
    }
    
    std::vector<::sys::SysApi::ProcessInfo> getProcessList() override {
        if (kernelOwner) {
            return kernelOwner->getProcessList();
        }
        return {};
    }
    
    bool processExists(int pid) override {
        if (kernelOwner) {
            return kernelOwner->processExists(pid);
        }
        return false;
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
        if (kernelOwner) {
            return kernelOwner->addCPUWork(pid, cpuCycles);
        }
        return false;
    }
    
    bool waitForProcess(int pid) override {
        if (kernelOwner) {
            return kernelOwner->waitForProcess(pid);
        }
        return false;
    }
    
    bool exit(int pid, int exitCode = 0) override {
        if (kernelOwner) {
            return kernelOwner->exit(pid, exitCode);
        }
        return false;
    }
    
    bool reapProcess(int pid) override {
        if (kernelOwner) {
            return kernelOwner->reapProcess(pid);
        }
        return false;
    }
    
    bool isProcessComplete(int pid) override {
        if (kernelOwner) {
            return kernelOwner->isProcessComplete(pid);
        }
        return true; // If no kernel, consider it complete
    }
    
    int getProcessRemainingCycles(int pid) override {
        if (kernelOwner) {
            return kernelOwner->getProcessRemainingCycles(pid);
        }
        return -1;
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
