#include "memory/MemoryManager.h"
#include "scheduler/Scheduler.h"
#include "process/ProcessManager.h"
#include <algorithm>
#include <iostream>

namespace process {

ProcessManager::ProcessManager(memory::MemoryManager& mem, scheduler::CPUScheduler& cpu)
    : memManager(mem), cpuScheduler(cpu) 
{
    cpuScheduler.setProcessCompleteCallback([this](int pid) {
        onProcessComplete(pid);
    });
}

void ProcessManager::log(const std::string& level, const std::string& message) {
    if (logCallback) {
        logCallback(level, "PROCESS", message);
    }
}

Process* ProcessManager::find(int pid) {
    auto it = std::find_if(processTable.begin(), processTable.end(),
                           [pid](const Process& p) { return p.getPid() == pid; });
    return (it != processTable.end()) ? &(*it) : nullptr;
}

bool ProcessManager::processExists(int pid) const {
    return std::find_if(processTable.begin(), processTable.end(),
                        [pid](const Process& p) { return p.getPid() == pid; }) != processTable.end();
}

int ProcessManager::submit(const std::string& processName,
                           int cpuCycles,
                           int memoryNeeded,
                           int priority,
                           bool persistent) {
    if (processName.empty() || cpuCycles < 1 || memoryNeeded < 0) {
        log("ERROR", "Invalid process parameters: name=" + processName + 
                     ", cpuCycles=" + std::to_string(cpuCycles) + 
                     ", memoryNeeded=" + std::to_string(memoryNeeded));
        return -1;
    }

    const int pid = next_pid_++;
    
    // Create process metadata
    Process process(processName, pid, cpuCycles, memoryNeeded, priority, 0);
    process.setRemainingCycles(cpuCycles);
    process.setPersistent(persistent);
    
    // Set up process logging
    process.setLogCallback([this](const std::string& level, const std::string& message){
        if (logCallback) {
            logCallback(level, "PROCESS", message);
        }
    });
    
    if (!process.makeReady()) {
        log("ERROR", "Failed to initialize process '" + processName + "'");
        return -1;
    }
    
    // Persistent processes (init, daemons) start immediately in RUNNING state
    if (persistent) {
        process.start();
    }
    
    processTable.push_back(process);
    memManager.allocate(memoryNeeded, pid);
    cpuScheduler.enqueue(pid, cpuCycles, priority);

    log("INFO", "Submitted process '" + processName + "' (PID=" + std::to_string(pid) + 
        ", cycles=" + std::to_string(cpuCycles) + ", priority=" + std::to_string(priority) + ")");
    
    return pid;
}

void ProcessManager::onProcessComplete(int pid) {
    Process* process = find(pid);
    if (!process) return;
    
    // Check if process is persistent (long-running like init/daemons)
    if (process->isPersistent()) {
        log("DEBUG", "Persistent process '" + process->getName() + "' (PID=" + std::to_string(pid) + ") cycle completed, keeping alive");
        return;  // Don't terminate persistent processes
    }
    
    log("INFO", "Process '" + process->getName() + "' (PID=" + std::to_string(pid) + ") completed CPU scheduling");
    
    // Ensure process is in RUNNING state
    if (process->getState() == ProcessState::READY) {
        process->start();
    }
    
    // Keep process in RUNNING state - shell will transition to ZOMBIE after executing command
    
    if (completeCallback) {
        completeCallback(pid, 0);  // Exit code 0 for normal completion
    }
}

bool ProcessManager::reapProcess(int pid) {
    Process* process = find(pid);
    if (!process) {
        log("ERROR", "Cannot reap process: PID " + std::to_string(pid) + " not found");
        return false;
    }
    
    // Only reap zombie processes
    if (process->getState() != ProcessState::ZOMBIE) {
        log("WARN", "Cannot reap process PID " + std::to_string(pid) + ": not in ZOMBIE state");
        return false;
    }
    
    log("INFO", "Reaping zombie process '" + process->getName() + "' (PID=" + std::to_string(pid) + ")");
    
    // Remove from process table
    processTable.erase(std::remove_if(processTable.begin(), processTable.end(),
                                      [pid](const Process& pr){ return pr.getPid() == pid; }),
                      processTable.end());
    
    return true;
}

bool ProcessManager::exit(int pid, int exitCode) {
    Process* process = find(pid);
    if (!process) {
        log("ERROR", "Cannot exit: PID " + std::to_string(pid) + " not found");
        return false;
    }
    
    log("DEBUG", "Process '" + process->getName() + "' exited with code " + std::to_string(exitCode) + " (PID=" + std::to_string(pid) + ")");
    memManager.freeProcessMemory(pid);
    return process->makeZombie();
}

bool ProcessManager::suspendProcess(int pid) {
    Process* process = find(pid);
    if (!process) {
        log("ERROR", "Cannot suspend process: PID " + std::to_string(pid) + " not found");
        return false;
    }
    
    cpuScheduler.suspend(pid);
    return process->suspend();
}

bool ProcessManager::resumeProcess(int pid) {
    Process* process = find(pid);
    if (!process) {
        log("ERROR", "Cannot resume process: PID " + std::to_string(pid) + " not found");
        return false;
    }
    
    cpuScheduler.resume(pid);
    return process->resume();
}

bool ProcessManager::sendSignal(int pid, int signal) {
    Process* process = find(pid);
    if (!process) {
        log("ERROR", "Cannot send signal to PID " + std::to_string(pid) + ": not found");
        return false;
    }
    
    log("INFO", "Sending signal " + std::to_string(signal) + " to process '" + process->getName() + "' (PID=" + std::to_string(pid) + ")");
    
    // Notify listeners
    if (signalCallback) {
        signalCallback(pid, signal);
    }
    
    // Handle common signals
    switch (signal) {
        case 19: // SIGSTOP
            return suspendProcess(pid);
        case 18: // SIGCONT
            return resumeProcess(pid);
        case 9:  // SIGKILL
        case 15: // SIGTERM
            log("INFO", "Terminating process '" + process->getName() + "' (PID=" + std::to_string(pid) + ")");
            cpuScheduler.remove(pid);
            memManager.freeProcessMemory(pid);
            process->terminate();
                        processTable.erase(std::remove_if(processTable.begin(), processTable.end(),
                                                     [pid](const Process& pr){ return pr.getPid() == pid; }),
                                                 processTable.end());
            return true;
        default:
            log("WARN", "Signal " + std::to_string(signal) + " not implemented");
            return true;
    }
}

std::vector<Process> ProcessManager::snapshot() const {
    return processTable;
}

} // namespace process
