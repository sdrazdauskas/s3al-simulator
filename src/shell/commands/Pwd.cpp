#include "CommandAPI.h"
#include <iostream>

namespace shell {

int cmd_pwd(const std::vector<std::string>& /*args*/,
            const std::string& /*input*/,
            std::ostream& out,
            std::ostream& /*err*/,
            SysApi& sys)
{
    out << sys.getWorkingDir() << "\n";
    return 0;
}

} // namespace shell
