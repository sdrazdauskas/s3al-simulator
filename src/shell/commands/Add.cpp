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
        if (!requireArgs(args, 2, err)) return 1;
        
        long double sum = 0.0L;
        for (const auto& a : args) {
            long double val = std::stold(a);
            long double prev = sum;
            long double newSum = prev + val;

            if (newSum == prev) {
                err << "Error: precision lost (addition had no effect) for '" << a << "' (partial sum: " << prev << ")\n";
                return 1;
            }

            if ((newSum - prev) != val) {
                err << "Error: rounding occurred while adding '" << a << "' (added: " << val << ", actual increment: " << (newSum - prev) << ")\n";
                return 1;
            }

            if (std::isinf(newSum)) {
                err << "Error: overflow occurred while adding '" << a << "' (partial sum: " << prev << ")\n";
                return 1;
            }

            sum = newSum;
        }
        out << "Sum: " << sum << "\n";
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
