#include "CommandAPI.h"
#include <iostream>

namespace shell {

int cmd_ls(const std::vector<std::string>& /*args*/,
           const std::string& /*input*/,
           std::ostream& out,
           std::ostream& /*err*/,
           SysApi& sys)
{
    auto entries = sys.listDir();
    if (entries.empty()) out << "(empty)\n";
    for (const auto& e : entries) out << e << "\n";
    return 0;
}

} // namespace shell
