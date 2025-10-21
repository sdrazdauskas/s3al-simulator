#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace storage {

void sendToKernel(const std::string& message) {
    std::cout << "[Kernel] Message received: " << message << std::endl;
}

struct File {
    std::string name;
    std::string content;
};

struct Folder {
    std::string name;
    std::vector<std::unique_ptr<File>> files;
    std::vector<std::unique_ptr<Folder>> subfolders;
    Folder* parent = nullptr;
};

// root and current directory
Folder root = {"root", {}, {}, nullptr};
Folder* currentFolder = &root;

int findFileIndex(Folder* dir, const std::string& name) {
    for (size_t i = 0; i < dir->files.size(); ++i) {
        if (dir->files[i]->name == name) return static_cast<int>(i);
    }
    return -1;
}

int findFolderIndex(Folder* dir, const std::string& name) {
    for (size_t i = 0; i < dir->subfolders.size(); ++i) {
        if (dir->subfolders[i]->name == name) return static_cast<int>(i);
    }
    return -1;
}

void createFile(const std::string& name) {
    if (findFileIndex(currentFolder, name) != -1) {
        std::cout << "Error: File '" << name << "' already exists" << std::endl;
        return;
    }

    currentFolder->files.push_back(std::make_unique<File>(File{name, ""}));
    std::cout << "[Storage] Created file: " << name << std::endl;
    sendToKernel("Created file " + name);
}

void deleteFile(const std::string& name) {
    int i = findFileIndex(currentFolder, name);
    if (i == -1) {
        std::cout << "Error: File not found: " << name << std::endl;
        return;
    }

    currentFolder->files.erase(currentFolder->files.begin() + i);
    std::cout << "[Storage] Deleted file: " << name << std::endl;
    sendToKernel("Deleted file " + name);
}

