#pragma once

#include <functional>
#include <string>
#include <memory>
#include "common/LoggingMixin.h"
#include "daemon/DaemonRegistry.h"

namespace sys { class SysApi; }
namespace terminal { class Terminal; }

namespace init {

// Init process (PID 1) - First user-space process
// Responsible for:
// - Starting the shell
// - Coordinating system services (via DaemonRegistry)
class Init : public common::LoggingMixin {
public:
    using ShutdownCallback = std::function<void()>;

    Init(sys::SysApi& sys);
    
    void setShutdownCallback(ShutdownCallback callback) { shutdownCb = callback; }
    
    // Called by kernel to signal init to shutdown (like SIGTERM)
    void signalShutdown();

    // Called by kernel when any process (shell/daemon) signals or terminates
    void handleProcessSignal(int pid, int signal);
    
    // Start init process - this becomes PID 1
    // Returns false if daemon startup fails
    bool start();

protected:
    std::string getModuleName() const override { return "INIT"; }

private:
    sys::SysApi& sysApi;
    ShutdownCallback shutdownCb;
    terminal::Terminal* terminal = nullptr;
    int shellPid = -1;
    daemons::DaemonRegistry daemonRegistry;
    
    void initializeShell();
};

} // namespace init
