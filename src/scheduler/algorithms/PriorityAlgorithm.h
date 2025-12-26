#pragma once
#include "scheduler/algorithms/SchedulingAlgorithm.h"
#include <deque>

namespace scheduler {

class PriorityAlgorithm : public SchedulingAlgorithm {
public:
    PriorityAlgorithm() = default;

    ScheduledTask* getNextTask(ScheduledTask* currentTask, const std::deque<ScheduledTask*>& readyQueue) override;

    ScheduledTask* getHighestPriorityProcess(const std::deque<ScheduledTask*>& readyQueue);

    std::string getName() const override { return "Priority"; }
};

} // namespace scheduler
