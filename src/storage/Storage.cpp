#include <iostream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

void sendToKernel(string message) {
    cout << "[Kernel] Message received: " << message << endl;
}

struct File {
    string name;
    string content;
};

struct Folder {
    string name;
    vector<File> files;
    vector<Folder> subfolders;
    Folder* parent = nullptr;
};

Folder root = {"root", {}, {}, nullptr};
Folder* currentFolder = &root;

int findFileIndex(Folder* dir, string name) {
    for (size_t i = 0; i < dir->files.size(); i++) {
        if (dir->files[i].name == name) return i;
    }
    return -1;
}

int findFolderIndex(Folder* dir, string name) {
    for (size_t i = 0; i < dir->subfolders.size(); i++) {
        if (dir->subfolders[i].name == name) return i;
    }
    return -1;
}

void createFile(string name) {
    if (findFileIndex(currentFolder, name) != -1) {
        cout << "Error: File '" << name << "' already exists" << endl;
        return;
    }

    File f{name, ""};
    currentFolder->files.push_back(f);
    cout << "[Storage] Created file: " << name << endl;
    sendToKernel("Created file " + name);
}

void deleteFile(string name) {
    int i = findFileIndex(currentFolder, name);
    if (i == -1) {
        cout << "Error: File not found: " << name << endl;
        return;
    }

    currentFolder->files.erase(currentFolder->files.begin() + i);
    cout << "[Storage] Deleted file: " << name << endl;
    sendToKernel("Deleted file " + name);
}

void writeFile(string name) {
    int i = findFileIndex(currentFolder, name);
    if (i == -1) {
        cout << "Error: File not found: " << name << endl;
        return;
    }

    cout << "Enter content for " << name << ": ";
    string content;
    getline(cin, content);
    content += "\n";

    currentFolder->files[i].content = content;
    cout << "[Storage] Wrote to file: " << name << endl;
    sendToKernel("Wrote data to " + name);
}

void readFile(string name) {
    int i = findFileIndex(currentFolder, name);
    if (i == -1) {
        cout << "Error: File not found: " << name << endl;
        return;
    }

    cout << "[Storage] Reading " << name << endl;
    cout << "Content: " << currentFolder->files[i].content << endl;
}

void editFile(string fileName) {
    int i = findFileIndex(currentFolder, fileName);
    if (i == -1) {
        cout << "Error: File not found: " << fileName << endl;
        return;
    }

    cout << "=== Editing " << fileName << " ===" << endl;
    cout << "Current content:" << endl;

    if (currentFolder->files[i].content.empty()) {
        cout << "(empty)" << endl;
    } else {
        cout << currentFolder->files[i].content << endl;
    }

    cout << "--------------------------------------" << endl;
    cout << "Type new content below to ADD to the file." << endl;
    cout << "Type ':wq' on a new line to save and exit." << endl;
    cout << "--------------------------------------" << endl;

    string newLines = "";
    string line;

    while (true) {
        getline(cin, line);

        if (line == ":wq") {
            break;
        }

        newLines += line + "\n";
    }

    currentFolder->files[i].content += newLines;

    cout << "[Storage] Appended changes to file: " << fileName << endl;
    sendToKernel("Appended to file " + fileName);
}

void makeDir(string folderName) {
    if (findFolderIndex(currentFolder, folderName) != -1) {
        cout << "Error: Folder already exists" << endl;
        return;
    }

    Folder newFolder;
    newFolder.name = folderName;
    newFolder.parent = currentFolder;
    currentFolder->subfolders.push_back(newFolder);

    cout << "[Storage] Created folder: " << folderName << endl;
    sendToKernel("Created folder " + folderName);
}

void recursiveDelete(Folder& folder) {
    for (Folder& sub : folder.subfolders) {
        recursiveDelete(sub);
    }
    folder.files.clear();
    folder.subfolders.clear();
}

