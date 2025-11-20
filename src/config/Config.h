#pragma once

#include <string>
#include <cstddef>

namespace config {

struct Config {
    bool verbose = false;
    size_t memory_size = 1024 * 1024; // Default: 1MB
    
    // Parse command-line arguments
    // Returns true on success, false if help was shown or error occurred
    static bool parseArgs(int argc, char* argv[], Config& config);
    
    // Show help message
    static void showHelp(const char* program_name);
};

} // namespace config
