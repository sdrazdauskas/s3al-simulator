#include "storage/Storage.h"
#include <sstream>

namespace storage {

using Response = StorageManager::StorageResponse;

int StorageManager::findFolderIndex(const std::string& name) const {
    for (size_t i = 0; i < currentFolder->subfolders.size(); ++i)
        if (currentFolder->subfolders[i]->name == name)
            return static_cast<int>(i);
    return -1;
}

Response StorageManager::recursiveDelete(Folder& folder) {
    while (!folder.files.empty()) {
        std::string fileName = folder.files.front()->name;
        Response res = deleteFile(folder, fileName);
        if (res != Response::OK) {
            logError("Failed to delete file '" + fileName + "' during recursiveDelete");
            return res;
        }
    }
    for (auto& sub : folder.subfolders) {
        logDebug("Recursively deleting folder: " + sub->name);
        Response res = recursiveDelete(*sub);
        if (res != Response::OK) {
            return res;
        }
    }
    folder.subfolders.clear();
    return Response::OK;
}

Response StorageManager::makeDir(const std::string& path) {
    if (path.empty() || isNameInvalid(path)) {
        return Response::InvalidArgument;
    }
    PathInfo info = parsePath(path);
    if (!info.folder) {
        logError("Path not found: " + path);
        return Response::NotFound;
    }
    if (isNameInvalid(info.name)) return Response::InvalidArgument;
    
    // check if directory already exists
    for (const auto& sub : info.folder->subfolders) {
        if (sub->name == info.name) {
            logError("Directory already exists: " + path);
            return Response::AlreadyExists;
        }
    }

    auto folder = std::make_unique<Folder>();
    folder->name = info.name;
    folder->parent = info.folder;
    folder->createdAt = std::chrono::system_clock::now();
    folder->modifiedAt = folder->createdAt;

    info.folder->subfolders.push_back(std::move(folder));
    info.folder->modifiedAt = std::chrono::system_clock::now();
    logInfo("Created directory: " + path);
    return Response::OK;
}

Response StorageManager::removeDir(const std::string& path) {
    if (path.empty() || isNameInvalid(path)) {
        return Response::InvalidArgument;
    }

    PathInfo info = parsePath(path);
    if (!info.folder) {
        logError("Path not found: " + path);
        return Response::NotFound;
    }
    
    if (isNameInvalid(info.name)) return Response::InvalidArgument;

    // find and remove directory
    for (size_t i = 0; i < info.folder->subfolders.size(); ++i) {
        if (info.folder->subfolders[i]->name == info.name) {
            Folder* toDelete = info.folder->subfolders[i].get();

            // if we are currently inside the deleted folder or one of its subfolders
            Folder* tmp = currentFolder;
            while (tmp != nullptr) {
                if (tmp == toDelete) {
                    // jump up to parent or root if deleting current
                    currentFolder = info.folder; 
                    break;
                }
                tmp = tmp->parent;
            }

            Response delRes = recursiveDelete(*toDelete);
            if (delRes != Response::OK) {
                logError("Failed to recursively delete directory: " + path);
                return delRes;
            }
            info.folder->subfolders.erase(info.folder->subfolders.begin() + i);
            info.folder->modifiedAt = std::chrono::system_clock::now();
            logInfo("Removed directory: " + path);
            return Response::OK;
        }
    }

    logError("Directory not found: " + path);
    return Response::NotFound;
}

Response StorageManager::changeDir(const std::string& path) {
    if (isNameInvalid(path)) return Response::InvalidArgument;
    
    // handle special case "/"
    if (path == "/") {
        currentFolder = root.get();
        logInfo("Changed directory to: /");
        return Response::OK;
    }
    
    // handle special case ".."
    if (path == "..") {
        if (currentFolder->parent == nullptr) return Response::AtRoot;
        currentFolder = currentFolder->parent;
        logInfo("Changed directory to: " + currentFolder->name);
        return Response::OK;
    }
    
    // handle special case "."
    if (path == ".") {
        return Response::OK;
    }
    
    // handle paths with slashes - need custom navigation for cd
    if (path.find('/') != std::string::npos || path[0] == '/') {
        bool isAbsolute = (path[0] == '/');
        Folder* current = isAbsolute ? root.get() : currentFolder;
        
        // split path by '/'
        std::vector<std::string> parts;
        std::string part;
        
        for (char c : path) {
            if (c == '/') {
                if (!part.empty()) {
                    parts.push_back(part);
                    part.clear();
                }
            } else {
                part += c;
            }
        }
        if (!part.empty()) {
            parts.push_back(part);
        }
        
        // navigate through all parts
        for (const std::string& dirName : parts) {
            if (dirName == ".") {
                continue;
            } else if (dirName == "..") {
                if (current->parent) {
                    current = current->parent;
                } else {
                    return Response::AtRoot;
                }
            } else {
                // find subfolder
                bool found = false;
                for (auto& sub : current->subfolders) {
                    if (sub->name == dirName) {
                        current = sub.get();
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    logError("Directory not found: " + path);
                    return Response::NotFound;
                }
            }
        }
        
        currentFolder = current;
        logInfo("Changed directory to: " + currentFolder->name);
        return Response::OK;
    }
    
    // single directory name in current folder
    int i = findFolderIndex(path);
    if (i == -1) {
        logError("Directory not found: " + path);
        return Response::NotFound;
    }
    currentFolder = currentFolder->subfolders[i].get();
    logInfo("Changed directory to: " + currentFolder->name);
    return Response::OK;
}

Response StorageManager::listDir(const std::string& path, std::vector<std::string>& outEntries) const {
    outEntries.clear();
    Folder* targetFolder = nullptr;
    
    // handle special cases
    if (path == "." || path.empty()) {
        targetFolder = currentFolder;
    } else if (path == "..") {
        targetFolder = currentFolder->parent ? currentFolder->parent : currentFolder;
    } else {
        PathInfo info = parsePath(path);
        
        if (!info.folder) {
            return Response::NotFound;
        }
        
        if (info.name.empty()) {
            targetFolder = info.folder;
        } else {
            for (const auto& sub : info.folder->subfolders) {
                if (sub->name == info.name) {
                    targetFolder = sub.get();
                    break;
                }
            }
            
            if (!targetFolder) {
                return Response::NotFound;
            }
        }
    }
    
    // list the target folder's contents
    for (auto& f : targetFolder->subfolders) {
        std::ostringstream line;
        line << "[D] " << f->name
             << " | created: " << formatTime(f->createdAt)
             << " | modified: " << formatTime(f->modifiedAt);
        outEntries.push_back(line.str());
    }
    for (auto& fl : targetFolder->files) {
        std::ostringstream line;
        line << "[F] " << fl->name
             << " | created: " << formatTime(fl->createdAt)
             << " | modified: " << formatTime(fl->modifiedAt)
             << " | size: " << fl->contentSize << " bytes";
        outEntries.push_back(line.str());
    }
    
    return Response::OK;
}

std::string StorageManager::getWorkingDir() const {
    std::ostringstream path;
    std::vector<std::string> parts;
    auto* tmp = currentFolder;
    
    while (tmp) {
        if (tmp->name != "/") {
            parts.push_back(tmp->name);
        }
        tmp = tmp->parent;
    }
    
    if (parts.empty()) {
        return "/";
    }
    
    path << "/";
    for (int i = static_cast<int>(parts.size()) - 1; i >= 0; --i) {
        path << parts[i];
        if (i != 0) path << "/";
    }
    return path.str();
}

void StorageManager::recursiveCopyDir(const Folder& src, Folder& destParent) {
    auto target = std::make_unique<Folder>();
    target->name = src.name;
    target->parent = &destParent;
    target->createdAt = std::chrono::system_clock::now();
    target->modifiedAt = std::chrono::system_clock::now();

    for (const auto& f : src.files) {
        auto fileCopy = std::make_unique<File>(*f);
        fileCopy->createdAt = std::chrono::system_clock::now();
        fileCopy->modifiedAt = std::chrono::system_clock::now();
        target->files.push_back(std::move(fileCopy));
    }

    for (const auto& sub : src.subfolders) {
        recursiveCopyDir(*sub, *target);
    }

    destParent.subfolders.push_back(std::move(target));
}

Response StorageManager::copyDir(const std::string& srcPath, const std::string& destPath) {
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
    
    // find source folder
    Folder* srcFolder = nullptr;
    for (const auto& sub : srcInfo.folder->subfolders) {
        if (sub->name == srcInfo.name) {
            srcFolder = sub.get();
            break;
        }
    }
    
    if (!srcFolder) {
        logError("Source directory not found: " + srcPath);
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
        // dest is a directory, copy dir into it with original name
        for (const auto& sub : targetDir->subfolders) {
            if (sub->name == srcInfo.name) {
                logError("Directory already exists: " + srcInfo.name);
                return Response::AlreadyExists;
            }
        }
        
        recursiveCopyDir(*srcFolder, *targetDir);
        targetDir->modifiedAt = std::chrono::system_clock::now();
        
        logInfo("Copied directory '" + srcPath + "' into '" + destPath + "'");
        return Response::OK;
    }
    
    // dest is not a directory - copy with new name
    for (const auto& sub : destInfo.folder->subfolders) {
        if (sub->name == destInfo.name) {
            logError("Destination directory already exists: " + destPath);
            return Response::AlreadyExists;
        }
    }

    recursiveCopyDir(*srcFolder, *destInfo.folder);
    destInfo.folder->subfolders.back()->name = destInfo.name;
    destInfo.folder->modifiedAt = std::chrono::system_clock::now();

    logInfo("Copied directory '" + srcPath + "' to '" + destPath + "'");
    return Response::OK;
}

Response StorageManager::moveDir(const std::string& srcPath, const std::string& destPath) {
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
    
    // find source folder
    int srcIndex = -1;
    for (size_t i = 0; i < srcInfo.folder->subfolders.size(); ++i) {
        if (srcInfo.folder->subfolders[i]->name == srcInfo.name) {
            srcIndex = static_cast<int>(i);
            break;
        }
    }
    
    if (srcIndex == -1) {
        logError("Source directory not found: " + srcPath);
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

    // Only block if destination is inside the source folder (or is the source itself)
    Folder* srcFolder = nullptr;
    for (const auto& sub : srcInfo.folder->subfolders) {
        if (sub->name == srcInfo.name) {
            srcFolder = sub.get();
            break;
        }
    }
    if (!srcFolder) {
        logError("Source directory not found: " + srcPath);
        return Response::NotFound;
    }
    if (isDescendantOrSame(srcFolder, destInfo.folder)) {
        logError("cannot move '" + srcInfo.name + "' to a subdirectory of itself, '" + destPath + "'");
        return Response::InvalidArgument;
    }

    if (targetDir) {
        // dest is a directory, move dir into it with original name
        for (const auto& sub : targetDir->subfolders) {
            if (sub->name == srcInfo.name) {
                logError("Directory already exists: " + srcInfo.name);
                return Response::AlreadyExists;
            }
        }

        auto folderPtr = std::move(srcInfo.folder->subfolders[srcIndex]);
        srcInfo.folder->subfolders.erase(srcInfo.folder->subfolders.begin() + srcIndex);
        folderPtr->parent = targetDir;
        targetDir->subfolders.push_back(std::move(folderPtr));
        
        srcInfo.folder->modifiedAt = std::chrono::system_clock::now();
        targetDir->modifiedAt = std::chrono::system_clock::now();
        
        logInfo("Moved directory '" + srcPath + "' into '" + destPath + "'");
        return Response::OK;
    }
    
    // dest is not a directory, rename/move with new name
    for (const auto& sub : destInfo.folder->subfolders) {
        if (sub->name == destInfo.name) {
            logError("Destination directory already exists: " + destPath);
            return Response::AlreadyExists;
        }
    }

    auto folderPtr = std::move(srcInfo.folder->subfolders[srcIndex]);
    srcInfo.folder->subfolders.erase(srcInfo.folder->subfolders.begin() + srcIndex);
    folderPtr->name = destInfo.name;
    folderPtr->parent = destInfo.folder;
    folderPtr->modifiedAt = std::chrono::system_clock::now();
    destInfo.folder->subfolders.push_back(std::move(folderPtr));
    
    srcInfo.folder->modifiedAt = std::chrono::system_clock::now();
    destInfo.folder->modifiedAt = std::chrono::system_clock::now();

    logInfo("Moved directory '" + srcPath + "' to '" + destPath + "'");
    return Response::OK;
}

bool StorageManager::isDescendantOrSame(const Folder* ancestor, const Folder* descendant) {
    if (!ancestor || !descendant) return false;
    const Folder* p = descendant;
    while (p != nullptr) {
        if (p == ancestor) return true;
        p = p->parent;
    }
    return false;
}


}  // namespace storage
