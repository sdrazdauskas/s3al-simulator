#include "MemoryManager.h"
#include <iostream>

namespace memory {

MemoryManager::MemoryManager(size_t total_size)
    : totalMemory(total_size), usedMemory(0) {
    std::cout << "Memory manager initialized with "
              << total_size / 1024 << "KB\n";
}

void MemoryManager::log(const std::string& level, const std::string& message) {
    if (logCallback) {
        logCallback(level, "MEMORY", message);
    }
}

MemoryManager::~MemoryManager() {
    // Clean up any remaining allocations
    for (auto& [ptr, alloc] : allocations) {
        delete[] static_cast<std::byte*>(ptr);
    }
}

void *MemoryManager::allocate(size_t size, int processId)
{
    if (usedMemory + size > totalMemory) {
        log("ERROR", "Out of memory: requested " + std::to_string(size) + " bytes");
        std::cerr << "Error: Out of memory\n";
        return nullptr;
    }

    void *ptr = new std::byte[size];
    allocations[ptr] = {size, processId};
    usedMemory += size;

    log("DEBUG", "Allocated " + std::to_string(size) + " bytes for process " + std::to_string(processId));
    return ptr;
}

void MemoryManager::deallocate(void *ptr)
{
    auto it = allocations.find(ptr);
    if (it == allocations.end()) {
        log("ERROR", "Attempt to deallocate untracked memory");
        std::cerr << "Error: Attempt to deallocate untracked memory\n";
        return;
    }

    usedMemory -= it->second.size;
    log("DEBUG", "Deallocated " + std::to_string(it->second.size) + " bytes");
    delete[] static_cast<std::byte*>(ptr);
    allocations.erase(it);
}

void MemoryManager::freeProcessMemory(int processId)
{
    size_t freed = 0;
    for (auto it = allocations.begin(); it != allocations.end(); ) {
        if (it->second.processId == processId) {
            usedMemory -= it->second.size;
            freed += it->second.size;
            delete[] static_cast<std::byte*>(it->first);
            it = allocations.erase(it); 
        } else {
            ++it;
        }
    }
    if (freed > 0) {
        log("INFO", "Freed " + std::to_string(freed) + " bytes for process " + std::to_string(processId));
    }
}

} // namespace memory