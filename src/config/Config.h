#pragma once

#include <string>
#include <cstddef>

namespace config {

// Scheduling algorithm options
enum class SchedulerAlgorithm {
    FCFS,           // First Come First Serve
    RoundRobin,     // Time-slice based
    Priority        // Priority-based with preemption
};

struct Config {
    bool verbose = false;
    size_t memory_size = 1024 * 1024;       // Default: 1MB
    
    // Scheduler configuration
    SchedulerAlgorithm scheduler_algorithm = SchedulerAlgorithm::FCFS;
    int scheduler_quantum = 5;               // Time quantum for RoundRobin (cycles)
    int cycles_per_tick = 1;                 // CPU cycles per scheduler tick
    int tick_interval_ms = 100;              // Milliseconds between ticks (CPU speed)
    
    // Parse command-line arguments
    // Returns true on success, false if help was shown or error occurred
    static bool parseArgs(int argc, char* argv[], Config& config);
    
    // Show help message
    static void showHelp(const char* program_name);
};

} // namespace config
