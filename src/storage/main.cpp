#include "Storage.h"
#include <iostream>

int main() {
    std::cout << "=== Simulated OS Terminal ===\n";
    std::cout << "Commands: touch, rm, write, cat, edit, mkdir, rmdir, cd, ls, pwd, exit\n";

    std::string input;
    while (true) {
        std::cout << storage::currentFolder->name << " >> ";
        std::getline(std::cin, input);
        if (!input.empty()) storage::handleCommand(input);
    }
}