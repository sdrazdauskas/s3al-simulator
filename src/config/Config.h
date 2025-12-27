#pragma once

#include <string>
#include <cstddef>
#include "logger/Logger.h"
#include "scheduler/Scheduler.h"

namespace config {

struct Config {
    bool verbose = false;
    size_t memorySize = 1024 * 1024;
    logging::LogLevel logLevel = logging::LogLevel::DEBUG;
    
    // Scheduler configuration
    scheduler::Algorithm schedulerAlgorithm = scheduler::Algorithm::FCFS;
    int schedulerQuantum = 5;               // Time quantum for RoundRobin (cycles)
    int cyclesPerTick = 1;                 // CPU cycles per scheduler tick
    int tickIntervalMs = 100;              // Milliseconds between ticks (CPU speed)
    
    // Parse command-line arguments
    // Returns true on success, false if help was shown or error occurred
    static bool parseArgs(int argc, char* argv[], Config& config);
    
    // Show help message
    static void showHelp(const char* programName);
};

} // namespace config
