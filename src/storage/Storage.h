#pragma once

#include <memory>
#include <string>
#include <vector>

class StorageManager {
public:
    enum class StorageStatus {
        OK,
        AlreadyExists,
        NotFound,
        AtRoot,
        InvalidArgument,
        Error
    };

    static std::string toString(StorageStatus status);

    StorageManager();

    StorageStatus createFile(const std::string& name);
    StorageStatus deleteFile(const std::string& name);
    StorageStatus writeFile(const std::string& name, const std::string& content);
    StorageStatus readFile(const std::string& name, std::string& outContent) const;
    StorageStatus appendToFile(const std::string& name, const std::string& content);
    StorageStatus editFile(const std::string& name);

    StorageStatus makeDir(const std::string& name);
    StorageStatus removeDir(const std::string& name);
    StorageStatus changeDir(const std::string& name);

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