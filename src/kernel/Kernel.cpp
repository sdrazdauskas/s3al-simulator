#include "Kernel.h"

#include <iomanip>
#include "Terminal.h"
#include "Shell.h"
#include <iostream>
#include <sstream>
#include <numeric>

using namespace std;
using namespace storage; // for convenience; you can also fully qualify

Kernel::Kernel() : m_is_running(true), m_scheduler(), m_next_pid(1) {
    register_commands();
    cout << "Kernel initialized." << endl;
}

// New public method to be called by the Terminal's callback
std::string Kernel::execute_command(const std::string& line) {
    // Remove newline if present
    if (!line.empty() && line.back() == '\n') {
        return process_line(line.substr(0, line.length() - 1));
    }
    return process_line(line);
}

// Overload for command + args (for Shell to call directly)
std::string Kernel::execute_command(const std::string& cmd, const std::vector<std::string>& args) {
    std::string line = cmd;
    for (const auto& arg : args) {
        line += " " + arg;
    }
    return execute_command(line);
}

bool Kernel::is_running() const {
    return m_is_running;
}

void Kernel::register_commands() {
    // existing commands
    m_commands["help"] = [this](const auto& args) { return this->handle_help(args); };
    m_commands["echo"] = [this](const auto& args) { return this->handle_echo(args); };
    m_commands["add"] = [this](const auto& args) { return this->handle_add(args); };
    m_commands["quit"] = [this](const auto& args) { return this->handle_quit(args); };
    m_commands["exit"] = [this](const auto& args) { return this->handle_quit(args); };

    // storage commands (no prefix - direct as requested)
    m_commands["touch"] = [this](const auto& args) { return this->handle_touch(args); };
    m_commands["rm"] = [this](const auto& args) { return this->handle_rm(args); };
    m_commands["write"] = [this](const auto& args) { return this->handle_write(args); };
    m_commands["cat"] = [this](const auto& args) { return this->handle_cat(args); };
    m_commands["edit"] = [this](const auto& args) { return this->handle_edit(args); };
    m_commands["mkdir"] = [this](const auto& args) { return this->handle_mkdir(args); };
    m_commands["rmdir"] = [this](const auto& args) { return this->handle_rmdir(args); };
    m_commands["cd"] = [this](const auto& args) { return this->handle_cd(args); };
    m_commands["ls"] = [this](const auto& args) { return this->handle_ls(args); };
    m_commands["pwd"] = [this](const auto& args) { return this->handle_pwd(args); };
}

static int estimate_burst_for_action(const std::string& cmd, const std::vector<std::string>& args) {
    if (cmd == "write" || cmd == "edit") {
        size_t total_chars = 0;
        for (size_t i = 1; i < args.size(); ++i) total_chars += args[i].size();
        int b = static_cast<int>(total_chars / 10) + 1;
        if (b < 1) b = 1;
        return b;
    }
    if (cmd == "touch" || cmd == "ls" || cmd == "pwd") return 1;
    if (cmd == "rm" || cmd == "rmdir" || cmd == "mkdir" || cmd == "cd" || cmd == "cat") return 2;
    return 1;
}

static int default_priority_for_action(const std::string& cmd) {
    if (cmd == "write" || cmd == "edit") return 2;
    return 1;
}

std::string Kernel::process_line(const std::string& line) {
    if (line.empty()) return "";

    std::istringstream iss(line);
    std::string command_name;
    iss >> command_name;

    std::vector<std::string> args;
    std::string token;
    // parsing that supports quoted args
    while (iss >> token) {
        if (!token.empty() && token.front() == '"') {
            std::string quoted_arg = token.substr(1);
            while (iss && (quoted_arg.empty() || quoted_arg.back() != '"')) {
                if (!(iss >> token)) break;
                quoted_arg += " " + token;
            }
            if (!quoted_arg.empty() && quoted_arg.back() == '"') quoted_arg.pop_back();
            args.push_back(quoted_arg);
        } else {
            args.push_back(token);
        }
    }

    auto it = m_commands.find(command_name);
    if (it != m_commands.end()) {
        std::string res = it->second(args);

        // Scheduler integration: create a process for storage commands
        if (command_name == "touch" || command_name == "rm" || command_name == "write" ||
            command_name == "cat" || command_name == "edit" || command_name == "mkdir" ||
            command_name == "rmdir" || command_name == "cd" || command_name == "ls" ||
            command_name == "pwd") {

            int burst = estimate_burst_for_action(command_name, args);
            int prio = default_priority_for_action(command_name);
            int pid = m_next_pid++;
            m_scheduler.execute_process(pid, burst, prio);
        }

        return res;
    }

    return "Unknown command: '" + command_name + "'.";
}

