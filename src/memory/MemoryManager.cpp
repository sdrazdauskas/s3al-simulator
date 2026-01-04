#include "memory/MemoryManager.h"
#include <iostream>

namespace memory {

MemoryManager::MemoryManager(size_t total_size)
    : totalMemory(total_size), usedMemory(0) {
    std::cout << "Memory manager initialized with "
              << total_size / 1024 << "KB\n";
}

MemoryManager::~MemoryManager() {
    // Clean up any remaining allocations
    for (auto& [ptr, alloc] : allocations) {
        delete[] static_cast<std::byte*>(ptr);
    }
}

void* MemoryManager::allocate(size_t size, int processId)
{
    if (usedMemory + size > totalMemory) {
        logError("Out of memory: requested " + std::to_string(size) + " bytes");
        return nullptr;
    }

    void *ptr = new std::byte[size];
    allocations[ptr] = {size, processId};
    usedMemory += size;

    logDebug("Allocated " + std::to_string(size) + " bytes for process " + std::to_string(processId));
    return ptr;
}

bool MemoryManager::deallocate(void *ptr)
{
    auto it = allocations.find(ptr);
    if (it == allocations.end()) {
        logError("Attempt to deallocate untracked memory");
        return false;
    }

    usedMemory -= it->second.size;
    logDebug("Deallocated " + std::to_string(it->second.size) + " bytes");
    allocations.erase(it);
    delete[] static_cast<std::byte*>(ptr);
    return true;
}

void MemoryManager::freeProcessMemory(int processId)
{
    size_t freed = 0;
    for (auto it = allocations.begin(); it != allocations.end(); ) {
        if (it->second.processId == processId) {
            usedMemory -= it->second.size;
            freed += it->second.size;
            void* ptrToDelete = it->first;
            it = allocations.erase(it);
            delete[] static_cast<std::byte*>(ptrToDelete);
        } else {
            ++it;
        }
    }
    if (freed > 0) {
        logInfo("Freed " + std::to_string(freed) + " bytes for process " + std::to_string(processId));
    }
}

} // namespace memory