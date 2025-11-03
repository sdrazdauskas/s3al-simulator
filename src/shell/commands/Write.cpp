#include "CommandAPI.h"
#include <iostream>

namespace shell {

int cmd_write(const std::vector<std::string>& args,
              const std::string& /*input*/,
              std::ostream& out,
              std::ostream& err,
              SysApi& sys)
{
    if (args.size() < 2) {
        err << "Usage: write <filename> <content>\n";
        return 1;
    }
    std::string content;
    for (size_t i = 1; i < args.size(); ++i) content += args[i] + (i + 1 < args.size() ? " " : "");

    auto exists = sys.fileExists(args[0]);
    if (exists != SysResult::OK) {
        err << "write: " << args[0] << ": " << shell::toString(exists) << "\n";
        return 1;
    }
    auto res = sys.writeFile(args[0], content);
    out << "write: " << args[0] << ": " << shell::toString(res) << "\n";
    return (res == SysResult::OK) ? 0 : 1;
}

} // namespace shell
