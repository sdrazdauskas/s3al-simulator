#include "CommandAPI.h"
#include <iostream>

namespace shell {

int cmd_quit(const std::vector<std::string>& /*args*/,
    const std::string& /*input*/,
    std::ostream& out, std::ostream& /*err*/,
    SysApi& sys)
    
{
    out << "Shutting down..." << std::endl;
    sys.requestShutdown();
    return 0;
}

} // namespace shell
