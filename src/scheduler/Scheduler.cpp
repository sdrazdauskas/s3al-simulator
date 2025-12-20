#include "scheduler/Scheduler.h"
#include <stdexcept>
#include <algorithm>

namespace scheduler {

void CPUScheduler::setAlgorithm(Algorithm a) { 
    algo = a;
    logInfo("Algorithm set to: " + algorithmToString(a));
}

void CPUScheduler::setQuantum(int cycles) { 
    quantum = (cycles > 0) ? cycles : 1;
    logInfo("Quantum set to: " + std::to_string(quantum) + " cycles");
}

void CPUScheduler::setCyclesPerInterval(int cycles) {
    cyclesPerInterval = (cycles > 0) ? cycles : 1;
    logInfo("Cycles per interval set to: " + std::to_string(cyclesPerInterval));
}

void CPUScheduler::setTickIntervalMs(int ms) {
    tickIntervalMs = (ms > 0) ? ms : 1;
    logInfo("Tick interval set to: " + std::to_string(tickIntervalMs) + " ms");
}

Process* CPUScheduler::findProcess(int pid) {
    auto it = std::find_if(processes.begin(), processes.end(),
                           [pid](const Process& p) { return p.id == pid; });
    return (it != processes.end()) ? &(*it) : nullptr;
}

const Process* CPUScheduler::findProcess(int pid) const {
    auto it = std::find_if(processes.begin(), processes.end(),
                           [pid](const Process& p) { return p.id == pid; });
    return (it != processes.end()) ? &(*it) : nullptr;
}

int CPUScheduler::getRemainingCycles(int pid) const {
    const Process* p = findProcess(pid);
    return p ? p->burstTime : -1;
}

void CPUScheduler::enqueue(int pid, int burstTime, int priority) {
    // Check if already exists
    if (findProcess(pid)) {
        logWarn("Process " + std::to_string(pid) + " already in scheduler");
        return;
    }
    
    processes.emplace_back(pid, 0, burstTime, priority);
    readyQueue.push(pid);
    
    logInfo("Enqueued process " + std::to_string(pid) + 
        " (burst=" + std::to_string(burstTime) + 
        ", priority=" + std::to_string(priority) + ")");
}

void CPUScheduler::remove(int pid) {
    // If it's the current process, stop it
    if (currentPid == pid) {
        currentPid = -1;
        currentSlice = 0;
    }
    
    // Remove from processes list
    processes.erase(
        std::remove_if(processes.begin(), processes.end(),
                       [pid](const Process& p) { return p.id == pid; }),
        processes.end());
    
    // Remove from suspended list
    suspended.erase(
        std::remove(suspended.begin(), suspended.end(), pid),
        suspended.end());
    
    // Can't easily remove from queue, but it will be skipped when not found
    logInfo("Removed process " + std::to_string(pid) + " from scheduler queue");
}

void CPUScheduler::suspend(int pid) {
    if (currentPid == pid) {
        // Suspend running process
        suspended.push_back(pid);
        currentPid = -1;
        currentSlice = 0;
        logInfo("Suspended running process " + std::to_string(pid));
    } else {
        // Mark as suspended (will be skipped when dequeued)
        if (std::find(suspended.begin(), suspended.end(), pid) == suspended.end()) {
            suspended.push_back(pid);
            logInfo("Suspended process " + std::to_string(pid));
        }
    }
}

void CPUScheduler::resume(int pid) {
    auto it = std::find(suspended.begin(), suspended.end(), pid);
    if (it != suspended.end()) {
        suspended.erase(it);
        readyQueue.push(pid);
        logInfo("Resumed process " + std::to_string(pid));
    }
}

int CPUScheduler::selectNextProcess() {
    while (!readyQueue.empty()) {
        int pid = readyQueue.front();
        readyQueue.pop();
        
        // Skip if suspended
        if (std::find(suspended.begin(), suspended.end(), pid) != suspended.end()) {
            continue;
        }
        
        // Skip if process no longer exists
        if (!findProcess(pid)) {
            continue;
        }
        
        // For Priority algorithm, we need to find highest priority
        if (algo == Algorithm::Priority) {
            // Put this one back and scan all
            std::vector<int> candidates;
            candidates.push_back(pid);
            
            while (!readyQueue.empty()) {
                int other = readyQueue.front();
                readyQueue.pop();
                if (findProcess(other) && 
                    std::find(suspended.begin(), suspended.end(), other) == suspended.end()) {
                    candidates.push_back(other);
                }
            }
            
            // Find highest priority
            int bestPid = -1;
            int bestPriority = -1;
            for (int cand : candidates) {
                Process* p = findProcess(cand);
                if (p && p->priority > bestPriority) {
                    bestPriority = p->priority;
                    bestPid = cand;
                }
            }
            
            // Put non-selected back
            for (int cand : candidates) {
                if (cand != bestPid) {
                    readyQueue.push(cand);
                }
            }
            
            return bestPid;
        }
        
        return pid;
    }
    
    return -1;
}

void CPUScheduler::preemptCurrent() {
    if (currentPid < 0) return;
    
    Process* p = findProcess(currentPid);
    if (p && p->burstTime > 0) {
        readyQueue.push(currentPid);
        log("DEBUG", "Preempted process " + std::to_string(currentPid) + 
            " (remaining=" + std::to_string(p->burstTime) + ")");
    }
    
    currentPid = -1;
    currentSlice = 0;
}

void CPUScheduler::scheduleProcess(int pid) {
    currentPid = pid;
    currentSlice = 0;
    
    Process* process = findProcess(pid);
    if (process) {
        log("DEBUG", "Selected process " + std::to_string(pid) + 
            " for execution (burst=" + std::to_string(process->burstTime) + ")");
    }
}

void CPUScheduler::completeProcess(int pid) {
    logInfo("Process " + std::to_string(pid) + " completed");
    
    // Notify callback
    if (completeCallback) {
        completeCallback(pid);
    }
    
    // Remove from processes
    processes.erase(
        std::remove_if(processes.begin(), processes.end(),
                       [pid](const Process& p) { return p.id == pid; }),
        processes.end());
    
    currentPid = -1;
    currentSlice = 0;
}

bool CPUScheduler::shouldPreemptRoundRobin() {
    // Time slice expired and there are other processes waiting
    if (currentSlice >= quantum && !readyQueue.empty()) {
        return true;
    }
    return false;
}

bool CPUScheduler::shouldPreemptPriority() {
    // Check if any ready process has higher priority than current
    if (readyQueue.empty()) return false;
    
    Process* current = findProcess(currentPid);
    if (!current) return false;
    
    std::queue<int> temp = readyQueue;
    while (!temp.empty()) {
        int pid = temp.front();
        temp.pop();
        Process* other = findProcess(pid);
        if (other && other->priority > current->priority &&
            std::find(suspended.begin(), suspended.end(), pid) == suspended.end()) {
            return true;
        }
    }
    
    return false;
}

TickResult CPUScheduler::tick() {
    TickResult result;
    
    // Process cyclesPerInterval cycles
    for (int cycle = 0; cycle < cyclesPerInterval; ++cycle) {
        systemTime++;
        
        // If no current process, try to schedule one
        if (currentPid < 0) {
            int nextPid = selectNextProcess();
            if (nextPid >= 0) {
                scheduleProcess(nextPid);
                result.contextSwitch = true;
            }
        }
        
        // If still no process, we're idle
        if (currentPid < 0) {
            continue;
        }
        
        result.idle = false;
        result.currentPid = currentPid;
        
        Process* p = findProcess(currentPid);
        if (!p) {
            currentPid = -1;
            continue;
        }
        
        // Consume one cycle
        currentSlice++;
        p->burstTime--;
        result.remainingCycles = p->burstTime;
        
        log("DEBUG", "Process " + std::to_string(currentPid) + 
            " executed 1 cycle (remaining=" + std::to_string(p->burstTime) + 
            ", slice=" + std::to_string(currentSlice) + "/" + std::to_string(quantum) + ")");
        
        if (p->burstTime <= 0) {
            // Process finished
            result.processCompleted = true;
            result.completedPid = currentPid;
            completeProcess(currentPid);
            
        } else {
            // Check algorithm-specific preemption conditions
            bool shouldPreempt = false;
            
            if (algo == Algorithm::RoundRobin) {
                if (currentSlice >= quantum) {
                    if (!readyQueue.empty()) {
                        // Other processes waiting, preempt
                        shouldPreempt = true;
                    } else {
                        // No other processes, reset slice and continue
                        currentSlice = 0;
                        log("DEBUG", "Process " + std::to_string(currentPid) + 
                            " quantum expired, continuing (no other processes)");
                    }
                }
            } else if (algo == Algorithm::Priority) {
                shouldPreempt = shouldPreemptPriority();
            }
            
            if (shouldPreempt) {
                preemptCurrent();
                int nextPid = selectNextProcess();
                if (nextPid >= 0) {
                    scheduleProcess(nextPid);
                    result.contextSwitch = true;
                    result.currentPid = currentPid;
                    Process* next = findProcess(currentPid);
                    if (next) result.remainingCycles = next->burstTime;
                }
            }
        }
    }
    
    return result;
}

bool CPUScheduler::hasWork() const {
    return currentPid >= 0 || !readyQueue.empty();
}

} // namespace scheduler