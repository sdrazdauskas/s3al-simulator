#ifndef CPU_SCHEDULER_H
#define CPU_SCHEDULER_H

class CPUScheduler {
public:
    CPUScheduler() = default;

    void execute_process(int pid, int cpu_time_needed_ms);
};

#endif
