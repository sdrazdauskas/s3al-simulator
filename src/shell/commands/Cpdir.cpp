#include "CommandAPI.h"

namespace shell {

int cmd_cpdir(const std::vector<std::string>& args,
              const std::string&,
              std::ostream& out,
              std::ostream& err,
              SysApi& sys) {
    if (args.size() != 2) {
        err << "Usage: cpdir <srcDir> <destDir>\n";
        return 1;
    }

    auto res = sys.copyDir(args[0], args[1]);
    if (res != SysResult::OK) {
        err << "cpdir: " << args[0] << " -> " << args[1]
            << ": " << toString(res) << "\n";
        return 1;
    }
    out << "Copied directory: " << args[0] << " -> " << args[1] << "\n";
    return 0;
}

}  // namespace shell
