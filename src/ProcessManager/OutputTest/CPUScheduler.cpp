#include "CPUScheduler.h"
#include <iostream>

void CPUScheduler::execute_process(int pid, int cpu_time_needed) {
    std::cout << "[CPUScheduler] Command to execute process "
              << pid << " for " << cpu_time_needed
              << " received.\n";
}
