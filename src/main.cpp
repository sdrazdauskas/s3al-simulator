#include "kernel/Kernel.h"
#include "config/Config.h"
#include "logger/Logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <atomic>
#include <csignal>
#include <unistd.h>
#include <chrono>

namespace {
    std::atomic<int> globalSigintCount{0};
    std::atomic<std::chrono::steady_clock::time_point> globalLastSigintTime{std::chrono::steady_clock::now()};
    extern "C" void globalSigintHandler(int sig) {
        using namespace std::chrono;
        auto now = steady_clock::now();
        auto prev = globalLastSigintTime.load();
        if (duration_cast<seconds>(now - prev).count() > 2) {
            globalSigintCount.store(0);
        }
        globalLastSigintTime.store(now);
        int count = globalSigintCount.fetch_add(1) + 1;
        if (count >= 5) {
            write(STDERR_FILENO, "\nForce quitting after multiple Ctrl+C presses.\n", 48);
            std::_Exit(130);
        }
    }
}

int main(int argc, char* argv[]) {
    // Install global SIGINT handler for force quit on repeated Ctrl+C
    std::signal(SIGINT, globalSigintHandler);
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

    logging::Logger::getInstance().init(ss.str(), config.logLevel);
    logging::Logger::getInstance().setConsoleOutput(config.verbose);

    // Start OS
    logging::logInfo("MAIN", "Starting s3al OS simulator");
    logging::logInfo("MAIN", "Memory size: " + std::to_string(config.memorySize) + " bytes (" + 
             std::to_string(config.memorySize / 1024) + " KB)");

    kernel::Kernel kernel(config);
    kernel.boot();

    logging::logInfo("MAIN", "Shutdown complete");
    return 0;
}
