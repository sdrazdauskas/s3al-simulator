#pragma once

#include "../Process.h"
#include <iostream>
#include <vector>
#include <algorithm>


namespace scheduler {

class FCFS {
    std::vector<Process>& process_queue;
public:
    FCFS(std::vector<Process>& p) : process_queue(p) {}

    void run() {
        sort(process_queue.begin(), process_queue.end(),
             [](const Process& a, const Process& b) { return a.arrivalTime < b.arrivalTime; });

        for (auto& p : process_queue) {
            p.burstTime--;
        }
    }
};

} //namespace scheduler
