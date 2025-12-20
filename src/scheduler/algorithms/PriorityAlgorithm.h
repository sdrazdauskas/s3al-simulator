#pragma once
#include "scheduler/algorithms/SchedulingAlgorithm.h"

namespace scheduler {

class PriorityAlgorithm : public SchedulingAlgorithm {
public:
    PriorityAlgorithm() = default;
    
    int selectNext(const std::vector<ScheduledTask*>& readyQueue) override;
    
    bool shouldPreempt(const ScheduledTask* current, const std::vector<ScheduledTask*>& readyQueue, int currentSlice) override;
    
    void onCycleExecuted(int pid, int& currentSlice) override;
    
    void onContextSwitch(int& currentSlice) override;
    
    std::string getDebugInfo(int currentSlice, int quantum) const override;
    
    std::string getName() const override { return "Priority"; }
};

} // namespace scheduler
