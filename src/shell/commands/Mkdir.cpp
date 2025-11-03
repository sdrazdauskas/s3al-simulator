#include "CommandAPI.h"
#include <iostream>

namespace shell {

int cmd_mkdir(const std::vector<std::string>& args,
              const std::string& /*input*/,
              std::ostream& out,
              std::ostream& err,
              SysApi& sys)
{
    if (args.empty()) {
        err << "Usage: mkdir <foldername>\n";
        return 1;
    }
    auto res = sys.makeDir(args[0]);
    if (res != SysResult::OK) {
        err << "mkdir: " << args[0] << ": " << shell::toString(res) << "\n";
        return 1;
    }
    out << "mkdir: " << args[0] << ": " << shell::toString(res) << "\n";
    return 0;
}

} // namespace shell
