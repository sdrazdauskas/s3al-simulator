#include <iostream>
#include <string>
#include "Storage.h"

int main() {
    std::cout << "=== Simulated OS Terminal ===" << std::endl;
    std::cout << "Commands: touch, rm, write, cat, edit, mkdir, rmdir, cd, ls, pwd, exit"
              << std::endl;

    std::string input;
    while (true) {
        std::cout << storage::currentFolder->name << " >> ";
        std::getline(std::cin, input);
        if (!input.empty())
            storage::handleCommand(input);
    }

    return 0;
}