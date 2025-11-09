#include "CommandAPI.h"
#include <iostream>

namespace shell {

int cmd_echo(const std::vector<std::string>& args,
             const std::string& /*input*/,
             std::ostream& out,
             std::ostream& err,
             SysApi& /*sys*/)
{
    if (args.empty()) {
        err << "Usage: echo <text>\n";
        return 1;
    }
    for (size_t i = 0; i < args.size(); ++i) {
        out << args[i] << (i + 1 < args.size() ? " " : "");
    }
    out << "\n";
    return 0;
}

} // namespace shell
