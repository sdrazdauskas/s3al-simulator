#ifndef FCFS_H
#define FCFS_H

#include "../process.h"
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

class FCFS {
    vector<Process>& process_queue;
public:
    FCFS(vector<Process>& p) : process_queue(p) {}

    void run() {
        sort(process_queue.begin(), process_queue.end(),
             [](const Process& a, const Process& b) { return a.arrivalTime < b.arrivalTime; });

        for (auto& p : process_queue) {
            p.burstTime--;
            cout << "[\"CPU\"]" << " Simulating execution on process PiD "<< p.id <<" for 1 tick." <<endl;
        }
    }
};

#endif
