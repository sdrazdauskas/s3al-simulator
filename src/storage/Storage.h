#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>
#include <filesystem>
#include "json.hpp"

namespace storage {

class StorageManager {
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

    // CALLBACK TYPES
    using LogCallback =
        std::function<void(const std::string&, const std::string&, const std::string&)>;

    // STRUCTURES
    struct File {
        std::string name;
        std::string content;
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

    // UTILITIES
    static std::string toString(StorageResponse status);
    static bool isNameInvalid(const std::string& s);
    void setLogCallback(LogCallback callback);
    void log(const std::string& level, const std::string& message);

    // FILE OPERATIONS
    StorageResponse fileExists(const std::string& name) const;
    StorageResponse createFile(const std::string& name);
    StorageResponse touchFile(const std::string& name);
    StorageResponse deleteFile(const std::string& name);
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

    // DISK IO OPERATIONS
    StorageResponse saveToDisk(const std::string& fileName) const;
    StorageResponse loadFromDisk(const std::string& fileName);
    StorageResponse listDataFiles(std::vector<std::string>& outFiles) const;

    // RESET
    StorageResponse reset();

private:
    // INTERNAL HELPERS
    PathInfo parsePath(const std::string& path) const;
    int findFileIndex(const std::string& name) const;
    int findFolderIndex(const std::string& name) const;
    void recursiveDelete(Folder& folder);
    void recursiveCopyDir(const Folder& src, Folder& destParent);

    // DATA MEMBERS
    std::unique_ptr<Folder> root;
    Folder* currentFolder;
    LogCallback log_callback;
};

// Utility function declarations
std::string toString(StorageManager::StorageResponse status);
bool isNameInvalid(const std::string& s);
std::string formatTime(const std::chrono::system_clock::time_point& tp);

}  // namespace storage
