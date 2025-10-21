#include "Storage.h"
#include <iostream>
#include <sstream>

StorageManager::StorageManager() {
    root = std::make_unique<Folder>();
    root->name = "root";
    root->parent = nullptr;
    currentFolder = root.get();
}

int StorageManager::findFileIndex(const std::string& name) const {
    for (size_t i = 0; i < currentFolder->files.size(); ++i)
        if (currentFolder->files[i]->name == name)
            return static_cast<int>(i);
    return -1;
}

int StorageManager::findFolderIndex(const std::string& name) const {
    for (size_t i = 0; i < currentFolder->subfolders.size(); ++i)
        if (currentFolder->subfolders[i]->name == name)
            return static_cast<int>(i);
    return -1;
}

void StorageManager::recursiveDelete(Folder& folder) {
    folder.files.clear();
    for (auto& sub : folder.subfolders) recursiveDelete(*sub);
    folder.subfolders.clear();
}

bool StorageManager::createFile(const std::string& name) {
    if (findFileIndex(name) != -1)
        return false;
    currentFolder->files.push_back(std::make_unique<File>(File{name, ""}));
    return true;
}

bool StorageManager::deleteFile(const std::string& name) {
    int i = findFileIndex(name);
    if (i == -1)
        return false;
    currentFolder->files.erase(currentFolder->files.begin() + i);
    return true;
}

bool StorageManager::writeFile(const std::string& name,
                               const std::string& content) {
    int i = findFileIndex(name);
    if (i == -1)
        return false;
    currentFolder->files[i]->content = content + "\n";
    return true;
}

bool StorageManager::readFile(const std::string& name,
                              std::string& outContent) const {
    int i = findFileIndex(name);
    if (i == -1)
        return false;
    outContent = currentFolder->files[i]->content;
    return true;
}

bool StorageManager::appendToFile(const std::string& name,
                                  const std::string& content) {
    int i = findFileIndex(name);
    if (i == -1)
        return false;
    currentFolder->files[i]->content += content + "\n";
    return true;
}

bool StorageManager::editFile(const std::string& name) {
    int i = findFileIndex(name);
    if (i == -1)
        return false;

    File& target = *currentFolder->files[i];
    std::cout << "=== Editing " << name << " ===\n";
    if (target.content.empty())
        std::cout << "(empty)\n";
    else
        std::cout << target.content;

    std::cout << "--------------------------------------\n";
    std::cout << "Type new content below to ADD to the file.\n";
    std::cout << "Type ':wq' on a new line to save and exit.\n";
    std::cout << "--------------------------------------\n";

    std::string newLines, line;
    while (true) {
        std::getline(std::cin, line);
        if (line == ":wq")
            break;
        newLines += line + "\n";
    }

    target.content += newLines;
    return true;
}

bool StorageManager::makeDir(const std::string& name) {
    if (findFolderIndex(name) != -1)
        return false;
    auto folder = std::make_unique<Folder>();
    folder->name = name;
    folder->parent = currentFolder;
    currentFolder->subfolders.push_back(std::move(folder));
    return true;
}

bool StorageManager::removeDir(const std::string& name) {
    int i = findFolderIndex(name);
    if (i == -1)
        return false;
    currentFolder->subfolders.erase(currentFolder->subfolders.begin() + i);
    return true;
}

bool StorageManager::changeDir(const std::string& name) {
    if (name == "..") {
        if (currentFolder->parent == nullptr)
            return false;
        currentFolder = currentFolder->parent;
        return true;
    }

    int i = findFolderIndex(name);
    if (i == -1)
        return false;

    currentFolder = currentFolder->subfolders[i].get();
    return true;
}

std::vector<std::string> StorageManager::listDir() const {
    std::vector<std::string> entries;
    for (auto& f : currentFolder->subfolders)
        entries.push_back("[D] " + f->name);
    for (auto& fl : currentFolder->files)
        entries.push_back("[F] " + fl->name);
    return entries;
}

std::string StorageManager::getWorkingDir() const {
    std::ostringstream path;
    std::vector<std::string> parts;
    auto* tmp = currentFolder;
    while (tmp) {
        parts.push_back(tmp->name);
        tmp = tmp->parent;
    }
    path << "/";
    for (int i = static_cast<int>(parts.size()) - 1; i >= 0; --i) {
        path << parts[i];
        if (i != 0)
            path << "/";
    }
    return path.str();
}