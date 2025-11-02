#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <Storage.h>
#include <MemoryManager.h>
#include <ProcessManager.h>
#include <Scheduler.h>

class Kernel {
public:
    using CommandHandler = std::function<std::string(const std::vector<std::string>&)>;

    Kernel();

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

    // Kernel command handlers exposed for direct invocation by syscalls.
    std::string handle_quit(const std::vector<std::string>& args);

private:
    void register_commands();
    std::string process_line(const std::string& line);
    std::string handle_meminfo(const std::vector<std::string>& args);
    std::string handle_membar(const std::vector<std::string>& args);

    std::map<std::string, CommandHandler> m_commands;
    bool m_is_running;

    storage::StorageManager m_storage;
    memory::MemoryManager m_mem_mgr;
    scheduler::CPUScheduler m_scheduler;
    process::ProcessManager m_proc_manager;
};
