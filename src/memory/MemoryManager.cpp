#include "MemoryManager.h"
#include <iostream>

namespace memory {

MemoryManager::MemoryManager(size_t total_size)
    : total_memory(total_size), used_memory(0) {
    std::cout << "Memory manager initialized with "
              << total_size / 1024 << "KB\n";
}

MemoryManager::~MemoryManager() {
    // Clean up any remaining allocations
    for (auto& [ptr, alloc] : allocations) {
        delete[] static_cast<std::byte*>(ptr);
    }
}

void *MemoryManager::allocate(size_t size, int process_id)
{
    if (used_memory + size > total_memory) {
        std::cerr << "Error: Out of memory\n";
        return nullptr;
    }

    void *ptr = new std::byte[size];
    allocations[ptr] = {size, process_id};
    used_memory += size;

    return ptr;
}

void MemoryManager::deallocate(void *ptr)
{
    auto it = allocations.find(ptr);
    if (it == allocations.end()) {
        std::cerr << "Error: Attempt to deallocate untracked memory\n";
        return;
    }

    used_memory -= it->second.size;
    delete[] static_cast<std::byte*>(ptr);
    allocations.erase(it);
}

void MemoryManager::free_process_memory(int process_id)
{
    for (auto it = allocations.begin(); it != allocations.end(); ) {
        if (it->second.process_id == process_id) {
            used_memory -= it->second.size;
            delete[] static_cast<std::byte*>(it->first);
            it = allocations.erase(it); 
        } else {
            ++it;
        }
    }
}

} // namespace memory