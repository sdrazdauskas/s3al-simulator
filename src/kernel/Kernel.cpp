#include "kernel/Kernel.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include "terminal/Terminal.h"
#include "shell/Shell.h"
#include "logger/Logger.h"
#include "kernel/SysCalls.h"
#include "shell/CommandsInit.h"
#include "shell/CommandAPI.h"
#include "init/Init.h"
#include "config/Config.h"

namespace kernel {

Kernel::Kernel(const config::Config& config)
        : cpuScheduler(),
            memManager(config.memorySize),
            procManager(memManager, cpuScheduler) {
    auto loggerCallback = [](const std::string& level, const std::string& module, const std::string& message){
        logging::Logger::getInstance().log(level, module, message);
    };

    procManager.setLogCallback(loggerCallback);
    cpuScheduler.setLogCallback(loggerCallback);
    storageManager.setLogCallback(loggerCallback);
    memManager.setLogCallback(loggerCallback);
    
    // Configure scheduler from config
    switch (config.schedulerAlgorithm) {
        case config::SchedulerAlgorithm::FCFS:
            cpuScheduler.setAlgorithm(scheduler::Algorithm::FCFS);
            break;
        case config::SchedulerAlgorithm::RoundRobin:
            cpuScheduler.setAlgorithm(scheduler::Algorithm::RoundRobin);
            break;
        case config::SchedulerAlgorithm::Priority:
            cpuScheduler.setAlgorithm(scheduler::Algorithm::Priority);
            break;
    }
    cpuScheduler.setQuantum(config.schedulerQuantum);
    cpuScheduler.setCyclesPerInterval(config.cyclesPerTick);
    cpuScheduler.setTickIntervalMs(config.tickIntervalMs);

    std::cout << "Kernel initialized with scheduler: " 
              << scheduler::algorithmToString(cpuScheduler.getAlgorithm())
              << " (quantum=" << cpuScheduler.getQuantum() 
              << ", cycles/tick=" << cpuScheduler.getCyclesPerInterval()
              << ", tick=" << cpuScheduler.getTickIntervalMs() << "ms)"
              << std::endl;
}

shell::SysApi::SysInfo Kernel::getSysInfo() const {
    shell::SysApi::SysInfo info;
    info.totalMemory = memManager.getTotalMemory();
    info.usedMemory = memManager.getUsedMemory();
    return info;
}


std::string Kernel::executeCommand(const std::string& line) {
    if (!line.empty() && line.back() == '\n') {
        return processLine(line.substr(0, line.size() - 1));
    }
    return processLine(line);
}

std::string Kernel::executeCommand(const std::string& cmd, const std::vector<std::string>& args) {
    std::string line = cmd;
    for (const auto& arg : args) line += " " + arg;
    return executeCommand(line);
}

bool Kernel::isKernelRunning() const { return kernelRunning.load(); }

std::string Kernel::processLine(const std::string& line) {
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
    const int memory_required = 1024 * arg_count;
    if (procManager.submit(command_name, cpu_required, memory_required, 0) != -1) {
        return "OK";
    } else {
        return "Error: Unable to execute process for command '" + command_name + "'.";
    }

    return "Unknown command: '" + command_name + "'.";
}

std::string Kernel::handleQuit(const std::vector<std::string>& args){
    (void)args;
    LOG_INFO("KERNEL", "Shutdown requested");
    
    kernelRunning.store(false);
    
    // Signal init to shutdown (like kernel sending SIGTERM to PID 1)
    if (initShutdownCb) {
        initShutdownCb();
    }
    
    // Submit shutdown event to wake up kernel thread
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        eventQueue.push({KernelEvent::Type::SHUTDOWN, ""});
    }
    queueCondition.notify_one();
    
    return "Shutting down kernel.";
}

void Kernel::submitCommand(const std::string& line) {
    std::lock_guard<std::mutex> lock(queueMutex);
    eventQueue.push({KernelEvent::Type::COMMAND, line});
    queueCondition.notify_one();
}

