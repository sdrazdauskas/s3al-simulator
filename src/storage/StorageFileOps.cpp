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

Response StorageManager::createFile(const std::string& path) {
    if (path.empty() || isNameInvalid(path)) {
        return Response::InvalidArgument;
    }
    PathInfo info = parsePath(path);
    if (!info.folder) {
        log("ERROR", "Path not found: " + path);
        return Response::NotFound;
    }
    if (isNameInvalid(info.name)) return Response::InvalidArgument;
    
    // check if file already exists
    for (const auto& file : info.folder->files) {
        if (file->name == info.name) {
            log("ERROR", "File already exists: " + path);
            return Response::AlreadyExists;
        }
    }

    auto newFile = std::make_unique<File>();
    newFile->name = info.name;
    newFile->content = "";
    newFile->createdAt = std::chrono::system_clock::now();
    newFile->modifiedAt = std::chrono::system_clock::now();

    info.folder->files.push_back(std::move(newFile));
    info.folder->modifiedAt = std::chrono::system_clock::now();
    log("INFO", "Created file: " + path);
    return Response::OK;
}

Response StorageManager::touchFile(const std::string& path) {
    if (path.empty() || isNameInvalid(path)) {
        return Response::InvalidArgument;
    }
    PathInfo info = parsePath(path);
    if (!info.folder) {
        log("ERROR", "Path not found: " + path);
        return Response::NotFound;
    }

    if (isNameInvalid(info.name)) return Response::InvalidArgument;

    // check if file exists
    for (auto& file : info.folder->files) {
        if (file->name == info.name) {
            file->modifiedAt = std::chrono::system_clock::now();
            info.folder->modifiedAt = std::chrono::system_clock::now();
            log("INFO", "File already exists, timestamp updated: " + path);
            return Response::OK;
        }
    }

    // file does not exist, create it
    log("INFO", "File does not exist, will be created: " + path);
    return createFile(path);
}

Response StorageManager::deleteFile(const std::string& path) {
    if (path.empty() || isNameInvalid(path)) {
        return Response::InvalidArgument;
    }
    PathInfo info = parsePath(path);
    if (!info.folder) {
        log("ERROR", "Path not found: " + path);
        return Response::NotFound;
    }
    if (isNameInvalid(info.name)) return Response::InvalidArgument;
    
    // find and delete file
    for (size_t i = 0; i < info.folder->files.size(); ++i) {
        if (info.folder->files[i]->name == info.name) {
            info.folder->files.erase(info.folder->files.begin() + i);
            info.folder->modifiedAt = std::chrono::system_clock::now();
            log("INFO", "Deleted file: " + path);
            return Response::OK;
        }
    }
    
    log("ERROR", "File not found: " + path);
    return Response::NotFound;
}

Response StorageManager::writeFile(const std::string& path, const std::string& content) {
    if (path.empty() || isNameInvalid(path)) {
        return Response::InvalidArgument;
    }
    PathInfo info = parsePath(path);
    if (!info.folder) {
        log("ERROR", "Path not found: " + path);
        return Response::NotFound;
    }
    if (isNameInvalid(info.name)) return Response::InvalidArgument;

    // find file
    for (auto& file : info.folder->files) {
        if (file->name == info.name) {
            file->content = content + "\n";
            file->modifiedAt = std::chrono::system_clock::now();
            info.folder->modifiedAt = std::chrono::system_clock::now();
            log("INFO", "Wrote to file: " + path);
            return Response::OK;
        }
    }

    log("ERROR", "File not found: " + path);
    return Response::NotFound;
}

Response StorageManager::readFile(const std::string& path, std::string& outContent) const {
    if (path.empty() || isNameInvalid(path)) {
        return Response::InvalidArgument;
    }
    PathInfo info = parsePath(path);
    if (!info.folder) return Response::NotFound;
    if (isNameInvalid(info.name)) return Response::InvalidArgument;
    
    for (const auto& file : info.folder->files) {
        if (file->name == info.name) {
            outContent = file->content;
            return Response::OK;
        }
    }
    
    return Response::NotFound;
}

Response StorageManager::editFile(const std::string& path, const std::string& newContent) {
    if (path.empty() || isNameInvalid(path)) {
        return Response::InvalidArgument;
    }
    PathInfo info = parsePath(path);
    if (!info.folder) {
        log("ERROR", "Path not found: " + path);
        return Response::NotFound;
    }
    if (isNameInvalid(info.name)) return Response::InvalidArgument;
    
    for (auto& file : info.folder->files) {
        if (file->name == info.name) {
            file->content += newContent;
            file->modifiedAt = std::chrono::system_clock::now();
            info.folder->modifiedAt = std::chrono::system_clock::now();
            log("INFO", "Edited file: " + path);
            return Response::OK;
        }
    }
    
    log("ERROR", "File not found: " + path);
    return Response::NotFound;
}

