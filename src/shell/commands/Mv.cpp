#include "../CommandAPI.h"
#include <memory>

namespace shell {

class MvCommand : public ICommand {
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

        auto res = sys.moveFile(args[0], args[1]);
        if (res != SysResult::OK) {
            err << "mv: " << args[0] << " -> " << args[1] << ": " << shell::toString(res) << "\n";
            return 1;
        }

        out << "Moved/Renamed file: " << args[0] << " -> " << args[1] << "\n";
        return 0;
    }

    const char* getName() const override { return "mv"; }
    const char* getDescription() const override {return "Move or rename a file";}
    const char* getUsage() const override { return "mv <oldFile> <newFile>"; }
    int getCpuCost() const override { return 3; }
};

std::unique_ptr<ICommand> create_mv_command() {
    return std::make_unique<MvCommand>();
}

} // namespace shell
