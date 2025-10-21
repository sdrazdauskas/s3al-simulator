#include "Storage.h"
#include <iostream>
#include <sstream>

using Status = StorageManager::StorageStatus;

int main() {
    StorageManager storage;

    std::cout << "=== Simulated OS Terminal ===\n";
    std::cout << "Commands: touch, rm, write, cat, edit, mkdir, rmdir, cd, ls, pwd, exit\n";

    std::string input, cmd, arg;

    while (true) {
        cmd.clear();
        arg.clear();

        std::cout << "Kernel >> ";
        std::getline(std::cin, input);
        if (input.empty()) continue;

        std::stringstream ss(input);
        ss >> cmd; // first word
        ss >> arg; // second word (may remain empty)

        if (cmd == "exit")
            break;

        else if (cmd == "touch") {
            if (arg.empty()) {
                std::cout << "[Kernel] Usage: touch <filename>\n";
                continue;
            }
            std::cout << "[Kernel] "
                      << StorageManager::toString(storage.createFile(arg)) << "\n";
        }

        else if (cmd == "rm") {
            if (arg.empty()) {
                std::cout << "[Kernel] Usage: rm <filename>\n";
                continue;
            }
            std::cout << "[Kernel] "
                      << StorageManager::toString(storage.deleteFile(arg)) << "\n";
        }

        else if (cmd == "write") {
            if (arg.empty()) {
                std::cout << "[Kernel] Usage: write <filename>\n";
                continue;
            }
            std::cout << "Enter content: ";
            std::string content;
            std::getline(std::cin, content);
            std::cout << "[Kernel] "
                      << StorageManager::toString(storage.writeFile(arg, content)) << "\n";
        }

        else if (cmd == "cat") {
            if (arg.empty()) {
                std::cout << "[Kernel] Usage: cat <filename>\n";
                continue;
            }
            std::string contents;
            auto status = storage.readFile(arg, contents);
            if (status == Status::OK)
                std::cout << contents;
            else
                std::cout << "[Kernel] " << StorageManager::toString(status) << "\n";
        }

        else if (cmd == "edit") {
            if (arg.empty()) {
                std::cout << "[Kernel] Usage: edit <filename>\n";
                continue;
            }
            std::cout << "[Kernel] "
                      << StorageManager::toString(storage.editFile(arg)) << "\n";
        }

        else if (cmd == "mkdir") {
            if (arg.empty()) {
                std::cout << "[Kernel] Usage: mkdir <foldername>\n";
                continue;
            }
            std::cout << "[Kernel] "
                      << StorageManager::toString(storage.makeDir(arg)) << "\n";
        }

        else if (cmd == "rmdir") {
            if (arg.empty()) {
                std::cout << "[Kernel] Usage: rmdir <foldername>\n";
                continue;
            }
            std::cout << "[Kernel] "
                      << StorageManager::toString(storage.removeDir(arg)) << "\n";
        }

        else if (cmd == "cd") {
            if (arg.empty()) {
                std::cout << "[Kernel] Usage: cd <foldername | ..>\n";
                continue;
            }
            std::cout << "[Kernel] "
                      << StorageManager::toString(storage.changeDir(arg)) << "\n";
        }

        else if (cmd == "ls") {
            auto entries = storage.listDir();
            std::cout << "=== Contents of " << storage.getWorkingDir() << " ===\n";
            if (entries.empty()) std::cout << "(empty)\n";
            for (auto& e : entries)
                std::cout << e << "\n";
        }

        else if (cmd == "pwd") {
            std::cout << storage.getWorkingDir() << "\n";
        }

        else
            std::cout << "[Kernel] Unknown command: " << cmd << "\n";
    }

    std::cout << "Exiting simulated OS...\n";
    return 0;
}