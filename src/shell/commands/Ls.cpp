#include "CommandAPI.h"
#include <iostream>

namespace shell {

int cmd_ls(const std::vector<std::string>& args,
           const std::string& /*input*/,
           std::ostream& out,
           std::ostream& err,
           SysApi& sys)
{
    std::string path = args.empty() ? "." : args[0];
    std::vector<std::string> entries;
    auto res = sys.listDir(path, entries);
    
    if (res != SysResult::OK) {
        err << "ls: " << path << ": " << toString(res) << "\n";
        return 1;
    }
    
    if (entries.empty()) {
        out << "(empty)\n";
    } else {
        for (const auto& e : entries) {
            out << e << "\n";
        }
    }
    return 0;
}

} // namespace shell
