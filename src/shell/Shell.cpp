#include "Shell.h"
#include <sstream>
#include <iostream>

namespace shell {

Shell::Shell(SysApi& sys_, const CommandRegistry& reg)
    : sys(sys_), registry(reg) {}

void Shell::setLogCallback(LogCallback callback) {
    log_callback = callback;
}

void Shell::setOutputCallback(OutputCallback callback) {
    outputCallback = callback;
}

void Shell::log(const std::string& level, const std::string& message) {
    if (log_callback) {
        log_callback(level, "SHELL", message);
    }
}

std::string Shell::parseQuotedToken(std::istringstream& iss, std::string token) {
    std::string quoted = token.substr(1);
    std::string next;

    while (!quoted.empty() && quoted.back() != '"' && iss >> next)
        quoted += " " + next;

    if (!quoted.empty() && quoted.back() == '"')
        quoted.pop_back();

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

std::vector<std::string> Shell::splitByPipeOperator(const std::string& commandLine) {
    std::vector<std::string> parts;
    std::string temp = commandLine;
    size_t pos = 0;

    while ((pos = temp.find("|")) != std::string::npos) {
        std::string part = temp.substr(0, pos);
        size_t start = part.find_first_not_of(" \t");
        size_t end = part.find_last_not_of(" \t");
        if (start != std::string::npos)
            parts.push_back(part.substr(start, end - start + 1));
        temp.erase(0, pos + 1);
    }

    size_t start = temp.find_first_not_of(" \t");
    size_t end = temp.find_last_not_of(" \t");
    if (start != std::string::npos)
        parts.push_back(temp.substr(start, end - start + 1));

    return parts;
}

void Shell::processCommandLine(const std::string& commandLine) {
    if (commandLine.empty()) {
        log("DEBUG", "Empty command line received");
        if (outputCallback) outputCallback("Error: No command entered");
        return;
    }

    log("DEBUG", "Processing command: " + commandLine);

    std::vector<std::string> andCommands = splitByAndOperator(commandLine);
    std::string combinedOutput;

    for (const auto& andCmd : andCommands) {
        std::vector<std::string> pipeCommands = splitByPipeOperator(andCmd);
        std::string pipeInput;

        for (const auto& segment : pipeCommands) {
            std::string command;
            std::vector<std::string> args;
            parseCommand(segment, command, args);

            if (command.empty())
                continue;

            std::string result = executeCommand(command, args, pipeInput);
            pipeInput = result;
        }

        if (!combinedOutput.empty())
            combinedOutput += "\n";

        combinedOutput += pipeInput;

        if (pipeInput.rfind("Error", 0) == 0)
            break;
    }

    if (outputCallback && !combinedOutput.empty()) {
        outputCallback(combinedOutput);
    }
}

std::string Shell::executeCommand(const std::string& command, const std::vector<std::string>& args, const std::string& input) {

    if (command.empty()) {
        log("ERROR", "No command specified");
        return "Error: No command specified";
    }

    log("INFO", "Executing command: " + command);

    CommandFn fn = registry.find(command);
    if (!fn) {
        log("ERROR", "Unknown command: " + command);
        return "Error: Unknown command: " + command;
    }

    std::ostringstream out, err;
    int rc = fn(args, input, out, err, sys);

    std::string result;
    if (!err.str().empty()) {
        log("ERROR", err.str());
        result += err.str();
    }
    result += out.str();

    return result;
}

void Shell::parseCommand(const std::string& commandLine, std::string& command, std::vector<std::string>& args) {
    std::istringstream iss(commandLine);
    iss >> command;

    args.clear();
    std::string token;
    while (iss >> token) {
        if (token.front() == '"')
            args.push_back(parseQuotedToken(iss, token));
        else
            args.push_back(token);
    }
}

} // namespace shell