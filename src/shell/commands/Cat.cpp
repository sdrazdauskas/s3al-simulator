#include "CommandAPI.h"

namespace shell {

int cmd_cat(const std::vector<std::string>& args,
            const std::string& /*input*/,
            std::ostream& out,
            std::ostream& err,
            SysApi& sys)
{
    if (args.empty()) {
        err << "Usage: cat <filename> [filename...]\n";
        return 1;
    }

    int rc = 0;
    for (const auto& name : args) {
        std::string content;
        auto r = sys.readFile(name, content);
        if (r != shell::SysResult::OK) {
            err << "cat: " << name << ": " << shell::toString(r) << "\n";
            rc = 1;
            continue;
        }

        out << "=== contents of " << name << " ===\n";
        if (content.empty())
            out << "(empty)\n";
        else
            out << content;
        out << "=============================\n";
    }
    return rc;
}

} // namespace shell
