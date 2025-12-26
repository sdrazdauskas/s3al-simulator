#include "scheduler/algorithms/PriorityAlgorithm.h"
#include <algorithm>
#include <climits>
#include <deque>

namespace scheduler {


ScheduledTask* PriorityAlgorithm::getNextTask(ScheduledTask* currentTask, const std::deque<ScheduledTask*>& readyQueue) {
    // If no process is running, pick the highest priority from the ready queue
    if (currentTask == nullptr) {
        if (!readyQueue.empty()) {
            return getHighestPriorityProcess(readyQueue);
        }
        return nullptr;
    }

    // Check if any waiting process has higher priority (lower number)
    ScheduledTask* highestPrioProc = getHighestPriorityProcess(readyQueue);
    if (highestPrioProc && highestPrioProc->priority < currentTask->priority) {
        return highestPrioProc;
    }

    return currentTask;
}

ScheduledTask* PriorityAlgorithm::getHighestPriorityProcess(const std::deque<ScheduledTask*>& readyQueue) {
    if (readyQueue.empty()) return nullptr;

    auto it = std::min_element(readyQueue.begin(), readyQueue.end(),
        [](const ScheduledTask* a, const ScheduledTask* b) {
            return a->priority < b->priority;
        });
    return *it;
}

} // namespace scheduler
