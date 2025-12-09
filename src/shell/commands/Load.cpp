#include "shell/CommandAPI.h"
#include <memory>

namespace shell {

class LoadCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& args,
                const std::string&,
                std::ostream& out,
                std::ostream& err,
                SysApi& sys) override
    {
        if (args.size() < 1) {
            err << "Usage: " << getUsage() << "\n";
            return 1;
        }
        auto fileName = args[0];
        auto res = sys.loadFromDisk(fileName);
        out << "Load result: " << toString(res) << "\n";
        return res == SysResult::OK ? 0 : 1;
    }
    
    const char* getName() const override { return "load"; }
    const char* getDescription() const override { return "Load storage state from disk"; }
    const char* getUsage() const override { return "load <name>"; }
};

std::unique_ptr<ICommand> create_load_command() {
    return std::make_unique<LoadCommand>();
}

} // namespace shell
