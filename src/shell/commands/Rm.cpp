#include "CommandAPI.h"
#include <iostream>

namespace shell {

int cmd_rm(const std::vector<std::string>& args,
           const std::string& /*input*/,
           std::ostream& out,
           std::ostream& err,
           SysApi& sys)
{
    if (args.empty()) {
        err << "Usage: rm <filename> [filename...]\n";
        return 1;
    }
    int rc = 0;
    for (const auto& name : args) {
        auto res = sys.deleteFile(name);
        if (res != SysResult::OK) {
            err << "rm: " << name << ": " << shell::toString(res) << "\n";
            rc = 1;
        } else {
            out << "rm: " << name << ": " << shell::toString(res) << "\n";
        }
    }
    return rc;
}

} // namespace shell
