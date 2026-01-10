#pragma once

#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "common/LoggingMixin.h"

namespace sys { class SysApi; }

namespace daemons {

class Daemon;

// DaemonRegistry - Factory and lifecycle manager for daemon instances
class DaemonRegistry : public common::LoggingMixin {
public:

    DaemonRegistry(sys::SysApi& sys);
    
    // Create a daemon by name - Returns nullptr if daemon type is unknown
    static std::shared_ptr<Daemon> createDaemon(
        const std::string& name,
        sys::SysApi& sys
    );
    
    // Get list of available daemon names
    static std::vector<std::string> getAvailableDaemons();
    
    // Start all available daemons (fork processes, register, start threads)
    bool startAll();
    
    // Stop all daemons (send signals, stop threads, unregister)
    void stopAll();
    
    // Forward signal to daemon thread by PID
    void forwardSignal(int pid, int signal);
    
    // Reap a terminated daemon process (called after it becomes zombie)
    void reapDaemon(int pid);

protected:
    std::string getModuleName() const override { return "DAEMON_REGISTRY"; }
    
private:
    struct DaemonProcess {
        std::shared_ptr<Daemon> daemon;
        int pid;
    };

    sys::SysApi& sysApi;
    
    std::unordered_map<int, std::shared_ptr<Daemon>> registry;
    std::mutex registryMutex;
    std::vector<DaemonProcess> daemons;
    
    // Track PIDs of terminated daemons waiting to be reaped
    std::unordered_set<int> terminatedDaemons;
    std::mutex terminatedMutex;
};

} // namespace daemons
