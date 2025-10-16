#include "Shell.h"
#include <sstream>

namespace shell {
Shell::Shell(std::string (*kernelFunc)(const std::string&, const std::vector<std::string>&))
    : kernelHandler(kernelFunc) {}

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

    if (kernelHandler) {
        return kernelHandler(command, args);
    } else {
        return "Error: No kernel handler available";
    }
}

std::string Shell::executeCommand(const std::string& command, const std::vector<std::string>& args) {
    if (!kernelHandler) {
        return "Error: No kernel handler available";
    }

    if (command.empty()) {
        return "Error: No command specified";
    }

    return kernelHandler(command, args);
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
    return kernelHandler != nullptr;
}
}
