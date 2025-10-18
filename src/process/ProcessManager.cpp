#include "ProcessManager.h"
#include <algorithm>

namespace process {

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

//-----------Tools used by pm itself-------------------
int ProcessManager::create_process(const std::string& name,
                                   int cpuTimeNeeded,
                                   int memoryNeeded,
                                   int priority) {
    if (name.empty() || cpuTimeNeeded <= 0 || memoryNeeded <= 0) return -1;

    const int pid = next_pid_++;

    Process p{name, pid, cpuTimeNeeded, memoryNeeded, priority, 0}; // 0=new
    p.state = 1; // 1=ready
    table_.push_back(p);

    return pid;
}

bool ProcessManager::run_process(int pid) {
    auto it = std::find_if(table_.begin(), table_.end(),
                           [pid](const Process& p) { return p.pid == pid; });
    if (it == table_.end()) return false;

    it->state = 2; // running

    // //single thread simulation. Thread per process in future?
    // mem_.allocate_memory_for_process(it->pid, it->memoryNeeded);
    // cpu_.execute_process(it->pid, it->cpuTimeNeeded);
    // mem_.deallocate_memory_for_process(it->pid);

    return true;
}

// TODO: more implementations of "stop"- zombie, waiting, finished etc. 
bool ProcessManager::stop_process(int pid) {
    auto it = std::find_if(table_.begin(), table_.end(),
                           [pid](const Process& p) { return p.pid == pid; });
    if (it == table_.end()) return false;

    it->state = 4; // terminated/finished/stopped etc.- no difference for now.

    table_.erase(std::remove_if(table_.begin(), table_.end(),
                   [pid](const Process& pr){ return pr.pid == pid; }),
                 table_.end());
    return true;
}

} // namespace process