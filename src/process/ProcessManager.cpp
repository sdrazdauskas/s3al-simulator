#include "../memory/MemoryManager.h"
#include "../scheduler/Scheduler.h"
#include "ProcessManager.h"
#include <algorithm>
#include <iostream>

namespace process {

ProcessManager::ProcessManager(memory::MemoryManager& mem, scheduler::CPUScheduler& cpu)
    : mem(mem), cpu(cpu) {}

void ProcessManager::log(const std::string& level, const std::string& message) {
    if (log_callback) {
        log_callback(level, "PROCESS", message);
    }
}

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
    return table;
}

//-----------Tools used by pm itself-------------------
int ProcessManager::create_process(const std::string& name,
                                   int cpuTimeNeeded,
                                   int memoryNeeded,
                                   int priority) {
    if (name.empty() || cpuTimeNeeded <= 0 || memoryNeeded <= 0) {
        log("ERROR", "Invalid process parameters: name=" + name);
        return -1;
    }

    const int pid = next_pid_++;

    Process p{name, pid, cpuTimeNeeded, memoryNeeded, priority, 0}; // 0=new
    p.state = 1; // 1=ready
    table.push_back(p);

    log("INFO", "Created process '" + name + "' (PID=" + std::to_string(pid) + ")");
    return pid;
}

bool ProcessManager::run_process(int pid) {
    auto it = std::find_if(table.begin(), table.end(),
                           [pid](const Process& p) { return p.pid == pid; });
    if (it == table.end()) {
        log("ERROR", "Cannot run process: PID " + std::to_string(pid) + " not found");
        return false;
    }

    it->state = 2; // running
    log("INFO", "Running process '" + it->name + "' (PID=" + std::to_string(pid) + ")");

    // //single thread simulation. Thread per process in future?
    mem.allocate(it->memoryNeeded, it->pid);
    cpu.execute_process(it->pid, it->cpuTimeNeeded, it->priority);
    mem.free_process_memory(it->pid);

    return true;
}

// TODO: more implementations of "stop"- zombie, waiting, finished etc. 
bool ProcessManager::stop_process(int pid) {
    auto it = std::find_if(table.begin(), table.end(),
                           [pid](const Process& p) { return p.pid == pid; });
    if (it == table.end()) {
        log("ERROR", "Cannot stop process: PID " + std::to_string(pid) + " not found");
        return false;
    }

    it->state = 4; // terminated/finished/stopped etc.- no difference for now.
    log("INFO", "Stopped process '" + it->name + "' (PID=" + std::to_string(pid) + ")");

    table.erase(std::remove_if(table.begin(), table.end(),
                   [pid](const Process& pr){ return pr.pid == pid; }),
                 table.end());
    return true;
}

} // namespace process