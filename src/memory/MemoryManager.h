#pragma once

#include <map>
#include <cstddef>
#include <functional>
#include <string>

namespace memory {

class MemoryManager {
public:
    using LogCallback = std::function<void(const std::string& level, 
                                           const std::string& module, 
                                           const std::string& message)>;

    MemoryManager(size_t total_size);
    virtual ~MemoryManager();

    void setLogCallback(LogCallback callback) { log_callback = callback; }

    // Allocate memory for a process
    virtual void *allocate(size_t size, int processId = 0);

    // Deallocate specific pointer
    virtual void deallocate(void* ptr);
    
    // Deallocate ALL memory owned by a process
    virtual void freeProcessMemory(int processId);

    size_t get_total_memory() const { return total_memory; }
    size_t get_used_memory() const { return used_memory; }
    size_t get_free_memory() const { return total_memory - used_memory; }

private:
    struct Allocation {
        size_t size;
        int processId;
    };

    std::map<void*, Allocation> allocations;
    size_t total_memory;
    size_t used_memory;
    LogCallback log_callback;

    void log(const std::string& level, const std::string& message);
};

} // namespace memory