#include "shell/CommandAPI.h"
#include <memory>

namespace shell {

class RmdirCommand : public ICommand {
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
        auto res = sys.removeDir(args[0]);
        if (res != SysResult::OK) {
            err << "rmdir: " << args[0] << ": " << shell::toString(res) << "\n";
            return 1;
        }
        out << "rmdir: " << args[0] << ": " << shell::toString(res) << "\n";
        return 0;
    }
    
    const char* getName() const override { return "rmdir"; }
    const char* getDescription() const override { return "Remove a directory"; }
    const char* getUsage() const override { return "rmdir <dirName>"; }
};

std::unique_ptr<ICommand> createRmdirCommand() {
    return std::make_unique<RmdirCommand>();
}

} // namespace shell
