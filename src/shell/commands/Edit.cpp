#include "../CommandAPI.h"
#include <memory>

namespace shell {

class EditCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& args,
                const std::string& /*input*/,
                std::ostream& out,
                std::ostream& err,
                SysApi& sys) override
    {
        if (args.size() < 1) {
            err << "Usage: " << getUsage() << "\n";
            return 1;
        }

        const std::string& filename = args[0];

        auto exists = sys.fileExists(filename);
        if (exists == SysResult::NotFound) {
            err << "edit: " << filename << ": file not found\n";
            return 1;
        }

        auto res = sys.editFile(filename);
        out << "edit: " << filename << ": "
            << shell::toString(res) << "\n";
        return (res == SysResult::OK) ? 0 : 1;
    }
    
    const char* getName() const override { return "edit"; }
    const char* getDescription() const override { return "Open editor to append text to a file"; }
    const char* getUsage() const override { return "edit <filename>"; }
};

std::unique_ptr<ICommand> create_edit_command() {
    return std::make_unique<EditCommand>();
}

} // namespace shell
