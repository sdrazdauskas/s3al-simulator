#include "../CommandAPI.h"
#include <memory>

namespace shell {

class AddCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& args,
                const std::string& /*input*/,
                std::ostream& out,
                std::ostream& err,
                SysApi& /*sys*/) override
    {
        if (args.empty()) {
            err << "Usage: add [number1] [number2] ...\n";
            return 1;
        }
        double sum = 0.0;
        for (const auto& a : args) {
            try { sum += std::stod(a); } catch(...) { err << "Error: '" << a << "' is not a number\n"; return 1; }
        }
        out << sum << "\n";
        return 0;
    }
    
    const char* getName() const override { return "add"; }
    const char* getDescription() const override { return "Sum numbers"; }
    const char* getUsage() const override { return "add [number1] [number2] ..."; }
};

std::unique_ptr<ICommand> create_add_command() {
    return std::make_unique<AddCommand>();
}

} // namespace shell
