#pragma once
#include "scheduler/algorithms/SchedulingAlgorithm.h"

namespace scheduler {

class PriorityAlgorithm : public SchedulingAlgorithm {
public:
    PriorityAlgorithm() = default;

    int selectNext(const std::vector<ScheduledTask*>& readyQueue) override;

    bool shouldPreempt(const ScheduledTask* current, const std::vector<ScheduledTask*>& readyQueue) override;

    std::string getName() const override { return "Priority"; }
};

} // namespace scheduler
