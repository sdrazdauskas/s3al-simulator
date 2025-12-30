// shell/cmd_touch.cpp
#include "shell/CommandAPI.h"
#include <memory>

namespace shell {

class TouchCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& args,
                const std::string& /*input*/,
                std::ostream& out,
                std::ostream& err,
                SysApi& sys) override
    {
        if (!requireArgs(args, 1, err)) return 1;

        int rc = 0;
        for (const auto& name : args) {
            auto res = sys.createFile(name);
            if (res != shell::SysResult::OK) {
                err << "touch: " << name << ": " << shell::toString(res) << "\n";
                rc = 1;
            } else {
                out << "touch: " << name << ": " << shell::toString(res) << "\n";
            }
        }
        return rc;
    }
    
    const char* getName() const override { return "touch"; }
    const char* getDescription() const override { return "Update the modification timestamp of the provided file, if file doesn't exist, it will be created"; }
    const char* getUsage() const override { return "touch <fileName> [fileName...]"; }
    int getCpuCost() const override { return 2; }
};

std::unique_ptr<ICommand> createTouchCommand() {
    return std::make_unique<TouchCommand>();
}

} // namespace shell
