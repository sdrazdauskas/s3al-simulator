#pragma once

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

class StorageManager {
public:
    StorageManager();

    void createFile(const std::string& name);
    void deleteFile(const std::string& name);
    void writeFile(const std::string& name);
    void readFile(const std::string& name);
    void editFile(const std::string& name);
    void makeDir(const std::string& name);
    void removeDir(const std::string& name);
    void changeDir(const std::string& name);
    void listDir() const;
    void printWorkingDir() const;

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
    void sendToKernel(const std::string& message) const;
};