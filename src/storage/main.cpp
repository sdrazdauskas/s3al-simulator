#include "Storage.h"
#include <iostream>

int main() {
    StorageManager storage;
    std::cout << "=== Simulated OS Terminal ===\n";
    std::cout << "Commands: touch, rm, write, cat, edit, mkdir, rmdir, cd, ls, pwd, exit\n";

    std::string input;
    while (true) {
        std::cout << "Kernel >> ";
        std::getline(std::cin, input);

        if (input == "exit")
            break;

        else if (input.rfind("touch ", 0) == 0) {
            std::string name = input.substr(6);
            if (storage.createFile(name))
                std::cout << "[Kernel] File created: " << name << "\n";
            else
                std::cout << "[Kernel] Error: file already exists\n";
        }

        else if (input.rfind("rm ", 0) == 0) {
            std::string name = input.substr(3);
            if (storage.deleteFile(name))
                std::cout << "[Kernel] Deleted file: " << name << "\n";
            else
                std::cout << "[Kernel] Error: file not found\n";
        }

        else if (input.rfind("write ", 0) == 0) {
            std::string name = input.substr(6);
            std::cout << "Enter content: ";
            std::string content;
            std::getline(std::cin, content);
            if (storage.writeFile(name, content))
                std::cout << "[Kernel] Written to file: " << name << "\n";
            else
                std::cout << "[Kernel] Error: file not found\n";
        }

        else if (input.rfind("cat ", 0) == 0) {
            std::string name = input.substr(4);
            std::string contents;
            if (storage.readFile(name, contents))
                std::cout << contents << "\n";
            else
                std::cout << "[Kernel] Error: file not found\n";
        }

        else if (input.rfind("edit ", 0) == 0) {
            std::string name = input.substr(5);
            if (!storage.editFile(name))
                std::cout << "[Kernel] Error: file not found\n";
        }

        else if (input.rfind("mkdir ", 0) == 0) {
            std::string name = input.substr(6);
            if (storage.makeDir(name))
                std::cout << "[Kernel] Folder created: " << name << "\n";
            else
                std::cout << "[Kernel] Error: folder already exists\n";
        }

        else if (input.rfind("rmdir ", 0) == 0) {
            std::string name = input.substr(6);
            if (storage.removeDir(name))
                std::cout << "[Kernel] Folder removed: " << name << "\n";
            else
                std::cout << "[Kernel] Error: folder not found\n";
        }

        else if (input.rfind("cd ", 0) == 0) {
            std::string path = input.substr(3);
            if (storage.changeDir(path))
                std::cout << "[Kernel] Changed directory\n";
            else
                std::cout << "[Kernel] Error: folder not found or at root\n";
        }

        else if (input == "ls") {
            auto entries = storage.listDir();
            std::cout << "=== Contents ===\n";
            if (entries.empty()) std::cout << "(empty)\n";
            for (auto& e : entries) std::cout << e << "\n";
        }

        else if (input == "pwd")
            std::cout << storage.getWorkingDir() << "\n";

        else
            std::cout << "[Kernel] Unknown command.\n";
    }

    std::cout << "Exiting simulated OS...\n";
    return 0;
}