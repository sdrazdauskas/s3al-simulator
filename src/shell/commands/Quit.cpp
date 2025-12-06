#include "shell/CommandAPI.h"
#include <memory>

namespace shell {

class QuitCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& /*args*/,
                const std::string& /*input*/,
                std::ostream& out,
                std::ostream& /*err*/,
                SysApi& sys) override
    {
        out << "Shutting down..." << std::endl;
        sys.requestShutdown();
        return 0;
    }
    
    const char* getName() const override { return "quit"; }
    const char* getDescription() const override { return "Exit the shell"; }
    const char* getUsage() const override { return "quit"; }
};

std::unique_ptr<ICommand> create_quit_command() {
    return std::make_unique<QuitCommand>();
}

} // namespace shell