void removeDir(string name) {
    int i = findFolderIndex(currentFolder, name);
    if (i == -1) {
        cout << "Error: Folder not found" << endl;
        return;
    }

    Folder& target = currentFolder->subfolders[i];
    recursiveDelete(target);
    currentFolder->subfolders.erase(currentFolder->subfolders.begin() + i);

    cout << "[Storage] Deleted folder (recursively): " << name << endl;
    sendToKernel("Deleted folder " + name);
}

void changeDir(string folderName) {
    if (folderName == "..") {
        if (currentFolder->parent == nullptr) {
            cout << "Already at root folder." << endl;
            return;
        }

        currentFolder = currentFolder->parent;
        cout << "[Storage] Now in folder: " << currentFolder->name << endl;
        sendToKernel("Changed directory up one level");
        return;
    }

    int i = findFolderIndex(currentFolder, folderName);
    if (i == -1) {
        cout << "Error: Folder not found: " << folderName << endl;
        return;
    }

    currentFolder = &currentFolder->subfolders[i];
    cout << "[Storage] Now in folder: " << currentFolder->name << endl;
    sendToKernel("Changed into folder " + folderName);
}

void listDir() {
    cout << "=== Contents of " << currentFolder->name << " ===" << endl;

    cout << "Folders:" << endl;
    if (currentFolder->subfolders.empty())
        cout << "  (none)" << endl;
    else
        for (auto f : currentFolder->subfolders)
            cout << "  [D] " << f.name << endl;

    cout << "Files:" << endl;
    if (currentFolder->files.empty())
        cout << "  (none)" << endl;
    else
        for (auto file : currentFolder->files)
            cout << "  [F] " << file.name << endl;
}

void printWorkingDir() {
    vector<string> pathParts;
    Folder* temp = currentFolder;

    // Move upward collecting names
    while (temp != nullptr) {
        pathParts.push_back(temp->name);
        temp = temp->parent;
    }

    // print in reverse (root -> current)
    cout << "/";
    for (int i = pathParts.size() - 1; i >= 0; --i) {
        cout << pathParts[i];
        if (i != 0) cout << "/";
    }
    cout << endl;
}

void handleCommand(string command) {
    stringstream ss(command);
    string cmd, arg;
    ss >> cmd;

    if (cmd == "touch") {
        ss >> arg;
        if (arg.empty()) cout << "Usage: touch <filename>" << endl;
        else createFile(arg);

    } else if (cmd == "rm") {
        ss >> arg;
        if (arg.empty()) cout << "Usage: rm <filename>" << endl;
        else deleteFile(arg);

    } else if (cmd == "write") {
        ss >> arg;
        if (arg.empty()) cout << "Usage: write <filename>" << endl;
        else writeFile(arg);

    } else if (cmd == "cat") {
        ss >> arg;
        if (arg.empty()) cout << "Usage: cat <filename>" << endl;
        else readFile(arg);

    } else if (cmd == "edit") {
        ss >> arg;
        if (arg.empty())
            cout << "Usage: edit <filename>" << endl;
        else editFile(arg);

    } else if (cmd == "mkdir") {
        ss >> arg;
        if (arg.empty()) cout << "Usage: mkdir <foldername>" << endl;
        else makeDir(arg);

    } else if (cmd == "rmdir") {
        ss >> arg;
        if (arg.empty())
            cout << "Usage: rmdir <foldername>" << endl;
        else removeDir(arg);

    } else if (cmd == "cd") {
        ss >> arg;
        if (arg.empty()) cout << "Usage: cd <foldername>" << endl;
        else changeDir(arg);

    } else if (cmd == "ls") {
        listDir();

    } else if (cmd == "pwd") {
        printWorkingDir();

    } else if (cmd == "exit") {
        cout << "Exiting simulated OS..." << endl;
        exit(0);

    } else {
        cout << "Unknown command: " << cmd << endl;
    }
}