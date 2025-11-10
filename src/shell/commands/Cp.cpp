#include "CommandAPI.h"

namespace shell {

int cmd_cp(const std::vector<std::string>& args,
           const std::string&,
           std::ostream& out,
           std::ostream& err,
           SysApi& sys) {
    if (args.size() != 2) {
        err << "Usage: cp <source> <destination>\n";
        return 1;
    }

    auto res = sys.copyFile(args[0], args[1]);
    if (res != SysResult::OK) {
        err << "cp: " << args[0] << " -> " << args[1]
            << ": " << toString(res) << "\n";
        return 1;
    }
    out << "Copied file: " << args[0] << " -> " << args[1] << "\n";
    return 0;
}

}  // namespace shell
