#include "scheduler/algorithms/RoundRobinAlgorithm.h"
#include <sstream>
#include <deque>

namespace scheduler {

void RoundRobinAlgorithm::resetSliceIfProcessChanged(int currentPid) {
    if (currentPid != lastPid) {
        sliceCounter = 0;
        lastPid = currentPid;
    }
}

void RoundRobinAlgorithm::incrementSlice() {
    ++sliceCounter;
}

bool RoundRobinAlgorithm::quantumExpired() const {
    return sliceCounter >= quantum;
}

void RoundRobinAlgorithm::resetSlice() {
    sliceCounter = 0;
}

ScheduledTask* RoundRobinAlgorithm::selectNextProcess(ScheduledTask* currentTask, const std::deque<ScheduledTask*>& readyQueue) {
    if (currentTask == nullptr) {
        return readyQueue.front();
    }
    logDebug("Process " + std::to_string(currentTask->id) + ", slice=" + std::to_string(sliceCounter) + "/" + std::to_string(quantum));

    if (quantumExpired()) {
        logDebug("Quantum expired for process " + std::to_string(currentTask->id) + ", picking next process, slice=" + std::to_string(sliceCounter) + "/" + std::to_string(quantum));
        resetSlice();
        if (readyQueue.empty()) return nullptr;

        if (readyQueue.size() == 1) {
            // Only one process: if it's already running, keep running it
            if (currentTask && currentTask == readyQueue.front()) {
                return currentTask;
            } else {
                return readyQueue.front();
            }
        }
        if (currentTask == nullptr) {
            return readyQueue.front();
        }
        auto it = std::find(readyQueue.begin(), readyQueue.end(), currentTask);
        if (it != readyQueue.end()) {
            // Move to the next process in a circular fashion
            auto nextIt = it + 1;
            if (nextIt == readyQueue.end()) {
                nextIt = readyQueue.begin();
            }
            return *nextIt;
        }
        // Unknown current task, just return the first in queue
        return readyQueue.front();
    }
    return currentTask;
}

ScheduledTask* RoundRobinAlgorithm::getNextTask(ScheduledTask* currentTask, const std::deque<ScheduledTask*>& readyQueue) {
    resetSliceIfProcessChanged(currentTask ? currentTask->id : -1);
    incrementSlice();
    ScheduledTask* nextTask = selectNextProcess(currentTask, readyQueue);
    if (nextTask != currentTask) {
        resetSlice();
    }
    return nextTask;
}

} // namespace scheduler
