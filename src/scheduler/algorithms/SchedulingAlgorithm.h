#pragma once
#include <vector>
#include <string>
#include "scheduler/ScheduledTask.h"

namespace scheduler {

struct SchedulingDecision {
    int selectedPid = -1;
    bool shouldPreempt = false;
    std::string reason;
};

class SchedulingAlgorithm {
public:
    virtual ~SchedulingAlgorithm() = default;
    
    // Select next process to run from the ready queue
    virtual int selectNext(const std::vector<ScheduledTask*>& readyQueue) = 0;
    
    // Check if current process should be preempted
    virtual bool shouldPreempt(const ScheduledTask* current, const std::vector<ScheduledTask*>& readyQueue, int currentSlice) = 0;
    
    // Called when a cycle is executed (for slice tracking, etc.)
    virtual void onCycleExecuted(int pid, int& currentSlice) = 0;
    
    // Reset state when context switching
    virtual void onContextSwitch(int& currentSlice) = 0;
    
    // Get additional debug info for logging
    virtual std::string getDebugInfo(int currentSlice, int quantum) const = 0;
    
    // Get algorithm name
    virtual std::string getName() const = 0;
};

} // namespace scheduler
