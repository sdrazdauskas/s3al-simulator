#include "CommandAPI.h"
#include "SysCallsAPI.h"
#include <iostream>

namespace shell {

int cmd_save(const std::vector<std::string>& args,
             const std::string&,
             std::ostream& out,
             std::ostream& err,
             SysApi& sys) {
    if (args.size() < 1) {
        err << "Usage: save <name>\n";
        return 1;
    }
    auto fileName = args[0];
    auto res = sys.saveToDisk(fileName);
    out << "Save result: " << toString(res) << "\n";
    return res == SysResult::OK ? 0 : 1;
}

} // namespace shell
