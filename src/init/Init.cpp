#include "Init.h"
#include "../terminal/Terminal.h"
#include "../shell/Shell.h"
#include "../shell/CommandsInit.h"
#include "../kernel/SysCallsAPI.h"
#include "../kernel/SysCalls.h"
#include "../daemon/Daemon.h"
#include "../daemon/DaemonRegistry.h"
#include <iostream>

namespace init {

// Initialize static members
std::unordered_map<int, daemons::Daemon*> Init::s_daemon_registry;
std::mutex Init::s_registry_mutex;

// Implement the custom deleter
void Init::DaemonDeleter::operator()(daemons::Daemon* p) const {
    delete p;
}

Init::Init(shell::SysApi& sys)
    : m_sys(sys) {}

void Init::log(const std::string& level, const std::string& message) {
    if (log_callback) {
        log_callback(level, "INIT", message);
    }
}

void Init::start() {
    log("INFO", "Init process (PID 1) starting...");
    
    // Start background daemons (system services)
    start_daemons();
    
    // Initialize shell (blocks until shell exits)
    initialize_shell();
    
    // Shutdown daemons when shell exits
    stop_daemons();
    
    log("INFO", "Init process shutdown complete");
}

void Init::initialize_shell() {
    log("INFO", "Starting shell service...");
    
    shell::CommandRegistry registry;
    shell::init_commands(registry);
    
    auto logger_callback = [this](const std::string& level, const std::string& module, const std::string& message){
        if (log_callback) {
            log_callback(level, module, message);
        }
    };

    // Shell with Kernel callback
    shell::Shell sh(
        m_sys,
        registry,
        [&](const std::string& cmd, const std::vector<std::string>& args) {
            auto* sysKernel = dynamic_cast<kernel::SysApiKernel*>(&m_sys);
            if (!sysKernel) {
                log("ERROR", "Init: m_sys is not SysApiKernel.");
                return;
            }
            auto* kernelPtr = sysKernel->getKernel();
            if (!kernelPtr) {
                log("ERROR", "Init: Kernel pointer is null.");
                return;
            }
            std::string result = kernelPtr->execute_command(cmd, args);
            log("DEBUG", "Kernel returned: " + result);
    });

    sh.setLogCallback(logger_callback);
    
    terminal::Terminal term;
    m_terminal = &term;
    term.setLogCallback(logger_callback);
    
    term.setPromptCallback([this](){
        return m_sys.getWorkingDir() + "$ ";
    });
    
    sh.setOutputCallback([&term](const std::string& output){
        if(!output.empty()){ 
            term.print(output); 
            if(output.back()!='\n') term.print("\n"); 
        }
    });
    
    term.setSendCallback([&](const std::string& line){
        sh.processCommandLine(line);
    });
    
    term.setSignalCallback([this](int sig){ 
        log("INFO", "Received signal " + std::to_string(sig) + " from terminal, forwarding to kernel");
        std::cout << "^C" << std::flush;
        m_sys.sendSignal(sig);
    });
    
    log("INFO", "Starting terminal...");
    
    term.start();
    
    // Wait for terminal to finish (blocks until user exits or kernel signals shutdown)
    term.join();
    
    log("INFO", "Shell service terminated");
    m_terminal = nullptr;
}

void Init::signalShutdown() {
    log("INFO", "Received shutdown signal from kernel (SIGTERM)");
    
    if (m_terminal) {
        m_terminal->requestShutdown();
    }
}

void Init::handleDaemonSignal(int pid, int signal) {
    // Find the daemon with this PID and notify it
    for (auto& dp : m_daemons) {
        if (dp.pid == pid && dp.daemon) {
            dp.daemon->handleSignal(signal);
            break;
        }
    }
}

void Init::start_daemons() {
    log("INFO", "Starting system daemons...");
    
    auto logger_callback = [this](const std::string& level, const std::string& module, const std::string& message){
        if (log_callback) {
            log_callback(level, module, message);
        }
    };
    
    auto system_daemons = daemons::DaemonRegistry::getAvailableDaemons();
    
    for (const auto& daemon_name : system_daemons) {
        // Fork a new process for this daemon
        int pid = m_sys.fork(daemon_name, 1, 512, 5);
        if (pid <= 0) {
            log("ERROR", "Failed to fork daemon: " + daemon_name);
            continue;
        }
        
        // Create the daemon instance using the registry
        auto daemon = daemons::DaemonRegistry::createDaemon(daemon_name, m_sys, logger_callback);
        if (!daemon) {
            log("ERROR", "Unknown daemon type: " + daemon_name);
            continue;
        }
        
        // Convert to unique_ptr with custom deleter
        std::unique_ptr<daemons::Daemon, DaemonDeleter> daemon_with_deleter(daemon.release());
        
        daemon_with_deleter->setPid(pid);
        
        // Set up signal handler - when process gets signaled, notify daemon
        auto daemon_ptr = daemon_with_deleter.get();
        daemon_with_deleter->setSignalCallback([daemon_ptr](int sig) {
            daemon_ptr->handleSignal(sig);
        });
        
        daemon_with_deleter->start();
        
        // Register in global registry for signal forwarding
        {
            std::lock_guard<std::mutex> lock(s_registry_mutex);
            s_daemon_registry[pid] = daemon_with_deleter.get();
        }
        
        m_daemons.push_back(DaemonProcess{std::move(daemon_with_deleter), pid});
    }
    
    log("INFO", "Started " + std::to_string(m_daemons.size()) + " system daemons");
}

void Init::stop_daemons() {
    log("INFO", "Stopping system daemons...");
    
    // Send SIGTERM to all daemon processes
    for (auto& dp : m_daemons) {
        m_sys.sendSignalToProcess(dp.pid, 15);  // SIGTERM
    }
    
    // Stop daemon threads
    for (auto& dp : m_daemons) {
        dp.daemon->stop();
    }
    
    // Wait for daemon threads to finish
    for (auto& dp : m_daemons) {
        dp.daemon->join();
    }
    
    // Unregister from global registry
    {
        std::lock_guard<std::mutex> lock(s_registry_mutex);
        for (auto& dp : m_daemons) {
            s_daemon_registry.erase(dp.pid);
        }
    }
    
    m_daemons.clear();
    
    log("INFO", "All system daemons stopped");
}

void Init::forwardSignalToDaemon(int pid, int signal) {
    std::lock_guard<std::mutex> lock(s_registry_mutex);
    auto it = s_daemon_registry.find(pid);
    if (it != s_daemon_registry.end()) {
        it->second->handleSignal(signal);
    }
}

} // namespace init
