#include "Kernel.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "Terminal.h"
#include "Shell.h"
#include "Logger.h"
#include "SysCalls.h"
#include "CommandsInit.h"

using namespace std;

Kernel::Kernel()
    : m_is_running(true),
      m_scheduler(),
      m_mem_mgr(1024 * 1024),
      m_proc_manager(m_mem_mgr, m_scheduler) {
    auto logger_callback = [](const std::string& level, const std::string& module, const std::string& message){
        logging::Logger::getInstance().log(level, module, message);
    };

    m_proc_manager.setLogCallback(logger_callback);
    m_scheduler.setLogCallback(logger_callback);
    m_storage.setLogCallback(logger_callback);
    m_mem_mgr.setLogCallback(logger_callback);

    register_commands();
    std::cout << "Kernel initialized." << std::endl;
}


std::string Kernel::execute_command(const std::string& line) {
    if (!line.empty() && line.back() == '\n') {
        return process_line(line.substr(0, line.size() - 1));
    }
    return process_line(line);
}

std::string Kernel::execute_command(const std::string& cmd, const std::vector<std::string>& args) {
    std::string line = cmd;
    for (const auto& arg : args) line += " " + arg;
    return execute_command(line);
}

bool Kernel::is_running() const { return m_is_running; }

void Kernel::register_commands() {
    // Kernel-owned commands: keep quit/exit and memory introspection here.
    m_commands["quit"] = [this](const auto& args){ return handle_quit(args); };
    m_commands["exit"] = [this](const auto& args){ return handle_quit(args); };

    m_commands["meminfo"] = [this](const auto& args){ return this->handle_meminfo(args); };
    m_commands["membar"]  = [this](const auto& args){ return this->handle_membar(args); };
}

std::string Kernel::process_line(const std::string& line) {
    if(line.empty()) return "";

    istringstream iss(line);
    string command_name;
    iss >> command_name;

    vector<string> args;
    string token;
    while(iss >> token) {
        if(!token.empty() && token.front()=='"') {
            string quoted = token.substr(1);
            while(iss && (quoted.empty() || quoted.back()!='"')) {
                if(!(iss>>token)) break;
                quoted += " "+token;
            }
            if(!quoted.empty() && quoted.back()=='"') quoted.pop_back();
            args.push_back(quoted);
        } else {
            args.push_back(token);
        }
    }

    auto it = m_commands.find(command_name);
    if(it != m_commands.end()) {
        string result = it->second(args);
        m_proc_manager.execute_process(command_name, 1, 1, 0);
        return result;
    }

    return "Unknown command: '" + command_name + "'.";
}

std::string Kernel::handle_quit(const std::vector<std::string>& args){
    (void)args;
    m_is_running = false;
    return "Shutting down kernel.";
}

void Kernel::boot(){
    LOG_INFO("KERNEL", "Booting s3al OS...");
    
    auto logger_callback = [](const std::string& level, const std::string& module, const std::string& message){
        logging::Logger::getInstance().log(level, module, message);
    };
    

    SysApiKernel sys(m_storage);//build sysclals
    shell::CommandRegistry reg;//build command REGISTRY
    init_commands(reg); // register commands
    shell::Shell sh(sys, reg);

    sh.setLogCallback(logger_callback); 
    
    terminal::Terminal term;
    term.setLogCallback(logger_callback);
    
    // Set prompt callback to show current directory
    term.setPromptCallback([this](){
        return m_storage.getWorkingDir() + "$ ";
    });
    
    // Shell's output callback - prints results to terminal
    sh.setOutputCallback([&term](const string& output){
        if(!output.empty()){ 
            term.print(output); 
            if(output.back()!='\n') term.print("\n"); 
        }
    });
    
    term.setSendCallback([&](const string& line){
        sh.processCommandLine(line);
        
        // If kernel wants to shutdown, signal terminal to exit
        if(!m_is_running) {
            term.requestShutdown();
        }
    });
    term.setSignalCallback([&term](int){ 
        std::cout << "^C\n" << std::flush;
        // TODO: Interrupt currently running process
        // For now, just print ^C and continue
    });

    LOG_INFO("KERNEL", "Init process started");
    term.runBlockingStdioLoop();
    LOG_INFO("KERNEL", "Shutdown complete");
}

std::string Kernel::handle_meminfo(const std::vector<std::string>& args) {
    (void)args;
    size_t total = m_mem_mgr.get_total_memory();
    size_t used  = m_mem_mgr.get_used_memory();
    size_t free  = total - used;

    std::ostringstream oss;
    oss << "=== Memory Info ===\n";
    oss << "Total: " << total / 1024 << " KB\n";
    oss << "Used : " << used / 1024 << " KB\n";
    oss << "Free : " << free / 1024 << " KB\n";
    return oss.str();
}

std::string Kernel::handle_membar(const std::vector<std::string>& args) {
    (void)args;
    size_t total = m_mem_mgr.get_total_memory();
    size_t used  = m_mem_mgr.get_used_memory();
    int bar_width = 40;
    int used_blocks = static_cast<int>((double)used / total * bar_width);

    std::ostringstream oss;
    oss << "[Memory] [";
    for (int i = 0; i < bar_width; ++i) {
        oss << (i < used_blocks ? '#' : '-');
    }
    oss << "] " << (used * 100 / total) << "% used\n";
    return oss.str();
}
