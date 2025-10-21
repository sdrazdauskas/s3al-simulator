#include "Storage.h"
#include <iostream>

int main() {
    StorageManager storage;
    std::cout << "=== Simulated OS Terminal ===\n";
    std::cout << "Commands: touch, rm, write, cat, edit, mkdir, rmdir, cd, ls, pwd, exit\n";

    std::string input;
    while (true) {
        std::cout << ">> ";
        std::getline(std::cin, input);
        if (input == "exit") break;
        else if (input.rfind("touch ", 0) == 0) storage.createFile(input.substr(6));
        else if (input.rfind("rm ", 0) == 0) storage.deleteFile(input.substr(3));
        else if (input.rfind("write ", 0) == 0) storage.writeFile(input.substr(6));
        else if (input.rfind("cat ", 0) == 0) storage.readFile(input.substr(4));
        else if (input.rfind("edit ", 0) == 0) storage.editFile(input.substr(5));
        else if (input.rfind("mkdir ", 0) == 0) storage.makeDir(input.substr(6));
        else if (input.rfind("rmdir ", 0) == 0) storage.removeDir(input.substr(6));
        else if (input.rfind("cd ", 0) == 0) storage.changeDir(input.substr(3));
        else if (input == "ls") storage.listDir();
        else if (input == "pwd") storage.printWorkingDir();
        else std::cout << "Unknown command.\n";
    }

    std::cout << "Exiting simulated OS...\n";
    return 0;
}