#include "../CommandAPI.h"
#include <memory>
#include <iostream>

namespace shell {

class ResetCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& /*args*/,
                const std::string&,
                std::ostream& out,
                std::ostream& /*err*/,
                SysApi& sys) override
    {
        std::string confirm;
        std::cout << "Are you sure you want to reset current storage? (yes/no): ";
        std::getline(std::cin, confirm);

        if (confirm != "yes") {
            out << "Reset aborted.\n";
            return 0;
        }

        auto res = sys.resetStorage();
        out << "Reset result: " << toString(res) << "\n";
        return res == SysResult::OK ? 0 : 1;
    }
    
    const char* getName() const override { return "reset"; }
    const char* getDescription() const override { return "Clear current storage and start fresh"; }
    const char* getUsage() const override { return "reset"; }
};

std::unique_ptr<ICommand> create_reset_command() {
    return std::make_unique<ResetCommand>();
}

} // namespace shell
