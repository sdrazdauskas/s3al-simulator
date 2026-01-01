#include "scheduler/Scheduler.h"
#include "scheduler/algorithms/RoundRobinAlgorithm.h"
#include "scheduler/algorithms/PriorityAlgorithm.h"
#include "scheduler/algorithms/FCFSAlgorithm.h"
#include <stdexcept>
#include <algorithm>
#include "Scheduler.h"
#include <iostream>

namespace scheduler {

CPUScheduler::CPUScheduler() {
    setAlgorithm(std::make_unique<FCFSAlgorithm>());
    currentTask = nullptr;
}

CPUScheduler::CPUScheduler(const config::Config& config) 
    : CPUScheduler() {
    setConfig(config);

    std::cout << "Scheduler initialized with: " 
            << algorithm->getName()
            << ", cycles/tick=" << getCyclesPerInterval()
            << ", tick=" << getTickIntervalMs() << "ms)"
            << std::endl;
}

void CPUScheduler::setConfig(const config::Config& config) {
    std::unique_ptr<SchedulingAlgorithm> algoPtr;

    setAlgorithm(config.schedulerAlgorithm, config.schedulerQuantum);
    setCyclesPerInterval(config.cyclesPerTick);
    setTickIntervalMs(config.tickIntervalMs);
}

bool CPUScheduler::setAlgorithm(scheduler::SchedulerAlgorithm algo, int quantum){
    std::unique_ptr<SchedulingAlgorithm> algoPtr;

    switch (algo) {
        case scheduler::SchedulerAlgorithm::FCFS:
            algoPtr = std::make_unique<FCFSAlgorithm>();
            break;
        case scheduler::SchedulerAlgorithm::RoundRobin:
            algoPtr = std::make_unique<RoundRobinAlgorithm>(quantum);
            break;
        case scheduler::SchedulerAlgorithm::Priority:
            algoPtr = std::make_unique<PriorityAlgorithm>();
            break;
    }
    return setAlgorithm(std::move(algoPtr));
}

bool CPUScheduler::setAlgorithm(std::unique_ptr<SchedulingAlgorithm> algorithm) {
    if (algorithm) {
        this->algorithm = std::move(algorithm);
        logInfo("Algorithm set to: " + this->algorithm->getName());
        return true;
    } else {
        logWarn("Algorithm pointer is null; no changes made.");
    }
    return false;
}

void CPUScheduler::setCyclesPerInterval(int cycles) {
    cyclesPerTick = (cycles > 0) ? cycles : 1;
    logInfo("Cycles per interval set to: " + std::to_string(cyclesPerTick));
}

void CPUScheduler::setTickIntervalMs(int ms) {
    tickIntervalMs = (ms > 0) ? ms : 1;
    logInfo("Tick interval set to: " + std::to_string(tickIntervalMs) + " ms");
}


ScheduledTask* CPUScheduler::findProcess(int pid) {
    auto it = std::find_if(processes.begin(), processes.end(),
                           [pid](ScheduledTask* p) { return p->id == pid; });
    return (it != processes.end()) ? *it : nullptr;
}

const ScheduledTask* CPUScheduler::findProcess(int pid) const {
    return const_cast<CPUScheduler*>(this)->findProcess(pid);
}

std::deque<ScheduledTask*> CPUScheduler::getReadyProcesses() {
    return readyQueue;
}

void CPUScheduler::removeFromReadyQueue(ScheduledTask* task) {
    if (!task) return;
    auto it = std::find(readyQueue.begin(), readyQueue.end(), task);
    if (it != readyQueue.end()) {
        readyQueue.erase(it);
    }
}

int CPUScheduler::getRemainingCycles(int pid) const {
    const ScheduledTask* p = findProcess(pid);
    return p ? p->burstTime : -1;
}

void CPUScheduler::enqueue(int pid, int burstTime, int priority) {
    if (findProcess(pid)) {
        logWarn("ScheduledTask " + std::to_string(pid) + " already in scheduler");
        return;
    }

    ScheduledTask* task = new ScheduledTask(pid, 0, burstTime, priority);
    processes.push_back(task);
    readyQueue.push_back(task);
    logInfo("Enqueued ScheduledTask " + std::to_string(pid) + 
        " (burst=" + std::to_string(burstTime) + 
        ", priority=" + std::to_string(priority) + ")");
}

bool CPUScheduler::addCycles(int pid, int cycles) {
    ScheduledTask* p = findProcess(pid);
    if (p) {
        // Process exists in scheduler - just add cycles
        p->burstTime += cycles;
        // Re-enqueue if not currently running or in ready queue
        if (currentTask && currentTask->id != pid) {
            auto it = std::find(readyQueue.begin(), readyQueue.end(), p);
            if (it == readyQueue.end()) {
                readyQueue.push_back(p);
            }
        }
        logInfo("Added " + std::to_string(cycles) + " cycles to ScheduledTask " + std::to_string(pid) + 
            " (total=" + std::to_string(p->burstTime) + ", priority=" + std::to_string(p->priority) + ")");
        return true;
    }
    // Process not in scheduler - caller should enqueue it with proper priority
    return false;
}

void CPUScheduler::remove(int pid) {
    if (currentTask && currentTask->id == pid) {
        currentTask = nullptr;
    }
    
    ScheduledTask* task = findProcess(pid);
    if (!task) return;
    processes.erase(
        std::remove(processes.begin(), processes.end(), task),
        processes.end());
    readyQueue.erase(
        std::remove(readyQueue.begin(), readyQueue.end(), task),
        readyQueue.end());
    suspended.erase(
        std::remove(suspended.begin(), suspended.end(), task),
        suspended.end());
    delete task;
    logInfo("Removed ScheduledTask " + std::to_string(pid) + " from scheduler queue");
}

void CPUScheduler::suspend(int pid) {
    ScheduledTask* task = findProcess(pid);
    if (!task) return;
    if (currentTask && currentTask->id == pid) {
        // Suspend running process
        suspended.push_back(task);
        currentTask = nullptr;
        logInfo("Suspended running ScheduledTask " + std::to_string(pid));
    } else {
        // Mark as suspended (will be skipped when dequeued)
        if (std::find(suspended.begin(), suspended.end(), task) == suspended.end()) {
            suspended.push_back(task);
            logInfo("Suspended ScheduledTask " + std::to_string(pid));
        }
    }
}

void CPUScheduler::resume(int pid) {
    ScheduledTask* task = findProcess(pid);
    if (!task) return;
    auto it = std::find(suspended.begin(), suspended.end(), task);
    if (it != suspended.end()) {
        suspended.erase(it);
        readyQueue.push_back(task);
        logInfo("Resumed ScheduledTask " + std::to_string(pid));
    }
}

void CPUScheduler::preemptCurrent() {
    if (!currentTask) return;

    if (currentTask && currentTask->burstTime > 0) {
        readyQueue.push_back(currentTask);
        logDebug("Preempted ScheduledTask " + std::to_string(currentTask->id) + 
            " (remaining=" + std::to_string(currentTask->burstTime) + ")");
    }
    currentTask = nullptr;
}

void CPUScheduler::completeProcess(ScheduledTask* task) {
    if (!task) return;
    logInfo("ScheduledTask " + std::to_string(task->id) + " completed");
    if (completeCallback) {
        completeCallback(task->id);
    }
    processes.erase(std::remove(processes.begin(), processes.end(), task), processes.end());
    readyQueue.erase(std::remove(readyQueue.begin(), readyQueue.end(), task), readyQueue.end());
    suspended.erase(std::remove(suspended.begin(), suspended.end(), task), suspended.end());
    currentTask = nullptr;
    delete task;
}

TickResult CPUScheduler::tick() {
    TickResult result;
    if (!algorithm) return result;
    for (int cycle = 0; cycle < cyclesPerTick; ++cycle) {
        systemTime++;
        auto readyProcs = getReadyProcesses();
        logDebug("Tick " + std::to_string(systemTime) + ", Cycle " + std::to_string(cycle + 1) + "/" + std::to_string(cyclesPerTick) +
            ", Current PID: " + std::to_string(currentTask ? currentTask->id : -1) +
            ", Ready Queue Size: " + std::to_string(readyProcs.size()));

        ScheduledTask* nextTask = currentTask;
        if (!readyProcs.empty()) {
            nextTask = algorithm->getNextTask(currentTask, readyProcs);
            logDebug("Algorithm selected ScheduledTask " + 
                std::to_string(nextTask ? nextTask->id : -1));
            if (!currentTask) currentTask = nextTask; // Can assign safely if we were idle
        } else {
            nextTask = nullptr;
        }

        // Preempt if task changed
        if (nextTask && currentTask && nextTask != currentTask) {
            logDebug("Context switch: ScheduledTask " + 
                std::to_string(currentTask->id) + " -> " + 
                std::to_string(nextTask->id));
            if (currentTask) preemptCurrent();
            currentTask = nextTask;
            if (currentTask) {
                auto it = std::find(readyQueue.begin(), readyQueue.end(), currentTask);
                if (it != readyQueue.end()) readyQueue.erase(it);
            }
            result.contextSwitch = true;
            result.currentPid = currentTask ? currentTask->id : -1;
        }

        if (!currentTask) {
            result.idle = true;
            continue;
        }

        currentTask->burstTime--;
        result.remainingCycles = currentTask->burstTime;
        result.currentPid = currentTask->id;
        result.idle = false;
        logDebug("Executing ScheduledTask " + 
            std::to_string(currentTask->id) + 
            " (remaining=" + std::to_string(currentTask->burstTime) + ")");

        // Check for process completion
        if (currentTask->burstTime <= 0) {
            logDebug("ScheduledTask " + std::to_string(currentTask->id) + " has completed execution");
            result.processCompleted = true;
            result.completedPid = currentTask->id;
            completeProcess(currentTask);
            currentTask = nullptr;
        }
    }
    return result;
}

bool CPUScheduler::hasWork() const {
    return currentTask != nullptr || !readyQueue.empty();
}

} // namespace scheduler