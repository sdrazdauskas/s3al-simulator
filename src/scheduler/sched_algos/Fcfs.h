#pragma once

#include "../ProcessData.h"
#include <iostream>
#include <vector>
#include <algorithm>


namespace scheduler {

class FCFS {
    std::vector<ScheduledTask>& process_queue;
public:
    FCFS(std::vector<ScheduledTask>& p) : process_queue(p) {}

    void run() {
        sort(process_queue.begin(), process_queue.end(),
             [](const ScheduledTask& a, const ScheduledTask& b) { return a.arrivalTime < b.arrivalTime; });

        for (auto& p : process_queue) {
            p.burstTime--;
        }
    }
};

} //namespace scheduler
