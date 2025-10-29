#include "Kernel.h"
#include "Logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>

int main() {
    std::filesystem::create_directories("logs"); // <- ensures logs/ exists

    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << "logs/s3al_" << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S") << ".log";

    logging::Logger::getInstance().init(ss.str(), logging::LogLevel::DEBUG);

    LOG_INFO("MAIN", "Starting s3al OS simulator");

    Kernel kernel;
    kernel.boot();

    LOG_INFO("MAIN", "Shutdown complete");
    return 0;
}