// Handler implementations

std::string Kernel::handle_help(const std::vector<std::string>& args) {
    (void)args;

    struct CmdInfo {
        std::string name;
        std::string params;
        std::string desc;
    };

    std::vector<CmdInfo> commands = {
        {"help",  "", "Display this help message"},
        {"echo",  "[text]", "Repeat the text back"},
        {"add",   "[num1] [num2] ...", "Sum the numbers"},
        {"quit",  "", "Exit the kernel"},
        {"exit",  "", "Alias for quit"},
        {"touch", "[filename]", "Create a new empty file"},
        {"rm",    "[filename]", "Delete a file"},
        {"write", "[filename] [text]", "Write text to a file (overwrite)"},
        {"cat",   "[filename]", "Display file contents"},
        {"edit",  "[filename] [text]", "Append text to a file"},
        {"mkdir", "[foldername]", "Create a new folder"},
        {"rmdir", "[foldername]", "Remove an empty folder"},
        {"cd",    "[foldername|..]", "Change current directory"},
        {"ls",    "", "List contents of current directory"},
        {"pwd",   "", "Show current directory path"}
    };

    size_t max_name_len = 4;
    size_t max_param_len = 9;
    for (auto& cmd : commands) {
        if (cmd.name.size() > max_name_len) max_name_len = cmd.name.size();
        if (cmd.params.size() > max_param_len) max_param_len = cmd.params.size();
    }

    auto draw_line = [&]() {
        return "+" + std::string(max_name_len + 2, '-') + "+"
               + std::string(max_param_len + 2, '-') + "+"
               + std::string(50, '-') + "+\n";
    };

    std::ostringstream oss;
    oss << draw_line();
    oss << "| " << std::left << std::setw(max_name_len) << "Name" << " | "
        << std::left << std::setw(max_param_len) << "Parameters" << " | "
        << "Description" << " |\n";
    oss << draw_line();

    for (auto& cmd : commands) {
        oss << "| " << std::left << std::setw(max_name_len) << cmd.name << " | "
            << std::left << std::setw(max_param_len) << cmd.params << " | "
            << cmd.desc << " |\n";
    }
    oss << draw_line();

    return oss.str();
}


std::string Kernel::handle_echo(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "Usage: echo [text to repeat]";
    }
    std::string result;
    for (size_t i = 0; i < args.size(); ++i) {
        result += args[i] + (i == args.size() - 1 ? "" : " ");
    }
    return result;
}

std::string Kernel::handle_add(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "Usage: add [number1] [number2] ...";
    }
    double sum = 0.0;
    for (const auto& arg : args) {
        try {
            sum += std::stod(arg);
        } catch (const std::invalid_argument& e) {
            return "Error: '" + arg + "' is not a valid number.";
        }
    }
    return "Sum: " + std::to_string(sum);
}

std::string Kernel::handle_quit(const std::vector<std::string>& args) {
    m_is_running = false;
    return "Shutting down kernel. Goodbye!";
}

// ---------------- Storage handlers ----------------

std::string Kernel::handle_touch(const std::vector<std::string>& args) {
    if (args.size() < 1) return "Usage: touch <filename>";
    auto res = m_storage.createFile(args[0]);
    return "[Kernel] " + storage::StorageManager::toString(res);
}

std::string Kernel::handle_rm(const std::vector<std::string>& args) {
    if (args.size() < 1) return "Usage: rm <filename>";
    auto res = m_storage.deleteFile(args[0]);
    return "[Kernel] " + storage::StorageManager::toString(res);
}

