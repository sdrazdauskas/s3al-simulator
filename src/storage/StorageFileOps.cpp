#include "storage/Storage.h"
#include "kernel/SysCallsAPI.h"
#include <iostream>
#include <cstring>

namespace storage {

using Response = StorageManager::StorageResponse;

Response StorageManager::allocateFileMemory(File& file, const void* data, size_t size) {
    // Free old memory if exists
    if (file.memoryToken && sysApi) {
        auto result = sysApi->deallocateMemory(file.memoryToken);
        if (result != sys::SysResult::OK) {
            logError("Failed to deallocate memory for file: " + file.name);
            return Response::Error;
        }
        file.memoryToken = nullptr;
        file.contentSize = 0;
    }
    
    // Allocate new memory if size > 0
    if (size > 0 && sysApi) {
        file.memoryToken = sysApi->allocateMemory(size, 0);
        if (!file.memoryToken) {
            logError("Out of memory for file: " + file.name);
            return Response::Error;
        }
        
        if (data) {
            std::memcpy(file.memoryToken, data, size);
        }
        file.contentSize = size;
    } else {
        file.memoryToken = nullptr;
        file.contentSize = 0;
    }
    
    return Response::OK;
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
        logError("Path not found: " + path);
        return Response::NotFound;
    }
    if (isNameInvalid(info.name)) return Response::InvalidArgument;
    
    // check if file already exists
    for (const auto& file : info.folder->files) {
        if (file->name == info.name) {
            logError("File already exists: " + path);
            return Response::AlreadyExists;
        }
    }

    auto newFile = std::make_unique<File>();
    newFile->name = info.name;
    newFile->memoryToken = nullptr;
    newFile->contentSize = 0;
    newFile->createdAt = std::chrono::system_clock::now();
    newFile->modifiedAt = std::chrono::system_clock::now();

    info.folder->files.push_back(std::move(newFile));
    info.folder->modifiedAt = std::chrono::system_clock::now();
    logInfo("Created file: " + path);
    return Response::OK;
}

Response StorageManager::touchFile(const std::string& path) {
    if (path.empty() || isNameInvalid(path)) {
        return Response::InvalidArgument;
    }
    PathInfo info = parsePath(path);
    if (!info.folder) {
        logError("Path not found: " + path);
        return Response::NotFound;
    }

    if (isNameInvalid(info.name)) return Response::InvalidArgument;

    // check if file exists
    for (auto& file : info.folder->files) {
        if (file->name == info.name) {
            file->modifiedAt = std::chrono::system_clock::now();
            info.folder->modifiedAt = std::chrono::system_clock::now();
            logInfo("File already exists, timestamp updated: " + path);
            return Response::OK;
        }
    }

    // file does not exist, create it
    logInfo("File does not exist, will be created: " + path);
    return createFile(path);
}

Response StorageManager::deleteFile(const std::string& path) {
    if (path.empty() || isNameInvalid(path)) {
        return Response::InvalidArgument;
    }
    PathInfo info = parsePath(path);
    if (!info.folder) {
        logError("Path not found: " + path);
        return Response::NotFound;
    }
    if (isNameInvalid(info.name)) return Response::InvalidArgument;

    return deleteFile(*info.folder, info.name);
}

Response StorageManager::deleteFile(Folder& folder, const std::string& name) {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    for (size_t i = 0; i < folder.files.size(); ++i) {
        if (folder.files[i]->name == name) {
            if (folder.files[i]->memoryToken && sysApi) {
                auto result = sysApi->deallocateMemory(folder.files[i]->memoryToken);
                if (result != sys::SysResult::OK) {
                    logError("Failed to deallocate memory for file: " + name);
                    return Response::Error;
                }
            }
            folder.files.erase(folder.files.begin() + i);
            folder.modifiedAt = std::chrono::system_clock::now();
            logInfo("Deleted file: " + name);
            return Response::OK;
        }
    }
    logError("File not found: " + name);
    return Response::NotFound;
}

