#include "daemon/DaemonRegistry.h"
#include "daemon/Daemon.h"
#include "daemon/MonitoringDaemon.h"
#include <unordered_map>
#include <functional>

namespace daemons {

// Registry of all available daemons
// To add a new daemon: just add an entry here
static const std::unordered_map<std::string, std::function<std::unique_ptr<Daemon>(sys::SysApi&)>> daemon_factories = {
    {"sysmon", [](sys::SysApi& sys) { return std::make_unique<MonitoringDaemon>(sys); }}
};

std::unique_ptr<Daemon> DaemonRegistry::createDaemon(
    const std::string& name,
    sys::SysApi& sys,
    LogCallback logCallback
) {
    auto it = daemon_factories.find(name);
    if (it == daemon_factories.end()) {
        return nullptr;  // Unknown daemon type
    }
    
    auto daemon = it->second(sys);
    
    if (daemon && logCallback) {
        daemon->setLogCallback(logCallback);
    }
    
    return daemon;
}

std::vector<std::string> DaemonRegistry::getAvailableDaemons() {
    std::vector<std::string> names;
    names.reserve(daemon_factories.size());
    for (const auto& [name, factory] : daemon_factories) {
        names.push_back(name);
    }
    return names;
}

} // namespace daemons
