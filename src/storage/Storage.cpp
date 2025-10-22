#include "Storage.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace storage {

using Response = StorageManager::StorageResponse;

std::string StorageManager::toString(StorageResponse status) {
    switch (status) {
        case Response::OK:
            return "OK";
        case Response::AlreadyExists:
            return "Already Exists";
        case Response::NotFound:
            return "Not Found";
        case Response::AtRoot:
            return "Already at Root";
        case Response::InvalidArgument:
            return "Invalid Argument";
        case Response::Error:
        default:
            return "Error";
    }
}

bool StorageManager::isNameInvalid(const std::string& s) {
    if (s.empty()) return true;
    return std::all_of(s.begin(), s.end(), [](unsigned char c) {
        return std::isspace(c);
    });
}

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

Response StorageManager::fileExists(const std::string& name) const {
    if (isNameInvalid(name))
        return Response::InvalidArgument;

    return (findFileIndex(name) != -1)
               ? Response::OK
               : Response::NotFound;
}

Response StorageManager::createFile(const std::string& name) {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    if (findFileIndex(name) != -1) return Response::AlreadyExists;
    currentFolder->files.push_back(std::make_unique<File>(File{name, ""}));
    return Response::OK;
}

Response StorageManager::deleteFile(const std::string& name) {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    int i = findFileIndex(name);
    if (i == -1) return Response::NotFound;
    currentFolder->files.erase(currentFolder->files.begin() + i);
    return Response::OK;
}

Response StorageManager::writeFile(const std::string& name,
                                   const std::string& content) {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    int i = findFileIndex(name);
    if (i == -1) return Response::NotFound;
    currentFolder->files[i]->content = content + "\n";
    return Response::OK;
}

Response StorageManager::readFile(const std::string& name,
                                  std::string& outContent) const {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    int i = findFileIndex(name);
    if (i == -1) return Response::NotFound;
    outContent = currentFolder->files[i]->content;
    return Response::OK;
}

Response StorageManager::appendToFile(const std::string& name,
                                      const std::string& content) {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    int i = findFileIndex(name);
    if (i == -1) return Response::NotFound;
    currentFolder->files[i]->content += content + "\n";
    return Response::OK;
}

Response StorageManager::editFile(const std::string& name) {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    int i = findFileIndex(name);
    if (i == -1) return Response::NotFound;

    File& f = *currentFolder->files[i];
    std::cout << "=== Editing " << name << " ===\n";
    if (f.content.empty()) std::cout << "(empty)\n";
    else std::cout << f.content;
    std::cout << "--------------------------------------\n";
    std::cout << "Type new content below to ADD to the file.\n";
    std::cout << "Type ':wq' on a new line to save and exit.\n";
    std::cout << "--------------------------------------\n";

    std::string newLines, line;
    while (true) {
        std::getline(std::cin, line);
        if (line == ":wq") break;
        newLines += line + "\n";
    }
    f.content += newLines;
    return Response::OK;
}

Response StorageManager::makeDir(const std::string& name) {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    if (findFolderIndex(name) != -1) return Response::AlreadyExists;
    auto folder = std::make_unique<Folder>();
    folder->name = name;
    folder->parent = currentFolder;
    currentFolder->subfolders.push_back(std::move(folder));
    return Response::OK;
}

Response StorageManager::removeDir(const std::string& name) {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    int i = findFolderIndex(name);
    if (i == -1) return Response::NotFound;
    currentFolder->subfolders.erase(currentFolder->subfolders.begin() + i);
    return Response::OK;
}

Response StorageManager::changeDir(const std::string& path) {
    if (isNameInvalid(path)) return Response::InvalidArgument;
    if (path == "..") {
        if (currentFolder->parent == nullptr) return Response::AtRoot;
        currentFolder = currentFolder->parent;
        return Response::OK;
    }
    int i = findFolderIndex(path);
    if (i == -1) return Response::NotFound;
    currentFolder = currentFolder->subfolders[i].get();
    return Response::OK;
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
        if (i != 0) path << "/";
    }
    return path.str();
}

} // namespace storage
