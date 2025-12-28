#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <ostream>
#include <atomic>
#include <iostream>
#include "kernel/SysCallsAPI.h"

namespace shell {

// Import SysApi from sys namespace for shell commands
using sys::SysApi;
using sys::SysResult;
using sys::toString;

// Global interrupt flag for Ctrl+C handling
// Commands should check this periodically and exit gracefully if set
extern std::atomic<bool> interruptRequested;

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
    
    // CPU cost in cycles (how many scheduler ticks to complete)
    virtual int getCpuCost() const { return 1; }
    
    // Helper function to check for required arguments and print usage
    // minCount: minimum required arguments
    // maxCount: maximum allowed arguments (-1 = no limit)
    bool requireArgs(const std::vector<std::string>& args, int minCount, std::ostream& err, int maxCount = -1) const {
        if (minCount > 0 && static_cast<int>(args.size()) < minCount) {
            err << "Usage: " << getUsage() << "\n";
            return false;
        }
        if (maxCount >= 0 && static_cast<int>(args.size()) > maxCount) {
            err << "Usage: " << getUsage() << "\n";
            return false;
        }
        return true;
    }

    bool confirmAction(const std::string& prompt, SysApi& sys, std::ostream& out) const {
        out << prompt << " (yes/no): ";
        out.flush();
        std::string response = sys.readLine();
        if (response != "yes" && response != "y") {
            out << "Action aborted.\n";
            return false;
        }
        return true;
    }
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
