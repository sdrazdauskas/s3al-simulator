#pragma once

#include <functional>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>

namespace shell { class SysApi; }
namespace terminal { class Terminal; }

// Forward declare daemons namespace for the .cpp file includes
namespace daemons { 
    class Daemon; 
}

namespace init {

// Init process (PID 1) - First user-space process
// Responsible for:
// - Starting the shell
// - Managing system services (daemons)
// - Handling orphaned processes (not implemented yet)
class Init {
public:
    using LogCallback = std::function<void(const std::string& level, 
                                           const std::string& module, 
                                           const std::string& message)>;
    using ShutdownCallback = std::function<void()>;

    Init(shell::SysApi& sys);
    
    void setLogCallback(LogCallback callback) { logCallback = callback; }
    void setShutdownCallback(ShutdownCallback callback) { shutdownCb = callback; }
    
    // Called by kernel to signal init to shutdown (like SIGTERM)
    void signalShutdown();
    
    // Called by kernel when a signal is sent to a daemon process
    void handleDaemonSignal(int pid, int signal);
    
    // Start init process - this becomes PID 1
    void start();
    
    // Static method to forward signals to daemon threads
    static void forwardSignalToDaemon(int pid, int signal);

private:
    // Static registry for mapping PIDs to daemon instances
    static std::unordered_map<int, daemons::Daemon*> daemonRegistry;
    static std::mutex registryMutex;
    // Custom deleter to allow forward declaration of Daemon
    struct DaemonDeleter {
        void operator()(daemons::Daemon* p) const;
    };
    
    struct DaemonProcess {
        std::unique_ptr<daemons::Daemon, DaemonDeleter> daemon;
        int pid;
        
        // Non-owning pointer for signal callbacks
        daemons::Daemon* daemon_ptr() const { return daemon.get(); }
    };
    
    shell::SysApi& sysApi;
    LogCallback logCallback;
    ShutdownCallback shutdownCb;
    terminal::Terminal* terminal = nullptr;
    std::vector<DaemonProcess> daemons;
    
    void log(const std::string& level, const std::string& message);
    void startDaemons();
    void stopDaemons();
    void initializeShell();
};

} // namespace init
