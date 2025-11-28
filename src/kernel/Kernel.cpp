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
#include "CommandAPI.h"
#include "../init/Init.h"

namespace kernel {

Kernel::Kernel(size_t memory_size)
    : m_is_running(true),
      m_scheduler(),
      m_mem_mgr(memory_size),
      m_proc_manager(m_mem_mgr, m_scheduler) {
    auto logger_callback = [](const std::string& level, const std::string& module, const std::string& message){
        logging::Logger::getInstance().log(level, module, message);
    };

    m_proc_manager.setLogCallback(logger_callback);
    m_scheduler.setLogCallback(logger_callback);
    m_storage.setLogCallback(logger_callback);
    m_mem_mgr.setLogCallback(logger_callback);
    
    // Set up signal callback to notify Init about signals sent to daemon processes
    m_proc_manager.setSignalCallback([this](int pid, int signal) {
        // Init will handle this via its daemon signal callbacks
        // This allows daemons to respond to signals sent via kill command
    });

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

    std::istringstream iss(line);
    std::string command_name;
    iss >> command_name;

    std::vector<std::string> args;
    std::string token;
    while(iss >> token) {
        if(!token.empty() && token.front()=='"') {
            std::string quoted = token.substr(1);
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
        return "OK";
    } else {
        return "Error: Unable to execute process for command '" + command_name + "'.";
    }

    return "Unknown command: '" + command_name + "'.";
}

std::string Kernel::handle_quit(const std::vector<std::string>& args){
    (void)args;
    LOG_INFO("KERNEL", "Shutdown requested");
    
    m_is_running = false;
    kernel_running.store(false);
    
    // Signal init to shutdown (like kernel sending SIGTERM to PID 1)
    if (m_init_shutdown) {
        m_init_shutdown();
    }
    
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

void Kernel::handle_interrupt_signal(int signal) {
    LOG_INFO("KERNEL", "Received interrupt signal: " + std::to_string(signal));
    
    // Set interrupt flag immediately for responsiveness
    // Commands check this flag and should exit promptly
    shell::g_interrupt_requested.store(true);
    
    // Also submit to kernel queue for proper processing
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        KernelEvent event;
        event.type = KernelEvent::Type::INTERRUPT_SIGNAL;
        event.signal_number = signal;
        event_queue.push(event);
    }
    queue_cv.notify_one();
}

bool Kernel::send_signal_to_process(int pid, int signal) {
    return m_proc_manager.send_signal(pid, signal);
}

int Kernel::fork_process(const std::string& name, int cpuTimeNeeded, int memoryNeeded, int priority) {
    return m_proc_manager.create_process(name, cpuTimeNeeded, memoryNeeded, priority);
}

std::vector<shell::SysApi::ProcessInfo> Kernel::get_process_list() const {
    auto processes = m_proc_manager.snapshot();
    std::vector<shell::SysApi::ProcessInfo> result;
    
    for (const auto& proc : processes) {
        shell::SysApi::ProcessInfo info;
        info.pid = proc.pid();
        info.name = proc.name();
        info.state = process::stateToString(proc.state());
        info.priority = proc.priority();
        result.push_back(info);
    }
    
    return result;
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
            
        case KernelEvent::Type::INTERRUPT_SIGNAL:
            LOG_DEBUG("KERNEL", "Processing interrupt signal: " + std::to_string(event.signal_number));

            // Flag was already set immediately in handle_interrupt_signal()
            // This event is for kernel bookkeeping and future process management
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
    // - System monitoring
    
    // Example: Timer tick counter
    static int tick_count = 0;
    static int last_logged_tick = 0;
    tick_count++;
    
    // Log system status periodically (every 50 ticks = ~5 seconds)
    // In a real OS, this would be dmesg or system monitoring tools
    if (tick_count - last_logged_tick >= 50) {
        size_t used_mem = m_mem_mgr.get_used_memory();
        size_t total_mem = m_mem_mgr.get_total_memory();
        double mem_usage = (double)used_mem / total_mem * 100.0;
        
        LOG_DEBUG("KERNEL", "System status [tick:" + std::to_string(tick_count) + 
                  ", mem:" + std::to_string((int)mem_usage) + "%]");
        last_logged_tick = tick_count;
    }
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
    
    auto logger_callback = [](const std::string& level, const std::string& module, const std::string& message){
        logging::Logger::getInstance().log(level, module, message);
    };
    
    // Set up signal callback so ProcessManager can notify daemon threads
    m_proc_manager.setSignalCallback([](int pid, int signal) {
        // Forward signal to daemon thread if it exists
        init::Init::forwardSignalToDaemon(pid, signal);
    });
    
    // Start kernel event loop in a separate thread
    kernel_running.store(true);
    kernel_thread = std::thread([this]() {
        this->run_event_loop();
    });
    
    LOG_INFO("KERNEL", "Starting init process (PID 1)...");
    
    // Create init as actual process with PID 1
    int init_pid = m_proc_manager.create_process("init", 1, 1024, 10);
    if (init_pid != 1) {
        LOG_ERROR("KERNEL", "Failed to create init process");
        return;
    }
    
    // Create syscall interface for user-space processes
    SysApiKernel sys(m_storage, this);
    
    // Create and start init process (PID 1)
    init::Init init(sys);
    init.setLogCallback(logger_callback);
    
    // Set up callback so Init can handle signals sent to its daemon processes
    setDaemonSignalCallback([&init](int pid, int signal) {
        init.handleDaemonSignal(pid, signal);
    });
    
    // Store reference to init so kernel can signal it on shutdown (like sending SIGTERM to PID 1)
    auto init_ptr = &init;
    m_init_shutdown = [init_ptr]() {
        init_ptr->signalShutdown();
    };
    
    init.start();
    
    // Init has exited - remove from process table
    if (m_proc_manager.process_exists(1)) {
        m_proc_manager.stop_process(1);
    }
    
    // After init exits, stop kernel event loop
    kernel_running.store(false);
    queue_cv.notify_one();
    
    if (kernel_thread.joinable()) {
        kernel_thread.join();
    }
    
    LOG_INFO("KERNEL", "Shutdown complete");
}

} // namespace kernel