#include "shell/CommandAPI.h"
#include <ostream>
#include <string>
#include <vector>
#include <algorithm>

namespace shell {

class SetTickIntervalCommand : public ICommand {
public:
    const char* getName() const override { return "settickinterval"; }
    const char* getDescription() const override { return "Set scheduler tick interval (ms)"; }
    const char* getUsage() const override { return "settickinterval <ms>"; }

    int execute(const std::vector<std::string>& args,
                const std::string& input,
                std::ostream& out,
                std::ostream& err,
                SysApi& sys) override
    {
        if (!requireArgs(args, 1, err)) return 1;
        int ms = 0;
        try {
            ms = std::stoi(args[0]);
        } catch (...) {
            err << "Invalid tick interval value: " << args[0] << "\n";
            return 1;
        }
        if (ms <= 0) {
            err << "Tick interval must be a positive integer\n";
            return 1;
        }
        bool res = sys.setSchedulerTickIntervalMs(ms);
        if (res) {
            out << "Scheduler tick interval set to: " << ms << " ms\n";
            return 0;
        } else {
            err << "Failed to set scheduler tick interval\n";
            return 1;
        }
    }
};

std::unique_ptr<ICommand> createSetTickIntervalCommand() {
    return std::make_unique<SetTickIntervalCommand>();
}

} // namespace shell
