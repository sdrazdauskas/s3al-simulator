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

    // FOLDER OPERATIONS
    StorageResponse makeDir(const std::string& name);
    StorageResponse removeDir(const std::string& name);
    StorageResponse changeDir(const std::string& path);
    std::vector<std::string> listDir() const;
    std::string getWorkingDir() const;

    // DISK IO OPERATIONS
    StorageResponse saveToDisk(const std::string& fileName) const;
    StorageResponse loadFromDisk(const std::string& fileName);

    // RESET
    StorageResponse reset();

private:
    // INTERNAL HELPERS
    int findFileIndex(const std::string& name) const;
    int findFolderIndex(const std::string& name) const;
    void recursiveDelete(Folder& folder);

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
