#ifndef FCFS_H
#define FCFS_H

#include "../process.h"
#include <iostream>
#include <vector>
#include <algorithm>


namespace scheduler{

class FCFS {
    std::vector<Process>& process_queue;
public:
    FCFS(std::vector<Process>& p) : process_queue(p) {}

    void run() {
        sort(process_queue.begin(), process_queue.end(),
             [](const Process& a, const Process& b) { return a.arrivalTime < b.arrivalTime; });

        for (auto& p : process_queue) {
            p.burstTime--;
            //std::cout << "[\"CPU\"]" << " Simulating execution on process PiD "<< p.id <<" for 1 tick." <<std::endl;
        }
    }
};

}

#endif
