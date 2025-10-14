#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

class MemoryManager {
public:
    MemoryManager() = default;

    // Placeholders: just print that the commands were received
    void allocate_memory_for_process(int pid, int memory_needed_mb);
    void deallocate_memory_for_process(int pid);
};

#endif
