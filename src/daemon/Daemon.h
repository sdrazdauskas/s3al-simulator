#pragma once

#include <functional>
#include <string>
#include <atomic>
#include <thread>

namespace shell { class SysApi; }

namespace daemons {

// Base class for daemon processes - long-running background services
// Similar to systemd services in Linux
class Daemon {
public:
    using LogCallback = std::function<void(const std::string& level, 
                                           const std::string& module, 
                                           const std::string& message)>;
    using SignalCallback = std::function<void(int signal)>;

    Daemon(shell::SysApi& sys, const std::string& name);
    virtual ~Daemon() = default;
    
    void setLogCallback(LogCallback callback) { logCallback = callback; }
    void setSignalCallback(SignalCallback callback) { signalCallback = callback; }
    void setPid(int pid) { this->pid = pid; }
    int getPid() const { return pid; }
    
    // Start daemon in background thread
    void start();
    
    // Stop daemon (called by signal handlers or shutdown)
    void stop();
    
    // Wait for daemon to finish
    void join();
    
    const std::string& name() const { return daemonName; }
    bool isRunning() const { return running.load(); }
    bool isSuspended() const { return suspended.load(); }
    // Called when process receives a signal
    void handleSignal(int signal);

protected:
    shell::SysApi& sysApi;
    std::atomic<bool> running;
    std::atomic<bool> suspended{false};
    int pid{-1};
    
    void log(const std::string& level, const std::string& message);
    
    // Override this in derived classes to implement daemon behavior
    virtual void run() = 0;

private:
    std::string daemonName;
    LogCallback logCallback;
    SignalCallback signalCallback;
    std::thread thread;
};

} // namespace daemons
