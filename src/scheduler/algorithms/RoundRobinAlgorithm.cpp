#include "scheduler/algorithms/RoundRobinAlgorithm.h"

namespace scheduler {

std::string RoundRobinAlgorithm::getDebugInfo(int pid) const {
    if (pid == lastPid) {
        return ", slice=" + std::to_string(sliceCounter) + "/" + std::to_string(quantum);
    }
    return "";
}

int RoundRobinAlgorithm::selectNext(const std::vector<ScheduledTask*>& readyQueue) {
    // Simple FIFO selection for Round Robin
    if (readyQueue.empty()) return -1;
    return readyQueue.front()->id;
}

void RoundRobinAlgorithm::onSchedule(int pid) {
    if (pid != lastPid) {
        sliceCounter = 0;
        lastPid = pid;
    }
}

void RoundRobinAlgorithm::onTick(int pid) {
    if (pid == lastPid) {
        sliceCounter++;
    } else {
        sliceCounter = 1;
        lastPid = pid;
    }
}

bool RoundRobinAlgorithm::shouldPreempt(const ScheduledTask* current, const std::vector<ScheduledTask*>& readyQueue) {
    // Preempt if quantum expired AND there are other processes waiting
    return sliceCounter >= quantum && !readyQueue.empty();
}

} // namespace scheduler
