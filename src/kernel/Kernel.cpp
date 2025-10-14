#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <sstream>

class Kernel {
public:
    Kernel() : is_running(true) {
        register_commands();
        std::cout << "Kernel initialized. Type 'help' for commands or 'quit' to exit." << std::endl;
    }

    void run() {
        std::string line;
        while (is_running) {
            std::cout << "> ";
            std::getline(std::cin, line);
            if (line.empty()) {
                continue;
            }
            std::cout << process_line(line) << std::endl;
        }
    }

private:
    using CommandHandler = std::function<std::string(const std::vector<std::string>&)>;
    std::map<std::string, CommandHandler> commands;
    bool is_running;

    void register_commands() {
        commands["help"] = [this](const auto& args) { return this->handle_help(args); };
        commands["echo"] = [this](const auto& args) { return this->handle_echo(args); };
        commands["add"] = [this](const auto& args) { return this->handle_add(args); };
        commands["quit"] = [this](const auto& args) { return this->handle_quit(args); };
        commands["exit"] = [this](const auto& args) { return this->handle_quit(args); };
    }

    std::string process_line(const std::string& line) {
        std::istringstream iss(line);
        std::string command_name;
        iss >> command_name;

        std::vector<std::string> args;
        std::string token;
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

        auto it = commands.find(command_name);
        if (it != commands.end()) {
            return it->second(args);
        }

        return "Unknown command: '" + command_name + "'.";
    }

    std::string handle_help(const std::vector<std::string>& args) {
        std::string help_text = "Available commands:\n";
        for (const auto& pair : commands) {
            help_text += "- " + pair.first + "\n";
        }
        return help_text;
    }

    std::string handle_echo(const std::vector<std::string>& args) {
        if (args.empty()) {
            return "Usage: echo [text to repeat]";
        }
        std::string result;
        for (size_t i = 0; i < args.size(); ++i) {
            result += args[i] + (i == args.size() - 1 ? "" : " ");
        }
        return result;
    }

    std::string handle_add(const std::vector<std::string>& args) {
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

    std::string handle_quit(const std::vector<std::string>& args) {
        is_running = false;
        return "Shutting down kernel. Goodbye!";
    }
};

int main() {
    Kernel my_kernel;
    my_kernel.run();
    return 0;
}