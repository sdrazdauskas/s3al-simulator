#include "kernel/Kernel.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include "terminal/Terminal.h"
#include "logger/Logger.h"
#include "kernel/SysCalls.h"
#include "shell/Shell.h"
#include "shell/CommandsInit.h"
#include "shell/CommandAPI.h"
#include "init/Init.h"
#include "config/Config.h"

namespace kernel {

Kernel::Kernel(const config::Config& config)
        : cpuScheduler(config),
            memManager(config.memorySize),
            procManager(memManager, cpuScheduler) {
    auto loggerCallback = [](const std::string& level, const std::string& module, const std::string& message){
        logging::Logger::getInstance().log(level, module, message);
    };

    procManager.setLogCallback(loggerCallback);
    cpuScheduler.setLogCallback(loggerCallback);
    storageManager.setLogCallback(loggerCallback);
    memManager.setLogCallback(loggerCallback);
}

sys::SysApi::SysInfo Kernel::getSysInfo() const {
    sys::SysApi::SysInfo info;
    info.totalMemory = memManager.getTotalMemory();
    info.usedMemory = memManager.getUsedMemory();
    return info;
}

void* Kernel::allocateMemory(size_t size, int processId) {
    return memManager.allocate(size, processId);
}

void Kernel::deallocateMemory(void* ptr) {
    memManager.deallocate(ptr);
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
    logInfo("Shutdown requested");
    
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
    logInfo("Received interrupt signal: " + std::to_string(signal));
    
    // Set interrupt flag immediately for responsiveness
    // Commands check this flag and should exit promptly
    shell::interruptRequested.store(true);
    
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

std::vector<sys::SysApi::ProcessInfo> Kernel::getProcessList() const {
    auto processes = procManager.snapshot();
    std::vector<sys::SysApi::ProcessInfo> result;
    
    for (const auto& proc : processes) {
        sys::SysApi::ProcessInfo info;
        info.pid = proc.getPid();
        info.name = proc.getName();
        info.state = process::stateToString(proc.getState());
        info.priority = proc.getPriority();
        result.push_back(info);
    }
    
    return result;
}

bool Kernel::processExists(int pid) const {
    return procManager.processExists(pid);
}

int Kernel::submitAsyncCommand(const std::string& name, int cpuCycles, int priority) {
    // Submit directly to scheduler (no memory allocation needed for command execution)
    int pid = procManager.submit(name, cpuCycles, priority);
    logDebug("Submitted async command '" + name + "' (PID=" + std::to_string(pid) + 
              ", cycles=" + std::to_string(cpuCycles) + ")");
    return pid;
}

bool Kernel::addCPUWork(int pid, int cpuCycles) {
    // Try to add cycles to existing scheduler entry first
    bool success = cpuScheduler.addCycles(pid, cpuCycles);
    
    if (!success) {
        // Process not in scheduler - get priority from process manager and enqueue
        auto procs = procManager.snapshot();
        for (const auto& p : procs) {
            if (p.getPid() == pid) {
                cpuScheduler.enqueue(pid, cpuCycles, p.getPriority());
                logDebug("Re-enqueued process PID=" + std::to_string(pid) + 
                    " with " + std::to_string(cpuCycles) + " cycles (priority=" + 
                    std::to_string(p.getPriority()) + ")");
                return true;
            }
        }
        logWarn("Failed to add CPU cycles to non-existent process PID=" + std::to_string(pid));
        return false;
    }
    
    logDebug("Added " + std::to_string(cpuCycles) + " CPU cycles to process PID=" + std::to_string(pid));
    return true;
}

bool Kernel::waitForProcess(int pid) {
    // Get remaining cycles - if not in scheduler, already complete
    int remaining = cpuScheduler.getRemainingCycles(pid);
    if (remaining < 0) {
        return true;
    }
    
    // Wait until all cycles are consumed
    std::unique_lock<std::mutex> lock(cycleWaitMutex);
    while (cpuScheduler.getRemainingCycles(pid) >= 0) {
        // Check if kernel is shutting down
        if (!kernelRunning.load()) {
            logDebug("Cycle wait for PID=" + std::to_string(pid) + " interrupted by kernel shutdown");
            return false;
        }
        
        // Check for user interrupt
        if (shell::interruptRequested.load()) {
            logDebug("Cycle wait for PID=" + std::to_string(pid) + " interrupted by user");
            // Remove pending CPU work from scheduler
            cpuScheduler.remove(pid);
            
            // Only kill non-persistent processes (external commands)
            // Persistent processes (shell, daemons) should continue running
            if (!isProcessPersistent(pid)) {
                memManager.freeProcessMemory(pid);
                procManager.sendSignal(pid, 9); // SIGKILL
            }
            return false;
        }
        
        // Wait for scheduler tick notification
        cycleWaitCV.wait(lock);
    }
    
    return true;
}

bool Kernel::isProcessPersistent(int pid) const {
    return procManager.isProcessPersistent(pid);
}

bool Kernel::exit(int pid, int exitCode) {
    return procManager.exit(pid, exitCode);
}

bool Kernel::reapProcess(int pid) {
    return procManager.reapProcess(pid);
}

bool Kernel::isProcessComplete(int pid) const {
    // Process is complete if it's not in the scheduler anymore
    return cpuScheduler.getRemainingCycles(pid) < 0;
}

int Kernel::getProcessRemainingCycles(int pid) const {
    return cpuScheduler.getRemainingCycles(pid);
}

bool Kernel::changeSchedulingAlgorithm(scheduler::SchedulerAlgorithm algo, int quantum) {
    return cpuScheduler.setAlgorithm(algo, quantum);
}

bool Kernel::setSchedulerCyclesPerInterval(int cycles) {
    cpuScheduler.setCyclesPerInterval(cycles);
    return true;
}

bool Kernel::setSchedulerTickIntervalMs(int ms) {
    cpuScheduler.setTickIntervalMs(ms);
    return true;
}

void Kernel::processEvent(const KernelEvent& event) {
    switch (event.type) {
        case KernelEvent::Type::COMMAND:
            logDebug("Processing command: " + event.data);
            // Command processing happens in shell, we just logged it
            break;
            
        case KernelEvent::Type::TIMER_TICK:
            handleTimerTick();
            break;
            
        case KernelEvent::Type::INTERRUPT_SIGNAL:
            logDebug("Processing interrupt signal: " + std::to_string(event.signalNumber));

            // Flag was already set immediately in handleInterruptSignal()
            // This event is for kernel bookkeeping and future process management
            break;
            
        case KernelEvent::Type::SHUTDOWN:
            logInfo("Shutdown event received");
            break;
    }
}

void Kernel::handleTimerTick() {
    // Run scheduler tick - this advances all queued processes
    if (cpuScheduler.hasWork()) {
        auto result = cpuScheduler.tick();
        
        // Notify any threads waiting for cycle consumption
        {
            std::lock_guard<std::mutex> lock(cycleWaitMutex);
            cycleWaitCV.notify_all();
        }
        
    }
    
    // System monitoring
    static int tickCount = 0;
    static int lastLoggedTick = 0;
    tickCount++;
    
    // Log system status periodically (every 50 ticks = ~5 seconds)
    if (tickCount - lastLoggedTick >= 50) {
        size_t usedMem = memManager.getUsedMemory();
        size_t totalMem = memManager.getTotalMemory();
        double mem_usage = (double)usedMem / totalMem * 100.0;
        
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << mem_usage;
        logDebug("System status [tick:" + std::to_string(tickCount)
                + ", mem:" + oss.str() + "%]");
        lastLoggedTick = tickCount;
    }
}

void Kernel::runEventLoop() {
    logInfo("Kernel event loop started");
    
    auto lastTick = std::chrono::steady_clock::now();
    while (kernelRunning.load()) {
        // Always get the latest tick interval
        auto tickInterval = std::chrono::milliseconds(cpuScheduler.getTickIntervalMs());
        std::unique_lock<std::mutex> lock(queueMutex);

        // Wait for event or timeout
        if (queueCondition.wait_for(lock, tickInterval, [this] {
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
            if (now - lastTick >= tickInterval) {
                processEvent({KernelEvent::Type::TIMER_TICK, ""});
                lastTick = now;
            }
        }
    }
    
    logInfo("Kernel event loop stopped");
}

void Kernel::boot(){
    logging::logInfo("KERNEL", "Booting s3al OS...");
    
    auto loggerCallback = [](const std::string& level, const std::string& module, const std::string& message){
        logging::Logger::getInstance().log(level, module, message);
    };
    
    // Start kernel event loop in a separate thread
    kernelRunning.store(true);
    kernelThread = std::thread([this]() {
        this->runEventLoop();
    });
    
    logInfo("Starting init process (PID 1)...");
    
    // Create init as actual process with PID 1 (persistent process)
    int initPid = procManager.submit("init", 1, 1024, 10, true);
    if (initPid != 1) {
        logError("Failed to create init process");
        return;
    }
    
    // Create syscall interface for user-space processes
    SysApiKernel sys(storageManager, this);
    
    // Wire storage to use syscalls for memory management
    storageManager.setSysApi(&sys);
    
    // Create and start init process (PID 1)
    init::Init init(sys);
    init.setLogCallback(loggerCallback);
    
    // Forward all process signals to init (daemons + shell handling)
    procManager.setSignalCallback([&init](int pid, int signal) {
        init.handleProcessSignal(pid, signal);
    });

    // Notify init on process completion/termination
    procManager.setProcessCompleteCallback([&init](int pid, int exitCode) {
        init.handleProcessSignal(pid, exitCode);
    });

    // Store reference to init so kernel can signal it on shutdown
    auto initPtr = &init;
    initShutdownCb = [initPtr]() {
        initPtr->signalShutdown();
    };
    
    init.start();
    
    
    // After init exits, stop kernel event loop
    kernelRunning.store(false);
    queueCondition.notify_one();
    
    if (kernelThread.joinable()) {
        kernelThread.join();
    }
    
    logInfo("Shutdown complete");
}

} // namespace kernel