#include "CommandAPI.h"
#include <iostream>

namespace shell {

int cmd_rmdir(const std::vector<std::string>& args,
              const std::string& /*input*/,
              std::ostream& out,
              std::ostream& err,
              SysApi& sys)
{
    if (args.empty()) {
        err << "Usage: rmdir <foldername>\n";
        return 1;
    }
    auto res = sys.removeDir(args[0]);
    if (res != SysResult::OK) {
        err << "rmdir: " << args[0] << ": " << shell::toString(res) << "\n";
        return 1;
    }
    out << "rmdir: " << args[0] << ": " << shell::toString(res) << "\n";
    return 0;
}

} // namespace shell
