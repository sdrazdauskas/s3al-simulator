#ifndef STORAGE_H
#define STORAGE_H

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

namespace storage {

struct File {
    std::string name;
    std::string content;
};

struct Folder {
    std::string name;
    std::vector<File> files;
    std::vector<Folder> subfolders;
    Folder* parent = nullptr;
};

extern Folder root;
extern Folder* currentFolder;

void sendToKernel(std::string message);
int findFileIndex(Folder* dir, std::string name);
int findFolderIndex(Folder* dir, std::string name);
void createFile(std::string name);
void deleteFile(std::string name);
void writeFile(std::string name);
void readFile(std::string name);
void editFile(std::string fileName);
void makeDir(std::string folderName);
void recursiveDelete(Folder& folder);
void removeDir(std::string name);
void changeDir(std::string folderName);
void listDir();
void printWorkingDir();
void handleCommand(std::string command);

}  // namespace storage

#endif  // STORAGE_H