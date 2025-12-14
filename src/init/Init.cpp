#include "init/Init.h"
#include "terminal/Terminal.h"
#include "shell/Shell.h"
#include "shell/CommandsInit.h"
#include "kernel/SysCallsAPI.h"
#include "kernel/SysCalls.h"
#include "daemon/Daemon.h"
#include "daemon/DaemonRegistry.h"
#include <iostream>

namespace init {

// Initialize static members
std::unordered_map<int, daemons::Daemon*> Init::daemonRegistry;
std::mutex Init::registryMutex;

// Implement the custom deleter
void Init::DaemonDeleter::operator()(daemons::Daemon* p) const {
    delete p;
}

Init::Init(sys::SysApi& sys)
    : sysApi(sys) {}

void Init::log(const std::string& level, const std::string& message) {
    if (logCallback) {
        logCallback(level, "INIT", message);
    }
}

void Init::start() {
    log("INFO", "Init process (PID 1) starting...");
    
    // Start background daemons (system services)
    startDaemons();
    
    // Wait for all persistent processes (init + daemons) to complete their first scheduling cycle
    // This ensures the scheduler has processed them before we start accepting user input
    log("INFO", "Waiting for system initialization...");
    for (const auto& daemonProc : daemons) {
        sysApi.waitForProcess(daemonProc.pid);
    }
    sysApi.waitForProcess(1); // Wait for init itself
    
    // Initialize shell (blocks until shell exits)
    initializeShell();
    
    // Shutdown daemons when shell exits
    stopDaemons();
    
    log("INFO", "Init process shutdown complete");
}

void Init::initializeShell() {
    log("INFO", "Starting shell service...");
    
    shell::CommandRegistry registry;
    shell::initCommands(registry);
    
    auto loggerCallback = [this](const std::string& level, const std::string& module, const std::string& message){
        if (logCallback) {
            logCallback(level, module, message);
        }
    };

    // Shell with Kernel callback
    shell::Shell sh(
        sysApi,
        registry,
        [&](const std::string& cmd, const std::vector<std::string>& args) {
            auto* sysKernel = dynamic_cast<kernel::SysApiKernel*>(&sysApi);
            if (!sysKernel) {
                log("ERROR", "Init: sysApi is not SysApiKernel.");
                return;
            }
            auto* kernelPtr = sysKernel->getKernel();
            if (!kernelPtr) {
                log("ERROR", "Init: Kernel pointer is null.");
                return;
            }
            std::string result = kernelPtr->executeCommand(cmd, args);
            log("DEBUG", "Kernel returned: " + result);
    });

    sh.setLogCallback(loggerCallback);
    
    terminal::Terminal term;
    terminal = &term;
    term.setLogCallback(loggerCallback);
    
    term.setPromptCallback([this](){
        return sysApi.getWorkingDir() + "$ ";
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
        sysApi.sendSignal(sig);
    });
    
    log("INFO", "Starting terminal...");
    
    term.start();
    
    // Wait for terminal to finish (blocks until user exits or kernel signals shutdown)
    term.join();
    
    log("INFO", "Shell service terminated");
    terminal = nullptr;
}

void Init::signalShutdown() {
    log("INFO", "Received shutdown signal from kernel (SIGTERM)");
    
    if (terminal) {
        terminal->requestShutdown();
    }
}

void Init::handleDaemonSignal(int pid, int signal) {
    // Find the daemon with this PID and notify it
    for (auto& dp : daemons) {
        if (dp.pid == pid && dp.daemon) {
            dp.daemon->handleSignal(signal);
            break;
        }
    }
}

void Init::startDaemons() {
    log("INFO", "Starting system daemons...");
    
    auto loggerCallback = [this](const std::string& level, const std::string& module, const std::string& message){
        if (logCallback) {
            logCallback(level, module, message);
        }
    };
    
    auto system_daemons = daemons::DaemonRegistry::getAvailableDaemons();
    
    for (const auto& daemon_name : system_daemons) {
        // Fork a new persistent process for this daemon
        int pid = sysApi.fork(daemon_name, 1, 512, 5, true);
        if (pid <= 0) {
            log("ERROR", "Failed to fork daemon: " + daemon_name);
            continue;
        }
        
        // Create the daemon instance using the registry
        auto daemon = daemons::DaemonRegistry::createDaemon(daemon_name, sysApi, loggerCallback);
        if (!daemon) {
            log("ERROR", "Unknown daemon type: " + daemon_name);
            continue;
        }
        
        // Convert to unique_ptr with custom deleter
        std::unique_ptr<daemons::Daemon, DaemonDeleter> daemonWithDeleter(daemon.release());
        
        daemonWithDeleter->setPid(pid);
        
        // Set up signal handler - when process gets signaled, notify daemon
        auto daemon_ptr = daemonWithDeleter.get();
        daemonWithDeleter->setSignalCallback([daemon_ptr](int sig) {
            daemon_ptr->handleSignal(sig);
        });
        
        daemonWithDeleter->start();
        
        // Register in global registry for signal forwarding
        {
            std::lock_guard<std::mutex> lock(registryMutex);
            daemonRegistry[pid] = daemonWithDeleter.get();
        }
        
        daemons.push_back(DaemonProcess{std::move(daemonWithDeleter), pid});
    }
    
    log("INFO", "Started " + std::to_string(daemons.size()) + " system daemons");
}

void Init::stopDaemons() {
    log("INFO", "Stopping system daemons...");
    
    // Send SIGTERM to all daemon processes
    for (auto& dp : daemons) {
        sysApi.sendSignalToProcess(dp.pid, 15);  // SIGTERM
    }
    
    // Stop daemon threads
    for (auto& dp : daemons) {
        dp.daemon->stop();
    }
    
    // Wait for daemon threads to finish
    for (auto& dp : daemons) {
        dp.daemon->join();
    }
    
    // Unregister from global registry
    {
        std::lock_guard<std::mutex> lock(registryMutex);
        for (auto& dp : daemons) {
            daemonRegistry.erase(dp.pid);
        }
    }
    
    daemons.clear();
    
    log("INFO", "All system daemons stopped");
}

void Init::forwardSignalToDaemon(int pid, int signal) {
    std::lock_guard<std::mutex> lock(registryMutex);
    auto it = daemonRegistry.find(pid);
    if (it != daemonRegistry.end()) {
        it->second->handleSignal(signal);
    }
}

} // namespace init
