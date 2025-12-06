#pragma once

#include "../ProcessData.h"
#include <vector>
#include <algorithm>
#include <iostream>

namespace scheduler {

class PriorityQueueSched {
    std::vector<ScheduledTask>& process_queue;

public:
    PriorityQueueSched(std::vector<ScheduledTask>& p) : process_queue(p) {}

    void run() {
        std::sort(process_queue.begin(), process_queue.end(),
             [](const ScheduledTask& a, const ScheduledTask& b) {
                 if (a.arrivalTime == b.arrivalTime)
                     return a.priority > b.priority;
                 return a.arrivalTime < b.arrivalTime;
             });

        for (auto& p : process_queue) {
            p.burstTime--;
        }
    }
};
} //namespace scheduler