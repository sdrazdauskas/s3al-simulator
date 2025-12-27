#include "shell/CommandAPI.h"
#include <ostream>
#include <string>
#include <vector>
#include <algorithm>

namespace shell {

class SetCyclesPerIntervalCommand : public ICommand {
public:
    const char* getName() const override { return "setcycles"; }
    const char* getDescription() const override { return "Set scheduler cycles per interval"; }
    const char* getUsage() const override { return "setcycles <cycles>"; }

    int execute(const std::vector<std::string>& args,
                const std::string& input,
                std::ostream& out,
                std::ostream& err,
                SysApi& sys) override
    {
        if (!requireArgs(args, 1, err)) return 1;
        int cycles = 0;
        try {
            cycles = std::stoi(args[0]);
        } catch (...) {
            err << "Invalid cycles value: " << args[0] << "\n";
            return 1;
        }
        if (cycles <= 0) {
            err << "Cycles must be a positive integer\n";
            return 1;
        }
        bool res = sys.setSchedulerCyclesPerInterval(cycles);
        if (res) {
            out << "Scheduler cycles per interval set to: " << cycles << "\n";
            return 0;
        } else {
            err << "Failed to set scheduler cycles per interval\n";
            return 1;
        }
    }
};

std::unique_ptr<ICommand> createSetCyclesPerIntervalCommand() {
    return std::make_unique<SetCyclesPerIntervalCommand>();
}

} // namespace shell
