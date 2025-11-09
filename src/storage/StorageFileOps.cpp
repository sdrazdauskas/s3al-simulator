#include "Storage.h"
#include <iostream>

namespace storage {

using Response = StorageManager::StorageResponse;

int StorageManager::findFileIndex(const std::string& name) const {
    for (size_t i = 0; i < currentFolder->files.size(); ++i)
        if (currentFolder->files[i]->name == name)
            return static_cast<int>(i);
    return -1;
}

Response StorageManager::fileExists(const std::string& name) const {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    return (findFileIndex(name) != -1) ? Response::OK : Response::NotFound;
}

Response StorageManager::createFile(const std::string& name) {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    if (findFileIndex(name) != -1) {
        log("ERROR", "File already exists: " + name);
        return Response::AlreadyExists;
    }

    auto newFile = std::make_unique<File>();
    newFile->name = name;
    newFile->content = "";
    newFile->createdAt = std::chrono::system_clock::now();
    newFile->modifiedAt = std::chrono::system_clock::now();

    currentFolder->files.push_back(std::move(newFile));
    currentFolder->modifiedAt = std::chrono::system_clock::now();
    log("INFO", "Created file: " + name);
    return Response::OK;
}

Response StorageManager::touchFile(const std::string& name) {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    int i = findFileIndex(name);

    if (i == -1) {
        log("INFO", "File does not exist, will be created: " + name);
        createFile(name);
        return Response::OK;
    }

    currentFolder->files[i]->modifiedAt = std::chrono::system_clock::now();
    currentFolder->modifiedAt = std::chrono::system_clock::now();
    log("INFO", "File already exists, timestamp was updated: " + name);
    return Response::OK;
}

Response StorageManager::deleteFile(const std::string& name) {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    int i = findFileIndex(name);
    if (i == -1) {
        log("ERROR", "File not found: " + name);
        return Response::NotFound;
    }
    currentFolder->files.erase(currentFolder->files.begin() + i);
    currentFolder->modifiedAt = std::chrono::system_clock::now();
    log("INFO", "Deleted file: " + name);
    return Response::OK;
}

Response StorageManager::writeFile(const std::string& name,
                                   const std::string& content) {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    int i = findFileIndex(name);
    if (i == -1) {
        log("ERROR", "File not found: " + name);
        return Response::NotFound;
    }
    currentFolder->files[i]->content = content + "\n";
    currentFolder->files[i]->modifiedAt = std::chrono::system_clock::now();
    currentFolder->modifiedAt = std::chrono::system_clock::now();
    log("INFO", "Wrote to file: " + name);
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
    f.modifiedAt = std::chrono::system_clock::now();
    currentFolder->modifiedAt = std::chrono::system_clock::now();
    return Response::OK;
}

}  // namespace storage
