#include "CommandAPI.h"

namespace shell {

int cmd_touch(const std::vector<std::string>& args,
              const std::string& /*input*/,
              std::ostream& out,
              std::ostream& err,
              SysApi& sys)
{
    if (args.empty()) {
        err << "Usage: touch <filename> [filename...]\n";
        return 1;
    }

    int rc = 0;
    for (const auto& name : args) {
        auto res = sys.createFile(name);
        if (res != shell::SysResult::OK) {
            err << "touch: " << name << ": " << shell::toString(res) << "\n";
            rc = 1;
        } else {
            out << "touch: " << name << ": " << shell::toString(res) << "\n";
        }
    }
    return rc;
}

} // namespace shell
