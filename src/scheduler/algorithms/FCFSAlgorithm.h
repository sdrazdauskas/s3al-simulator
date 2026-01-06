#pragma once
#include "scheduler/algorithms/SchedulingAlgorithm.h"
#include <deque>

namespace scheduler {

class FCFSAlgorithm : public SchedulingAlgorithm {
public:
    FCFSAlgorithm() = default;

    ScheduledTask* getNextTask(ScheduledTask* currentTask, const std::deque<ScheduledTask*>& readyQueue) override;

    std::string getName() const override { return "FCFS"; }
};

} // namespace scheduler
