#include "shell/CommandAPI.h"
#include <memory>

namespace shell {

class CpDirCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& args,
                const std::string& /*input*/,
                std::ostream& out,
                std::ostream& err,
                SysApi& sys) override
    {
        if (!requireArgs(args, 2, err, 2)) return 1;

        auto res = sys.copyDir(args[0], args[1]);
        if (res != SysResult::OK) {
            err << "cpdir: " << args[0] << " -> " << args[1] << ": " << shell::toString(res) << "\n";
            return 1;
        }

        out << "Copied directory: " << args[0] << " -> " << args[1] << "\n";
        return 0;
    }

    const char* getName() const override { return "cpdir"; }
    const char* getDescription() const override { return "Copy a directory and its contents to a new location";}
    const char* getUsage() const override { return "cpdir <srcDir> <destDir>"; }
};

std::unique_ptr<ICommand> createCpdirCommand() {
    return std::make_unique<CpDirCommand>();
}

} // namespace shell
