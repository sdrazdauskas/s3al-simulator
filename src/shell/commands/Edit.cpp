#include "CommandAPI.h"
#include <iostream>

namespace shell {

int cmd_edit(const std::vector<std::string>& args,
             const std::string& /*input*/,
             std::ostream& out,
             std::ostream& err,
             SysApi& sys)
{
    if (args.empty()) {
        err << "Usage: edit <filename>\n";
        return 1;
    }
    
    const std::string& filename = args[0];

    std::string content;
    auto readResult = sys.readFile(filename, content);
    if (readResult != SysResult::OK) {
        err << "edit: " << filename << ": " << shell::toString(readResult) << "\n";
        return 1;
    }

    out << "=== contents of " << filename << " ===\n";
    if (content.empty())
        out << "(empty)\n";
    else
        out << content;
    out << "--------------------------------------\n";
    out << "Type new content below to ADD to the file.\n";
    out << "Type ':wq' on a new line to save and exit.\n";
    out << "--------------------------------------\n";

    std::string newLines, line;
    while (true) {
        std::getline(std::cin, line);
        if (line == ":wq")
            break;
        newLines += line + "\n";
    }

    auto res = sys.editFile(filename, newLines);
    out << "edit: " << filename << ": " << shell::toString(res) << "\n";
    return (res == SysResult::OK) ? 0 : 1;
}

} // namespace shell
