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
    
    void setLogCallback(LogCallback callback) { log_callback = callback; }
    void setSignalCallback(SignalCallback callback) { signal_callback = callback; }
    void setPid(int pid) { m_pid = pid; }
    int pid() const { return m_pid; }
    
    // Start daemon in background thread
    void start();
    
    // Stop daemon (called by signal handlers or shutdown)
    void stop();
    
    // Wait for daemon to finish
    void join();
    
    const std::string& name() const { return m_name; }
    bool is_running() const { return m_running.load(); }
    
    // Called when process receives a signal
    void handleSignal(int signal);

protected:
    shell::SysApi& m_sys;
    std::atomic<bool> m_running;
    int m_pid{-1};
    
    void log(const std::string& level, const std::string& message);
    
    // Override this in derived classes to implement daemon behavior
    virtual void run() = 0;

private:
    std::string m_name;
    LogCallback log_callback;
    SignalCallback signal_callback;
    std::thread m_thread;
};

} // namespace daemons
