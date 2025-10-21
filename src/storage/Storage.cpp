#include "Storage.h"

StorageManager::StorageManager() {
    root = std::make_unique<Folder>();
    root->name = "root";
    root->parent = nullptr;
    currentFolder = root.get();
}

void StorageManager::sendToKernel(const std::string& message) const {
    std::cout << "[Kernel] Message received: " << message << std::endl;
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

void StorageManager::createFile(const std::string& name) {
    if (findFileIndex(name) != -1) {
        std::cout << "Error: File '" << name << "' already exists\n";
        return;
    }
    currentFolder->files.push_back(std::make_unique<File>(File{name, ""}));
    std::cout << "[Storage] Created file: " << name << std::endl;
    sendToKernel("Created file " + name);
}

void StorageManager::deleteFile(const std::string& name) {
    int i = findFileIndex(name);
    if (i == -1) {
        std::cout << "Error: File not found: " << name << std::endl;
        return;
    }
    currentFolder->files.erase(currentFolder->files.begin() + i);
    std::cout << "[Storage] Deleted file: " << name << std::endl;
    sendToKernel("Deleted file " + name);
}

void StorageManager::writeFile(const std::string& name) {
    int i = findFileIndex(name);
    if (i == -1) {
        std::cout << "Error: File not found: " << name << std::endl;
        return;
    }
    std::cout << "Enter content for " << name << ": ";
    std::string content;
    std::getline(std::cin, content);
    content += "\n";
    currentFolder->files[i]->content = content;
    std::cout << "[Storage] Wrote to file: " << name << std::endl;
    sendToKernel("Wrote data to " + name);
}

void StorageManager::readFile(const std::string& name) {
    int i = findFileIndex(name);
    if (i == -1) {
        std::cout << "Error: File not found: " << name << std::endl;
        return;
    }
    std::cout << "[Storage] Reading " << name << std::endl;
    std::cout << "Content:\n" << currentFolder->files[i]->content << std::endl;
}

void StorageManager::editFile(const std::string& fileName) {
    int i = findFileIndex(fileName);
    if (i == -1) {
        std::cout << "Error: File not found: " << fileName << std::endl;
        return;
    }

    File& f = *currentFolder->files[i];
    std::cout << "=== Editing " << fileName << " ===\n";
    if (f.content.empty())
        std::cout << "(empty)\n";
    else
        std::cout << f.content << std::endl;

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
    std::cout << "[Storage] Appended changes to file: " << fileName << std::endl;
    sendToKernel("Appended to file " + fileName);
}

void StorageManager::makeDir(const std::string& folderName) {
    if (findFolderIndex(folderName) != -1) {
        std::cout << "Error: Folder already exists\n";
        return;
    }
    auto newFolder = std::make_unique<Folder>();
    newFolder->name = folderName;
    newFolder->parent = currentFolder;
    currentFolder->subfolders.push_back(std::move(newFolder));
    std::cout << "[Storage] Created folder: " << folderName << std::endl;
    sendToKernel("Created folder " + folderName);
}

void StorageManager::recursiveDelete(Folder& folder) {
    folder.files.clear();
    for (auto& sub : folder.subfolders) recursiveDelete(*sub);
    folder.subfolders.clear();
}

void StorageManager::removeDir(const std::string& name) {
    int i = findFolderIndex(name);
    if (i == -1) {
        std::cout << "Error: Folder not found\n";
        return;
    }
    currentFolder->subfolders.erase(currentFolder->subfolders.begin() + i);
    std::cout << "[Storage] Deleted folder (recursively): " << name << std::endl;
    sendToKernel("Deleted folder " + name);
}

void StorageManager::changeDir(const std::string& folderName) {
    if (folderName == "..") {
        if (currentFolder->parent == nullptr) {
            std::cout << "Already at root folder.\n";
            return;
        }
        currentFolder = currentFolder->parent;
        std::cout << "[Storage] Now in folder: " << currentFolder->name << std::endl;
        sendToKernel("Changed directory up one level");
        return;
    }

    int i = findFolderIndex(folderName);
    if (i == -1) {
        std::cout << "Error: Folder not found: " << folderName << std::endl;
        return;
    }
    currentFolder = currentFolder->subfolders[i].get();
    std::cout << "[Storage] Now in folder: " << currentFolder->name << std::endl;
    sendToKernel("Changed into folder " + folderName);
}

void StorageManager::listDir() const {
    std::cout << "=== Contents of " << currentFolder->name << " ===\nFolders:\n";
    if (currentFolder->subfolders.empty())
        std::cout << "  (none)\n";
    else
        for (auto& f : currentFolder->subfolders)
            std::cout << "  [D] " << f->name << "\n";

    std::cout << "Files:\n";
    if (currentFolder->files.empty())
        std::cout << "  (none)\n";
    else
        for (auto& file : currentFolder->files)
            std::cout << "  [F] " << file->name << "\n";
}

void StorageManager::printWorkingDir() const {
    std::vector<std::string> pathParts;
    Folder* temp = currentFolder;
    while (temp != nullptr) {
        pathParts.push_back(temp->name);
        temp = temp->parent;
    }
    std::cout << "/";
    for (int i = static_cast<int>(pathParts.size()) - 1; i >= 0; --i) {
        std::cout << pathParts[i];
        if (i != 0) std::cout << "/";
    }
    std::cout << std::endl;
}