void Kernel::handleInterruptSignal(int signal) {
    LOG_INFO("KERNEL", "Received interrupt signal: " + std::to_string(signal));
    
    // Set interrupt flag immediately for responsiveness
    // Commands check this flag and should exit promptly
    shell::g_interrupt_requested.store(true);
    
    // Also submit to kernel queue for proper processing
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        KernelEvent event;
        event.type = KernelEvent::Type::INTERRUPT_SIGNAL;
        event.signalNumber = signal;
        eventQueue.push(event);
    }
    queueCondition.notify_one();
}

bool Kernel::sendSignalToProcess(int pid, int signal) {
    return procManager.sendSignal(pid, signal);
}

int Kernel::forkProcess(const std::string& name, int cpuTimeNeeded, int memoryNeeded, int priority, bool persistent) {
    return procManager.submit(name, cpuTimeNeeded, memoryNeeded, priority, persistent);
}

std::vector<shell::SysApi::ProcessInfo> Kernel::getProcessList() const {
    auto processes = procManager.snapshot();
    std::vector<shell::SysApi::ProcessInfo> result;
    
    for (const auto& proc : processes) {
        shell::SysApi::ProcessInfo info;
        info.pid = proc.getPid();
        info.name = proc.getName();
        info.state = process::stateToString(proc.getState());
        info.priority = proc.getPriority();
        result.push_back(info);
    }
    
    return result;
}

int Kernel::submitAsyncCommand(const std::string& name, int cpuCycles, int priority) {
    // Submit directly to scheduler (no memory allocation needed for command execution)
    int pid = procManager.submit(name, cpuCycles, priority);
    LOG_DEBUG("KERNEL", "Submitted async command '" + name + "' (PID=" + std::to_string(pid) + 
              ", cycles=" + std::to_string(cpuCycles) + ")");
    return pid;
}

