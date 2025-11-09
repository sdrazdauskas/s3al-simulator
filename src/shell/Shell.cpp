#include "Shell.h"
#include <sstream>
#include <iostream>

namespace shell {

// Global interrupt flag for Ctrl+C handling
std::atomic<bool> g_interrupt_requested{false};

Shell::Shell(SysApi& sys_, const CommandRegistry& reg, KernelCallback kernelCb)
    : sys(sys_), registry(reg), kernelCallback(std::move(kernelCb)) {}

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

    std::istringstream checkStream(commandLine);
    std::string firstWord;
    checkStream >> firstWord;

    if (firstWord == "write" || firstWord == "edit") {
        std::string command;
        std::vector<std::string> args;
        parseCommand(commandLine, command, args);
        std::string result = executeCommand(command, args, "");
        if (outputCallback && !result.empty())
            outputCallback(result);
        return;
    }

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

std::string Shell::executeCommand(const std::string& command,
                                  const std::vector<std::string>& args,
                                  const std::string& input) {
    if (command.empty()) {
        log("ERROR", "No command specified");
        return "Error: No command specified";
    }

    log("INFO", "Executing command: " + command);

    if (command.rfind("./", 0) == 0) {
        std::string filename = command.substr(2);
        return executeScriptFile(filename);
    }

    std::vector<std::string> argsWithInput = args;
    if (!input.empty())
        argsWithInput.push_back(input);

    ICommand* cmd = registry.find(command);
    if (!cmd) {
        log("ERROR", "Unknown command: " + command);
        return "Error: Unknown command: " + command;
    }

    if (kernelCallback) {
        kernelCallback(command, argsWithInput);
    }

    // Reset interrupt flag before executing command
    g_interrupt_requested.store(false);

    std::ostringstream out, err;
    int rc = cmd->execute(argsWithInput, input, out, err, sys);

    // Check if command was interrupted
    if (g_interrupt_requested.load()) {
        log("INFO", "Command interrupted by user");
        g_interrupt_requested.store(false);
        return "^C\nCommand interrupted";
    }

    std::string result;
    if (!err.str().empty()) {
        log("ERROR", err.str());
        result += err.str();
    }
    result += out.str();

    return result;
}

std::string Shell::executeScriptFile(const std::string& filename) {
    log("INFO", "Executing script file: " + filename);

    std::ostringstream out, err;
    std::vector<std::string> readArgs = { filename };
    std::string fileContent;

    ICommand* catCmd = registry.find("cat");
    if (catCmd) {
        int rc = catCmd->execute(readArgs, "", out, err, sys);
        if (!err.str().empty()) {
            log("ERROR", err.str());
            return "Error: Failed to read script: " + filename + "\n" + err.str();
        }
        fileContent = out.str();
    } else {
        log("ERROR", "No 'cat' command available to read script file");
        return "Error: Missing 'cat' command to read file";
    }

    std::istringstream file(fileContent);
    std::string line;
    std::string output;

    auto originalOutputCB = outputCallback;
    outputCallback = [&](const std::string& outStr) {
        if (!output.empty()) output += "\n";
        output += outStr;
    };

    while (std::getline(file, line)) {
        // Check if script execution was interrupted
        if (g_interrupt_requested.load()) {
            log("INFO", "Script execution interrupted by user");
            output += "\n^C\nScript interrupted";
            break;
        }
        
        auto l = line.find_first_not_of(" \t\r\n");
        if (l == std::string::npos) continue;
        auto r = line.find_last_not_of(" \t\r\n");
        std::string trimmed = line.substr(l, r - l + 1);
        if (trimmed.empty() || trimmed.front() == '#') continue;

        log("DEBUG", "Script line: " + trimmed);
        processCommandLine(trimmed);
    }

    outputCallback = originalOutputCB;
    return output;
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

bool Shell::isConnectedToKernel() const {
    return kernelCallback != nullptr;
}

} // namespace shell
