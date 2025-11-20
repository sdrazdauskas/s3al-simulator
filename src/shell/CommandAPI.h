#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <ostream>
#include <atomic>
#include "SysCallsAPI.h"

namespace shell {

// Global interrupt flag for Ctrl+C handling
// Commands should check this periodically and exit gracefully if set
extern std::atomic<bool> g_interrupt_requested;

// Abstract base class for all commands
class ICommand {
public:
    virtual ~ICommand() = default;
    
    // Execute the command
    virtual int execute(const std::vector<std::string>& args,
                       const std::string& input,
                       std::ostream& out,
                       std::ostream& err,
                       SysApi& sys) = 0;
    
    // Metadata
    virtual const char* getName() const = 0;
    virtual const char* getDescription() const = 0;
    virtual const char* getUsage() const = 0;
};

struct CommandRegistry {
    std::unordered_map<std::string, std::unique_ptr<ICommand>> map;
    
    void add(std::unique_ptr<ICommand> cmd) {
        std::string name = cmd->getName();
        map[name] = std::move(cmd);
    }
    
    ICommand* find(const std::string& name) const {
        auto it = map.find(name);
        return (it == map.end()) ? nullptr : it->second.get();
    }
    
    std::vector<std::string> getAllCommands() const {
        std::vector<std::string> cmds;
        for (const auto& pair : map) {
            cmds.push_back(pair.first);
        }
        return cmds;
    }
};

} // namespace shell
