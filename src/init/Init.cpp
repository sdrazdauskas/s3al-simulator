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
std::unordered_map<int, std::shared_ptr<daemons::Daemon>> Init::daemonRegistry;
std::mutex Init::registryMutex;

Init::Init(sys::SysApi& sys)
    : sysApi(sys) {}

void Init::start() {
    logInfo("Init process (PID 1) starting...");
    
    // Start background daemons (system services)
    startDaemons();
    
    // Wait for all persistent processes (init + daemons) to complete their first scheduling cycle
    // This ensures the scheduler has processed them before we start accepting user input
    logInfo("Waiting for system initialization...");
    for (const auto& daemonProc : daemons) {
        sysApi.waitForProcess(daemonProc.pid);
    }
    sysApi.waitForProcess(1); // Wait for init itself
    
    // Initialize shell (blocks until shell exits)
    initializeShell();
    
    // Shutdown daemons when shell exits
    stopDaemons();
    
    logInfo("Init process shutdown complete");
}

void Init::initializeShell() {
    logInfo("Starting shell service...");
    
    // Create shell as a persistent process
    shellPid = sysApi.fork("sh", 1, 0, 1, true);
    if (shellPid <= 0) {
        logError("Failed to create shell process");
        return;
    }
    logInfo("Shell process created (PID=" + std::to_string(shellPid) + ")");
    
    shell::CommandRegistry registry;
    shell::initCommands(registry);

    shell::Shell sh(sysApi, registry);

    sh.setShellPid(shellPid);
    
    terminal::Terminal term;
    terminal = &term;
    
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
        // Check if shell process still exists (could have been killed)
        if (!sysApi.processExists(shellPid)) {
            logWarn("Shell process (PID=" + std::to_string(shellPid) + ") was killed - shutting down terminal");
            term.requestShutdown();
            return;
        }
        sh.processCommandLine(line);
    });
    
    term.setSignalCallback([this](int sig){ 
        logInfo("Received signal " + std::to_string(sig) + " from terminal, forwarding to kernel");
        std::cout << "^C" << std::flush;
        sysApi.sendSignal(sig);
    });
    
    logInfo("Starting terminal...");
    
    term.start();
    
    // Wait for terminal to finish (blocks until user exits or kernel signals shutdown)
    term.join();
    
    logInfo("Shell service terminated");
    terminal = nullptr;
}

void Init::signalShutdown() {
    logInfo("Received shutdown signal from kernel (SIGTERM)");
    
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

void Init::handleProcessSignal(int pid, int signal) {
    // If shell receives a termination signal, shut down terminal immediately
    if (pid == shellPid && (signal == 9 || signal == 15)) {
        logWarn("Shell process (PID=" + std::to_string(shellPid) + ") terminated by signal " + std::to_string(signal) + " - shutting down terminal");
        if (terminal) {
            terminal->requestShutdown();
        }
        return;
    }
    
    handleDaemonSignal(pid, signal);
}

void Init::startDaemons() {
    logInfo("Starting system daemons...");
    
    auto system_daemons = daemons::DaemonRegistry::getAvailableDaemons();
    
    for (const auto& daemonName : system_daemons) {
        // Fork a new persistent process for this daemon
        int pid = sysApi.fork(daemonName, 1, 512, 5, true);
        if (pid <= 0) {
            logError("Failed to fork daemon: " + daemonName);
            continue;
        }
        
        // Create the daemon instance using the registry (now returns shared_ptr)
        std::shared_ptr<daemons::Daemon> daemonShared = daemons::DaemonRegistry::createDaemon(daemonName, sysApi);
        if (!daemonShared) {
            logError("Unknown daemon type: " + daemonName);
            continue;
        }
        
        daemonShared->setPid(pid);
        
        // Set up signal handler - when process gets signaled, notify daemon
        auto daemonWeak = std::weak_ptr<daemons::Daemon>(daemonShared);
        daemonShared->setSignalCallback([daemonWeak](int sig) {
            if (auto daemon_ptr = daemonWeak.lock()) {
                daemon_ptr->handleSignal(sig);
            }
        });
        
        daemonShared->start();
        
        // Register in global registry for signal forwarding
        {
            std::lock_guard<std::mutex> lock(registryMutex);
            daemonRegistry[pid] = daemonShared;
        }
        
        daemons.push_back(DaemonProcess{daemonShared, pid});
    }
    
    logInfo("Started " + std::to_string(daemons.size()) + " system daemons");
}

void Init::stopDaemons() {
    logInfo("Stopping system daemons...");
    
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
    
    logInfo("All system daemons stopped");
}

void Init::forwardSignalToDaemon(int pid, int signal) {
    std::lock_guard<std::mutex> lock(registryMutex);
    auto it = daemonRegistry.find(pid);
    if (it != daemonRegistry.end()) {
        it->second->handleSignal(signal);
    }
}

} // namespace init
