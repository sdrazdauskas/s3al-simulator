#include "shell/CommandAPI.h"
#include <memory>

namespace shell {

class MkdirCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& args,
                const std::string& /*input*/,
                std::ostream& out,
                std::ostream& err,
                SysApi& sys) override
    {
        if (args.empty()) {
            err << "Usage: " << getUsage() << "\n";
            return 1;
        }
        auto res = sys.makeDir(args[0]);
        if (res != SysResult::OK) {
            err << "mkdir: " << args[0] << ": " << shell::toString(res) << "\n";
            return 1;
        }
        out << "mkdir: " << args[0] << ": " << shell::toString(res) << "\n";
        return 0;
    }
    
    const char* getName() const override { return "mkdir"; }
    const char* getDescription() const override { return "Create a new directory"; }
    const char* getUsage() const override { return "mkdir <dirName>"; }
    int getCpuCost() const override { return 2; }
};

std::unique_ptr<ICommand> createMkdirCommand() {
    return std::make_unique<MkdirCommand>();
}

} // namespace shell
