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
    std::string execute_command(const std::string& line);

    /**
     * @brief Executes a command with separated command name and arguments.
     * @param cmd The command name.
     * @param args The command arguments.
     * @return The output of the command.
     */
    std::string execute_command(const std::string& cmd, const std::vector<std::string>& args);

    /**
     * @brief Checks if the kernel is still in a running state.
     * @return true if the kernel should continue running, false otherwise.
     */
    bool is_running() const;

    /**
     * @brief Boot sequence: initialize subsystems, wire components, start init process.
     * This is the kernel entry point that sets up the entire system.
     */
    void boot();

    shell::SysApi::SysInfo getSysInfo() const;

    std::string handle_quit(const std::vector<std::string>& args);
    
    // Signal handling - kernel receives interrupts from hardware/terminal
    void handle_interrupt_signal(int signal);
    
    // Process signal handling
    bool send_signal_to_process(int pid, int signal);
    
    // Process creation - creates a process entry, returns PID
    int fork_process(const std::string& name, int cpuTimeNeeded, int memoryNeeded, int priority = 0, bool persistent = false);
    
    // Get list of all processes
    std::vector<shell::SysApi::ProcessInfo> get_process_list() const;
    
    // Set callback for daemon signal notifications
    void setDaemonSignalCallback(std::function<void(int pid, int signal)> callback) {
        m_daemon_signal_callback = callback;
    }

    // Kernel event loop - runs background tasks
    void run_event_loop();
    
    // Submit a command to be processed by the kernel
    void submit_command(const std::string& line);
    
    // Submit a command for scheduler-based execution
    int submit_async_command(const std::string& name, int cpuCycles, int priority = 0);
    
    // Wait for a command process to complete (blocks)
    bool wait_for_process(int pid);
    
    // Check if a process has completed
    bool is_process_complete(int pid) const;
    
    // Get remaining cycles for a process
    int get_process_remaining_cycles(int pid) const;

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
        int signal_number{0}; // For INTERRUPT_SIGNAL events
    };
    
    void process_event(const KernelEvent& event);
    void handle_timer_tick();
    
    std::queue<KernelEvent> event_queue;
    std::mutex queue_mutex;
    std::condition_variable queue_cv;
    std::atomic<bool> kernel_running{true};
    std::thread kernel_thread;
    
    std::string process_line(const std::string& line);
    std::string handle_meminfo(const std::vector<std::string>& args);
    std::string handle_membar(const std::vector<std::string>& args);

    std::map<std::string, CommandHandler> m_commands;
    bool m_is_running;
    
    // Signal callback from ProcessManager to Init (for daemon notification)
    std::function<void(int pid, int signal)> m_daemon_signal_callback;
    
    // Callback to signal init process to shutdown
    std::function<void()> m_init_shutdown;

    storage::StorageManager m_storage;
    memory::MemoryManager m_mem_mgr;
    scheduler::CPUScheduler m_scheduler;
    process::ProcessManager m_proc_manager;
};

} // namespace kernel
