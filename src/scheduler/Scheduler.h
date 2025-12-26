#pragma once

#include <vector>
#include <deque>
#include <functional>
#include <string>
#include <memory>
#include "scheduler/ScheduledTask.h"
#include "scheduler/algorithms/SchedulingAlgorithm.h"
#include "config/Config.h"
#include "common/LoggingMixin.h"

namespace config { struct Config; }

namespace scheduler {

enum class Algorithm {
    FCFS,           // First Come First Serve - no preemption
    RoundRobin,     // Time-slice based preemption
    Priority        // Priority-based scheduling with preemption
};

// Result of a scheduler tick
struct TickResult {
    bool processCompleted{false};   // A process finished this tick
    int completedPid{-1};           // PID of completed process
    int currentPid{-1};             // Currently running process (-1 if idle)
    int remainingCycles{0};         // Cycles left for current process
    bool contextSwitch{false};      // Did a context switch occur?
    bool idle{true};                // No process running
};

// Callback when a process completes
using ProcessCompleteCallback = std::function<void(int pid)>;

class CPUScheduler : public common::LoggingMixin {
public:

    CPUScheduler();

    explicit CPUScheduler(const config::Config& config);

    void setConfig(const config::Config &config);

    void setProcessCompleteCallback(ProcessCompleteCallback cb) { completeCallback = cb; }

    void setAlgorithm(std::unique_ptr<SchedulingAlgorithm> algorithm);
    Algorithm getAlgorithm() const { return algo; }
    
    // How many cycles per tick interval
    void setCyclesPerInterval(int cycles); 
    int getCyclesPerInterval() const { return cyclesPerInterval; }
    
    // Real-time duration between ticks
    void setTickIntervalMs(int ms); 
    int getTickIntervalMs() const { return tickIntervalMs; }

    // Add a process to the ready queue
    void enqueue(int pid, int burstTime, int priority = 0);
    
    // Add CPU cycles to an existing process
    bool addCycles(int pid, int cycles);
    
    // Remove a process (e.g., killed)
    void remove(int pid);
    
    // Suspend a running/ready process
    void suspend(int pid);
    
    // Resume a suspended process
    void resume(int pid);

    // Advance scheduler by one tick
    TickResult tick();
    
    // Check if scheduler has work
    bool hasWork() const;
    
    // Get scheduler state
    int getCurrentPid() const { return currentTask ? currentTask->id : -1; }
    int getSystemTime() const { return systemTime; }
    int getReadyCount() const { return static_cast<int>(readyQueue.size()); }
    
    // Get process remaining cycles (-1 if not found)
    int getRemainingCycles(int pid) const;

private:
    // State
    int systemTime{0};
    ScheduledTask* currentTask{nullptr};
    
    // Configuration
    Algorithm algo{Algorithm::FCFS};
    int cyclesPerInterval{1};       // Cycles consumed per tick
    int tickIntervalMs{100};        // Real-time tick interval
    
    // Process queues
    std::vector<ScheduledTask*> processes; // All processes (for lookup)
    std::deque<ScheduledTask*> readyQueue;     // Ready processes (pointers)
    std::vector<ScheduledTask*> suspended;     // Suspended processes (pointers)
    
    // Scheduling algorithm (strategy pattern)
    std::unique_ptr<SchedulingAlgorithm> algorithm;
    
    // Callbacks
    ProcessCompleteCallback completeCallback;

protected:
    std::string getModuleName() const override { return "SCHEDULER"; }

private:
    ScheduledTask* findProcess(int pid);
    const ScheduledTask* findProcess(int pid) const;
    std::deque<ScheduledTask*> getReadyProcesses();
    void removeFromReadyQueue(ScheduledTask* task);
    void preemptCurrent();
    void scheduleProcess(int pid);
    void completeProcess(ScheduledTask* task);
};

} // namespace scheduler