Response StorageManager::copyFile(const std::string& srcPath, const std::string& destPath) {
    // validate inputs
    if (srcPath.empty() || destPath.empty()) {
        return Response::InvalidArgument;
    }

    // parse source
    PathInfo srcInfo = parsePath(srcPath);
    if (!srcInfo.folder || isNameInvalid(srcInfo.name)) {
        log("ERROR", "Invalid source path: " + srcPath);
        return srcInfo.folder ? Response::InvalidArgument : Response::NotFound;
    }
    
    // find source file
    File* srcFile = nullptr;
    for (const auto& file : srcInfo.folder->files) {
        if (file->name == srcInfo.name) {
            srcFile = file.get();
            break;
        }
    }
    
    if (!srcFile) {
        log("ERROR", "Source file not found: " + srcPath);
        return Response::NotFound;
    }

    // parse destination
    PathInfo destInfo = parsePath(destPath);
    if (!destInfo.folder || isNameInvalid(destInfo.name)) {
        log("ERROR", "Invalid destination path: " + destPath);
        return destInfo.folder ? Response::InvalidArgument : Response::NotFound;
    }
    
    // check if destination is an existing directory
    Folder* targetDir = nullptr;
    for (const auto& sub : destInfo.folder->subfolders) {
        if (sub->name == destInfo.name) {
            targetDir = sub.get();
            break;
        }
    }
    
    if (targetDir) {
        // dest is a directory, copy file into it with original name
        for (const auto& f : targetDir->files) {
            if (f->name == srcInfo.name) {
                log("ERROR", "File already exists: " + srcInfo.name);
                return Response::AlreadyExists;
            }
        }
        
        auto newFile = std::make_unique<File>(*srcFile);
        newFile->createdAt = std::chrono::system_clock::now();
        newFile->modifiedAt = std::chrono::system_clock::now();
        targetDir->files.push_back(std::move(newFile));
        targetDir->modifiedAt = std::chrono::system_clock::now();
        
        log("INFO", "Copied file '" + srcPath + "' into directory '" + 
            destPath + "'");
        return Response::OK;
    }
    
    // dest is not a directory, copy to exact path with new name
    for (const auto& file : destInfo.folder->files) {
        if (file->name == destInfo.name) {
            log("ERROR", "Destination file already exists: " + destPath);
            return Response::AlreadyExists;
        }
    }

    auto newFile = std::make_unique<File>(*srcFile);
    newFile->name = destInfo.name;
    newFile->createdAt = std::chrono::system_clock::now();
    newFile->modifiedAt = std::chrono::system_clock::now();
    destInfo.folder->files.push_back(std::move(newFile));
    destInfo.folder->modifiedAt = std::chrono::system_clock::now();

    log("INFO", "Copied file '" + srcPath + "' to '" + destPath + "'");
    return Response::OK;
}

Response StorageManager::moveFile(const std::string& srcPath, const std::string& destPath) {
    // validate inputs
    if (srcPath.empty() || destPath.empty()) {
        return Response::InvalidArgument;
    }

    // parse source
    PathInfo srcInfo = parsePath(srcPath);
    if (!srcInfo.folder || isNameInvalid(srcInfo.name)) {
        log("ERROR", "Invalid source path: " + srcPath);
        return srcInfo.folder ? Response::InvalidArgument : Response::NotFound;
    }
    
    // find source file
    int srcIndex = -1;
    for (size_t i = 0; i < srcInfo.folder->files.size(); ++i) {
        if (srcInfo.folder->files[i]->name == srcInfo.name) {
            srcIndex = static_cast<int>(i);
            break;
        }
    }
    
    if (srcIndex == -1) {
        log("ERROR", "Source file not found: " + srcPath);
        return Response::NotFound;
    }

    // parse destination
    PathInfo destInfo = parsePath(destPath);
    if (!destInfo.folder || isNameInvalid(destInfo.name)) {
        log("ERROR", "Invalid destination path: " + destPath);
        return destInfo.folder ? Response::InvalidArgument : Response::NotFound;
    }
    
    // check if destination is an existing directory
    Folder* targetDir = nullptr;
    for (const auto& sub : destInfo.folder->subfolders) {
        if (sub->name == destInfo.name) {
            targetDir = sub.get();
            break;
        }
    }
    
    if (targetDir) {
        // dest is a directory, move file into it with original name
        for (const auto& f : targetDir->files) {
            if (f->name == srcInfo.name) {
                log("ERROR", "File already exists: " + srcInfo.name);
                return Response::AlreadyExists;
            }
        }
        
        auto filePtr = std::move(srcInfo.folder->files[srcIndex]);
        srcInfo.folder->files.erase(
            srcInfo.folder->files.begin() + srcIndex);
        targetDir->files.push_back(std::move(filePtr));
        
        srcInfo.folder->modifiedAt = std::chrono::system_clock::now();
        targetDir->modifiedAt = std::chrono::system_clock::now();
        
        log("INFO", "Moved file '" + srcPath + "' into directory '" + 
            destPath + "'");
        return Response::OK;
    }
    
    // dest is not a directory, rename/move to exact path
    for (const auto& file : destInfo.folder->files) {
        if (file->name == destInfo.name) {
            log("ERROR", "Destination file already exists: " + destPath);
            return Response::AlreadyExists;
        }
    }

    auto filePtr = std::move(srcInfo.folder->files[srcIndex]);
    srcInfo.folder->files.erase(srcInfo.folder->files.begin() + srcIndex);
    filePtr->name = destInfo.name;
    filePtr->modifiedAt = std::chrono::system_clock::now();
    destInfo.folder->files.push_back(std::move(filePtr));
    
    srcInfo.folder->modifiedAt = std::chrono::system_clock::now();
    destInfo.folder->modifiedAt = std::chrono::system_clock::now();

    log("INFO", "Moved file '" + srcPath + "' to '" + destPath + "'");
    return Response::OK;
}

}  // namespace storage
