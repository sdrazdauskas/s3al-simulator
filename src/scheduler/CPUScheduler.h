#ifndef CPU_SCHEDULER_H
#define CPU_SCHEDULER_H

#include <vector>
#include "process.h"

namespace scheduler {

enum class Algorithm {
    FCFS,
    //RoundRobin,
    PriorityQueue
};

class CPUScheduler {
public:
    explicit CPUScheduler();

    void setAlgorithm(Algorithm a);
    void setQuantum(int q);

    void execute_process(int pid, int burstTime, int priorty);

    void setProcesses(const std::vector<Process>& plist);

    void clear();

    // Runs the selected algorithm and prints to algorithm's display(). stdout for now.
    void run();

private:
    int sysTime;   // In real computers time is tracked with a hardware component advancing internal time by 1 second. 
                // For now "time" in our system is a variable incrementing by 1 "unit of time" every cycle of...
    Algorithm algo_;
    int quantum_;
    std::vector<Process> processes_;
};
}
#endif // CPU_SCHEDULER_H
