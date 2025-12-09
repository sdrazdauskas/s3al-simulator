#include "shell/CommandAPI.h"
#include <memory>
#include <sstream>

namespace shell {

class KillCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& args,
                const std::string& /*input*/,
                std::ostream& out,
                std::ostream& err,
                SysApi& sys) override
    {
        if (args.size() < 1) {
            out << "Usage: kill [-SIGNAL] <pid>\n"
                   "Signals:\n"
                   "  -STOP (19)  - Suspend process\n"
                   "  -CONT (18)  - Resume process\n"
                   "  -TERM (15)  - Terminate gracefully (default)\n"
                   "  -KILL (9)   - Force terminate" << std::endl;
            return 1;
        }
        
        int signal = 15; // SIGTERM by default
        int pid_arg_idx = 0;
        
        // Parse signal if provided
        if (args[0][0] == '-') {
            std::string sig_name = args[0].substr(1);
            
            if (sig_name == "STOP" || sig_name == "19") {
                signal = 19;
            } else if (sig_name == "CONT" || sig_name == "18") {
                signal = 18;
            } else if (sig_name == "TERM" || sig_name == "15") {
                signal = 15;
            } else if (sig_name == "KILL" || sig_name == "9") {
                signal = 9;
            } else {
                err << "Error: Unknown signal: " << sig_name << std::endl;
                return 1;
            }
            
            pid_arg_idx = 1;
            
            if (args.size() < 2) {
                err << "Error: PID required" << std::endl;
                return 1;
            }
        }
        
        // Parse PID
        int pid;
        try {
            pid = std::stoi(args[pid_arg_idx]);
        } catch (...) {
            err << "Error: Invalid PID: " << args[pid_arg_idx] << std::endl;
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

std::unique_ptr<ICommand> create_kill_command() {
    return std::make_unique<KillCommand>();
}

} // namespace shell
