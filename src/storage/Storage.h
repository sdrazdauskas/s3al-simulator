#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include "json.hpp"

namespace storage {

class StorageManager {
public:
    using LogCallback = std::function<void(const std::string& level,
                                           const std::string& module,
                                           const std::string& message)>;

    enum class StorageResponse {
        OK,
        AlreadyExists,
        NotFound,
        AtRoot,
        InvalidArgument,
        Error
    };

    static std::string toString(StorageResponse status);

    StorageManager();

    void setLogCallback(LogCallback callback);

    StorageResponse createFile(const std::string& name);
    StorageResponse touchFile(const std::string& name);
    StorageResponse deleteFile(const std::string& name);
    StorageResponse writeFile(const std::string& name, const std::string& content);
    StorageResponse readFile(const std::string& name, std::string& outContent) const;
    StorageResponse editFile(const std::string& name);
    StorageResponse fileExists(const std::string& name) const;

    StorageResponse makeDir(const std::string& name);
    StorageResponse removeDir(const std::string& name);
    StorageResponse changeDir(const std::string& name);

    StorageResponse saveToDisk(const std::string& filePath) const;
    StorageResponse loadFromDisk(const std::string& filePath);
    StorageResponse reset();

    std::vector<std::string> listDir() const;
    std::string getWorkingDir() const;

    struct File {
        std::string name;
        std::string content;
        std::chrono::system_clock::time_point createdAt;
        std::chrono::system_clock::time_point modifiedAt;
    };

    struct Folder {
        std::string name;
        std::vector<std::unique_ptr<File>> files;
        std::vector<std::unique_ptr<Folder>> subfolders;
        Folder* parent = nullptr;
        std::chrono::system_clock::time_point createdAt;
        std::chrono::system_clock::time_point modifiedAt;
    };

private:
    std::unique_ptr<Folder> root;
    Folder* currentFolder;
    LogCallback log_callback;

    static bool isNameInvalid(const std::string& s);
    int findFileIndex(const std::string& name) const;
    int findFolderIndex(const std::string& name) const;
    void recursiveDelete(Folder& folder);
    void log(const std::string& level, const std::string& message);
};

} // namespace storage
