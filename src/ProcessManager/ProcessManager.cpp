#include "ProcessManager.h"
#include "OutputTest/MemoryManager.h"
#include "OutputTest/CPUScheduler.h"

ProcessManager::ProcessManager(MemoryManager& mem, CPUScheduler& cpu)
    : mem_(mem), cpu_(cpu) {}

//general function describing lifecycle, accessed by kernel
int ProcessManager::execute_process(const std::string& name,
                                    int cpuTimeNeeded,
                                    int memoryNeeded,
                                    int priority) {
    const int pid = create_process(name, cpuTimeNeeded, memoryNeeded, priority);
    if (pid == -1) return -1;

    run_process(pid);
    stop_process(pid);

    return pid;//probably no need to return in future?
}

//called by kernel, unstructured data- let kernel do it.
std::vector<Process> ProcessManager::snapshot() const {
    return table_;
}

