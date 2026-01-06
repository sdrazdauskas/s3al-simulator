#pragma once

namespace scheduler {

enum class SchedulerAlgorithm {
    FCFS,           // First Come First Serve - no preemption
    RoundRobin,     // Time-slice based preemption
    Priority        // Priority-based scheduling with preemption
};

} // namespace scheduler
