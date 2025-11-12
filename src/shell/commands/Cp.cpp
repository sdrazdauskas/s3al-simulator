#include "../CommandAPI.h"
#include <memory>

namespace shell {

class CpCommand : public ICommand {
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

        auto res = sys.copyFile(args[0], args[1]);
        if (res != SysResult::OK) {
            err << "cp: " << args[0] << " -> " << args[1] << ": " << shell::toString(res) << "\n";
            return 1;
        }

        out << "Copied file: " << args[0] << " -> " << args[1] << "\n";
        return 0;
    }

    const char* getName() const override { return "cp"; }
    const char* getDescription() const override { return "Copy a file from source to destination"; }
    const char* getUsage() const override { return "cp <srcFile> <destFile>"; }
};

std::unique_ptr<ICommand> create_cp_command() {
    return std::make_unique<CpCommand>();
}

} // namespace shell
