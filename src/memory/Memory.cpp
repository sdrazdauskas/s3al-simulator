#include "Memory.h"
#include <iostream>

namespace memory {

Memory::Memory(size_t total_size)
    : total_memory(total_size), used_memory(0) {
    std::cout << "Memory manager initialized with "
              << total_size / 1024 << "KB\n";
}

Memory::~Memory() {
    // Clean up any remaining allocations
    for (auto& [ptr, alloc] : allocations) {
        delete[] static_cast<std::byte*>(ptr);
    }
}

void *Memory::allocate(size_t size)
{
    if (used_memory + size > total_memory) {
        std::cerr << "Error: Out of memory\n";
        return nullptr;
    }
    void *ptr = new std::byte[size];
    allocations[ptr] = size;
    used_memory += size;
    return ptr;
}

void Memory::deallocate(void *ptr)
{
    auto it = allocations.find(ptr);
    if (it == allocations.end()) {
        std::cerr << "Error: Attempt to deallocate untracked memory\n";
        return;
    }

    used_memory -= it->second;
    delete[] static_cast<std::byte*>(ptr);
    allocations.erase(it);
}

} // namespace memory