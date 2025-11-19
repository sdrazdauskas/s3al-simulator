#pragma once

#include "Daemon.h"

namespace daemons {

// System monitoring daemon, periodically collects and logs system statistics
class MonitoringDaemon : public Daemon {
public:
    MonitoringDaemon(shell::SysApi& sys);

protected:
    void run() override;

private:
    void collect_stats();
};

} // namespace daemons
