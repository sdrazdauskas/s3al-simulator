#include "CommandAPI.h"

namespace shell {

int cmd_mv(const std::vector<std::string>& args,
           const std::string&,
           std::ostream& out,
           std::ostream& err,
           SysApi& sys) {
    if (args.size() != 2) {
        err << "Usage: mv <oldName> <newName>\n";
        return 1;
    }

    auto res = sys.moveFile(args[0], args[1]);
    if (res != SysResult::OK) {
        err << "mv: " << args[0] << " -> " << args[1]
            << ": " << toString(res) << "\n";
        return 1;
    }
    out << "Moved/Renamed file: " << args[0] << " -> " << args[1] << "\n";
    return 0;
}

}  // namespace shell
