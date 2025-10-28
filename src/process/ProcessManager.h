#pragma once

#include <string>
#include <optional>
#include <vector>
#include "Process.h"
#include "../memory/MemoryManager.h"
#include "../scheduler/scheduler.h"
namespace memory {
    class MemoryManager;
}

namespace scheduler {
    class CPUScheduler;
}

class MemoryManager;
class CPUScheduler;

namespace process {
    
class ProcessManager {
public:
    ProcessManager(memory::MemoryManager& mem, scheduler::CPUScheduler& cpu);

    // One-shot orchestration: prepare -> run (alloc→execute→dealloc) -> stop
    // Returns PID on success; -1 on validation failure.
    int execute_process(const std::string& name,
                        int cpuTimeNeeded,
                        int memoryNeeded,
                        int priority = 0);

    // Low-level steps (also used by execute_process)
    int  create_process(const std::string& name,
                        int cpuTimeNeeded,
                        int memoryNeeded,
                        int priority = 0);   // prepare only; NO allocation
    bool run_process(int pid);                 // alloc → execute → dealloc
    bool stop_process(int pid);                 // mark terminated and remove

    // Read-only access for kernel/UI/tests
    std::vector<Process> snapshot() const;

private:
    int next_pid_{1};
    std::vector<Process> table_;
    memory::MemoryManager mem_;
    scheduler::CPUScheduler cpu_;

    Process*       find(int pid);
};

}