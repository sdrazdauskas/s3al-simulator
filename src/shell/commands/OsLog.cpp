
#include "shell/CommandAPI.h"
#include <string>
#include <map>

namespace shell {

class OsLogCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& args,
                const std::string& /*input*/,
                std::ostream& out,
                std::ostream& err,
                SysApi& sys) override
    {
        if (!requireArgs(args, 0, err, 1)) return 1;

        if (args.empty()) {
            out << "Logging is: " << (sys.getConsoleOutput() ? "on" : "off") << "\n";
            out << "Current minimum log level: " << sys.getLogLevel() << "\n";
            return 0;
        }
        std::string arg = args[0];
        if (arg == "off") {
            sys.setConsoleOutput(false);
            out << "Logging disabled.\n";
        } else {
            static const std::map<std::string, logging::LogLevel> logLevelMap = {
                {"debug", logging::LogLevel::DEBUG},
                {"info", logging::LogLevel::INFO},
                {"warn", logging::LogLevel::WARNING},
                {"error", logging::LogLevel::ERROR}
            };
            auto it = logLevelMap.find(arg);
            if (it != logLevelMap.end()) {
                sys.setConsoleOutput(true);
                sys.setLogLevel(it->second);
                out << "Log level set to " << arg << ".\n";
            } else {
                err << "Invalid argument. Use 'off', or log level ('debug', 'info', 'warn', 'error').\n";
                return 1;
            }
        }
        return 0;
    }
    const char* getName() const override { return "oslog"; }
    const char* getDescription() const override { return "Enable or disable OS logging."; }
    const char* getUsage() const override { return "oslog [off|debug|info|warn|error]"; }
    int getCpuCost() const override { return 1; }
};

std::unique_ptr<ICommand> createOsLogCommand() {
    return std::make_unique<OsLogCommand>();
}

} // namespace shell
