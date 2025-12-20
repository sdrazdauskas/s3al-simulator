#include "scheduler/Scheduler.h"
#include "scheduler/algorithms/RoundRobinAlgorithm.h"
#include "scheduler/algorithms/PriorityAlgorithm.h"
#include "scheduler/algorithms/FCFSAlgorithm.h"
#include <stdexcept>
#include <algorithm>

namespace scheduler {

void CPUScheduler::setAlgorithm(Algorithm a) { 
    algo = a;
    
    // Create the appropriate algorithm implementation
    switch (a) {
        case Algorithm::RoundRobin:
            algorithm = std::make_unique<RoundRobinAlgorithm>(quantum);
            break;
        case Algorithm::Priority:
            algorithm = std::make_unique<PriorityAlgorithm>();
            break;
        case Algorithm::FCFS:
            algorithm = std::make_unique<FCFSAlgorithm>();
            break;
    }
    
    logInfo("Algorithm set to: " + algorithmToString(a));
}

void CPUScheduler::setQuantum(int cycles) { 
    quantum = (cycles > 0) ? cycles : 1;
    logInfo("Quantum set to: " + std::to_string(quantum) + " cycles");
    
    // If Round Robin is already active, update its quantum
    if (algo == Algorithm::RoundRobin && algorithm) {
        auto* rrAlgo = dynamic_cast<RoundRobinAlgorithm*>(algorithm.get());
        if (rrAlgo) {
            rrAlgo->setQuantum(quantum);
        }
    }
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

std::vector<ScheduledTask*> CPUScheduler::getReadyProcesses() {
    std::vector<ScheduledTask*> ready;
    
    // Iterate through deque
    for (int pid : readyQueue) {
        Process* p = findProcess(pid);
        if (p) {
            ready.push_back(p);
        }
    }
    
    return ready;
}

void CPUScheduler::removeFromReadyQueue(int pid) {
    // Remove the specific pid from the deque
    auto it = std::find(readyQueue.begin(), readyQueue.end(), pid);
    if (it != readyQueue.end()) {
        readyQueue.erase(it);
    }
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
    readyQueue.push_back(pid);
    
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
        readyQueue.push_back(pid);
        logInfo("Resumed process " + std::to_string(pid));
    }
}

void CPUScheduler::preemptCurrent() {
    if (currentPid < 0) return;
    
    Process* p = findProcess(currentPid);
    if (p && p->burstTime > 0) {
        readyQueue.push_back(currentPid);
        logDebug("Preempted process " + std::to_string(currentPid) + 
            " (remaining=" + std::to_string(p->burstTime) + ")");
    }
    
    currentPid = -1;
    if (algorithm) {
        algorithm->onContextSwitch(currentSlice);
    }
}

void CPUScheduler::scheduleProcess(int pid) {
    currentPid = pid;
    if (algorithm) {
        algorithm->onContextSwitch(currentSlice);
    }
    
    Process* process = findProcess(pid);
    if (process) {
        logDebug("Selected process " + std::to_string(pid) + 
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
    if (algorithm) {
        algorithm->onContextSwitch(currentSlice);
    }
}

TickResult CPUScheduler::tick() {
    if (!algorithm) {
        return TickResult(); // No algorithm set
    }
    
    TickResult result;
    
    // Process cyclesPerInterval cycles
    for (int cycle = 0; cycle < cyclesPerInterval; ++cycle) {
        systemTime++;
        
        // If no current process, try to schedule one
        if (currentPid < 0) {
            auto readyProcs = getReadyProcesses();
            int nextPid = algorithm->selectNext(readyProcs);
            if (nextPid >= 0) {
                scheduleProcess(nextPid);
                result.contextSwitch = true;
                // Remove selected process from ready queue (pop front for FIFO)
                if (!readyQueue.empty() && readyQueue.front() == nextPid) {
                    readyQueue.pop_front();
                } else {
                    removeFromReadyQueue(nextPid);
                }
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
        p->burstTime--;
        result.remainingCycles = p->burstTime;
        
        // Let algorithm track cycle execution
        algorithm->onCycleExecuted(currentPid, currentSlice);
        
        // Get debug info from algorithm
        std::string debugInfo = algorithm->getDebugInfo(currentSlice, quantum);
        logDebug("Process " + std::to_string(currentPid) + 
            " executed 1 cycle (remaining=" + std::to_string(p->burstTime) + debugInfo + ")");
        
        if (p->burstTime <= 0) {
            // Process finished
            result.processCompleted = true;
            result.completedPid = currentPid;
            completeProcess(currentPid);
            
        } else {
            // Check if current process should be preempted
            auto readyProcs = getReadyProcesses();
            bool shouldPreempt = algorithm->shouldPreempt(p, readyProcs, currentSlice);
            
            if (shouldPreempt) {
                preemptCurrent();
                int nextPid = algorithm->selectNext(readyProcs);
                if (nextPid >= 0) {
                    scheduleProcess(nextPid);
                    result.contextSwitch = true;
                    result.currentPid = currentPid;
                    // Remove selected process from ready queue (pop front for FIFO)
                    if (!readyQueue.empty() && readyQueue.front() == nextPid) {
                        readyQueue.pop_front();
                    } else {
                        removeFromReadyQueue(nextPid);
                    }
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