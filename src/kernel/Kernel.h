#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <Storage.h>
#include <MemoryManager.h>
#include <ProcessManager.h>
#include <Scheduler.h>
#include "SysCallsAPI.h"

// Forward declaration
namespace config { struct Config; }

namespace kernel {

class Kernel {
public:
    using CommandHandler = std::function<std::string(const std::vector<std::string>&)>;

    explicit Kernel(const config::Config& config);

    /**
     * @brief Processes a single line of input as a command and returns the result.
     * @param line The full command line to execute.
     * @return The output of the command.
     */
    std::string executeCommand(const std::string& line);

    /**
     * @brief Executes a command with separated command name and arguments.
     * @param cmd The command name.
     * @param args The command arguments.
     * @return The output of the command.
     */
    std::string executeCommand(const std::string& cmd, const std::vector<std::string>& args);

    /**
     * @brief Checks if the kernel is still in a running state.
     * @return true if the kernel should continue running, false otherwise.
     */
    bool isKernelRunning() const;

    /**
     * @brief Boot sequence: initialize subsystems, wire components, start init process.
     * This is the kernel entry point that sets up the entire system.
     */
    void boot();

    shell::SysApi::SysInfo getSysInfo() const;

    std::string handleQuit(const std::vector<std::string>& args);
    
    // Signal handling - kernel receives interrupts from hardware/terminal
    void handleInterruptSignal(int signal);
    
    // Process signal handling
    bool sendSignalToProcess(int pid, int signal);
    
    // Process creation - creates a process entry, returns PID
    int forkProcess(const std::string& name, int cpuTimeNeeded, int memoryNeeded, int priority = 0, bool persistent = false);
    
    // Get list of all processes
    std::vector<shell::SysApi::ProcessInfo> getProcessList() const;
    
    // Kernel event loop - runs background tasks
    void runEventLoop();
    
    // Submit a command to be processed by the kernel
    void submitCommand(const std::string& line);
    
    // Submit a command for scheduler-based execution
    int submitAsyncCommand(const std::string& name, int cpuCycles, int priority = 0);
    
    // Wait for a command process to complete (blocks)
    bool waitForProcess(int pid);
    
    // Check if a process has completed
    bool isProcessComplete(int pid) const;
    
    // Get remaining cycles for a process
    int getProcessRemainingCycles(int pid) const;

private:
    struct KernelEvent {
        enum class Type {
            COMMAND,
            TIMER_TICK,
            INTERRUPT_SIGNAL,
            SHUTDOWN
        };
        
        Type type;
        std::string data;
        int signalNumber{0}; // For INTERRUPT_SIGNAL events
    };
    
    void processEvent(const KernelEvent& event);
    void handleTimerTick();
    
    std::queue<KernelEvent> eventQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    std::atomic<bool> kernelRunning{true};
    std::thread kernelThread;
    
    std::string processLine(const std::string& line);

    // Callback to signal init process to shutdown
    std::function<void()> initShutdownCb;
    
    storage::StorageManager storageManager;
    memory::MemoryManager memManager;
    scheduler::CPUScheduler cpuScheduler;
    process::ProcessManager procManager;
};

} // namespace kernel
