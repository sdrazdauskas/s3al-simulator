#include "shell/CommandAPI.h"
#include <memory>

namespace shell {

class LoadStateCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& args,
                const std::string&,
                std::ostream& out,
                std::ostream& err,
                SysApi& sys) override
    {
        if (!requireArgs(args, 1, err)) return 1;
        
        auto fileName = args[0];
        auto res = sys.loadFromDisk(fileName);
        out << "Load result: " << toString(res) << "\n";
        return res == SysResult::OK ? 0 : 1;
    }
    
    const char* getName() const override { return "loadstate"; }
    const char* getDescription() const override { return "Load entire filesystem state from disk"; }
    const char* getUsage() const override { return "loadstate <name>"; }
};

std::unique_ptr<ICommand> createLoadStateCommand() {
    return std::make_unique<LoadStateCommand>();
}

} // namespace shell
