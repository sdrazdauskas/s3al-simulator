#pragma once

#include <memory>
#include <string>
#include <vector>

class StorageManager {
public:
    StorageManager();

    bool createFile(const std::string& name);
    bool deleteFile(const std::string& name);
    bool writeFile(const std::string& name, const std::string& content);
    bool readFile(const std::string& name, std::string& outContent) const;
    bool appendToFile(const std::string& name, const std::string& content);
    bool editFile(const std::string& name);

    bool makeDir(const std::string& name);
    bool removeDir(const std::string& name);
    bool changeDir(const std::string& name);

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

    int findFileIndex(const std::string& name) const;
    int findFolderIndex(const std::string& name) const;
    void recursiveDelete(Folder& folder);
};