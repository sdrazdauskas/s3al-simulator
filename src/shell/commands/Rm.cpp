#include "shell/CommandAPI.h"
#include <memory>

namespace shell {

class RmCommand : public ICommand {
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
        int rc = 0;
        for (const auto& name : args) {
            auto res = sys.deleteFile(name);
            if (res != SysResult::OK) {
                err << "rm: " << name << ": " << shell::toString(res) << "\n";
                rc = 1;
            } else {
                out << "rm: " << name << ": " << shell::toString(res) << "\n";
            }
        }
        return rc;
    }
    
    const char* getName() const override { return "rm"; }
    const char* getDescription() const override { return "Delete a file"; }
    const char* getUsage() const override { return "rm <fileName> [fileName...]"; }
};

std::unique_ptr<ICommand> create_rm_command() {
    return std::make_unique<RmCommand>();
}

} // namespace shell