std::string Kernel::handle_write(const std::vector<std::string>& args) {
    // write <filename> <content...>
    if (args.size() < 2) return "Usage: write <filename> <content>";
    const std::string& fname = args[0];
    std::string content;
    for (size_t i = 1; i < args.size(); ++i) {
        content += args[i];
        if (i + 1 < args.size()) content += " ";
    }
    auto exists = m_storage.fileExists(fname);
    if (exists != storage::StorageManager::StorageResponse::OK) {
        return "[Kernel] " + storage::StorageManager::toString(exists);
    }
    auto res = m_storage.writeFile(fname, content);
    return "[Kernel] " + storage::StorageManager::toString(res);
}

std::string Kernel::handle_cat(const std::vector<std::string>& args) {
    if (args.size() < 1) return "Usage: cat <filename>";
    std::string out;
    auto res = m_storage.readFile(args[0], out);
    if (res != storage::StorageManager::StorageResponse::OK) {
        return "[Kernel] " + storage::StorageManager::toString(res);
    }
    return out;
}

std::string Kernel::handle_edit(const std::vector<std::string>& args) {
    // inline append: edit <filename> <content...>
    if (args.size() < 2) return "Usage: edit <filename> <content to append>";
    const std::string& fname = args[0];
    std::string content;
    for (size_t i = 1; i < args.size(); ++i) {
        content += args[i];
        if (i + 1 < args.size()) content += " ";
    }
    auto exists = m_storage.fileExists(fname);
    if (exists != storage::StorageManager::StorageResponse::OK) {
        return "[Kernel] " + storage::StorageManager::toString(exists);
    }
    auto res = m_storage.appendToFile(fname, content);
    return "[Kernel] " + storage::StorageManager::toString(res);
}

std::string Kernel::handle_mkdir(const std::vector<std::string>& args) {
    if (args.size() < 1) return "Usage: mkdir <foldername>";
    auto res = m_storage.makeDir(args[0]);
    return "[Kernel] " + storage::StorageManager::toString(res);
}

std::string Kernel::handle_rmdir(const std::vector<std::string>& args) {
    if (args.size() < 1) return "Usage: rmdir <foldername>";
    auto res = m_storage.removeDir(args[0]);
    return "[Kernel] " + storage::StorageManager::toString(res);
}

std::string Kernel::handle_cd(const std::vector<std::string>& args) {
    if (args.size() < 1) return "Usage: cd <foldername | ..>";
    auto res = m_storage.changeDir(args[0]);
    return "[Kernel] " + storage::StorageManager::toString(res);
}

std::string Kernel::handle_ls(const std::vector<std::string>& args) {
    (void)args;
    auto entries = m_storage.listDir();
    std::ostringstream out;
    out << "=== Contents of " << m_storage.getWorkingDir() << " ===\n";
    if (entries.empty()) out << "(empty)\n";
    for (const auto& e : entries) out << e << "\n";
    return out.str();
}

std::string Kernel::handle_pwd(const std::vector<std::string>& args) {
    (void)args;
    return m_storage.getWorkingDir();
}

// ----------------- boot -----------------
void Kernel::boot() {
    std::cout << "Booting s3al OS...\n";

    // Initialize shell subsystem with kernel callback
    std::cout << "Initializing shell subsystem...\n";
    shell::Shell sh([this](const std::string& cmd, const std::vector<std::string>& args) {
        return this->execute_command(cmd, args);
    });

    std::cout << "Initializing terminal subsystem...\n";
    terminal::Terminal term;

    // Set up send callback: process command and print output
    term.setSendCallback([this, &sh, &term](const std::string& line) {
        std::string result = sh.processCommandLine(line);
        if (!result.empty()) {
            term.print(result);
            if (result.back() != '\n') {
                term.print("\n");
            }
        }
    });

    // Signal handling (Ctrl+C)
    term.setSignalCallback([&term](int sig) {
        term.print("^C\n");
    });

    std::cout << "\nStarting init process...\n";
    std::cout << "Type 'help' for commands, 'quit' to exit\n\n";

    // Main CLI loop with dynamic prompt
    while (m_is_running) {
        // Print prompt with current working directory
        std::cout << m_storage.getWorkingDir() << "$ ";
        std::string line;
        std::getline(std::cin, line);

        if (line.empty()) continue;

        std::string output = execute_command(line);
        if (!output.empty()) std::cout << output << "\n";
    }

    std::cout << "\nShutdown complete.\n";
}
