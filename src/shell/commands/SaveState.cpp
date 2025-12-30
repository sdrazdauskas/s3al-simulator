#include "shell/CommandAPI.h"
#include <memory>

namespace shell {

class SaveStateCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& args,
                const std::string&,
                std::ostream& out,
                std::ostream& err,
                SysApi& sys) override
    {
        if (!requireArgs(args, 1, err)) return 1;
        
        auto fileName = args[0];
        auto res = sys.saveToDisk(fileName);
        out << "Save result: " << toString(res) << "\n";
        return res == SysResult::OK ? 0 : 1;
    }
    
    const char* getName() const override { return "savestate"; }
    const char* getDescription() const override { return "Save entire filesystem state to disk"; }
    const char* getUsage() const override { return "savestate <name>"; }
};

std::unique_ptr<ICommand> createSaveStateCommand() {
    return std::make_unique<SaveStateCommand>();
}

} // namespace shell