void writeFile(const std::string& name) {
    int i = findFileIndex(currentFolder, name);
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

void readFile(const std::string& name) {
    int i = findFileIndex(currentFolder, name);
    if (i == -1) {
        std::cout << "Error: File not found: " << name << std::endl;
        return;
    }

    std::cout << "[Storage] Reading " << name << std::endl;
    std::cout << "Content:\n" << currentFolder->files[i]->content << std::endl;
}

void editFile(const std::string& fileName) {
    int i = findFileIndex(currentFolder, fileName);
    if (i == -1) {
        std::cout << "Error: File not found: " << fileName << std::endl;
        return;
    }

    File& f = *currentFolder->files[i];

    std::cout << "=== Editing " << fileName << " ===" << std::endl;
    std::cout << "Current content:\n";
    if (f.content.empty())
        std::cout << "(empty)" << std::endl;
    else
        std::cout << f.content << std::endl;

    std::cout << "--------------------------------------" << std::endl;
    std::cout << "Type new content below to ADD to the file." << std::endl;
    std::cout << "Type ':wq' on a new line to save and exit." << std::endl;
    std::cout << "--------------------------------------" << std::endl;

    std::string newLines;
    std::string line;

    while (true) {
        std::getline(std::cin, line);
        if (line == ":wq") break;
        newLines += line + "\n";
    }

    f.content += newLines;

    std::cout << "[Storage] Appended changes to file: " << fileName << std::endl;
    sendToKernel("Appended to file " + fileName);
}

void makeDir(const std::string& folderName) {
    if (findFolderIndex(currentFolder, folderName) != -1) {
        std::cout << "Error: Folder already exists" << std::endl;
        return;
    }

    auto newFolder = std::make_unique<Folder>();
    newFolder->name = folderName;
    newFolder->parent = currentFolder;
    currentFolder->subfolders.push_back(std::move(newFolder));

    std::cout << "[Storage] Created folder: " << folderName << std::endl;
    sendToKernel("Created folder " + folderName);
}

void recursiveDelete(Folder& folder) {
    folder.files.clear();
    for (auto& sub : folder.subfolders) {
        recursiveDelete(*sub);
    }
    folder.subfolders.clear();
}

void removeDir(const std::string& name) {
    int i = findFolderIndex(currentFolder, name);
    if (i == -1) {
        std::cout << "Error: Folder not found" << std::endl;
        return;
    }

    currentFolder->subfolders.erase(currentFolder->subfolders.begin() + i);
    std::cout << "[Storage] Deleted folder (recursively): " << name << std::endl;
    sendToKernel("Deleted folder " + name);
}

void changeDir(const std::string& folderName) {
    if (folderName == "..") {
        if (currentFolder->parent == nullptr) {
            std::cout << "Already at root folder." << std::endl;
            return;
        }
        currentFolder = currentFolder->parent;
        std::cout << "[Storage] Now in folder: " << currentFolder->name << std::endl;
        sendToKernel("Changed directory up one level");
        return;
    }

    int i = findFolderIndex(currentFolder, folderName);
    if (i == -1) {
        std::cout << "Error: Folder not found: " << folderName << std::endl;
        return;
    }

    currentFolder = currentFolder->subfolders[i].get();
    std::cout << "[Storage] Now in folder: " << currentFolder->name << std::endl;
    sendToKernel("Changed into folder " + folderName);
}

void listDir() {
    std::cout << "=== Contents of " << currentFolder->name << " ===" << std::endl;
    std::cout << "Folders:" << std::endl;
    if (currentFolder->subfolders.empty())
        std::cout << "  (none)" << std::endl;
    else
        for (auto& f : currentFolder->subfolders)
            std::cout << "  [D] " << f->name << std::endl;

    std::cout << "Files:" << std::endl;
    if (currentFolder->files.empty())
        std::cout << "  (none)" << std::endl;
    else
        for (auto& file : currentFolder->files)
            std::cout << "  [F] " << file->name << std::endl;
}

void printWorkingDir() {
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

void handleCommand(const std::string& command) {
    std::stringstream ss(command);
    std::string cmd, arg;
    ss >> cmd;

    if (cmd == "touch") {
        ss >> arg;
        if (arg.empty()) std::cout << "Usage: touch <filename>" << std::endl;
        else createFile(arg);

    } else if (cmd == "rm") {
        ss >> arg;
        if (arg.empty()) std::cout << "Usage: rm <filename>" << std::endl;
        else deleteFile(arg);

    } else if (cmd == "write") {
        ss >> arg;
        if (arg.empty()) std::cout << "Usage: write <filename>" << std::endl;
        else writeFile(arg);

    } else if (cmd == "cat") {
        ss >> arg;
        if (arg.empty()) std::cout << "Usage: cat <filename>" << std::endl;
        else readFile(arg);

    } else if (cmd == "edit") {
        ss >> arg;
        if (arg.empty())
            std::cout << "Usage: edit <filename>" << std::endl;
        else editFile(arg);

    } else if (cmd == "mkdir") {
        ss >> arg;
        if (arg.empty()) std::cout << "Usage: mkdir <foldername>" << std::endl;
        else makeDir(arg);

    } else if (cmd == "rmdir") {
        ss >> arg;
        if (arg.empty())
            std::cout << "Usage: rmdir <foldername>" << std::endl;
        else removeDir(arg);

    } else if (cmd == "cd") {
        ss >> arg;
        if (arg.empty()) std::cout << "Usage: cd <foldername>" << std::endl;
        else changeDir(arg);

    } else if (cmd == "ls") {
        listDir();

    } else if (cmd == "pwd") {
        printWorkingDir();

    } else if (cmd == "exit") {
        std::cout << "Exiting simulated OS..." << std::endl;
        exit(0);

    } else {
        std::cout << "Unknown command: " << cmd << std::endl;
    }
}

}  // namespace storage

int main() {
    std::cout << "=== Simulated OS Terminal ===" << std::endl;
    std::cout << "Commands: touch, rm, write, cat, edit, mkdir, rmdir, cd, ls, pwd, exit"
              << std::endl;

    std::string input;
    while (true) {
        std::cout << storage::currentFolder->name << " >> ";
        std::getline(std::cin, input);
        if (!input.empty()) storage::handleCommand(input);
    }
}