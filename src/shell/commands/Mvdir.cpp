#include "CommandAPI.h"

namespace shell {

int cmd_mvdir(const std::vector<std::string>& args,
              const std::string&,
              std::ostream& out,
              std::ostream& err,
              SysApi& sys) {
    if (args.size() != 2) {
        err << "Usage: mvdir <oldDir> <newDir>\n";
        return 1;
    }

    auto res = sys.moveDir(args[0], args[1]);
    if (res != SysResult::OK) {
        err << "mvdir: " << args[0] << " -> " << args[1]
            << ": " << toString(res) << "\n";
        return 1;
    }
    out << "Moved/Renamed directory: " << args[0] << " -> " << args[1] << "\n";
    return 0;
}

}  // namespace shell
