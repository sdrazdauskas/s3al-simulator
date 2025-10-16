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

std::string Shell::processCommandLine(const std::string& commandLine) {
    std::istringstream iss(commandLine);
    std::string command;
    iss >> command;

    if (command.empty()) {
        return "Error: No command entered";
    }

    std::vector<std::string> args;
    std::string token;

    while (iss >> token) {
        if (token.front() == '"') {
            args.push_back(parseQuotedToken(iss, token));
        } else {
            args.push_back(token);
        }
    }

    if (kernelCallback) {
        return kernelCallback(command, args);
    } else {
        return "Error: No kernel handler available";
    }
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
}
