#pragma once

#include <string>
#include <vector>

namespace shell {

struct Command {
    std::string command;       
    std::vector<std::string> args;
};

// Splits on && and newlines; tokenizes respecting quotes
std::vector<Command> parseCommands(const std::string& input);

} // namespace shell
