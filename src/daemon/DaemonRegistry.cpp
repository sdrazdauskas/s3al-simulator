#include "daemon/DaemonRegistry.h"
#include "daemon/Daemon.h"
#include "daemon/MonitoringDaemon.h"
#include "kernel/SysCallsAPI.h"
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <iostream>

namespace daemons {

// Registry of all available daemons
// To add a new daemon: just add an entry here
static const std::unordered_map<std::string, std::function<std::shared_ptr<Daemon>(sys::SysApi&)>> daemonFactories = {
    {"sysmon", [](sys::SysApi& sys) { return std::make_shared<MonitoringDaemon>(sys); }}
};

DaemonRegistry::DaemonRegistry(sys::SysApi& sys)
    : sysApi(sys) {}

std::shared_ptr<Daemon> DaemonRegistry::createDaemon(
    const std::string& name,
    sys::SysApi& sys
) {
    auto it = daemonFactories.find(name);
    if (it == daemonFactories.end()) {
        return nullptr;  // Unknown daemon type
    }
    return it->second(sys);
}

std::vector<std::string> DaemonRegistry::getAvailableDaemons() {
    std::vector<std::string> names;
    names.reserve(daemonFactories.size());
    for (const auto& [name, factory] : daemonFactories) {
        names.push_back(name);
    }
    return names;
}

bool DaemonRegistry::startAll() {
    logInfo("Starting system daemons...");
    
    auto systemDaemons = getAvailableDaemons();
    
    for (const auto& daemonName : systemDaemons) {
        // Fork a new persistent process for this daemon
        int pid = sysApi.fork(daemonName, 1, 512, 5, true);
        if (pid <= 0) {
            logError("Failed to fork daemon: " + daemonName);
            return false;
        }
        
        std::shared_ptr<Daemon> daemonShared = createDaemon(daemonName, sysApi);
        if (!daemonShared) {
            logError("Unknown daemon type: " + daemonName);
            return false;
        }
        
        daemonShared->setPid(pid);
        
        auto daemonWeak = std::weak_ptr<Daemon>(daemonShared);
        daemonShared->setSignalCallback([daemonWeak](int sig) {
            if (auto daemon_ptr = daemonWeak.lock()) {
                daemon_ptr->handleSignal(sig);
            }
        });
        
        daemonShared->start();
        
        {
            std::lock_guard<std::mutex> lock(registryMutex);
            registry[pid] = daemonShared;
        }
        
        daemons.push_back(DaemonProcess{daemonShared, pid});
    }
    
    logInfo("Started " + std::to_string(daemons.size()) + " system daemons");
    return true;
}

void DaemonRegistry::stopAll() {
    logInfo("Stopping system daemons...");
    
    for (auto& dp : daemons) {
        sysApi.sendSignalToProcess(dp.pid, 15);  // SIGTERM
    }
    
    for (auto& dp : daemons) {
        dp.daemon->stop();
    }
    
    for (auto& dp : daemons) {
        dp.daemon->join();
    }

    for (auto& dp : daemons) {
        sysApi.exit(dp.pid, 0);
        sysApi.reapProcess(dp.pid);
    }
    
    {
        std::lock_guard<std::mutex> lock(registryMutex);
        for (auto& dp : daemons) {
            registry.erase(dp.pid);
        }
    }
    
    daemons.clear();
    logInfo("All system daemons stopped");
}

void DaemonRegistry::forwardSignal(int pid, int signal) {
    std::shared_ptr<Daemon> daemon;

    {
        std::lock_guard<std::mutex> lock(registryMutex);
        auto it = registry.find(pid);
        if (it != registry.end()) {
            daemon = it->second;
        }
    }

    if (daemon) {
        daemon->handleSignal(signal);
        
        // If termination signal (SIGKILL=9 or SIGTERM=15), stop daemon and mark for reaping
        if (signal == 9 || signal == 15) {
            daemon->join();
            
            {
                std::lock_guard<std::mutex> lock(registryMutex);
                registry.erase(pid);
            }
            
            daemons.erase(
                std::remove_if(daemons.begin(), daemons.end(),
                    [pid](const DaemonProcess& dp) { return dp.pid == pid; }),
                daemons.end()
            );
            
            {
                std::lock_guard<std::mutex> lock(terminatedMutex);
                terminatedDaemons.insert(pid);
            }
        }
    }
}

void DaemonRegistry::reapDaemon(int pid) {
    bool shouldReap = false;
    {
        std::lock_guard<std::mutex> lock(terminatedMutex);
        auto it = terminatedDaemons.find(pid);
        if (it != terminatedDaemons.end()) {
            shouldReap = true;
            terminatedDaemons.erase(it);
        }
    }
    
    if (shouldReap) {
        sysApi.reapProcess(pid);
    }
}

} // namespace daemons
