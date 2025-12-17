#include "shell/CommandAPI.h"
#include <memory>
#include <cmath>

namespace shell {

class AddCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& args,
                const std::string& /*input*/,
                std::ostream& out,
                std::ostream& err,
                SysApi& /*sys*/) override
    {
        if (!requireArgs(args, 1, err)) return 1;
        
        double sum = 0.0;
        for (const auto& a : args) {
            try {
                double val = std::stod(a);
                double prev = sum;
                sum += val;
                if (std::isinf(sum)) {
                    err << "Error: overflow occurred while adding '" << a << "' (partial sum: " << prev << ")\n";
                    return 1;
                }
            } catch(...) {
                err << "Error: '" << a << "' is not a number\n";
                return 1;
            }
        }
        out << sum << "\n";
        return 0;
    }
    
    const char* getName() const override { return "add"; }
    const char* getDescription() const override { return "Sum the numbers"; }
    const char* getUsage() const override { return "add <num1> [num2] [num...]"; }
};

std::unique_ptr<ICommand> createAddCommand() {
    return std::make_unique<AddCommand>();
}

} // namespace shell
