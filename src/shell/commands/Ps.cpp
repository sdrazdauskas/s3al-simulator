#include "../CommandAPI.h"
#include "../../kernel/SysCallsAPI.h"
#include <sstream>
#include <iomanip>
#include <memory>

namespace shell {

class PsCommand : public ICommand {
public:
    int execute(const std::vector<std::string>&,
                const std::string&,
                std::ostream& out,
                std::ostream&,
                SysApi& sys) override {
        auto processes = sys.getProcessList();
        
        if (processes.empty()) {
            out << "No processes running\n";
            return 0;
        }
        
        out << std::left << std::setw(6) << "PID" 
            << std::setw(16) << "NAME" 
            << std::setw(12) << "STATE"
            << std::setw(10) << "PRIORITY" << "\n";
        out << std::string(44, '-') << "\n";
        
        for (const auto& proc : processes) {
            out << std::left << std::setw(6) << proc.pid
                << std::setw(16) << proc.name
                << std::setw(12) << proc.state
                << std::setw(10) << proc.priority << "\n";
        }
        
        return 0;
    }
    
    const char* getName() const override { return "ps"; }
    const char* getDescription() const override { return "List running processes"; }
    const char* getUsage() const override { return "ps"; }
};

std::unique_ptr<ICommand> create_ps_command() {
    return std::make_unique<PsCommand>();
}

} // namespace shell
