#pragma once
#include "scheduler/algorithms/SchedulingAlgorithm.h"

namespace scheduler {

class FCFSAlgorithm : public SchedulingAlgorithm {
public:
    FCFSAlgorithm() = default;
    
    int selectNext(const std::vector<ScheduledTask*>& readyQueue) override;
    
    bool shouldPreempt(const ScheduledTask* current, const std::vector<ScheduledTask*>& readyQueue) override;
    
    std::string getName() const override { return "FCFS"; }
};

} // namespace scheduler
