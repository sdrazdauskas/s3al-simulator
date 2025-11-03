#include "CommandAPI.h"
#include <iostream>

namespace shell {

int cmd_edit(const std::vector<std::string>& args,
             const std::string& /*input*/,
             std::ostream& out,
             std::ostream& err,
             SysApi& sys) {
    if (args.size() < 1) {
        err << "Usage: edit <filename>\n";
        return 1;
    }

    const std::string& filename = args[0];

    auto exists = sys.fileExists(filename);
    if (exists == SysResult::NotFound) {
        err << "edit: " << filename << ": file not found\n";
        return 1;
    }

    auto res = sys.editFile(filename);
    out << "edit: " << filename << ": "
        << shell::toString(res) << "\n";
    return (res == SysResult::OK) ? 0 : 1;
}

} // namespace shell
