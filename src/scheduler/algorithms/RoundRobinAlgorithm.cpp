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

void RoundRobinAlgorithm::onCycleExecuted(int pid, int& currentSlice) {
    if (currentSlice < quantum) {
        currentSlice++;
    } else {
        currentSlice = 1;
    }
}

void RoundRobinAlgorithm::onContextSwitch(int& currentSlice) {
    currentSlice = 0;
}

std::string RoundRobinAlgorithm::getDebugInfo(int currentSlice, int quantum) const {
    return ", slice=" + std::to_string(currentSlice) + "/" + std::to_string(quantum);
}

} // namespace scheduler
