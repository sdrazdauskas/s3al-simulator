#pragma once

#include <vector>
#include <queue>
#include <functional>
#include <string>
#include "scheduler/ScheduledTask.h"

namespace scheduler {

// Type alias for internal use
using Process = ScheduledTask;

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

class CPUScheduler {
public:
    using LogCallback = std::function<void(const std::string& level, 
                                           const std::string& module, 
                                           const std::string& message)>;

    explicit CPUScheduler();

    void setLogCallback(LogCallback callback);
    void setProcessCompleteCallback(ProcessCompleteCallback cb) { completeCallback = cb; }

    void setAlgorithm(Algorithm a);
    Algorithm getAlgorithm() const { return algo; }
    
    void setQuantum(int cycles);           // For RoundRobin
    int getQuantum() const { return quantum; }
    
    void setCyclesPerInterval(int cycles); // How many cycles per tick interval
    int getCyclesPerInterval() const { return cyclesPerInterval; }
    
    void setTickIntervalMs(int ms);        // Real-time duration between ticks
    int getTickIntervalMs() const { return tickIntervalMs; }

    // Add a process to the ready queue
    void enqueue(int pid, int burstTime, int priority = 0);
    
    // Remove a process (e.g., killed)
    void remove(int pid);
    
    // Suspend a running/ready process
    void suspend(int pid);
    
    // Resume a suspended process
    void resume(int pid);

    // Advance scheduler by one tick (consumes cycles_per_interval cycles)
    TickResult tick();
    
    // Check if scheduler has work
    bool hasWork() const;
    
    // Get scheduler state
    int getCurrentPid() const { return currentPid; }
    int getSystemTime() const { return systemTime; }
    int getReadyCount() const { return static_cast<int>(readyQueue.size()); }
    
    // Get process remaining cycles (-1 if not found)
    int getRemainingCycles(int pid) const;

private:
    // State
    int systemTime{0};
    int currentPid{-1};
    int currentSlice{0};           // Cycles used in current time slice
    
    // Configuration
    Algorithm algo{Algorithm::FCFS};
    int quantum{5};                 // Time quantum for RoundRobin
    int cyclesPerInterval{1};       // Cycles consumed per tick
    int tickIntervalMs{100};        // Real-time tick interval
    
    // Process queues
    std::vector<Process> processes; // All processes (for lookup)
    std::queue<int> readyQueue;     // PIDs of ready processes
    std::vector<int> suspended;     // PIDs of suspended processes
    
    // Callbacks
    LogCallback logCallback;
    ProcessCompleteCallback completeCallback;

    // Internal methods
    void log(const std::string& level, const std::string& message);
    Process* findProcess(int pid);
    const Process* findProcess(int pid) const;
    int selectNextProcess();
    void preemptCurrent();
    void scheduleProcess(int pid);
    void completeProcess(int pid);
    
    // Algorithm-specific preemption checks
    bool shouldPreemptRoundRobin();
    bool shouldPreemptPriority();
};

inline std::string algorithmToString(Algorithm a) {
    switch (a) {
        case Algorithm::FCFS: return "FCFS";
        case Algorithm::RoundRobin: return "RoundRobin";
        case Algorithm::Priority: return "Priority";
        default: return "Unknown";
    }
}

} // namespace scheduler
