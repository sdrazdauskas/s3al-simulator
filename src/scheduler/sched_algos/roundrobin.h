// #ifndef ROUNDROBIN_H
// #define ROUNDROBIN_H

// #include "../process.h"
// #include <queue>
// #include <iostream>
// #include <vector>
// using namespace std;

// class RoundRobin {
//     vector<Process> processes;
//     int quantum;

// public:
//     RoundRobin(const vector<Process>& p, int q) : processes(p), quantum(q) {}

//     //CPU time simulation
//     void run() {
//         queue<Process*> ready;
//         int currentTime = 0;
//         int n = processes.size();
//         int completed = 0;

//         // Sort by arrival
//         sort(processes.begin(), processes.end(),
//              [](const Process& a, const Process& b) { return a.arrivalTime < b.arrivalTime; });

//         int i = 0;
//         while (completed < n) {
//             // Add newly arrived processes
//             while (i < n && processes[i].arrivalTime <= currentTime)
//                 ready.push(&processes[i++]);

//             if (ready.empty()) {
//                 currentTime++;
//                 continue;
//             }

//             Process* p = ready.front();
//             ready.pop();

//             int timeSlice = min(quantum, p->remainingTime);
//             cout << "[CPU] simulated giving process PiD " <<p->id<< timeSlice << "Units of CPU time" << endl;
//             p->remainingTime -= timeSlice;
//             currentTime += timeSlice;

//             // Add any processes that arrived during this time
//             while (i < n && processes[i].arrivalTime <= currentTime)
//                 ready.push(&processes[i++]);

//             if (p->remainingTime > 0) {
//                 ready.push(p);
//             } else {
//                 p->turnaroundTime = currentTime - p->arrivalTime;
//                 p->waitingTime = p->turnaroundTime - p->burstTime;
//                 completed++;
//             }
//         }
//     }

// };

// #endif
