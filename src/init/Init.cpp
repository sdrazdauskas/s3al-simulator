#include "init/Init.h"
#include "terminal/Terminal.h"
#include "shell/Shell.h"
#include "shell/CommandsInit.h"
#include "kernel/SysCallsAPI.h"
#include "kernel/SysCalls.h"
#include "daemon/DaemonRegistry.h"
#include <iostream>

namespace init {

Init::Init(sys::SysApi& sys)
    : sysApi(sys) {}

bool Init::start() {
    logInfo("Init process (PID 1) starting...");
    
    // Start background daemons (system services)
    if (!daemonRegistry.startAll(sysApi)) {
        logError("Failed to start system daemons - aborting init");
        daemonRegistry.stopAll(sysApi);
        return false;
    }
    
    logInfo("Waiting for system initialization...");
    sysApi.waitForProcess(1); // Wait for init itself
    
    // Initialize shell (blocks until shell exits)
    initializeShell();
    
    // Shutdown daemons when shell exits
    daemonRegistry.stopAll(sysApi);
    
    logInfo("Init process shutdown complete");
    return true;
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

void Init::handleProcessSignal(int pid, int signal) {
    // If shell receives a termination signal, shut down terminal immediately
    if (pid == shellPid && (signal == 9 || signal == 15)) {
        logWarn("Shell process (PID=" + std::to_string(shellPid) + ") terminated by signal " + std::to_string(signal) + " - shutting down terminal");
        if (terminal) {
            terminal->requestShutdown();
        }
        return;
    }
    
    // Forward signal to daemon (if it's a daemon process)
    daemonRegistry.forwardSignal(pid, signal);
}

} // namespace init
