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

bool ProcessManager::process_exists(int pid) const {
    return std::find_if(table.begin(), table.end(),
                       [pid](const Process& p) { return p.pid() == pid; }) != table.end();
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
    if (name.empty() || cpuTimeNeeded < 0 || memoryNeeded < 0) {
        log("ERROR", "Invalid process parameters: name=" + name);
        return -1;
    }

    const int pid = next_pid_++;

    // Attempt memory allocation before registering the process
    void* mem_block = mem.allocate(memoryNeeded, pid);
    if (!mem_block) {
        log("ERROR", "Failed to allocate memory for process '" + name + "'");
        return -1;
    }

    Process p(name, pid, cpuTimeNeeded, memoryNeeded, priority, 0);
    
    // store allocated block info inside process (extend Process to support it)
    p.setMemoryBlock(mem_block);

    // Set up process logging to go through process manager
    p.setLogCallback([this](const std::string& level, const std::string& message){
        if (log_callback) {
            log_callback(level, "PROCESS", message);
        }
    });

    if (!p.makeReady()) {
        log("ERROR", "Failed to initialize process '" + name + "'");
        // rollback memory allocation on failure
        mem.free_process_memory(pid);
        return -1;
    }

    table.push_back(p);

    log("INFO", "Created process '" + name + "' (PID=" + std::to_string(pid) + ")");
    return pid;
}

bool ProcessManager::run_process(int pid) {
    auto it = std::find_if(table.begin(), table.end(),
                           [pid](const Process& p) { return p.pid() == pid; });
    if (it == table.end()) {
        log("ERROR", "Cannot run process: PID " + std::to_string(pid) + " not found");
        return false;
    }
    
    // Try to start the process (it will validate state internally)
    if (!it->start()) {
        return false;
    }

    log("INFO", "Running process '" + it->name() + "' (PID=" + std::to_string(pid) + ")");
    cpu.execute_process(it->pid(), it->cpuTimeNeeded(), it->priority());
    return true;
}

bool ProcessManager::stop_process(int pid) {
    auto it = std::find_if(table.begin(), table.end(),
                           [pid](const Process& p) { return p.pid() == pid; });
    if (it == table.end()) {
        log("ERROR", "Cannot stop process: PID " + std::to_string(pid) + " not found");
        return false;
    }

    if (!it->terminate()) {
        return false;
    }

    // free process memory now that it is terminated
    mem.free_process_memory(pid);
    log("INFO", "Freed memory for PID=" + std::to_string(pid));

    log("INFO", "Stopped process '" + it->name() + "' (PID=" + std::to_string(pid) + ")");

    table.erase(std::remove_if(table.begin(), table.end(),
                   [pid](const Process& pr){ return pr.pid() == pid; }),
                 table.end());
    return true;
}

bool ProcessManager::suspend_process(int pid) {
    auto it = std::find_if(table.begin(), table.end(),
                           [pid](const Process& p) { return p.pid() == pid; });
    if (it == table.end()) {
        log("ERROR", "Cannot suspend process: PID " + std::to_string(pid) + " not found");
        return false;
    }

    return it->suspend();
}

bool ProcessManager::resume_process(int pid) {
    auto it = std::find_if(table.begin(), table.end(),
                           [pid](const Process& p) { return p.pid() == pid; });
    if (it == table.end()) {
        log("ERROR", "Cannot resume process: PID " + std::to_string(pid) + " not found");
        return false;
    }

    return it->resume();
}

bool ProcessManager::send_signal(int pid, int signal) {
    auto it = std::find_if(table.begin(), table.end(),
                           [pid](const Process& p) { return p.pid() == pid; });
    if (it == table.end()) {
        log("ERROR", "Cannot send signal to PID " + std::to_string(pid) + ": not found");
        return false;
    }
    
    log("INFO", "Sending signal " + std::to_string(signal) + " to process '" + it->name() + "' (PID=" + std::to_string(pid) + ")");
    
    // Notify any listeners (like Init managing daemons) before modifying process state
    if (signal_callback) {
        signal_callback(pid, signal);
    }
    
    // Handle common signals - delegate to Process methods
    switch (signal) {
        case 19: // SIGSTOP
            return it->suspend();
        case 18: // SIGCONT
            return it->resume();
        case 9: // SIGKILL
            log("INFO", "SIGKILL: Force terminating process '" + it->name() + "' (PID=" + std::to_string(pid) + ")");
            return stop_process(pid);
        case 15: // SIGTERM
            log("INFO", "SIGTERM: Requesting graceful termination of '" + it->name() + "' (PID=" + std::to_string(pid) + ")");
            return stop_process(pid);
        default:
            log("WARN", "Signal " + std::to_string(signal) + " not implemented, ignoring");
            return true;
    }
}

} // namespace process
