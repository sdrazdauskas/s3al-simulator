#include "scheduler/algorithms/FCFSAlgorithm.h"
#include <deque>

namespace scheduler {

ScheduledTask* FCFSAlgorithm::getNextTask(ScheduledTask* currentTask, const std::deque<ScheduledTask*>& readyQueue) {
    SchedulingDecision decision;
    // If no process is running, pick the first from the ready queue
    if (currentTask == nullptr) {
        if (!readyQueue.empty()) {
            return readyQueue.front();
        }
        return nullptr;
    }
    // FCFS is non-preemptive, continue running current
    return currentTask;
}

} // namespace scheduler
