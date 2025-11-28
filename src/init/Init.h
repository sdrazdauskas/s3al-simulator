#pragma once

#include <functional>
#include <string>

namespace shell { class SysApi; }
namespace terminal { class Terminal; }

namespace init {

// Init process (PID 1) - First user-space process
// Responsible for:
// - Starting the shell
// - Managing system services
// - Handling orphaned processes (not implemented yet)
class Init {
public:
    using LogCallback = std::function<void(const std::string& level, 
                                           const std::string& module, 
                                           const std::string& message)>;
    using ShutdownCallback = std::function<void()>;

    Init(shell::SysApi& sys);
    
    void setLogCallback(LogCallback callback) { log_callback = callback; }
    void setShutdownCallback(ShutdownCallback callback) { shutdown_callback = callback; }
    
    // Called by kernel to signal init to shutdown (like SIGTERM)
    void signalShutdown();
    
    // Start init process - this becomes PID 1
    void start();

private:
    shell::SysApi& m_sys;
    LogCallback log_callback;
    ShutdownCallback shutdown_callback;
    terminal::Terminal* m_terminal = nullptr;
    
    void log(const std::string& level, const std::string& message);
    void initialize_shell();
};

} // namespace init
