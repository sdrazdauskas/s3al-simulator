#include "scheduler/algorithms/PriorityAlgorithm.h"
#include <algorithm>

namespace scheduler {

int PriorityAlgorithm::selectNext(const std::vector<ScheduledTask*>& readyQueue) {
    if (readyQueue.empty()) return -1;
    
    // Find highest priority (lowest number = highest priority)
    auto it = std::min_element(readyQueue.begin(), readyQueue.end(),
        [](const ScheduledTask* a, const ScheduledTask* b) {
            return a->priority < b->priority;
        });
    
    return (*it)->id;
}

bool PriorityAlgorithm::shouldPreempt(const ScheduledTask* current, const std::vector<ScheduledTask*>& readyQueue) {
    if (!current || readyQueue.empty()) return false;
    
    // Check if any waiting process has higher priority (lower number)
    for (const auto* p : readyQueue) {
        if (p->priority < current->priority) {
            return true;
        }
    }
    return false;
}

} // namespace scheduler
