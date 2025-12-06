#include "Shell.h"
#include <sstream>
#include <iostream>
#include <atomic>
#include <vector>
#include <string>

namespace shell {

// Global interrupt flag for Ctrl+C handling
std::atomic<bool> g_interrupt_requested{false};

Shell::Shell(SysApi& sys_, const CommandRegistry& reg, KernelCallback kernelCb)
    : sys(sys_), registry(reg), kernelCallback(std::move(kernelCb)) {}

void Shell::setLogCallback(LogCallback callback) {
    logCallback = callback;
}

void Shell::setOutputCallback(OutputCallback callback) {
    outputCallback = callback;
}

void Shell::log(const std::string& level, const std::string& message) {
    if (logCallback) {
        logCallback(level, "SHELL", message);
    }
}

std::string Shell::parseQuotedToken(std::istringstream& iss, std::string token) {
    std::string quoted = token.substr(1);
    std::string next;
    while (!quoted.empty() && quoted.back() != '"' && iss >> next) {
        quoted += " " + next;
    }
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

std::string Shell::trim(const std::string &s) {
    auto start = s.find_first_not_of(" \t");
    auto end = s.find_last_not_of(" \t");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

std::string Shell::extractAfterSymbol(const std::string &s, const std::string &symbol) {
    size_t pos = s.find(symbol);
    if (pos == std::string::npos)
        return "";
    return trim(s.substr(pos + symbol.size()));
}

std::string Shell::extractBeforeSymbol(const std::string &s, const std::string &symbol) {
    size_t pos = s.find(symbol);
    if (pos == std::string::npos)
        return trim(s);
    return trim(s.substr(0, pos));
}

std::string Shell::handleInputRedirection(const std::string &segment) {
    std::string filename = extractAfterSymbol(segment, "<");
    if (filename.empty()) {
        log("ERROR", "Input redirection missing filename");
        return "";
    }

    ICommand* catCmd = registry.find("cat");
    if (!catCmd) {
        log("ERROR", "No 'cat' command found for input redirection");
        return "";
    }

    std::ostringstream out, err;
    std::vector<std::string> readArgs = { filename };
    int rc = catCmd->execute(readArgs, "", out, err, sys);
    if (!err.str().empty()) {
        log("ERROR", err.str());
        return "";
    }

    return out.str();
}

std::string Shell::handleOutputRedirection(std::string segment, const std::string &output) {
    std::string filename = extractAfterSymbol(segment, ">");
    segment = extractBeforeSymbol(segment, ">");
    if (filename.empty()) {
        log("ERROR", "Output redirection missing filename");
        return "";
    }

    ICommand* writeCmd = registry.find("write");
    if (!writeCmd) {
        log("ERROR", "No 'write' command found for output redirection");
        return "";
    }

    std::string cleaned = output;
    if (!cleaned.empty() && cleaned.back() == '\n')
        cleaned.pop_back();

    if (sys.fileExists(filename) != shell::SysResult::OK) {
        sys.createFile(filename);
    }

    std::ostringstream out, err;
    std::vector<std::string> writeArgs = { filename, cleaned };
    int rc = writeCmd->execute(writeArgs, "", out, err, sys);
    if (!err.str().empty()) {
        log("ERROR", err.str());
        return "";
    }

    return out.str();
}

std::string Shell::handleAppendRedirection(std::string segment, const std::string &output) {
    std::string filename = extractAfterSymbol(segment, ">>");
    segment = extractBeforeSymbol(segment, ">>");

    if (filename.empty()) {
        log("ERROR", "Append redirection missing filename");
        return "";
    }

    std::string cleaned = output;
    if (!cleaned.empty() && cleaned.back() == '\n')
        cleaned.pop_back();

    if (sys.fileExists(filename) != shell::SysResult::OK) {
        sys.createFile(filename);
    }

    auto result = sys.appendFile(filename, cleaned);
    if (result != shell::SysResult::OK) {
        log("ERROR", "appendFile failed: " + shell::toString(result));
        return "";
    }

    return "append: " + filename + ": OK";
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

        ICommand* cmd = registry.find(command);
        if (cmd) {
            // write/edit commands should go directly to std::cout / std::cerr
            g_interrupt_requested.store(false);

            int rc = cmd->execute(args, "", std::cout, std::cerr, sys);

            std::cout.flush();
            std::cerr.flush();

            // handle interruption
            if (g_interrupt_requested.load()) {
                log("INFO", "Command interrupted by user");
                g_interrupt_requested.store(false);
                if (outputCallback) {
                    outputCallback("^C\nCommand interrupted\n");
                }
            }
        } else {
            log("ERROR", "Unknown command: " + command);
            if (outputCallback) {
                outputCallback("Error: Unknown command: " + command);
            }
        }

        return;
    }

    std::vector<std::string> andCommands = splitByAndOperator(commandLine);
    std::string combinedOutput;

    for (const auto& andCmd : andCommands) {
        std::vector<std::string> pipeCommands = splitByPipeOperator(andCmd);
        std::string pipeInput;

        for (size_t i = 0; i < pipeCommands.size(); ++i) {
            std::string segment = pipeCommands[i];
            std::string segmentCopy = segment;
            std::string inputData;

            if (segmentCopy.find('<') != std::string::npos) {
                inputData = handleInputRedirection(segmentCopy);
                segmentCopy = extractBeforeSymbol(segmentCopy, "<");
            }

            if (segmentCopy.find(">>") != std::string::npos) {
                std::string cleanCommand = extractBeforeSymbol(segmentCopy, ">>");
                std::string filename = extractAfterSymbol(segmentCopy, ">>");

                std::string command;
                std::vector<std::string> args;
                parseCommand(cleanCommand, command, args);

                if (command.empty())
                    continue;

                bool inPipeChain = true;
                std::string result = executeCommand(command, args, inputData.empty() ? pipeInput : inputData, inPipeChain);

                handleAppendRedirection(segmentCopy, result);
                pipeInput.clear();
                continue;
            }

            bool hasOutputRedirect = (segmentCopy.find('>') != std::string::npos && segmentCopy.find(">>") == std::string::npos);

            std::string filename;
            if (hasOutputRedirect) {
                filename = extractAfterSymbol(segmentCopy, ">");
                segmentCopy = extractBeforeSymbol(segmentCopy, ">");
            }

            std::string command;
            std::vector<std::string> args;
            parseCommand(segmentCopy, command, args);

            if (command.empty())
                continue;

            bool inPipeChain = hasOutputRedirect ? true : (i < pipeCommands.size() - 1);
            std::string result = executeCommand(command, args, inputData.empty() ? pipeInput : inputData, inPipeChain);

            if (hasOutputRedirect && !filename.empty()) {
                handleOutputRedirection(">" + filename, result);
                pipeInput.clear();
            } else {
                pipeInput = result;
            }

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
                                  const std::string& input,
                                  bool inPipeChain) {
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

    g_interrupt_requested.store(false);

    // Get CPU cost and submit to scheduler for async execution
    int cpuCycles = cmd->getCpuCost();
    int pid = sys.submitCommand(command, cpuCycles, 0);
    
    if (pid > 0) {
        log("DEBUG", "Command '" + command + "' submitted (PID=" + std::to_string(pid) + 
            ", cycles=" + std::to_string(cpuCycles) + ")");
        
        // Wait for the scheduler to complete this process
        bool completed = sys.waitForProcess(pid);
        
        if (!completed) {
            log("INFO", "Command '" + command + "' interrupted");
            if (outputCallback && !inPipeChain) {
                outputCallback("^C\nCommand interrupted\n");
            }
            return "";
        }
        
        log("DEBUG", "Command '" + command + "' finished scheduling, executing...");
    }
    
    // Now actually execute the command (scheduling is complete)
    int rc = 0;
    std::string result;

    if (inPipeChain) {
        std::ostringstream out, err;
        rc = cmd->execute(argsWithInput, input, out, err, sys);

        if (!err.str().empty()) {
            log("ERROR", err.str());
            result += err.str();
        }
        result += out.str();
    } else {
        CallbackStreamBuf out_buf(outputCallback);
        CallbackStreamBuf err_buf(outputCallback);
        std::ostream out(&out_buf);
        std::ostream err(&err_buf);

        rc = cmd->execute(argsWithInput, input, out, err, sys);

        out.flush();
        err.flush();
    }

    if (g_interrupt_requested.load()) {
        log("INFO", "Command interrupted by user");
        g_interrupt_requested.store(false);
        if (outputCallback && !inPipeChain) {
            outputCallback("^C\nCommand interrupted\n");
        }
        return "";
    }

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
        if (g_interrupt_requested.load()) {
            log("INFO", "Script execution interrupted by user");
            if (!output.empty()) output += "\n";
            output += "^C\nScript interrupted";
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
        if (!token.empty() && token.front() == '"')
            args.push_back(parseQuotedToken(iss, token));
        else
            args.push_back(token);
    }
}

bool Shell::isConnectedToKernel() const {
    return kernelCallback != nullptr;
}

} // namespace shell
