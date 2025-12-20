#include "scheduler/algorithms/RoundRobinAlgorithm.h"

namespace scheduler {

int RoundRobinAlgorithm::selectNext(const std::vector<ScheduledTask*>& readyQueue) {
    // Simple FIFO selection for Round Robin
    if (readyQueue.empty()) return -1;
    return readyQueue.front()->id;
}

bool RoundRobinAlgorithm::shouldPreempt(const ScheduledTask* current, const std::vector<ScheduledTask*>& readyQueue, int currentSlice) {
    // Preempt if quantum expired AND there are other processes waiting
    return currentSlice >= quantum && !readyQueue.empty();
}

} // namespace scheduler
