#include "config/Config.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <map>

namespace config {

bool Config::parseArgs(int argc, char* argv[], Config& config) {
    // Map log level strings to enum values
    static const std::map<std::string, logging::LogLevel> logLevelMap = {
        {"debug", logging::LogLevel::DEBUG},
        {"info", logging::LogLevel::INFO},
        {"warning", logging::LogLevel::WARNING},
        {"warn", logging::LogLevel::WARNING},
        {"error", logging::LogLevel::ERROR}
    };
    
    // Map scheduler algorithm strings to enum values
    static const std::map<std::string, scheduler::Algorithm> schedulerMap = {
        {"fcfs", scheduler::Algorithm::FCFS},
        {"rr", scheduler::Algorithm::RoundRobin},
        {"roundrobin", scheduler::Algorithm::RoundRobin},
        {"priority", scheduler::Algorithm::Priority},
        {"prio", scheduler::Algorithm::Priority}
    };
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--verbose" || arg == "-v") {
            config.verbose = true;
        } 
        else if ((arg == "--log-level" || arg == "-l") && i + 1 < argc) {
            std::string level = argv[++i];
            std::transform(level.begin(), level.end(), level.begin(), ::tolower);
            
            auto it = logLevelMap.find(level);
            if (it != logLevelMap.end()) {
                config.logLevel = it->second;
            } else {
                std::cerr << "Unknown log level: " << level << std::endl;
                std::cerr << "Valid options: debug, info, warning (warn), error" << std::endl;
                return false;
            }
        }
        else if ((arg == "--memory" || arg == "-m") && i + 1 < argc) {
            // Parse memory size (supports K/KB, M/MB, G/GB suffixes)
            std::string memStr = argv[++i];
            try {
                size_t multiplier = 1;
                
                // Remove trailing 'B' if present (KB->K, MB->M, GB->G)
                if (!memStr.empty() && std::toupper(memStr.back()) == 'B') {
                    memStr.pop_back();
                }
                
                // Check for single letter suffix (K, M, G)
                if (!memStr.empty()) {
                    char suffix = std::toupper(memStr.back());
                    
                    switch (suffix) {
                        case 'K':
                            multiplier = 1024;
                            memStr.pop_back();
                            break;
                        case 'M':
                            multiplier = 1024 * 1024;
                            memStr.pop_back();
                            break;
                        case 'G':
                            multiplier = 1024 * 1024 * 1024;
                            memStr.pop_back();
                            break;
                        default:
                            // No suffix, treat as bytes
                            break;
                    }
                }
                
                // Parse the numeric part
                size_t pos = 0;
                config.memorySize = std::stoull(memStr, &pos) * multiplier;
                
                // Check if entire string was consumed (no invalid characters)
                if (pos != memStr.length()) {
                    throw std::invalid_argument("contains non-numeric characters");
                }
            } catch (const std::exception& e) {
                std::cerr << "Invalid memory size: " << argv[i] << std::endl;
                showHelp(argv[0]);
                return false;
            }
        } 
        else if (arg == "--help" || arg == "-h") {
            showHelp(argv[0]);
            return false;
        }
        // Scheduler algorithm
        else if ((arg == "--scheduler" || arg == "-s") && i + 1 < argc) {
            std::string algo = argv[++i];
            std::transform(algo.begin(), algo.end(), algo.begin(), ::tolower);
            
            auto it = schedulerMap.find(algo);
            if (it != schedulerMap.end()) {
                config.schedulerAlgorithm = it->second;
            } else {
                std::cerr << "Unknown scheduler algorithm: " << algo << std::endl;
                std::cerr << "Valid options: fcfs, rr (roundrobin), priority" << std::endl;
                return false;
            }
        }
        // Time quantum for RoundRobin
        else if ((arg == "--quantum" || arg == "-q") && i + 1 < argc) {
            try {
                config.schedulerQuantum = std::stoi(argv[++i]);
                if (config.schedulerQuantum < 1) {
                    throw std::invalid_argument("must be positive");
                }
            } catch (const std::exception& e) {
                std::cerr << "Invalid quantum value: " << argv[i] << std::endl;
                return false;
            }
        }
        // Cycles per tick (CPU speed)
        else if ((arg == "--cycles" || arg == "-c") && i + 1 < argc) {
            try {
                config.cyclesPerTick = std::stoi(argv[++i]);
                if (config.cyclesPerTick < 1) {
                    throw std::invalid_argument("must be positive");
                }
            } catch (const std::exception& e) {
                std::cerr << "Invalid cycles value: " << argv[i] << std::endl;
                return false;
            }
        }
        // Tick interval in ms
        else if ((arg == "--tick-ms" || arg == "-t") && i + 1 < argc) {
            try {
                config.tickIntervalMs = std::stoi(argv[++i]);
                if (config.tickIntervalMs < 1) {
                    throw std::invalid_argument("must be positive");
                }
            } catch (const std::exception& e) {
                std::cerr << "Invalid tick interval: " << argv[i] << std::endl;
                return false;
            }
        }
        else {
            std::cerr << "Unknown option: " << arg << std::endl;
            showHelp(argv[0]);
            return false;
        }
    }
    
    return true;
}

void Config::showHelp(const char* programName) {
    std::cout << "Usage: " << programName << " [OPTIONS]\n";
    std::cout << "\n";
    std::cout << "s3al OS Simulator - A simple operating system simulator\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "  -v, --verbose          Enable verbose logging to console\n";
    std::cout << "  -l, --log-level LEVEL  Set minimum log level: debug, info, warning, error\n";
    std::cout << "                         Default: debug\n";
    std::cout << "  -m, --memory SIZE      Set memory size (e.g., 512K, 512KB, 2M, 2MB, 1G, 1GB)\n";
    std::cout << "                         Default: 1M (1048576 bytes)\n";
    std::cout << "  -h, --help             Show this help message\n";
    std::cout << "\n";
    std::cout << "Scheduler Options:\n";
    std::cout << "  -s, --scheduler ALGO   Scheduling algorithm: fcfs, rr (roundrobin), priority\n";
    std::cout << "                         Default: fcfs\n";
    std::cout << "  -q, --quantum N        Time quantum for RoundRobin (in cycles)\n";
    std::cout << "                         Default: 5\n";
    std::cout << "  -c, --cycles N         CPU cycles per scheduler tick\n";
    std::cout << "                         Default: 1 (slower CPU = lower value)\n";
    std::cout << "  -t, --tick-ms N        Milliseconds between scheduler ticks\n";
    std::cout << "                         Default: 100 (10 ticks per second)\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  " << programName << " --verbose\n";
    std::cout << "  " << programName << " --memory 2M\n";
    std::cout << "  " << programName << " -m 512KB -v\n";
    std::cout << "  " << programName << " --log-level info\n";
    std::cout << "  " << programName << " --scheduler rr --quantum 3\n";
    std::cout << "  " << programName << " -s priority -c 2 -t 50\n";
}

} // namespace config
