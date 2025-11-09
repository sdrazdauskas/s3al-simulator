#include "Config.h"
#include <iostream>
#include <string>

namespace config {

bool Config::parseArgs(int argc, char* argv[], Config& config) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--verbose" || arg == "-v") {
            config.verbose = true;
        } 
        else if ((arg == "--memory" || arg == "-m") && i + 1 < argc) {
            // Parse memory size (supports KB, MB, GB suffixes)
            std::string mem_str = argv[++i];
            try {
                size_t multiplier = 1;
                char suffix = std::toupper(mem_str.back());
                
                switch (suffix) {
                    case 'K':
                        multiplier = 1024;
                        mem_str.pop_back();
                        break;
                    case 'M':
                        multiplier = 1024 * 1024;
                        mem_str.pop_back();
                        break;
                    case 'G':
                        multiplier = 1024 * 1024 * 1024;
                        mem_str.pop_back();
                        break;
                    default:
                        // No suffix, treat as bytes
                        break;
                }
                
                config.memory_size = std::stoull(mem_str) * multiplier;
            } catch (...) {
                std::cerr << "Invalid memory size: " << mem_str << std::endl;
                return false;
            }
        } 
        else if (arg == "--help" || arg == "-h") {
            showHelp(argv[0]);
            return false;
        }
        else {
            std::cerr << "Unknown option: " << arg << std::endl;
            showHelp(argv[0]);
            return false;
        }
    }
    
    return true;
}

void Config::showHelp(const char* program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS]\n";
    std::cout << "\n";
    std::cout << "s3al OS Simulator - A simple operating system simulator\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "  -v, --verbose          Enable verbose logging to console\n";
    std::cout << "  -m, --memory SIZE      Set memory size (e.g., 512K, 2M, 1G)\n";
    std::cout << "                         Default: 1M (1048576 bytes)\n";
    std::cout << "  -h, --help             Show this help message\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  " << program_name << " --verbose\n";
    std::cout << "  " << program_name << " --memory 2M\n";
    std::cout << "  " << program_name << " -m 512K -v\n";
}

} // namespace config
