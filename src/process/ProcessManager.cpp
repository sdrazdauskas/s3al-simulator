#include "kernel/SysCallsAPI.h"
#include "process/ProcessManager.h"
#include <algorithm>
#include <iostream>

namespace process {

ProcessManager::ProcessManager(sys::SysApi* api)
    : sysApi(api) 
{
}

Process* ProcessManager::find(int pid) {
    auto it = std::find_if(processTable.begin(), processTable.end(),
                           [pid](const Process& p) { return p.getPid() == pid; });
    return (it != processTable.end()) ? &(*it) : nullptr;
}

bool ProcessManager::processExists(int pid) const {
    return const_cast<ProcessManager*>(this)->find(pid) != nullptr;
}

bool ProcessManager::isProcessPersistent(int pid) const {
    const Process* process = const_cast<ProcessManager*>(this)->find(pid);
    return process && process->isPersistent();
}

int ProcessManager::submit(const std::string& processName,
                           int cpuCycles,
                           int memoryNeeded,
                           int priority,
                           bool persistent) {
    if (processName.empty() || cpuCycles < 1 || memoryNeeded < 0) {
        logError("Invalid process parameters: name=" + processName + 
                     ", cpuCycles=" + std::to_string(cpuCycles) + 
                     ", memoryNeeded=" + std::to_string(memoryNeeded));
        return -1;
    }

    const int pid = nextPid++;
    
    // Create process metadata
    Process process(processName, pid, cpuCycles, memoryNeeded, priority, 0);
    process.setRemainingCycles(cpuCycles);
    process.setPersistent(persistent);
    
    if (!process.makeReady()) {
        logError("Failed to initialize process '" + processName + "'");
        return -1;
    }
    
    // Persistent processes (init, daemons) start immediately in RUNNING state
    if (persistent) {
        process.start();
    }
    
    processTable.push_back(process);
    if (sysApi && memoryNeeded > 0) {
        sysApi->allocateMemory(memoryNeeded, pid);
    }
    if (sysApi) {
        sysApi->scheduleProcess(pid, cpuCycles, priority);
    }

    logInfo("Submitted process '" + processName + "' (PID=" + std::to_string(pid) + 
        ", cycles=" + std::to_string(cpuCycles) + ", priority=" + std::to_string(priority) + ")");
    
    return pid;
}

void ProcessManager::onProcessComplete(int pid) {
    Process* process = find(pid);
    if (!process) return;
    
    // Check if process is persistent (long-running like init/daemons)
    if (process->isPersistent()) {
        logDebug("Persistent process '" + process->getName() + "' (PID=" + std::to_string(pid) + ") cycle completed, keeping alive");
        return;  // Don't terminate persistent processes
    }
    
    logInfo("Process '" + process->getName() + "' (PID=" + std::to_string(pid) + ") completed CPU scheduling");
    
    // Ensure process is in RUNNING state
    if (process->getState() == ProcessState::READY) {
        process->start();
    }
    
    // Keep process in RUNNING state - shell should transition to ZOMBIE after executing command
    if (completeCallback) {
        completeCallback(pid, 0);  // Exit code 0 for normal completion
    }
}

bool ProcessManager::reapProcess(int pid) {
    Process* process = find(pid);
    if (!process) {
        logError("Cannot reap process: PID " + std::to_string(pid) + " not found");
        return false;
    }
    
    // Only reap zombie processes
    if (process->getState() != ProcessState::ZOMBIE) {
        logWarn("Cannot reap process PID " + std::to_string(pid) + ": not in ZOMBIE state");
        return false;
    }
    
    logInfo("Reaping zombie process '" + process->getName() + "' (PID=" + std::to_string(pid) + ")");
    
    processTable.erase(std::remove_if(processTable.begin(), processTable.end(),
                                      [pid](const Process& pr){ return pr.getPid() == pid; }),
                      processTable.end());
    
    return true;
}

bool ProcessManager::exit(int pid, int exitCode) {
    Process* process = find(pid);
    if (!process) {
        logError("Cannot exit: PID " + std::to_string(pid) + " not found");
        return false;
    }
    
    logDebug("Process '" + process->getName() + "' exited with code " + std::to_string(exitCode) + " (PID=" + std::to_string(pid) + ")");
    if (sysApi) {
        sysApi->freeProcessMemory(pid);
    }
    return process->makeZombie();
}

bool ProcessManager::suspendProcess(int pid) {
    Process* process = find(pid);
    if (!process) {
        logError("Cannot suspend process: PID " + std::to_string(pid) + " not found");
        return false;
    }
    
    if (sysApi) {
        sysApi->suspendScheduledProcess(pid);
    }
    return process->suspend();
}

bool ProcessManager::resumeProcess(int pid) {
    Process* process = find(pid);
    if (!process) {
        logError("Cannot resume process: PID " + std::to_string(pid) + " not found");
        return false;
    }
    
    if (sysApi) {
        sysApi->resumeScheduledProcess(pid);
    }
    return process->resume();
}

bool ProcessManager::sendSignal(int pid, int signal) {
    Process* process = find(pid);
    if (!process) {
        logError("Cannot send signal to PID " + std::to_string(pid) + ": not found");
        return false;
    }
    
    logInfo("Sending signal " + std::to_string(signal) + " to process '" + process->getName() + "' (PID=" + std::to_string(pid) + ")");
    
    // Protect init process from termination signals - kernel blocks SIGKILL/SIGTERM to init
    if (process->getName() == "init" && (signal == 9 || signal == 15)) {
        logWarn("Cannot send signal " + std::to_string(signal) + " to init process - kernel protection");
        return false;
    }
    
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
            logInfo("Terminating process '" + process->getName() + "' (PID=" + std::to_string(pid) + ")");
            if (sysApi) {
                sysApi->unscheduleProcess(pid);
                sysApi->freeProcessMemory(pid);
            }
            if (!process->makeZombie()) {
                logError("Failed to make process zombie: PID=" + std::to_string(pid));
                return false;
            }
            // Notify completion callback on termination as well (exit code = signal)
            if (completeCallback) {
                completeCallback(pid, signal);
            }
            return true;
        default:
            logWarn("Signal " + std::to_string(signal) + " not implemented");
            return true;
    }
}

std::vector<Process> ProcessManager::snapshot() const {
    return processTable;
}

} // namespace process
