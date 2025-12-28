#include "shell/CommandAPI.h"
#include <memory>
#include <iostream>

namespace shell {

class QuitCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& /*args*/,
                const std::string& /*input*/,
                std::ostream& out,
                std::ostream& /*err*/,
                SysApi& sys) override
    {
        if (!confirmAction("Are you sure you want to quit? You must run saveState command to save your work.", sys, out)) {
            return 1;
        }

        out << "Shutting down..." << std::endl;
        sys.requestShutdown();
        return 0;
    }
    
    const char* getName() const override { return "quit"; }
    const char* getDescription() const override { return "Quit the shell"; }
    const char* getUsage() const override { return "quit"; }
};

std::unique_ptr<ICommand> createQuitCommand() {
    return std::make_unique<QuitCommand>();
}

} // namespace shell
