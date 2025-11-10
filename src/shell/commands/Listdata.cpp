#include "CommandAPI.h"

namespace shell {

int cmd_listdata(const std::vector<std::string>&,
                 const std::string&,
                 std::ostream& out,
                 std::ostream& err,
                 SysApi& sys) {
    std::vector<std::string> files;
    auto res = sys.listDataFiles(files);

    if (res == SysResult::NotFound) {
        out << "(no saved data files found)\n";
        return 0;
    }
    if (res != SysResult::OK) {
        err << "listdata: " << toString(res) << "\n";
        return 1;
    }

    out << "Available saved data files:\n";
    for (const auto& f : files) out << "  - " << f << "\n";
    return 0;
}

}  // namespace shell
