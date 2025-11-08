#include "CommandAPI.h"
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
            out << "(Kernel continues running background tasks)\n";
            out << std::flush;
            
            // Sleep in small increments to allow interruption if needed
            for (int i = 0; i < seconds; ++i) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            
            out << "Wake up!\n";
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
