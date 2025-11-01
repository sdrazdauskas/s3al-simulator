// kernels syscall implementations
#pragma once
#include "SysCallsAPI.h"
#include "Storage.h"
#include <string>

struct SysApiKernel : shell::SysApi {
    storage::StorageManager& fs;
    explicit SysApiKernel(storage::StorageManager& sm) : fs(sm) {}

    bool readFile(const std::string& name, std::string& out) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = fs.readFile(name, out);
        return res == Resp::OK;
    }

    shell::SysResult createFile(const std::string& name) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = fs.createFile(name);
        switch(res) {
            case Resp::OK: return shell::SysResult::OK;
            case Resp::AlreadyExists: return shell::SysResult::AlreadyExists;
            case Resp::NotFound: return shell::SysResult::NotFound;
            case Resp::AtRoot: return shell::SysResult::AtRoot;
            case Resp::InvalidArgument: return shell::SysResult::InvalidArgument;
            default: return shell::SysResult::Error;
        }
    }
    
    shell::SysResult deleteFile(const std::string& name) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = fs.deleteFile(name);
        switch(res) {
            case Resp::OK: return shell::SysResult::OK;
            case Resp::NotFound: return shell::SysResult::NotFound;
            case Resp::InvalidArgument: return shell::SysResult::InvalidArgument;
            default: return shell::SysResult::Error;
        }
    }

    shell::SysResult writeFile(const std::string& name, const std::string& content) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = fs.writeFile(name, content);
        switch(res) {
            case Resp::OK: return shell::SysResult::OK;
            case Resp::NotFound: return shell::SysResult::NotFound;
            case Resp::InvalidArgument: return shell::SysResult::InvalidArgument;
            default: return shell::SysResult::Error;
        }
    }

    shell::SysResult appendToFile(const std::string& name, const std::string& content) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = fs.appendToFile(name, content);
        switch(res) {
            case Resp::OK: return shell::SysResult::OK;
            case Resp::NotFound: return shell::SysResult::NotFound;
            case Resp::InvalidArgument: return shell::SysResult::InvalidArgument;
            default: return shell::SysResult::Error;
        }
    }

    shell::SysResult makeDir(const std::string& name) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = fs.makeDir(name);
        switch(res) {
            case Resp::OK: return shell::SysResult::OK;
            case Resp::AlreadyExists: return shell::SysResult::AlreadyExists;
            case Resp::InvalidArgument: return shell::SysResult::InvalidArgument;
            default: return shell::SysResult::Error;
        }
    }

    shell::SysResult removeDir(const std::string& name) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = fs.removeDir(name);
        switch(res) {
            case Resp::OK: return shell::SysResult::OK;
            case Resp::NotFound: return shell::SysResult::NotFound;
            case Resp::InvalidArgument: return shell::SysResult::InvalidArgument;
            default: return shell::SysResult::Error;
        }
    }

    shell::SysResult changeDir(const std::string& name) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = fs.changeDir(name);
        switch(res) {
            case Resp::OK: return shell::SysResult::OK;
            case Resp::NotFound: return shell::SysResult::NotFound;
            case Resp::InvalidArgument: return shell::SysResult::InvalidArgument;
            default: return shell::SysResult::Error;
        }
    }

    std::vector<std::string> listDir() override {
        return fs.listDir();
    }

    std::string getWorkingDir() override {
        return fs.getWorkingDir();
    }

    shell::SysResult fileExists(const std::string& name) override {
        using Resp = storage::StorageManager::StorageResponse;
        auto res = fs.fileExists(name);
        switch(res) {
            case Resp::OK: return shell::SysResult::OK;
            case Resp::NotFound: return shell::SysResult::NotFound;
            default: return shell::SysResult::Error;
        }
    }
};
