#pragma once

#include <memory>
#include <functional>
#include <string>

namespace sys { class SysApi; }

namespace daemons {

class Daemon;

// DaemonRegistry - Factory for creating and managing daemon instances
// Separates daemon creation from Init's lifecycle management responsibilities
class DaemonRegistry {
public:
    
    // Create a daemon by name
    // Returns nullptr if daemon type is unknown
    static std::shared_ptr<Daemon> createDaemon(
        const std::string& name,
        sys::SysApi& sys
    );
    
    // Get list of available daemon names
    static std::vector<std::string> getAvailableDaemons();
};

} // namespace daemons
