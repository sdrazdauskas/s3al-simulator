#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H

#include "../process.h"
#include <vector>
#include <algorithm>
#include <iostream>
using namespace std;

class PriorityQueueSched {
    vector<Process>& process_queue;

public:
    PriorityQueueSched(vector<Process>& p) : process_queue(p) {}

    void run() {
        sort(process_queue.begin(), process_queue.end(),
             [](const Process& a, const Process& b) {
                 if (a.arrivalTime == b.arrivalTime)
                     return a.priority > b.priority; // higher priority first
                 return a.arrivalTime < b.arrivalTime;
             });

        for (auto& p : process_queue) {
            p.burstTime--;
            cout << "[\"CPU\"]" << " Simulating execution on process PiD "<< p.id <<" for 1 tick." <<endl;
        }
    }
};

#endif
