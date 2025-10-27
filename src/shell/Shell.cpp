#include "Shell.h"
#include <sstream>

namespace shell {

Shell::Shell(KernelCallback cb)
    : kernelCallback(std::move(cb)) {}

std::string Shell::parseQuotedToken(std::istringstream& iss, std::string token) {
    std::string quoted = token.substr(1);

    while (!quoted.empty() && quoted.back() != '"' && iss >> token) {
        quoted += " " + token;
    }

    if (!quoted.empty() && quoted.back() == '"') {
        quoted.pop_back();
    }

    return quoted;
}

    std::vector<std::string> Shell::splitByAndOperator(const std::string& commandLine) {
    std::vector<std::string> commands;
    std::string temp = commandLine;
    size_t pos = 0;

    while ((pos = temp.find("&&")) != std::string::npos) {
        std::string part = temp.substr(0, pos);
        size_t start = part.find_first_not_of(" \t");
        size_t end = part.find_last_not_of(" \t");
        if (start != std::string::npos)
            commands.push_back(part.substr(start, end - start + 1));
        temp.erase(0, pos + 2);
    }
    size_t start = temp.find_first_not_of(" \t");
    size_t end = temp.find_last_not_of(" \t");
    if (start != std::string::npos)
        commands.push_back(temp.substr(start, end - start + 1));

    return commands;
}

    std::string Shell::processCommandLine(const std::string& commandLine) {
    if (commandLine.empty()) {
        return "Error: No command entered";
    }

    std::vector<std::string> commands = splitByAndOperator(commandLine);
    std::string combinedOutput;

    for (const auto& cmd : commands) {
        std::string command;
        std::vector<std::string> args;

        parseCommand(cmd, command, args);

        if (command.empty()) {
            continue;
        }

        std::string result = executeCommand(command, args);

        if (!combinedOutput.empty()) {
            combinedOutput += "\n";
        }
        combinedOutput += result;

        if (result.rfind("Error", 0) == 0) {
            break;
        }
    }

    return combinedOutput;
}

std::string Shell::executeCommand(const std::string& command, const std::vector<std::string>& args) {
    if (!kernelCallback) {
        return "Error: No kernel handler available";
    }

    if (command.empty()) {
        return "Error: No command specified";
    }

    return kernelCallback(command, args);
}

void Shell::parseCommand(const std::string& commandLine, std::string& command, std::vector<std::string>& args) {
    std::istringstream iss(commandLine);
    iss >> command;

    args.clear();
    std::string token;

    while (iss >> token) {
        if (token.front() == '"') {
            args.push_back(parseQuotedToken(iss, token));
        } else {
            args.push_back(token);
        }
    }
}

bool Shell::isConnectedToKernel() const {
    return kernelCallback != nullptr;
}

} // namespace shell