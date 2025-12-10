#include "shell/CommandAPI.h"
#include <iostream>
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

        const std::string& fileName = args[0];

        std::string content;
        auto readResult = sys.readFile(fileName, content);
        if (readResult != SysResult::OK) {
            err << "edit: " << fileName << ": " << shell::toString(readResult) << "\n";
            return 1;
        }

        out << "=== contents of " << fileName << " ===\n";
        if (content.empty())
            out << "(empty)\n";
        else
            out << content;
        out << "--------------------------------------\n";
        out << "Type new content below to ADD to the file.\n";
        out << "Type ':wq' on a new line to save and exit.\n";
        out << "--------------------------------------\n";

        std::string newLines;
        while (true) {
            std::string line = sys.readLine();
            if (line == ":wq")
                break;
            newLines += line + "\n";
        }

        auto res = sys.editFile(fileName, newLines);
        out << "edit: " << fileName << ": " << shell::toString(res) << "\n";
        return (res == SysResult::OK) ? 0 : 1;
    }

    const char* getName() const override { return "edit"; }
    const char* getDescription() const override {return "Open an editor to append text to a file";}
    const char* getUsage() const override { return "edit <fileName>"; }
};

std::unique_ptr<ICommand> createEditCommand() {
    return std::make_unique<EditCommand>();
}

} // namespace shell