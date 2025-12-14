#pragma once

#include "daemon/Daemon.h"

namespace daemons {

// System monitoring daemon, periodically collects and logs system statistics
class MonitoringDaemon : public Daemon {
public:
    MonitoringDaemon(sys::SysApi& sys);

protected:
    void doWork() override;
    int getWorkCycles() const override { return 5; }
    int getWaitIntervalMs() const override { return 10000; } // 10 seconds

private:
    void collect_stats();
};

} // namespace daemons