Response StorageManager::writeFile(const std::string& path, const std::string& content) {
    if (path.empty() || isNameInvalid(path)) {
        return Response::InvalidArgument;
    }
    PathInfo info = parsePath(path);
    if (!info.folder) {
        logError("Path not found: " + path);
        return Response::NotFound;
    }
    if (isNameInvalid(info.name)) return Response::InvalidArgument;

    // find file
    for (auto& file : info.folder->files) {
        if (file->name == info.name) {
            std::string contentWithNewline = content + "\n";
            
            auto result = allocateFileMemory(*file, contentWithNewline.c_str(), contentWithNewline.size());
            if (result != Response::OK) {
                return result;
            }
            
            file->modifiedAt = std::chrono::system_clock::now();
            info.folder->modifiedAt = std::chrono::system_clock::now();
            logInfo("Wrote to file: " + path);
            return Response::OK;
        }
    }

    logError("File not found: " + path);
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
            if (file->memoryToken && file->contentSize > 0) {
                const char* src = static_cast<const char*>(file->memoryToken);
                outContent.assign(src, file->contentSize);
            } else {
                outContent.clear();
            }
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
        logError("Path not found: " + path);
        return Response::NotFound;
    }
    if (isNameInvalid(info.name)) return Response::InvalidArgument;
    
    for (auto& file : info.folder->files) {
        if (file->name == info.name) {
            std::string existingContent;
            if (file->memoryToken && file->contentSize > 0) {
                const char* src = static_cast<const char*>(file->memoryToken);
                existingContent.assign(src, file->contentSize);
            }
            
            std::string combined = existingContent + newContent;
            
            auto result = allocateFileMemory(*file, combined.c_str(), combined.size());
            if (result != Response::OK) {
                return result;
            }
            
            file->modifiedAt = std::chrono::system_clock::now();
            info.folder->modifiedAt = std::chrono::system_clock::now();
            logInfo("Edited file: " + path);
            return Response::OK;
        }
    }
    
    logError("File not found: " + path);
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
        logError("Invalid source path: " + srcPath);
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
        logError("Source file not found: " + srcPath);
        return Response::NotFound;
    }

    // parse destination
    PathInfo destInfo = parsePath(destPath);
    if (!destInfo.folder || isNameInvalid(destInfo.name)) {
        logError("Invalid destination path: " + destPath);
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
                logError("File already exists: " + srcInfo.name);
                return Response::AlreadyExists;
            }
        }
        
        auto newFile = std::make_unique<File>();
        newFile->name = srcFile->name;
        newFile->createdAt = std::chrono::system_clock::now();
        newFile->modifiedAt = std::chrono::system_clock::now();
        
        auto result = allocateFileMemory(*newFile, srcFile->memoryToken, srcFile->contentSize);
        if (result != Response::OK) {
            return result;
        }
        
        targetDir->files.push_back(std::move(newFile));
        targetDir->modifiedAt = std::chrono::system_clock::now();
        
        logInfo("Copied file '" + srcPath + "' into directory '" + destPath + "'");
        return Response::OK;
    }
    
    // dest is not a directory, copy to exact path with new name
    for (const auto& file : destInfo.folder->files) {
        if (file->name == destInfo.name) {
            logError("Destination file already exists: " + destPath);
            return Response::AlreadyExists;
        }
    }

    auto newFile = std::make_unique<File>();
    newFile->name = destInfo.name;
    newFile->createdAt = std::chrono::system_clock::now();
    newFile->modifiedAt = std::chrono::system_clock::now();
    
    auto result = allocateFileMemory(*newFile, srcFile->memoryToken, srcFile->contentSize);
    if (result != Response::OK) {
        return result;
    }
    
    destInfo.folder->files.push_back(std::move(newFile));
    destInfo.folder->modifiedAt = std::chrono::system_clock::now();

    logInfo("Copied file '" + srcPath + "' to '" + destPath + "'");
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
        logError("Invalid source path: " + srcPath);
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
        logError("Source file not found: " + srcPath);
        return Response::NotFound;
    }

    // parse destination
    PathInfo destInfo = parsePath(destPath);
    if (!destInfo.folder || isNameInvalid(destInfo.name)) {
        logError("Invalid destination path: " + destPath);
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
                logError("File already exists: " + srcInfo.name);
                return Response::AlreadyExists;
            }
        }
        
        auto filePtr = std::move(srcInfo.folder->files[srcIndex]);
        srcInfo.folder->files.erase(srcInfo.folder->files.begin() + srcIndex);
        targetDir->files.push_back(std::move(filePtr));
        
        srcInfo.folder->modifiedAt = std::chrono::system_clock::now();
        targetDir->modifiedAt = std::chrono::system_clock::now();
        
        logInfo("Moved file '" + srcPath + "' into directory '" + destPath + "'");
        return Response::OK;
    }
    
    // dest is not a directory, rename/move to exact path
    for (const auto& file : destInfo.folder->files) {
        if (file->name == destInfo.name) {
            logError("Destination file already exists: " + destPath);
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

    logInfo("Moved file '" + srcPath + "' to '" + destPath + "'");
    return Response::OK;
}

}  // namespace storage
