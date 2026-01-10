#pragma once

#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include "common/LoggingMixin.h"

namespace sys { class SysApi; }

namespace daemons {

class Daemon;

// DaemonRegistry - Factory and lifecycle manager for daemon instances
class DaemonRegistry : public common::LoggingMixin {
public:
    DaemonRegistry() = default;
    ~DaemonRegistry() = default;
    
    // Create a daemon by name - Returns nullptr if daemon type is unknown
    static std::shared_ptr<Daemon> createDaemon(
        const std::string& name,
        sys::SysApi& sys
    );
    
    // Get list of available daemon names
    static std::vector<std::string> getAvailableDaemons();
    
    // Start all available daemons (fork processes, register, start threads)
    bool startAll(sys::SysApi& sys);
    
    // Stop all daemons (send signals, stop threads, unregister)
    void stopAll(sys::SysApi& sys);
    
    // Forward signal to daemon thread by PID
    void forwardSignal(int pid, int signal);

protected:
    std::string getModuleName() const override { return "DAEMON_REGISTRY"; }
    
private:
    struct DaemonProcess {
        std::shared_ptr<Daemon> daemon;
        int pid;
    };
    
    std::unordered_map<int, std::shared_ptr<Daemon>> registry;
    std::mutex registryMutex;
    std::vector<DaemonProcess> daemons;
};

} // namespace daemons
