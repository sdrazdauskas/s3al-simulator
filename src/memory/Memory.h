#ifndef MEMORY_H
#define MEMORY_H

#include <map>
#include <cstddef>

namespace memory {

class Memory {
public:
    Memory(size_t total_size);
    ~Memory();
    void *allocate(size_t size);
    void deallocate(void *ptr);

    size_t get_total_memory() const { return total_memory; }
    size_t get_used_memory() const { return used_memory; }
    size_t get_free_memory() const { return total_memory - used_memory; }

private:
    std::map<void*, size_t> allocations;
    size_t total_memory;
    size_t used_memory;
};

} // namespace memory

#endif // MEMORY_H