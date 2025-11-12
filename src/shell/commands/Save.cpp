#include "../CommandAPI.h"
#include <memory>

namespace shell {

class SaveCommand : public ICommand {
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
        auto res = sys.saveToDisk(fileName);
        out << "Save result: " << toString(res) << "\n";
        return res == SysResult::OK ? 0 : 1;
    }
    
    const char* getName() const override { return "save"; }
    const char* getDescription() const override { return "Save current storage state"; }
    const char* getUsage() const override { return "save <name>"; }
};

std::unique_ptr<ICommand> create_save_command() {
    return std::make_unique<SaveCommand>();
}

} // namespace shell
