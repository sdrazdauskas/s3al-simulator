#include "shell/CommandAPI.h"
#include "scheduler/algorithms/SchedulerAlgorithm.h"
#include <map>
#include <algorithm>
#include <cstring>
#include <ostream>
#include <string>
#include <vector>

namespace shell {

class SchedulerCommand : public ICommand {
public:
    const char* getName() const override { return "scheduler"; }
    const char* getDescription() const override { return "Manage scheduler settings (algorithm, tick, cycles)"; }
    const char* getUsage() const override {
        return "scheduler <algo|tick|cycles> ...\n"
               "  scheduler algo <algorithm> [--quantum N]\n"
               "  scheduler tick <ms>\n"
               "  scheduler cycles <n>";
    }

    int execute(const std::vector<std::string>& args,
                const std::string& input,
                std::ostream& out,
                std::ostream& err,
                SysApi& sys) override
    {
        if (args.empty()) {
            return usageError(err);
        }

        std::string sub = toLower(args[0]);

        if (sub == "algo")   return handleAlgo(args, out, err, sys);
        if (sub == "tick")   return handleTick(args, out, err, sys);
        if (sub == "cycles") return handleCycles(args, out, err, sys);

        err << "Unknown subcommand: " << sub << "\n";
        return usageError(err);
    }

private:

    int usageError(std::ostream& err) const {
        err << "Usage: " << getUsage() << "\n";
        return 1;
    }

    static std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }

    static std::string toUpper(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::toupper);
        return s;
    }

    static bool parseInt(const std::string& s, int& outVal) {
        try { outVal = std::stoi(s); return true; }
        catch (...) { return false; }
    }

    int handleAlgo(const std::vector<std::string>& args,
                   std::ostream& out,
                   std::ostream& err,
                   SysApi& sys)
    {
        if (args.size() < 2) {
            err << "Usage: scheduler algo <algorithm> [--quantum N]\n";
            return 1;
        }

        std::string algoName = toUpper(args[1]);
        int quantum = parseQuantum(args, err);
        if (quantum < 0) return 1; // error already printed

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
            err << "Unknown scheduler algorithm: " << args[1] << "\n";
            err << "Valid options: FCFS, RR, PRIORITY\n";
            return 1;
        }

        scheduler::SchedulerAlgorithm algo = it->second;

        if (quantum > 0 && algo != scheduler::SchedulerAlgorithm::RoundRobin) {
            out << "Warning: quantum is only used by RR scheduler; ignoring quantum parameter\n";
            quantum = 0;
        }

        if (!sys.changeSchedulingAlgorithm(algo, quantum)) {
            err << "Failed to change scheduler: " << args[1];
            if (quantum > 0) err << " (quantum=" << quantum << ")";
            err << "\n";
            return 1;
        }

        out << "Scheduler algorithm changed to: " << args[1];
        if (quantum > 0) out << " (quantum=" << quantum << ")";
        out << "\n";
        return 0;
    }

    int parseQuantum(const std::vector<std::string>& args, std::ostream& err) {
        int quantum = 0;

        for (size_t i = 2; i < args.size(); ++i) {
            const std::string& a = args[i];

            if (a.rfind("--quantum=", 0) == 0) {
                std::string val = a.substr(strlen("--quantum="));
                if (!parseInt(val, quantum)) {
                    err << "Invalid quantum value: " << val << "\n";
                    return -1;
                }
            }
            else if (a == "--quantum" || a == "-q") {
                if (i + 1 >= args.size()) {
                    err << "Missing value for " << a << "\n";
                    return -1;
                }
                if (!parseInt(args[++i], quantum)) {
                    err << "Invalid quantum value: " << args[i] << "\n";
                    return -1;
                }
            }
        }

        return quantum;
    }

    int handleTick(const std::vector<std::string>& args,
                   std::ostream& out,
                   std::ostream& err,
                   SysApi& sys)
    {
        if (args.size() < 2) {
            err << "Usage: scheduler tick <ms>\n";
            return 1;
        }

        int ms;
        if (!parseInt(args[1], ms) || ms <= 0) {
            err << "Tick interval must be a positive integer\n";
            return 1;
        }

        if (!sys.setSchedulerTickIntervalMs(ms)) {
            err << "Failed to set scheduler tick interval\n";
            return 1;
        }

        out << "Scheduler tick interval set to: " << ms << " ms\n";
        return 0;
    }

    int handleCycles(const std::vector<std::string>& args,
                     std::ostream& out,
                     std::ostream& err,
                     SysApi& sys)
    {
        if (args.size() < 2) {
            err << "Usage: scheduler cycles <n>\n";
            return 1;
        }

        int cycles;
        if (!parseInt(args[1], cycles) || cycles <= 0) {
            err << "Cycles must be a positive integer\n";
            return 1;
        }

        if (!sys.setSchedulerCyclesPerInterval(cycles)) {
            err << "Failed to set scheduler cycles per interval\n";
            return 1;
        }

        out << "Scheduler cycles per interval set to: " << cycles << "\n";
        return 0;
    }
};

std::unique_ptr<ICommand> createSchedulerCommand() {
    return std::make_unique<SchedulerCommand>();
}

} // namespace shell
