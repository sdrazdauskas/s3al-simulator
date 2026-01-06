#include "scheduler/algorithms/FCFSAlgorithm.h"
#include <deque>

namespace scheduler {

ScheduledTask* FCFSAlgorithm::getNextTask(ScheduledTask* currentTask, const std::deque<ScheduledTask*>& readyQueue) {
    // If no process is running, pick the first from the ready queue
    if (currentTask == nullptr) {
        if (!readyQueue.empty()) {
            return readyQueue.front();
        } else {
            return nullptr;
        }
    }
    // FCFS is non-preemptive, continue running current
    return currentTask;
}

} // namespace scheduler
