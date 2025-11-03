#include "CommandAPI.h"
#include <iostream>

namespace shell {

int cmd_cd(const std::vector<std::string>& args,
           const std::string& /*input*/,
           std::ostream& out,
           std::ostream& err,
           SysApi& sys)
{
    if (args.empty()) {
        err << "Usage: cd <foldername|..>\n";
        return 1;
    }
    auto res = sys.changeDir(args[0]);
    if (res != SysResult::OK) {
        err << "cd: " << args[0] << ": " << shell::toString(res) << "\n";
        return 1;
    }
    out << sys.getWorkingDir() << "\n";
    return 0;
}

} // namespace shell
