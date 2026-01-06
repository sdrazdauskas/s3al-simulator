#include "daemon/MonitoringDaemon.h"
#include "kernel/SysCallsAPI.h"
#include <sstream>
#include <iomanip>

namespace daemons {

MonitoringDaemon::MonitoringDaemon(sys::SysApi& sys)
    : Daemon(sys, "SYSMON") {}

void MonitoringDaemon::doWork() {
    collectStats();
}

void MonitoringDaemon::collectStats() {
    auto info = sysApi.getSysInfo();
    
    double memUsagePercent = (double)info.usedMemory / info.totalMemory * 100.0;
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "System stats: Memory " << info.usedMemory << "/" << info.totalMemory 
        << " bytes (" << memUsagePercent << "% used)";
    
    logInfo(oss.str());
}

} // namespace daemons
