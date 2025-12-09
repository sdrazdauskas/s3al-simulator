#include "shell/CommandAPI.h"
#include <memory>

namespace shell {

class PwdCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& /*args*/,
                const std::string& /*input*/,
                std::ostream& out,
                std::ostream& /*err*/,
                SysApi& sys) override
    {
        out << sys.getWorkingDir() << "\n";
        return 0;
    }
    
    const char* getName() const override { return "pwd"; }
    const char* getDescription() const override { return "Print working directory"; }
    const char* getUsage() const override { return "pwd"; }
};

std::unique_ptr<ICommand> createPwdCommand() {
    return std::make_unique<PwdCommand>();
}

} // namespace shell
