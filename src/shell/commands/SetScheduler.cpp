#include "shell/CommandAPI.h"
#

namespace shell {

class SetSchedulerCommand : public ICommand {
public:
    const char* getName() const override { return "setsched"; }
    const char* getDescription() const override { return "Change the scheduler algorithm"; }
    const char* getUsage() const override { return "setsched <algorithm>"; }

    int execute(const std::vector<std::string>& args,
                const std::string& input,
                std::ostream& out,
                std::ostream& err,
                SysApi& sys) override {
        if (!requireArgs(args, 1, err, 2)) return 1;
        const std::string& algo = args[0];
        int quantum = 0;
        if (args.size() > 1) {
            try {
                quantum = std::stoi(args[1]);
            } catch (const std::exception&) {
                err << "Invalid number argument: " << args[1] << "\n";
                return 1;
            }
        } else {
            quantum = 0;
        }
        bool res = sys.changeSchedulingAlgorithm(algo, quantum);
        if (res) {
            out << "Scheduler algorithm changed to: " << algo;
            if (args.size() > 1) out << " (" << quantum << ")";
            out << "\n";
            return 0;
        } else {
            err << "Failed to change scheduler: " << algo;
            if (args.size() > 1) err << " (" << quantum << ")";
            err << "\n";
            return 1;
        }
    }
};

std::unique_ptr<ICommand> createSetSchedulerCommand() {
    return std::make_unique<SetSchedulerCommand>();
}

} // namespace shell