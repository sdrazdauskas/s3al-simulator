#include "daemon/MonitoringDaemon.h"
#include "kernel/SysCallsAPI.h"
#include <sstream>
#include <iomanip>

namespace daemons {

MonitoringDaemon::MonitoringDaemon(sys::SysApi& sys)
    : Daemon(sys, "SYSMON") {}

void MonitoringDaemon::doWork() {
    collect_stats();
}

void MonitoringDaemon::collect_stats() {
    auto info = sysApi.getSysInfo();
    
    double mem_usage_percent = (double)info.usedMemory / info.totalMemory * 100.0;
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "System stats: Memory " << info.usedMemory << "/" << info.totalMemory 
        << " bytes (" << mem_usage_percent << "% used)";
    
    logInfo(oss.str());
}

} // namespace daemons
