#include "CommandAPI.h"
#include "SysCallsAPI.h"
#include <iostream>

namespace shell {

int cmd_reset(const std::vector<std::string>& args,
              const std::string&,
              std::ostream& out,
              std::ostream& err,
              SysApi& sys) {

    std::string confirm;
    std::cout << "Are you sure you want to reset current storage? (yes/no): ";
    std::getline(std::cin, confirm);

    if (confirm != "yes") {
        out << "Reset aborted.\n";
        return 0;
    }

    auto res = sys.resetStorage();
    out << "Reset result: " << toString(res) << "\n";
    return res == SysResult::OK ? 0 : 1;
}

} // namespace shell
