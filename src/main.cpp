#include "kernel/Kernel.h"
#include "config/Config.h"
#include "logger/Logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>

int main(int argc, char* argv[]) {
    // Parse command-line arguments
    config::Config config;
    if (!config::Config::parseArgs(argc, argv, config)) {
        return 1;
    }
    
    // Initialize logging
    std::filesystem::create_directories("logs");
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << "logs/s3al_" << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S") << ".log";

    logging::Logger::getInstance().init(ss.str(), logging::LogLevel::DEBUG);
    logging::Logger::getInstance().setConsoleOutput(config.verbose);

    // Start OS
    LOG_INFO("MAIN", "Starting s3al OS simulator");
    LOG_INFO("MAIN", "Memory size: " + std::to_string(config.memory_size) + " bytes (" + 
             std::to_string(config.memory_size / 1024) + " KB)");

    kernel::Kernel kernel(config);
    kernel.boot();

    LOG_INFO("MAIN", "Shutdown complete");
    return 0;
}
