// shell/cmd_touch.cpp
#include "../CommandAPI.h"
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
        if (args.empty()) {
            err << "Usage: " << getUsage() << "\n";
            return 1;
        }

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
    const char* getDescription() const override { return "Create a new empty file or update the modification timestamp"; }
    const char* getUsage() const override { return "touch <fileName> [fileName...]"; }
};

std::unique_ptr<ICommand> create_touch_command() {
    return std::make_unique<TouchCommand>();
}

} // namespace shell
