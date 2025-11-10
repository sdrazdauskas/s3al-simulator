#include "Storage.h"
#include <sstream>

namespace storage {

using Response = StorageManager::StorageResponse;

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

Response StorageManager::makeDir(const std::string& name) {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    if (findFolderIndex(name) != -1) {
        log("ERROR", "Directory already exists: " + name);
        return Response::AlreadyExists;
    }

    auto folder = std::make_unique<Folder>();
    folder->name = name;
    folder->parent = currentFolder;
    folder->createdAt = std::chrono::system_clock::now();
    folder->modifiedAt = folder->createdAt;

    currentFolder->subfolders.push_back(std::move(folder));
    currentFolder->modifiedAt = std::chrono::system_clock::now();
    log("INFO", "Created directory: " + name);
    return Response::OK;
}

Response StorageManager::removeDir(const std::string& name) {
    if (isNameInvalid(name)) return Response::InvalidArgument;
    int i = findFolderIndex(name);
    if (i == -1) {
        log("ERROR", "Directory not found: " + name);
        return Response::NotFound;
    }
    currentFolder->subfolders.erase(currentFolder->subfolders.begin() + i);
    currentFolder->modifiedAt = std::chrono::system_clock::now();
    log("INFO", "Removed directory: " + name);
    return Response::OK;
}

Response StorageManager::changeDir(const std::string& path) {
    if (isNameInvalid(path)) return Response::InvalidArgument;
    if (path == "..") {
        if (currentFolder->parent == nullptr) return Response::AtRoot;
        currentFolder = currentFolder->parent;
        log("INFO", "Changed directory to: " + currentFolder->name);
        return Response::OK;
    }
    int i = findFolderIndex(path);
    if (i == -1) {
        log("ERROR", "Directory not found: " + path);
        return Response::NotFound;
    }
    currentFolder = currentFolder->subfolders[i].get();
    log("INFO", "Changed directory to: " + currentFolder->name);
    return Response::OK;
}

std::vector<std::string> StorageManager::listDir() const {
    std::vector<std::string> entries;
    for (auto& f : currentFolder->subfolders) {
        std::ostringstream line;
        line << "[D] " << f->name
             << " | created: " << formatTime(f->createdAt)
             << " | modified: " << formatTime(f->modifiedAt);
        entries.push_back(line.str());
    }
    for (auto& fl : currentFolder->files) {
        std::ostringstream line;
        line << "[F] " << fl->name
             << " | created: " << formatTime(fl->createdAt)
             << " | modified: " << formatTime(fl->modifiedAt);
        entries.push_back(line.str());
    }
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

Response StorageManager::copyDir(const std::string& srcName,
                                 const std::string& destName) {
    int srcIndex = findFolderIndex(srcName);
    if (srcIndex == -1)
        return Response::NotFound;

    Folder* srcFolder = currentFolder->subfolders[srcIndex].get();

    // if destName is an existing dir, copy src inside it
    int destDirIndex = findFolderIndex(destName);
    if (destDirIndex != -1) {
        Folder* destDir = currentFolder->subfolders[destDirIndex].get();

        // prevent same name conflict inside destination dir
        for (const auto& sub : destDir->subfolders)
            if (sub->name == srcFolder->name)
                return Response::AlreadyExists;

        // copy inside destination dir
        recursiveCopyDir(*srcFolder, *destDir);
        log("INFO", "Copied '" + srcName + "' into directory '" + destName + "'");
        return Response::OK;
    }

    // or else copy and rename to destName
    if (findFolderIndex(destName) != -1)
        return Response::AlreadyExists;

    recursiveCopyDir(*srcFolder, *currentFolder);
    currentFolder->subfolders.back()->name = destName;

    log("INFO", "Copied directory '" + srcName + "' to '" + destName + "'");
    return Response::OK;
}

Response StorageManager::moveDir(const std::string& oldName,
                                 const std::string& newName) {
    int srcIndex = findFolderIndex(oldName);
    if (srcIndex == -1)
        return Response::NotFound;

    // destination is an existing dir, move inside
    int destDirIndex = findFolderIndex(newName);
    if (destDirIndex != -1) {
        Folder* destDir = currentFolder->subfolders[destDirIndex].get();

        // prevent same name conflict inside destination dir
        for (const auto& sub : destDir->subfolders)
            if (sub->name == oldName)
                return Response::AlreadyExists;

        // move folder pointer
        auto folderPtr = std::move(currentFolder->subfolders[srcIndex]);
        currentFolder->subfolders.erase(currentFolder->subfolders.begin() + srcIndex);
        folderPtr->parent = destDir;
        destDir->subfolders.push_back(std::move(folderPtr));

        log("INFO", "Moved folder '" + oldName + "' into directory '" + newName + "'");
        return Response::OK;
    }

    // or else destination is a new name, rename folder
    if (findFolderIndex(newName) != -1)
        return Response::AlreadyExists;

    currentFolder->subfolders[srcIndex]->name = newName;
    currentFolder->subfolders[srcIndex]->modifiedAt = std::chrono::system_clock::now();

    log("INFO", "Renamed folder from '" + oldName + "' to '" + newName + "'");
    return Response::OK;
}

}  // namespace storage
