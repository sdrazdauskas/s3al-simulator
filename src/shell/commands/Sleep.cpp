#include "shell/CommandAPI.h"
#include <thread>
#include <chrono>
#include <sstream>

namespace shell {

class SleepCommand : public ICommand {
public:
    const char* getName() const override { return "sleep"; }
    
    const char* getDescription() const override {
        return "Sleep for N seconds (demonstrates kernel continues running in background)";
    }
    
    const char* getUsage() const override {
        return "sleep <seconds>";
    }
    
    int execute(const std::vector<std::string>& args,
                const std::string& input,
                std::ostream& out,
                std::ostream& err,
                SysApi& sys) override {
        (void)input;
        (void)sys;
        
        if (args.empty()) {
            out << "Usage: " << getUsage() << "\n";
            return 1;
        }
        
        try {
            int seconds = std::stoi(args[0]);
            if (seconds < 0 || seconds > 60) {
                err << "Error: seconds must be between 0 and 60\n";
                return 1;
            }
            
            out << "Sleeping for " << seconds << " seconds...\n";
            out << "(Press Ctrl+C to interrupt)\n";
            out << std::flush;
            
            // Sleep in small increments and check for interrupt
            for (int i = 0; i < seconds; ++i) {
                // Check interrupt flag every 100ms (10 times per second)
                for (int j = 0; j < 10; ++j) {
                    if (interruptRequested.load()) {
                        out << "\nInterrupted after " << i << " seconds\n";
                        out << std::flush;
                        return 130; // Standard exit code for SIGINT
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            
            out << "Wake up!\n";
            out << std::flush;
            return 0;
            
        } catch (const std::exception& e) {
            err << "Error: Invalid number\n";
            return 1;
        }
    }
};

std::unique_ptr<ICommand> create_sleep_command() {
    return std::make_unique<SleepCommand>();
}

} // namespace shell
