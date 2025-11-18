#include "Init.h"
#include "../terminal/Terminal.h"
#include "../shell/Shell.h"
#include "../shell/CommandsInit.h"
#include "../kernel/SysCallsAPI.h"
#include <iostream>

namespace init {

Init::Init(shell::SysApi& sys)
    : m_sys(sys) {}

void Init::log(const std::string& level, const std::string& message) {
    if (log_callback) {
        log_callback(level, "INIT", message);
    }
}

void Init::start() {
    log("INFO", "Init process (PID 1) starting...");
    
    // Initialize system services
    initialize_shell();
    
    log("INFO", "Init process initialization complete");
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
    
    shell::Shell sh(m_sys, registry);
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
    
    sh.setKernelCallback([this](const std::string& cmd, const std::vector<std::string>& args){
        // Init receives kernel commands but delegates to kernel via callback
        log("DEBUG", "Command execution requested: " + cmd);
        
        // Check if this is a shutdown request
        if (cmd == "quit" || cmd == "exit") {
            log("INFO", "Shutdown requested via " + cmd);
        }
    });
    
    term.setSendCallback([&](const std::string& line){
        sh.processCommandLine(line);
    });
    
    // Terminal sends signals - in real OS this goes to kernel, not init
    term.setSignalCallback([&](int sig){ 
        log("INFO", "Received signal: " + std::to_string(sig));
        std::cout << "^C" << std::flush;
        // TODO: Send signal to kernel via syscall
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

} // namespace init
