#include "../memory/MemoryManager.h"
#include "../scheduler/Scheduler.h"
#include "ProcessManager.h"
#include <algorithm>
#include <iostream>

namespace process {

ProcessManager::ProcessManager(memory::MemoryManager& mem, scheduler::CPUScheduler& cpu)
    : mem_(mem), cpu_(cpu) 
{
    // Register callback for when scheduler completes a process
    cpu_.setProcessCompleteCallback([this](int pid) {
        onProcessComplete(pid);
    });
}

void ProcessManager::log(const std::string& level, const std::string& message) {
    if (log_callback_) {
        log_callback_(level, "PROCESS", message);
    }
}

// ============= Process Lookup =============

Process* ProcessManager::find(int pid) {
    auto it = std::find_if(table_.begin(), table_.end(),
                           [pid](const Process& p) { return p.pid() == pid; });
    return (it != table_.end()) ? &(*it) : nullptr;
}

const Process* ProcessManager::find(int pid) const {
    auto it = std::find_if(table_.begin(), table_.end(),
                           [pid](const Process& p) { return p.pid() == pid; });
    return (it != table_.end()) ? &(*it) : nullptr;
}

bool ProcessManager::process_exists(int pid) const {
    return find(pid) != nullptr;
}

// ============= Process Submission =============

int ProcessManager::submit(const std::string& name,
                           int cpuCycles,
                           int memoryNeeded,
                           int priority) {
    if (name.empty() || cpuCycles < 1 || memoryNeeded < 0) {
        log("ERROR", "Invalid process parameters: name=" + name);
        return -1;
    }

    const int pid = next_pid_++;
    
    // Create process metadata
    Process p(name, pid, cpuCycles, memoryNeeded, priority, 0);
    p.setRemainingCycles(cpuCycles);
    
    // Set up process logging
    p.setLogCallback([this](const std::string& level, const std::string& message){
        if (log_callback_) {
            log_callback_(level, "PROCESS", message);
        }
    });
    
    if (!p.makeReady()) {
        log("ERROR", "Failed to initialize process '" + name + "'");
        return -1;
    }
    
    table_.push_back(p);
    
    // Allocate memory
    mem_.allocate(memoryNeeded, pid);
    
    // Enqueue in scheduler
    cpu_.enqueue(pid, cpuCycles, priority);

    log("INFO", "Submitted process '" + name + "' (PID=" + std::to_string(pid) + 
        ", cycles=" + std::to_string(cpuCycles) + ", priority=" + std::to_string(priority) + ")");
    
    return pid;
}

// ============= Process Completion Callback =============

void ProcessManager::onProcessComplete(int pid) {
    Process* p = find(pid);
    if (!p) return;
    
    log("INFO", "Process '" + p->name() + "' (PID=" + std::to_string(pid) + ") completed");
    
    // Free memory
    mem_.free_process_memory(pid);
    
    // Notify user callback
    if (complete_callback_) {
        complete_callback_(pid, 0);  // Exit code 0 for normal completion
    }
    
    // Mark terminated and remove from table
    p->terminate();
    table_.erase(std::remove_if(table_.begin(), table_.end(),
                               [pid](const Process& pr) { return pr.pid() == pid; }),
                table_.end());
}

// ============= Process Control =============

bool ProcessManager::suspend_process(int pid) {
    Process* p = find(pid);
    if (!p) {
        log("ERROR", "Cannot suspend process: PID " + std::to_string(pid) + " not found");
        return false;
    }
    
    cpu_.suspend(pid);
    return p->suspend();
}

bool ProcessManager::resume_process(int pid) {
    Process* p = find(pid);
    if (!p) {
        log("ERROR", "Cannot resume process: PID " + std::to_string(pid) + " not found");
        return false;
    }
    
    cpu_.resume(pid);
    return p->resume();
}

bool ProcessManager::send_signal(int pid, int signal) {
    Process* p = find(pid);
    if (!p) {
        log("ERROR", "Cannot send signal to PID " + std::to_string(pid) + ": not found");
        return false;
    }
    
    log("INFO", "Sending signal " + std::to_string(signal) + " to process '" + p->name() + "' (PID=" + std::to_string(pid) + ")");
    
    // Notify listeners
    if (signal_callback_) {
        signal_callback_(pid, signal);
    }
    
    // Handle common signals
    switch (signal) {
        case 19: // SIGSTOP
            return suspend_process(pid);
        case 18: // SIGCONT
            return resume_process(pid);
        case 9:  // SIGKILL
        case 15: // SIGTERM
            log("INFO", "Terminating process '" + p->name() + "' (PID=" + std::to_string(pid) + ")");
            return stop_process(pid);
        default:
            log("WARN", "Signal " + std::to_string(signal) + " not implemented");
            return true;
    }
}

std::vector<Process> ProcessManager::snapshot() const {
    return table_;
}

// ============= Legacy API =============

int ProcessManager::execute_process(const std::string& name,
                                    int cpuTimeNeeded,
                                    int memoryNeeded,
                                    int priority) {
    const int pid = create_process(name, cpuTimeNeeded, memoryNeeded, priority);
    if (pid == -1) return -1;

    run_process(pid);
    stop_process(pid);

    return pid;
}

int ProcessManager::create_process(const std::string& name,
                                   int cpuTimeNeeded,
                                   int memoryNeeded,
                                   int priority) {
    if (name.empty() || cpuTimeNeeded < 0 || memoryNeeded < 0) {
        log("ERROR", "Invalid process parameters: name=" + name);
        return -1;
    }

    const int pid = next_pid_++;

    Process p(name, pid, cpuTimeNeeded, memoryNeeded, priority, 0);
    
    p.setLogCallback([this](const std::string& level, const std::string& message){
        if (log_callback_) {
            log_callback_(level, "PROCESS", message);
        }
    });
    
    if (!p.makeReady()) {
        log("ERROR", "Failed to initialize process '" + name + "'");
        return -1;
    }
    
    table_.push_back(p);

    log("INFO", "Created process '" + name + "' (PID=" + std::to_string(pid) + ")");
    return pid;
}

bool ProcessManager::run_process(int pid) {
    Process* p = find(pid);
    if (!p) {
        log("ERROR", "Cannot run process: PID " + std::to_string(pid) + " not found");
        return false;
    }
    
    if (!p->start()) {
        return false;
    }

    log("INFO", "Running process '" + p->name() + "' (PID=" + std::to_string(pid) + ")");

    mem_.allocate(p->memoryNeeded(), p->pid());
    cpu_.execute_process(p->pid(), p->cpuTimeNeeded(), p->priority());
    mem_.free_process_memory(p->pid());

    return true;
}

bool ProcessManager::stop_process(int pid) {
    Process* p = find(pid);
    if (!p) {
        log("ERROR", "Cannot stop process: PID " + std::to_string(pid) + " not found");
        return false;
    }

    // Remove from scheduler
    cpu_.remove(pid);
    
    // Free memory
    mem_.free_process_memory(pid);

    if (!p->terminate()) {
        return false;
    }

    log("INFO", "Stopped process '" + p->name() + "' (PID=" + std::to_string(pid) + ")");

    table_.erase(std::remove_if(table_.begin(), table_.end(),
                   [pid](const Process& pr){ return pr.pid() == pid; }),
                 table_.end());
    return true;
}

} // namespace process