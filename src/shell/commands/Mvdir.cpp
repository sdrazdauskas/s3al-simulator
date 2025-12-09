#include "shell/CommandAPI.h"
#include <memory>

namespace shell {

class MvDirCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& args,
                const std::string& /*input*/,
                std::ostream& out,
                std::ostream& err,
                SysApi& sys) override
    {
        if (args.size() != 2) {
            err << "Usage: " << getUsage() << "\n";
            return 1;
        }

        auto res = sys.moveDir(args[0], args[1]);
        if (res != SysResult::OK) {
            err << "mvdir: " << args[0] << " -> " << args[1] << ": " << shell::toString(res) << "\n";
            return 1;
        }

        out << "Moved/Renamed directory: " << args[0] << " -> " << args[1] << "\n";
        return 0;
    }

    const char* getName() const override { return "mvdir"; }
    const char* getDescription() const override { return "Move or rename a directory"; }
    const char* getUsage() const override { return "mvdir <oldDir> <newDir>"; }
};

std::unique_ptr<ICommand> create_mvdir_command() {
    return std::make_unique<MvDirCommand>();
}

} // namespace shell
