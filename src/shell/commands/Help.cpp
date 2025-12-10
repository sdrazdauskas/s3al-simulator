#include "shell/CommandAPI.h"
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
        
        // Start from header widths to avoid magic numbers and ensure columns
        // are at least as wide as the header labels.
        size_t maxNameLen = std::string_view("Name").size();
        size_t maxDescLen = std::string_view("Description").size();

        for (const auto& cmdName : cmds) {
            maxNameLen = std::max(maxNameLen, cmdName.size());
            if (ICommand* cmd = registry->find(cmdName)) {
                const char* commandDesc = cmd->getDescription();
                std::string_view desc = commandDesc ? std::string_view(commandDesc) : std::string_view();
                maxDescLen = std::max(maxDescLen, desc.size());
            }
        }
        
        auto drawLine = [&]() {
            return "+" + std::string(maxNameLen + 2, '-') + "+" +
                   std::string(maxDescLen + 2, '-') + "+\n";
        };
        
        std::ostringstream oss;
        oss << drawLine();
        oss << "| " << std::left << std::setw(maxNameLen) << "Name" << " | "
            << std::left << std::setw(maxDescLen) << "Description" << " |\n";
        oss << drawLine();
        
        for (const auto& cmdName : cmds) {
            ICommand* cmd = registry->find(cmdName);
            oss << "| " << std::left << std::setw(maxNameLen) << cmdName << " | ";
            if (cmd) {
                oss << std::left << std::setw(maxDescLen) << cmd->getDescription();
            } else {
                oss << std::left << std::setw(maxDescLen) << "";
            }
            oss << " |\n";
        }
        oss << drawLine();
        oss << "\nType 'help <command>' for more information.\n";
        
        out << oss.str();
        return 0;
    }
    
    const char* getName() const override { return "help"; }
    const char* getDescription() const override { return "Display help information"; }
    const char* getUsage() const override { return "help [command]"; }
};


std::unique_ptr<ICommand> createHelpCommand(CommandRegistry* reg) {
    auto cmd = std::make_unique<HelpCommand>();
    cmd->setRegistry(reg);
    return cmd;
}

} // namespace shell
