#include "shell/CommandAPI.h"
#include <memory>
#include <sstream>
#include <unordered_map>
#include <cctype>

namespace shell {

// Map signal names/numbers to values
static const std::unordered_map<std::string, int> signalMap = {
    {"STOP", 19}, {"19", 19},
    {"CONT", 18}, {"18", 18},
    {"TERM", 15}, {"15", 15},
    {"KILL", 9},  {"9", 9}
};

class KillCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& args,
                const std::string& /*input*/,
                std::ostream& out,
                std::ostream& err,
                SysApi& sys) override
    {
        if (!requireArgs(args, 1, err, 2)) return 1;
        
        int signal = 15; // SIGTERM by default
        const std::string* pidArg = &args[0];
        
        // Parse signal if provided
        if (args[0][0] == '-') {
            if (args.size() < 2) {
                err << "Error: PID required" << std::endl;
                return 1;
            }
            
            std::string sigName = args[0].substr(1);
            
            // Convert to uppercase for case-insensitive comparison
            for (char& c : sigName) {
                c = std::toupper(static_cast<unsigned char>(c));
            }
            
            auto it = signalMap.find(sigName);
            if (it != signalMap.end()) {
                signal = it->second;
            } else {
                err << "Error: Unknown signal: " << sigName << std::endl;
                return 1;
            }
            
            pidArg = &args[1];
        }
        
        // Parse PID
        int pid;
        try {
            size_t idx = 0;
            pid = std::stoi(*pidArg, &idx);
            if (idx != pidArg->size()) {
                err << "Error: Invalid PID (contains non-numeric characters): " << *pidArg << std::endl;
                return 1;
            }
        } catch (const std::out_of_range&) {
            err << "Error: PID value out of range: " << *pidArg << std::endl;
            return 1;
        } catch (...) {
            err << "Error: Invalid PID: " << *pidArg << std::endl;
            return 1;
        }
        if (pid <= 0) {
            err << "Error: Invalid PID: " << pid << std::endl;
            return 1;
        }
        
        // Send signal via syscall
        auto result = sys.sendSignalToProcess(pid, signal);
        
        if (result == SysResult::OK) {
            out << "Signal " << signal << " sent to process " << pid << std::endl;
            return 0;
        } else {
            err << "Error: Failed to send signal to process " << pid << std::endl;
            return 1;
        }
    }
    
    const char* getName() const override { return "kill"; }
    const char* getDescription() const override { return "Send signal to a process"; }
    const char* getUsage() const override { 
        return "kill [-SIGNAL] <pid>\n"
               "  -STOP    Suspend process execution\n"
               "  -CONT    Resume suspended process\n"
               "  -TERM    Terminate gracefully (default)\n"
               "  -KILL    Force terminate";
    }
};

std::unique_ptr<ICommand> createKillCommand() {
    return std::make_unique<KillCommand>();
}

} // namespace shell
