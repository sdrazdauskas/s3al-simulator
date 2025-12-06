#include "shell/CommandAPI.h"
#include <memory>

namespace shell {

class CatCommand : public ICommand {
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
            std::string content;
            auto r = sys.readFile(name, content);
            if (r != shell::SysResult::OK) {
                err << "cat: " << name << ": " << shell::toString(r) << "\n";
                rc = 1;
                continue;
            }

            out << "=== contents of " << name << " ===\n";
            if (content.empty())
                out << "(empty)\n";
            else
                out << content;
            out << "=============================\n";
        }
        return rc;
    }
    
    const char* getName() const override { return "cat"; }
    const char* getDescription() const override { return "Display file contents"; }
    int getCpuCost() const override { return 3; }
    const char* getUsage() const override { return "cat <fileName> [fileName...]"; }
};

std::unique_ptr<ICommand> create_cat_command() {
    return std::make_unique<CatCommand>();
}

} // namespace shell
