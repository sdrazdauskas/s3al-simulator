#include "MemoryManager.h"
#include <iostream>

void MemoryManager::allocate_memory_for_process(int pid, int memory_needed_mb) {
    std::cout << "[MemoryManager] Command to allocate "
              << memory_needed_mb << "MB memory to process "
              << pid << " received.\n";
}

void MemoryManager::deallocate_memory_for_process(int pid) {
    std::cout << "[MemoryManager] Command to deallocate memory for process "
              << pid << " received.\n";
}
