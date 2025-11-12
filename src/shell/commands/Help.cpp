#include "../CommandAPI.h"
#include <sstream>
#include <iomanip>
#include <memory>
#include <algorithm>

namespace shell {

class HelpCommand : public ICommand {
private:
    CommandRegistry* registry = nullptr;
    
public:
    void setRegistry(CommandRegistry* reg) { registry = reg; }
    
    int execute(const std::vector<std::string>& args,
                const std::string& /*input*/,
                std::ostream& out,
                std::ostream& /*err*/,
                SysApi& /*sys*/) override
    {
        if (!registry) {
            out << "Error: Registry not available\n";
            return 1;
        }
        
        // Show help for specific command
        if (args.size() > 0) {
            ICommand* cmd = registry->find(args[0]);
            if (cmd) {
                out << "Command: " << cmd->getName() << "\n";
                out << "Description: " << cmd->getDescription() << "\n";
                out << "Usage: " << cmd->getUsage() << "\n";
            } else {
                out << "Unknown command: " << args[0] << "\n";
            }
            return 0;
        }
        
        // List all commands
        auto cmds = registry->getAllCommands();
        std::sort(cmds.begin(), cmds.end());
        
        size_t max_name_len = 4;
        size_t max_desc_len = 11;
        
        for (const auto& cmdName : cmds) {
            if (cmdName.size() > max_name_len) max_name_len = cmdName.size();
            ICommand* cmd = registry->find(cmdName);
            if (cmd) {
                std::string desc = cmd->getDescription();
                if (desc.size() > max_desc_len) max_desc_len = desc.size();
            }
        }
        
        auto draw_line = [&]() {
            return "+" + std::string(max_name_len + 2, '-') + "+" +
                   std::string(max_desc_len + 2, '-') + "+\n";
        };
        
        std::ostringstream oss;
        oss << draw_line();
        oss << "| " << std::left << std::setw(max_name_len) << "Name" << " | "
            << std::left << std::setw(max_desc_len) << "Description" << " |\n";
        oss << draw_line();
        
        for (const auto& cmdName : cmds) {
            ICommand* cmd = registry->find(cmdName);
            oss << "| " << std::left << std::setw(max_name_len) << cmdName << " | ";
            if (cmd) {
                oss << std::left << std::setw(max_desc_len) << cmd->getDescription();
            } else {
                oss << std::left << std::setw(max_desc_len) << "";
            }
            oss << " |\n";
        }
        oss << draw_line();
        oss << "\nType 'help <command>' for more information.\n";
        
        out << oss.str();
        return 0;
    }
    
    const char* getName() const override { return "help"; }
    const char* getDescription() const override { return "Display help information"; }
    const char* getUsage() const override { return "help [command]"; }
};


std::unique_ptr<ICommand> create_help_command(CommandRegistry* reg) {
    auto cmd = std::make_unique<HelpCommand>();
    cmd->setRegistry(reg);
    return cmd;
}

} // namespace shell
