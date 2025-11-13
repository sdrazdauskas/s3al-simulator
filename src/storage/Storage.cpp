#include "Storage.h"
#include <iostream>

namespace storage {

StorageManager::StorageManager() {
    root = std::make_unique<Folder>();
    root->name = "root";
    root->parent = nullptr;
    currentFolder = root.get();
}

void StorageManager::setLogCallback(LogCallback callback) {
    log_callback = callback;
}

StorageManager::StorageResponse StorageManager::reset() {
    try {
        recursiveDelete(*root);
        root = std::make_unique<Folder>();
        root->name = "root";
        root->parent = nullptr;
        currentFolder = root.get();
        log("INFO", "Storage reset to empty state");
        return StorageResponse::OK;
    } catch (...) {
        return StorageResponse::Error;
    }
}

}  // namespace storage
