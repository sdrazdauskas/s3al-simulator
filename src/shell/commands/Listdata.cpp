#include "shell/CommandAPI.h"
#include <memory>

namespace shell {

class ListDataCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& /*args*/,
                const std::string& /*input*/,
                std::ostream& out,
                std::ostream& err,
                SysApi& sys) override
    {
        std::vector<std::string> files;
        auto res = sys.listDataFiles(files);

        if (res == SysResult::NotFound) {
            out << "(no saved data files found)\n";
            return 0;
        }
        if (res != SysResult::OK) {
            err << "listdata: " << shell::toString(res) << "\n";
            return 1;
        }

        out << "Available saved data files:\n";
        for (const auto& f : files) {
            out << "  - " << f << "\n";
        }
        return 0;
    }

    const char* getName() const override { return "listdata"; }
    const char* getDescription() const override {return "List all available saved data files";}
    const char* getUsage() const override { return "listdata"; }
};

std::unique_ptr<ICommand> create_listdata_command() {
    return std::make_unique<ListDataCommand>();
}

} // namespace shell
