#include "daemon/DaemonRegistry.h"
#include "daemon/Daemon.h"
#include "daemon/MonitoringDaemon.h"
#include <unordered_map>
#include <functional>

namespace daemons {

// Registry of all available daemons
// To add a new daemon: just add an entry here
static const std::unordered_map<std::string, std::function<std::shared_ptr<Daemon>(sys::SysApi&)>> daemonFactories = {
    {"sysmon", [](sys::SysApi& sys) { return std::make_shared<MonitoringDaemon>(sys); }}
};

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

} // namespace daemons