bool Kernel::waitForProcess(int pid) {
    // Poll until process completes
    // The kernel event loop is running in another thread and calling scheduler tick
    while (!isProcessComplete(pid)) {
        // Check for interrupt
        if (shell::g_interrupt_requested.load()) {
            LOG_DEBUG("KERNEL", "Process " + std::to_string(pid) + " interrupted by user");
            cpuScheduler.remove(pid);
            return false;
        }
        
        // Small sleep to avoid busy-waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return true;
}

bool Kernel::isProcessComplete(int pid) const {
    // Process is complete if it's not in the scheduler anymore
    return cpuScheduler.getRemainingCycles(pid) < 0;
}

int Kernel::getProcessRemainingCycles(int pid) const {
    return cpuScheduler.getRemainingCycles(pid);
}

void Kernel::processEvent(const KernelEvent& event) {
    switch (event.type) {
        case KernelEvent::Type::COMMAND:
            LOG_DEBUG("KERNEL", "Processing command: " + event.data);
            // Command processing happens in shell, we just logged it
            break;
            
        case KernelEvent::Type::TIMER_TICK:
            handleTimerTick();
            break;
            
        case KernelEvent::Type::INTERRUPT_SIGNAL:
            LOG_DEBUG("KERNEL", "Processing interrupt signal: " + std::to_string(event.signalNumber));

            // Flag was already set immediately in handleInterruptSignal()
            // This event is for kernel bookkeeping and future process management
            break;
            
        case KernelEvent::Type::SHUTDOWN:
            LOG_INFO("KERNEL", "Shutdown event received");
            break;
    }
}

void Kernel::handleTimerTick() {
    // Run scheduler tick - this advances all queued processes
    if (cpuScheduler.hasWork()) {
        auto result = cpuScheduler.tick();
        
        if (result.processCompleted) {
            LOG_DEBUG("KERNEL", "Process " + std::to_string(result.completedPid) + " completed");
        }
        
        if (result.contextSwitch && result.currentPid >= 0) {
            LOG_DEBUG("KERNEL", "Context switch to process " + std::to_string(result.currentPid) + 
                      " (remaining=" + std::to_string(result.remainingCycles) + ")");
        }
    }
    
    // System monitoring
    static int tick_count = 0;
    static int last_logged_tick = 0;
    tick_count++;
    
    // Log system status periodically (every 50 ticks = ~5 seconds)
    if (tick_count - last_logged_tick >= 50) {
        size_t used_mem = memManager.getUsedMemory();
        size_t total_mem = memManager.getTotalMemory();
        double mem_usage = (double)used_mem / total_mem * 100.0;
        
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << mem_usage;
        LOG_DEBUG("KERNEL", "System status [tick:" + std::to_string(tick_count)
                + ", mem:" + oss.str() + "%]");
        last_logged_tick = tick_count;
    }
}

void Kernel::runEventLoop() {
    LOG_INFO("KERNEL", "Kernel event loop started");
    
    auto last_tick = std::chrono::steady_clock::now();
    // Use the scheduler's configured tick interval
    const auto tick_interval = std::chrono::milliseconds(cpuScheduler.getTickIntervalMs());
    
    while (kernelRunning.load()) {
        std::unique_lock<std::mutex> lock(queueMutex);
        
        // Wait for event or timeout
        if (queueCondition.wait_for(lock, tick_interval, [this] { 
            return !eventQueue.empty() || !kernelRunning.load(); 
        })) {
            // Process all pending events
            while (!eventQueue.empty()) {
                auto event = eventQueue.front();
                eventQueue.pop();
                lock.unlock();
                
                processEvent(event);
                
                lock.lock();
            }
        } else {
            // Timeout - generate timer tick
            lock.unlock();
            
            auto now = std::chrono::steady_clock::now();
            if (now - last_tick >= tick_interval) {
                processEvent({KernelEvent::Type::TIMER_TICK, ""});
                last_tick = now;
            }
        }
    }
    
    LOG_INFO("KERNEL", "Kernel event loop stopped");
}

void Kernel::boot(){
    LOG_INFO("KERNEL", "Booting s3al OS...");
    
    auto loggerCallback = [](const std::string& level, const std::string& module, const std::string& message){
        logging::Logger::getInstance().log(level, module, message);
    };
    
    // Set up signal callback so ProcessManager can notify daemon threads
    // Forward signals to the Init instance below (we set this after creating Init)
    
    // Start kernel event loop in a separate thread
    kernelRunning.store(true);
    kernelThread = std::thread([this]() {
        this->runEventLoop();
    });
    
    LOG_INFO("KERNEL", "Starting init process (PID 1)...");
    
    // Create init as actual process with PID 1 (persistent process)
    int init_pid = procManager.submit("init", 1, 1024, 10, true);
    if (init_pid != 1) {
        LOG_ERROR("KERNEL", "Failed to create init process");
        return;
    }
    
    // Create syscall interface for user-space processes
    SysApiKernel sys(storageManager, this);
    
    // Create and start init process (PID 1)
    init::Init init(sys);
    init.setLogCallback(loggerCallback);
    
    // Forward ProcessManager signals to this Init instance
    procManager.setSignalCallback([&init](int pid, int signal) {
        init.handleDaemonSignal(pid, signal);
    });

    // Store reference to init so kernel can signal it on shutdown
    auto init_ptr = &init;
    initShutdownCb = [init_ptr]() {
        init_ptr->signalShutdown();
    };
    
    init.start();
    
    // Init has exited - remove from process table
    if (procManager.processExists(1)) {
        procManager.sendSignal(1, 15);  // SIGTERM
    }
    
    // After init exits, stop kernel event loop
    kernelRunning.store(false);
    queueCondition.notify_one();
    
    if (kernelThread.joinable()) {
        kernelThread.join();
    }
    
    LOG_INFO("KERNEL", "Shutdown complete");
}

} // namespace kernel