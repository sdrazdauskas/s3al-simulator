#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>

namespace storage {
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

extern Folder root;
extern Folder* currentFolder;

void sendToKernel(const std::string& message);

void createFile(const std::string& name);
void deleteFile(const std::string& name);
void writeFile(const std::string& name);
void readFile(const std::string& name);
void editFile(const std::string& name);
void makeDir(const std::string& name);
void removeDir(const std::string& name);
void changeDir(const std::string& name);
void listDir();
void printWorkingDir();
void handleCommand(const std::string& command);

}  // namespace storage