#include "Kernel.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <chrono>
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

    std::cout << "Kernel initialized." << std::endl;
}

shell::SysApi::SysInfo Kernel::get_sysinfo() const {
    shell::SysApi::SysInfo info;
    info.total_memory = m_mem_mgr.get_total_memory();
    info.used_memory = m_mem_mgr.get_used_memory();
    return info;
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

    // Make it a bit dynamic by passing args size as resource needs
    const int arg_count = static_cast<int>(std::max(static_cast<size_t>(1), args.size()));
    const int cpu_required = 2 * arg_count;
    const int memory_required = 64 * arg_count;
    if (m_proc_manager.execute_process(command_name, cpu_required, memory_required, 0) != -1) {
        //string result = it->second(args);
        return "OK";
    } else {
        return "Error: Unable to execute process for command '" + command_name + "'.";
    }

    return "Unknown command: '" + command_name + "'.";
}

std::string Kernel::handle_quit(const std::vector<std::string>& args){
    (void)args;
    m_is_running = false;
    kernel_running.store(false);
    
    // Submit shutdown event to wake up kernel thread
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        event_queue.push({KernelEvent::Type::SHUTDOWN, ""});
    }
    queue_cv.notify_one();
    
    return "Shutting down kernel.";
}

void Kernel::submit_command(const std::string& line) {
    std::lock_guard<std::mutex> lock(queue_mutex);
    event_queue.push({KernelEvent::Type::COMMAND, line});
    queue_cv.notify_one();
}

void Kernel::process_event(const KernelEvent& event) {
    switch (event.type) {
        case KernelEvent::Type::COMMAND:
            LOG_DEBUG("KERNEL", "Processing command: " + event.data);
            // Command processing happens in shell, we just logged it
            break;
            
        case KernelEvent::Type::TIMER_TICK:
            handle_timer_tick();
            break;
            
        case KernelEvent::Type::SHUTDOWN:
            LOG_INFO("KERNEL", "Shutdown event received");
            break;
    }
}

void Kernel::handle_timer_tick() {
    // This is where background tasks would run:
    // - Process scheduling
    // - Memory garbage collection
    // - I/O completion checks
    // - System monitoring
    
    // Example: Log system status periodically
    static int tick_count = 0;
    static int last_logged_tick = 0;
    tick_count++;
    
    // Log less frequently to avoid spam (every 50 ticks = ~5 seconds)
    if (tick_count - last_logged_tick >= 50) {
        size_t used_mem = m_mem_mgr.get_used_memory();
        size_t total_mem = m_mem_mgr.get_total_memory();
        double mem_usage = (double)used_mem / total_mem * 100.0;
        
        LOG_DEBUG("KERNEL", "Heartbeat [tick:" + std::to_string(tick_count) + 
                  ", mem:" + std::to_string((int)mem_usage) + "%]");
        last_logged_tick = tick_count;
    }
    
    // Here you could add:
    // - Process scheduler time slice expiration
    // - Disk I/O completion polling
    // - Network packet processing
    // - Timer-based events (cron-like tasks)
}

void Kernel::run_event_loop() {
    LOG_INFO("KERNEL", "Kernel event loop started");
    
    auto last_tick = std::chrono::steady_clock::now();
    const auto tick_interval = std::chrono::milliseconds(100); // 10 Hz tick rate
    
    while (kernel_running.load()) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        
        // Wait for event or timeout
        if (queue_cv.wait_for(lock, tick_interval, [this] { 
            return !event_queue.empty() || !kernel_running.load(); 
        })) {
            // Process all pending events
            while (!event_queue.empty()) {
                auto event = event_queue.front();
                event_queue.pop();
                lock.unlock();
                
                process_event(event);
                
                lock.lock();
            }
        } else {
            // Timeout - generate timer tick
            lock.unlock();
            
            auto now = std::chrono::steady_clock::now();
            if (now - last_tick >= tick_interval) {
                process_event({KernelEvent::Type::TIMER_TICK, ""});
                last_tick = now;
            }
        }
    }
    
    LOG_INFO("KERNEL", "Kernel event loop stopped");
}

void Kernel::boot(){
    LOG_INFO("KERNEL", "Booting s3al OS...");
    
    // Initialize command registry
    shell::CommandRegistry registry;
    shell::init_commands(registry);
    
    auto logger_callback = [](const std::string& level, const std::string& module, const std::string& message){
        logging::Logger::getInstance().log(level, module, message);
    };
    

    SysApiKernel sys(m_storage, this);//build syscalls 
    shell::Shell sh(sys, registry);

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

    
    sh.setKernelCallback([this](const std::string& cmd, const std::vector<std::string>& args){
        this->execute_command(cmd, args);
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
    
    // Start kernel event loop in a separate thread
    kernel_running.store(true);
    kernel_thread = std::thread([this]() {
        this->run_event_loop();
    });
    
    // Start terminal in a separate thread
    term.start();
    
    // Main thread waits for terminal to finish
    // The kernel continues to run in its own thread, handling background tasks
    term.join();
    
    // After terminal exits, stop kernel event loop
    kernel_running.store(false);
    queue_cv.notify_one();
    
    if (kernel_thread.joinable()) {
        kernel_thread.join();
    }
    
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
