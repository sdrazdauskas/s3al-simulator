#include "shell/CommandAPI.h"
#include "scheduler/algorithms/SchedulerAlgorithm.h"
#include <map>
#include <algorithm>
#include <cstring>

namespace shell {

class SetSchedulerCommand : public ICommand {
public:
    const char* getName() const override { return "setsched"; }
    const char* getDescription() const override { return "Change the scheduler algorithm"; }
    const char* getUsage() const override { return "setsched <algorithm> [--quantum N]"; }

    int execute(const std::vector<std::string>& args,
                const std::string& input,
                std::ostream& out,
                std::ostream& err,
                SysApi& sys) override
    {
        if (!requireArgs(args, 1, err)) return 1;

        // Normalize algorithm name to uppercase for lookup
        std::string algoName = args[0];
        std::transform(algoName.begin(), algoName.end(), algoName.begin(), ::toupper);

        // Parse remaining args for flags like --quantum / -q or positional numeric quantum
        int quantum = 0;
        for (size_t i = 1; i < args.size(); ++i) {
            const std::string& a = args[i];

            // --quantum=3 style
            if (a.rfind("--quantum=", 0) == 0) {
                std::string val = a.substr(strlen("--quantum="));
                try {
                    quantum = std::stoi(val);
                } catch (...) {
                    err << "Invalid quantum value: " << val << "\n";
                    return 1;
                }
            }
            // --quantum 3 or -q 3
            else if (a == "--quantum" || a == "-q") {
                if (i + 1 >= args.size()) {
                    err << "Missing value for " << a << "\n";
                    return 1;
                }
                try {
                    quantum = std::stoi(args[++i]);
                } catch (...) {
                    err << "Invalid quantum value: " << args[i] << "\n";
                    return 1;
                }
            }
        }

        // Map of accepted algorithm names (include common synonyms)

        static const std::map<std::string, scheduler::SchedulerAlgorithm> algoMap = {
            {"FCFS", scheduler::SchedulerAlgorithm::FCFS},
            {"FIFO", scheduler::SchedulerAlgorithm::FCFS},
            {"RR", scheduler::SchedulerAlgorithm::RoundRobin},
            {"ROUNDROBIN", scheduler::SchedulerAlgorithm::RoundRobin},
            {"ROUND-ROBIN", scheduler::SchedulerAlgorithm::RoundRobin},
            {"PRIORITY", scheduler::SchedulerAlgorithm::Priority},
            {"PRIO", scheduler::SchedulerAlgorithm::Priority}
        };

        auto it = algoMap.find(algoName);
        if (it == algoMap.end()) {
            err << "Unknown scheduler algorithm: " << args[0] << "\n";
            err << "Valid options: FCFS, RR, PRIORITY\n";
            return 1;
        }

        scheduler::SchedulerAlgorithm algo = it->second;

        if (quantum > 0 && algo != scheduler::SchedulerAlgorithm::RoundRobin) {
            out << "Warning: quantum is only used by RR scheduler; ignoring quantum for selected algorithm\n";
            quantum = 0;
        }

        if (quantum < 0) {
            err << "Quantum must be a non-negative integer\n";
            return 1;
        }

        bool res = sys.changeSchedulingAlgorithm(algo, quantum);
        if (res) {
            out << "Scheduler algorithm changed to: " << args[0];
            if (quantum > 0) out << " (quantum=" << quantum << ")";
            out << "\n";
            return 0;
        } else {
            err << "Failed to change scheduler: " << args[0];
            if (quantum > 0) err << " (quantum=" << quantum << ")";
            err << "\n";
            return 1;
        }
    }
};

std::unique_ptr<ICommand> createSetSchedulerCommand() {
    return std::make_unique<SetSchedulerCommand>();
}

} // namespace shell