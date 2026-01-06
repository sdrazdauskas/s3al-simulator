#pragma once
#include <vector>
#include <string>
#include <deque>
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

    virtual ScheduledTask* getNextTask(ScheduledTask* currentTask, const std::deque<ScheduledTask*>& readyQueue) = 0;

    // Get algorithm name
    virtual std::string getName() const = 0;
};

} // namespace scheduler
