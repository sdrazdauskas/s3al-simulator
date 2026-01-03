#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>
#include <filesystem>
#include "json.hpp"
#include "common/LoggingMixin.h"

namespace sys { struct SysApi; }

namespace storage {

class StorageManager : public common::LoggingMixin {
public:
    // ENUMS
    enum class StorageResponse {
        OK,
        AlreadyExists,
        NotFound,
        AtRoot,
        InvalidArgument,
        Error
    };

    // STRUCTURES
    struct File {
        std::string name;
        void* memoryToken = nullptr;
        size_t contentSize = 0;
        std::chrono::system_clock::time_point createdAt;
        std::chrono::system_clock::time_point modifiedAt;
    };

    struct Folder {
        std::string name;
        Folder* parent = nullptr;
        std::vector<std::unique_ptr<File>> files;
        std::vector<std::unique_ptr<Folder>> subfolders;
        std::chrono::system_clock::time_point createdAt;
        std::chrono::system_clock::time_point modifiedAt;
    };

    struct PathInfo {
        Folder* folder;
        std::string name;
    };

public:
    // CONSTRUCTOR
    StorageManager();
    
    // Set system API (must be called before using storage)
    void setSysApi(sys::SysApi* sys) { sysApi = sys; }

    // UTILITIES
    static std::string toString(StorageResponse status);
    static bool isNameInvalid(const std::string& s);

    // FILE OPERATIONS
    StorageResponse fileExists(const std::string& name) const;
    StorageResponse createFile(const std::string& name);
    StorageResponse touchFile(const std::string& name);
    StorageResponse deleteFile(const std::string& name);
    StorageResponse deleteFile(Folder& folder, const std::string& name);
    StorageResponse writeFile(const std::string& name, const std::string& content);
    StorageResponse readFile(const std::string& name, std::string& outContent) const;
    StorageResponse editFile(const std::string& name, const std::string& newContent);
    StorageResponse copyFile(const std::string& srcName, const std::string& destName);
    StorageResponse moveFile(const std::string& oldName, const std::string& newName);

    // FOLDER OPERATIONS
    StorageResponse makeDir(const std::string& name);
    StorageResponse removeDir(const std::string& name);
    StorageResponse changeDir(const std::string& path);
    StorageResponse listDir(const std::string& path, std::vector<std::string>& outEntries) const;
    std::string getWorkingDir() const;
    StorageResponse copyDir(const std::string& srcName, const std::string& destName);
    StorageResponse moveDir(const std::string& oldName, const std::string& newName);
    static bool isDescendantOrSame(const Folder* ancestor, const Folder* descendant);

    // DISK IO OPERATIONS
    StorageResponse saveToDisk(const std::string& fileName) const;
    StorageResponse loadFromDisk(const std::string& fileName);
    StorageResponse listDataFiles(std::vector<std::string>& outFiles) const;

    // RESET
    StorageResponse reset();

private:
    // INTERNAL HELPERS
    PathInfo parsePath(const std::string& path) const;
    int findFolderIndex(const std::string& name) const;
    StorageResponse recursiveDelete(Folder& folder);
    void recursiveCopyDir(const Folder& src, Folder& destParent);
    StorageResponse allocateFileMemory(File& file, const void* data, size_t size);

    // DATA MEMBERS
    std::unique_ptr<Folder> root;
    Folder* currentFolder;
    sys::SysApi* sysApi = nullptr;

protected:
    std::string getModuleName() const override { return "STORAGE"; }
};

// Utility function declarations
std::string toString(StorageManager::StorageResponse status);
bool isNameInvalid(const std::string& s);
std::string formatTime(const std::chrono::system_clock::time_point& tp);

}  // namespace storage
