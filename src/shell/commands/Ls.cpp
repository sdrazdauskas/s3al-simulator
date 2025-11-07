#include "../CommandAPI.h"
#include <memory>

namespace shell {

class LsCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& /*args*/,
                const std::string& /*input*/,
                std::ostream& out,
                std::ostream& /*err*/,
                SysApi& sys) override
    {
        auto entries = sys.listDir();
        if (entries.empty()) out << "(empty)\n";
        for (const auto& e : entries) out << e << "\n";
        return 0;
    }
    
    const char* getName() const override { return "ls"; }
    const char* getDescription() const override { return "List contents of current directory"; }
    const char* getUsage() const override { return "ls"; }
};

std::unique_ptr<ICommand> create_ls_command() {
    return std::make_unique<LsCommand>();
}

} // namespace shell
