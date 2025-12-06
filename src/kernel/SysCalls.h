#pragma once
#include "SysCallsAPI.h"
#include "Storage.h"
#include "Kernel.h"
#include "Logger.h"
#include <string>
#include <iostream>

namespace kernel {

struct SysApiKernel : ::shell::SysApi {
    storage::StorageManager& storageManager;
    Kernel* kernelOwner{nullptr};
    explicit SysApiKernel(storage::StorageManager& sm, Kernel* owner = nullptr)
        : storageManager(sm), kernelOwner(owner) {}

    ::shell::SysResult readFile(const std::string& name, std::string& out) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.readFile(name, out);
        switch(res) {
            case Resp::OK: return ::shell::SysResult::OK;
            case Resp::NotFound: return ::shell::SysResult::NotFound;
            case Resp::InvalidArgument: return ::shell::SysResult::InvalidArgument;
            default: return ::shell::SysResult::Error;
        }
    }

    ::shell::SysResult createFile(const std::string& name) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.touchFile(name);
        switch(res) {
            case Resp::OK: return ::shell::SysResult::OK;
            case Resp::AlreadyExists: return ::shell::SysResult::AlreadyExists;
            case Resp::NotFound: return ::shell::SysResult::NotFound;
            case Resp::AtRoot: return ::shell::SysResult::AtRoot;
            case Resp::InvalidArgument: return ::shell::SysResult::InvalidArgument;
            default: return ::shell::SysResult::Error;
        }
    }

    ::shell::SysResult editFile(const std::string& name, const std::string& newContent) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.editFile(name, newContent);
        switch (res) {
            case Resp::OK: return ::shell::SysResult::OK;
            case Resp::NotFound: return ::shell::SysResult::NotFound;
            case Resp::InvalidArgument: return ::shell::SysResult::InvalidArgument;
            default: return ::shell::SysResult::Error;
        }
    }
    
    ::shell::SysResult deleteFile(const std::string& name) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.deleteFile(name);
        switch(res) {
            case Resp::OK: return ::shell::SysResult::OK;
            case Resp::NotFound: return ::shell::SysResult::NotFound;
            case Resp::InvalidArgument: return ::shell::SysResult::InvalidArgument;
            default: return ::shell::SysResult::Error;
        }
    }

    shell::SysResult appendFile(const std::string& name, const std::string& content) override {
        std::string existing;
        auto readRes = readFile(name, existing);
        if (readRes != shell::SysResult::OK)
            return readRes;

        existing += content;
        return writeFile(name, existing);
    }

    ::shell::SysResult writeFile(const std::string& name, const std::string& content) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.writeFile(name, content);
        switch(res) {
            case Resp::OK: return ::shell::SysResult::OK;
            case Resp::NotFound: return ::shell::SysResult::NotFound;
            case Resp::InvalidArgument: return ::shell::SysResult::InvalidArgument;
            default: return ::shell::SysResult::Error;
        }
    }

    ::shell::SysResult makeDir(const std::string& name) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.makeDir(name);
        switch(res) {
            case Resp::OK: return ::shell::SysResult::OK;
            case Resp::NotFound: return shell::SysResult::NotFound;
            case Resp::AlreadyExists: return ::shell::SysResult::AlreadyExists;
            case Resp::InvalidArgument: return ::shell::SysResult::InvalidArgument;
            default: return ::shell::SysResult::Error;
        }
    }

    ::shell::SysResult removeDir(const std::string& name) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.removeDir(name);
        switch(res) {
            case Resp::OK: return ::shell::SysResult::OK;
            case Resp::NotFound: return ::shell::SysResult::NotFound;
            case Resp::InvalidArgument: return ::shell::SysResult::InvalidArgument;
            default: return ::shell::SysResult::Error;
        }
    }

    ::shell::SysResult changeDir(const std::string& name) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.changeDir(name);
        switch(res) {
            case Resp::OK: return ::shell::SysResult::OK;
            case Resp::AtRoot: return shell::SysResult::AtRoot;
            case Resp::NotFound: return ::shell::SysResult::NotFound;
            case Resp::InvalidArgument: return ::shell::SysResult::InvalidArgument;
            default: return ::shell::SysResult::Error;
        }
    }

    ::shell::SysResult saveToDisk(const std::string& fileName) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.saveToDisk(fileName);
        switch (res) {
            case Resp::OK: return ::shell::SysResult::OK;
            case Resp::InvalidArgument: return ::shell::SysResult::InvalidArgument;
            default: return ::shell::SysResult::Error;
        }
    }

    ::shell::SysResult loadFromDisk(const std::string& fileName) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.loadFromDisk(fileName);
        switch (res) {
            case Resp::OK: return ::shell::SysResult::OK;
            case Resp::NotFound: return ::shell::SysResult::NotFound;
            case Resp::InvalidArgument: return ::shell::SysResult::InvalidArgument;
            default: return ::shell::SysResult::Error;
        }
    }

    ::shell::SysResult listDataFiles(std::vector<std::string>& out) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.listDataFiles(out);
        switch (res) {
            case Resp::OK: return shell::SysResult::OK;
            case Resp::NotFound: return shell::SysResult::NotFound;
            default: return shell::SysResult::Error;
        }
    }

    ::shell::SysResult resetStorage() override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.reset();
        switch (res) {
            case Resp::OK: return ::shell::SysResult::OK;
            default: return ::shell::SysResult::Error;
        }
    }

    ::shell::SysResult copyFile(const std::string& src, const std::string& dest) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.copyFile(src, dest);
        switch (res) {
            case Resp::OK: return shell::SysResult::OK;
            case Resp::AlreadyExists: return shell::SysResult::AlreadyExists;
            case Resp::NotFound: return shell::SysResult::NotFound;
            case Resp::InvalidArgument: return shell::SysResult::InvalidArgument;
            default: return shell::SysResult::Error;
        }
    }

    ::shell::SysResult copyDir(const std::string& src, const std::string& dest) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.copyDir(src, dest);
        switch (res) {
            case Resp::OK: return shell::SysResult::OK;
            case Resp::AlreadyExists: return shell::SysResult::AlreadyExists;
            case Resp::NotFound: return shell::SysResult::NotFound;
            case Resp::InvalidArgument: return shell::SysResult::InvalidArgument;
            default: return shell::SysResult::Error;
        }
    }

    ::shell::SysResult moveFile(const std::string& src, const std::string& dest) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.moveFile(src, dest);
        switch (res) {
            case Resp::OK: return shell::SysResult::OK;
            case Resp::AlreadyExists: return shell::SysResult::AlreadyExists;
            case Resp::NotFound: return shell::SysResult::NotFound;
            case Resp::InvalidArgument: return shell::SysResult::InvalidArgument;
            default: return shell::SysResult::Error;
        }
    }

    ::shell::SysResult moveDir(const std::string& src, const std::string& dest) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.moveDir(src, dest);
        switch (res) {
            case Resp::OK: return shell::SysResult::OK;
            case Resp::AlreadyExists: return shell::SysResult::AlreadyExists;
            case Resp::NotFound: return shell::SysResult::NotFound;
            case Resp::InvalidArgument: return shell::SysResult::InvalidArgument;
            default: return shell::SysResult::Error;
        }
    }

    ::shell::SysResult listDir(const std::string& path, std::vector<std::string>& out) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.listDir(path, out);
        switch (res) {
            case Resp::OK: return shell::SysResult::OK;
            case Resp::NotFound: return shell::SysResult::NotFound;
            default: return shell::SysResult::Error;
        }
    }

    std::string getWorkingDir() override {
        return storageManager.getWorkingDir();
    }

    ::shell::SysResult fileExists(const std::string& name) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = storageManager.fileExists(name);
        switch(res) {
            case Resp::OK: return ::shell::SysResult::OK;
            case Resp::NotFound: return ::shell::SysResult::NotFound;
            default: return ::shell::SysResult::Error;
        }
    }

    ::shell::SysApi::SysInfo getSysInfo() override {
        ::shell::SysApi::SysInfo info;
        if (kernelOwner) {
            info = kernelOwner->getSysInfo();
        }
        return info;
    }

    void requestShutdown() override {
        if (kernelOwner) kernelOwner->handleQuit(std::vector<std::string>());
    }

    void sendSignal(int signal) override {
        if (kernelOwner) kernelOwner->handleInterruptSignal(signal);
    }
    
    ::shell::SysResult sendSignalToProcess(int pid, int signal) override {
        if (kernelOwner) {
            bool success = kernelOwner->sendSignalToProcess(pid, signal);
            return success ? ::shell::SysResult::OK : ::shell::SysResult::Error;
        }
        return ::shell::SysResult::Error;
    }
    
    int fork(const std::string& name, int cpuTimeNeeded, int memoryNeeded, int priority = 0, bool persistent = false) override {
        if (kernelOwner) {
            return kernelOwner->forkProcess(name, cpuTimeNeeded, memoryNeeded, priority, persistent);
        }
        return -1;
    }
    
    std::vector<::shell::SysApi::ProcessInfo> getProcessList() override {
        if (kernelOwner) {
            return kernelOwner->getProcessList();
        }
        return {};
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
    

    int submitCommand(const std::string& name, int cpuCycles, int priority = 0) override {
        if (kernelOwner) {
            return kernelOwner->submitAsyncCommand(name, cpuCycles, priority);
        }
        return -1;
    }
    
    bool waitForProcess(int pid) override {
        if (kernelOwner) {
            return kernelOwner->waitForProcess(pid);
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
    
private:
    bool savedConsoleLogging = false;
};

} // namespace kernel
