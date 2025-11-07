#include "../CommandAPI.h"
#include <memory>

namespace shell {

class EchoCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& args,
                const std::string& /*input*/,
                std::ostream& out,
                std::ostream& err,
                SysApi& /*sys*/) override
    {
        if (args.empty()) {
            err << "Usage: echo [text]\n";
            return 1;
        }
        for (size_t i = 0; i < args.size(); ++i) {
            out << args[i] << (i + 1 < args.size() ? " " : "");
        }
        out << "\n";
        return 0;
    }
    
    const char* getName() const override { return "echo"; }
    const char* getDescription() const override { return "Print text to output"; }
    const char* getUsage() const override { return "echo [text]"; }
};

std::unique_ptr<ICommand> create_echo_command() {
    return std::make_unique<EchoCommand>();
}

} // namespace shell
