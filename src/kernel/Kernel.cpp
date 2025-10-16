#include "Kernel.h"
#include <iostream>
#include <sstream>

Kernel::Kernel() : m_is_running(true) {
    register_commands();
    std::cout << "Kernel initialized. Type 'help' for commands or 'quit' to exit." << std::endl;
}

// New public method to be called by the Terminal's callback
std::string Kernel::execute_command(const std::string& line) {
    // We remove the newline character added by the Terminal class before processing
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

// The old run() method is removed completely.

void Kernel::register_commands() {
    m_commands["help"] = [this](const auto& args) { return this->handle_help(args); };
    m_commands["echo"] = [this](const auto& args) { return this->handle_echo(args); };
    m_commands["add"] = [this](const auto& args) { return this->handle_add(args); };
    m_commands["quit"] = [this](const auto& args) { return this->handle_quit(args); };
    m_commands["exit"] = [this](const auto& args) { return this->handle_quit(args); };
}

std::string Kernel::process_line(const std::string& line) {
    if (line.empty()) {
        return "";
    }
    
    std::istringstream iss(line);
    std::string command_name;
    iss >> command_name;

    std::vector<std::string> args;
    std::string token;
    // This parsing logic is kept as you wrote it. It works well.
    while (iss >> token) {
        if (token.front() == '"') {
            std::string quoted_arg = token.substr(1);
            while (iss && (quoted_arg.empty() || quoted_arg.back() != '"')) {
                if (!(iss >> token)) break;
                quoted_arg += " " + token;
            }
            if (!quoted_arg.empty() && quoted_arg.back() == '"') {
                quoted_arg.pop_back();
            }
            args.push_back(quoted_arg);
        } else {
            args.push_back(token);
        }
    }

    auto it = m_commands.find(command_name);
    if (it != m_commands.end()) {
        return it->second(args);
    }

    return "Unknown command: '" + command_name + "'.";
}

// Handler implementations remain the same.
std::string Kernel::handle_help(const std::vector<std::string>& args) {
    std::string help_text = "Available commands:\n";
    for (const auto& pair : m_commands) {
        help_text += "- " + pair.first + "\n";
    }
    // Remove the final newline for cleaner output
    if (!help_text.empty()) {
        help_text.pop_back();
    }
    return help_text;
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