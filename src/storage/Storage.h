#pragma once

#include <memory>
#include <string>
#include <vector>

namespace storage {

class StorageManager {
public:
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

    StorageResponse createFile(const std::string& name);
    StorageResponse deleteFile(const std::string& name);
    StorageResponse writeFile(const std::string& name, const std::string& content);
    StorageResponse readFile(const std::string& name, std::string& outContent) const;
    StorageResponse appendToFile(const std::string& name, const std::string& content);
    StorageResponse editFile(const std::string& name);
    StorageResponse fileExists(const std::string& name) const;

    StorageResponse makeDir(const std::string& name);
    StorageResponse removeDir(const std::string& name);
    StorageResponse changeDir(const std::string& name);

    std::vector<std::string> listDir() const;
    std::string getWorkingDir() const;

private:
    struct File {
        std::string name;
        std::string content;
    };

    struct Folder {
        std::string name;
        std::vector<std::unique_ptr<File>> files;
        std::vector<std::unique_ptr<Folder>> subfolders;
        Folder* parent = nullptr;
    };

    std::unique_ptr<Folder> root;
    Folder* currentFolder;

    static bool isNameInvalid(const std::string& s);
    int findFileIndex(const std::string& name) const;
    int findFolderIndex(const std::string& name) const;
    void recursiveDelete(Folder& folder);
};

} // namespace storage
