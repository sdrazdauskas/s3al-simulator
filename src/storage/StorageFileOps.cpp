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

Response StorageManager::fileExists(const std::string& path) const {
    PathInfo info = parsePath(path);
    if (!info.folder) return Response::NotFound;
    if (isNameInvalid(info.name)) return Response::InvalidArgument;
    
    for (const auto& file : info.folder->files) {
        if (file->name == info.name) {
            return Response::OK;
        }
    }
    
    return Response::NotFound;
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

Response StorageManager::writeFile(const std::string& path,
                                   const std::string& content) {
    PathInfo info = parsePath(path);
    if (!info.folder) {
        log("ERROR", "Path not found: " + path);
        return Response::NotFound;
    }
    if (isNameInvalid(info.name)) return Response::InvalidArgument;

    int i = -1;
    for (size_t idx = 0; idx < info.folder->files.size(); ++idx) {
        if (info.folder->files[idx]->name == info.name) {
            i = static_cast<int>(idx);
            break;
        }
    }

    if (i == -1) {
        log("ERROR", "File not found: " + path);
        return Response::NotFound;
    }

    info.folder->files[i]->content = content + "\n";
    info.folder->files[i]->modifiedAt = std::chrono::system_clock::now();
    info.folder->modifiedAt = std::chrono::system_clock::now();
    log("INFO", "Wrote to file: " + path);
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

Response StorageManager::editFile(const std::string& name,
                                  const std::string& newContent) {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    int i = findFileIndex(name);
    if (i == -1) {
        log("ERROR", "File not found: " + name);
        return Response::NotFound;
    }
    File& f = *currentFolder->files[i];
    f.content += newContent;
    f.modifiedAt = std::chrono::system_clock::now();
    currentFolder->modifiedAt = std::chrono::system_clock::now();
    log("INFO", "Edited file: " + name);
    return Response::OK;
}

Response StorageManager::copyFile(const std::string& srcName,
                                  const std::string& destName) {
    int i = findFileIndex(srcName);
    if (i == -1) {
        log("ERROR", "File not found: " + srcName);
        return Response::NotFound;
    }

    if (findFileIndex(destName) != -1) {
        log("ERROR", "File already exists: " + destName);
        return Response::AlreadyExists;
    }
    const File& src = *currentFolder->files[i];
    auto newFile = std::make_unique<File>(src);
    newFile->name = destName;
    newFile->createdAt = std::chrono::system_clock::now();
    newFile->modifiedAt = std::chrono::system_clock::now();
    currentFolder->files.push_back(std::move(newFile));

    log("INFO", "Copied file '" + srcName + "' to '" + destName + "'");
    return Response::OK;
}

Response StorageManager::moveFile(const std::string& srcName,
                                  const std::string& destName) {
    int srcIndex = findFileIndex(srcName);
    if (srcIndex == -1) {
        log("ERROR", "File not found: " + srcName);
        return Response::NotFound;
    }

    // check if destName is a dir
    int destDirIndex = findFolderIndex(destName);
    if (destDirIndex != -1) {
        // destination is a dir, move file inside it
        Folder* destFolder = currentFolder->subfolders[destDirIndex].get();

        // check for same file name in destination dir
        for (const auto& f : destFolder->files)
            if (f->name == srcName) {
                log("ERROR", "File already exists: " + srcName);
                return Response::AlreadyExists;
            }

        // move the file
        auto filePtr = std::move(currentFolder->files[srcIndex]);
        currentFolder->files.erase(currentFolder->files.begin() + srcIndex);
        destFolder->files.push_back(std::move(filePtr));

        log("INFO", "Moved file '" + srcName + "' into directory '" + destName + "'");
        return Response::OK;
    }

    // or else rename the file
    if (findFileIndex(destName) != -1) {
        log("ERROR", "File already exists: " + destName);
        return Response::AlreadyExists;
    }

    currentFolder->files[srcIndex]->name = destName;
    currentFolder->files[srcIndex]->modifiedAt = std::chrono::system_clock::now();
    log("INFO", "Renamed file from '" + srcName + "' to '" + destName + "'");
    return Response::OK;
}

}  // namespace storage
