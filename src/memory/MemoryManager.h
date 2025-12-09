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

    void setLogCallback(LogCallback callback) { logCallback = callback; }

    // Allocate memory for a process
    virtual void *allocate(size_t size, int processId = 0);

    // Deallocate specific pointer
    virtual void deallocate(void* ptr);
    
    // Deallocate ALL memory owned by a process
    virtual void freeProcessMemory(int processId);

    size_t getTotalMemory() const { return totalMemory; }
    size_t getUsedMemory() const { return usedMemory; }
    size_t get_free_memory() const { return totalMemory - usedMemory; }

private:
    struct Allocation {
        size_t size;
        int processId;
    };

    std::map<void*, Allocation> allocations;
    size_t totalMemory;
    size_t usedMemory;
    LogCallback logCallback;

    void log(const std::string& level, const std::string& message);
};

} // namespace memory