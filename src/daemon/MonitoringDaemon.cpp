#include "MonitoringDaemon.h"
#include "../kernel/SysCallsAPI.h"
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace daemons {

MonitoringDaemon::MonitoringDaemon(shell::SysApi& sys)
    : Daemon(sys, "SYSMON") {}

void MonitoringDaemon::run() {
    log("INFO", "System monitoring daemon started (PID " + std::to_string(m_pid) + ")");
    
    while (m_running.load()) {
        collect_stats();
        
        // Sleep for 10 seconds between collections
        for (int i = 0; i < 100 && m_running.load(); ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    log("INFO", "System monitoring daemon stopped (PID " + std::to_string(m_pid) + ")");
}

void MonitoringDaemon::collect_stats() {
    auto info = m_sys.get_sysinfo();
    
    double mem_usage_percent = (double)info.used_memory / info.total_memory * 100.0;
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "System stats: Memory " << info.used_memory << "/" << info.total_memory 
        << " bytes (" << mem_usage_percent << "% used)";
    
    log("INFO", oss.str());
}

} // namespace daemons
