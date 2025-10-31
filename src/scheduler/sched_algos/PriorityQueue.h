#pragma once

#include "../Process.h"
#include <vector>
#include <algorithm>
#include <iostream>

namespace scheduler {

class PriorityQueueSched {
    std::vector<Process>& process_queue;

public:
    PriorityQueueSched(std::vector<Process>& p) : process_queue(p) {}

    void run() {
        std::sort(process_queue.begin(), process_queue.end(),
             [](const Process& a, const Process& b) {
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