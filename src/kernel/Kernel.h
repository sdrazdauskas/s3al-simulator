#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include "../storage/Storage.h"

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

private:
    void register_commands();
    std::string process_line(const std::string& line);

    // Command handler methods
    std::string handle_help(const std::vector<std::string>& args);
    std::string handle_echo(const std::vector<std::string>& args);
    std::string handle_add(const std::vector<std::string>& args);
    std::string handle_quit(const std::vector<std::string>& args);

    // Storage-related handlers
    std::string handle_touch(const std::vector<std::string>& args);
    std::string handle_rm(const std::vector<std::string>& args);
    std::string handle_write(const std::vector<std::string>& args); // write (replace) file
    std::string handle_cat(const std::vector<std::string>& args);
    std::string handle_edit(const std::vector<std::string>& args);  // inline append (chosen B)
    std::string handle_mkdir(const std::vector<std::string>& args);
    std::string handle_rmdir(const std::vector<std::string>& args);
    std::string handle_cd(const std::vector<std::string>& args);
    std::string handle_ls(const std::vector<std::string>& args);
    std::string handle_pwd(const std::vector<std::string>& args);

    std::map<std::string, CommandHandler> m_commands;
    bool m_is_running;

    // Storage manager instance
    storage::StorageManager m_storage;
};
