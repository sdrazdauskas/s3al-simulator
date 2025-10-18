#ifndef MEMORY_H
#define MEMORY_H

#include <map>
#include <cstddef>

namespace memory {

class Memory {
public:
    Memory(size_t total_size);
    ~Memory();
    
    // Allocate memory for a process
    void *allocate(size_t size, int process_id = 0);

    // Deallocate specific pointer
    void deallocate(void* ptr);
    
    // Deallocate ALL memory owned by a process
    void free_process_memory(int process_id);

    size_t get_total_memory() const { return total_memory; }
    size_t get_used_memory() const { return used_memory; }
    size_t get_free_memory() const { return total_memory - used_memory; }

private:
    struct Allocation {
        size_t size;
        int process_id;
    };

    std::map<void*, Allocation> allocations;
    size_t total_memory;
    size_t used_memory;
};

} // namespace memory

#endif // MEMORY_H