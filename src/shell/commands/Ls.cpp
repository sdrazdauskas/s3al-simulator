#include "shell/CommandAPI.h"
#include <memory>

namespace shell {

class LsCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& args,
                const std::string& /*input*/,
                std::ostream& out,
                std::ostream& err,
                SysApi& sys) override
    {
        std::string path = args.empty() ? "." : args[0];
        std::vector<std::string> entries;
        auto res = sys.listDir(path, entries);
        
        if (res != SysResult::OK) {
            err << "ls: " << path << ": " << toString(res) << "\n";
            return 1;
        }

        if (entries.empty()) {
            out << "(empty)\n";
        } else {
            for (const auto& e : entries) {
                out << e << "\n";
            }
        }

        return 0;
    }
    
    const char* getName() const override { return "ls"; }
    const char* getDescription() const override { return "List contents of directory"; }
    const char* getUsage() const override { return "ls [dirName|..]"; }
    int getCpuCost() const override { return 2; }
};

std::unique_ptr<ICommand> create_ls_command() {
    return std::make_unique<LsCommand>();
}

} // namespace shell
