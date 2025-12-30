#include "shell/CommandAPI.h"
#include <thread>
#include <chrono>
#include <sstream>

namespace shell {

class SleepCommand : public ICommand {
public:
    const char* getName() const override { return "sleep"; }
    
    const char* getDescription() const override {
        return "Sleep for provided seconds";
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
        
        if (!requireArgs(args, 1, err, 1)) return 1;
        
        try {
            size_t idx = 0;
            int seconds = 0;
            try {
                seconds = std::stoi(args[0], &idx);
            } catch (const std::out_of_range&) {
                err << "Error: value out of range for seconds\n";
                return 1;
            }
            if (idx != args[0].size()) {
                err << "Error: invalid characters in seconds argument\n";
                return 1;
            }
            if (seconds < 0) {
                err << "Error: seconds must be non-negative\n";
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

std::unique_ptr<ICommand> createSleepCommand() {
    return std::make_unique<SleepCommand>();
}

} // namespace shell
