#include "shell/CommandAPI.h"
#include <memory>

namespace shell {

class CdCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& args,
                const std::string& /*input*/,
                std::ostream& out,
                std::ostream& err,
                SysApi& sys) override
    {
        if (!requireArgs(args, 1, err, 1)) return 1;
        
        auto res = sys.changeDir(args[0]);
        if (res != SysResult::OK) {
            err << "cd: " << args[0] << ": " << shell::toString(res) << "\n";
            return 1;
        }
        out << sys.getWorkingDir() << "\n";
        return 0;
    }
    
    const char* getName() const override { return "cd"; }
    const char* getDescription() const override { return "Change current directory"; }
    const char* getUsage() const override { return "cd <dirName|..>"; }
};

std::unique_ptr<ICommand> createCdCommand() {
    return std::make_unique<CdCommand>();
}

} // namespace shell
