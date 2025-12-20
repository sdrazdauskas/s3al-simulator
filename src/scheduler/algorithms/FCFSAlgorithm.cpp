#include "scheduler/algorithms/FCFSAlgorithm.h"

namespace scheduler {

int FCFSAlgorithm::selectNext(const std::vector<ScheduledTask*>& readyQueue) {
    // FCFS: select first process in queue
    if (readyQueue.empty()) return -1;
    return readyQueue.front()->id;
}

bool FCFSAlgorithm::shouldPreempt(const ScheduledTask* current, const std::vector<ScheduledTask*>& readyQueue, int currentSlice) {
    // FCFS is non-preemptive
    return false;
}

} // namespace scheduler
