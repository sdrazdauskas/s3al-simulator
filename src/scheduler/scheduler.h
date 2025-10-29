#pragma once

#include <vector>
#include <functional>
#include <string>
#include "process.h"

namespace scheduler {

enum class Algorithm {
    FCFS,
    PriorityQueue
};

class CPUScheduler {
public:
    using LogCallback = std::function<void(const std::string& level, 
                                           const std::string& module, 
                                           const std::string& message)>;

    explicit CPUScheduler();

    void setLogCallback(LogCallback callback);

    void setAlgorithm(Algorithm a);
    void setQuantum(int q);

    void execute_process(int pid, int burstTime, int priorty);

    void setProcesses(const std::vector<Process>& plist);

    void clear();

    void run();

private:
    int sysTime;   // In real computers time is tracked with a hardware component advancing internal time by 1 second. 
                // For now "time" in our system is a variable incrementing by 1 "unit of time" every cycle of...
    Algorithm algo;
    int quantum;
    std::vector<Process> processes_;
    LogCallback log_callback;

    void log(const std::string& level, const std::string& message);
};
} //namespace scheduler
