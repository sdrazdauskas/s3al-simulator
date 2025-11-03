#include "CommandAPI.h"
#include <sstream>

namespace shell {

int cmd_meminfo(const std::vector<std::string>& /*args*/, const std::string& /*input*/, std::ostream& out, std::ostream& /*err*/, SysApi& sys) {
    auto info = sys.get_sysinfo();
    size_t total = info.total_memory;
    size_t used = info.used_memory;
    size_t free = (total > used) ? (total - used) : 0;

    std::ostringstream oss;
    oss << "=== Memory Info ===\n";
    oss << "Total: " << total / 1024 << " KB\n";
    oss << "Used : " << used / 1024 << " KB\n";
    oss << "Free : " << free / 1024 << " KB\n";

    out << oss.str();
    return 0;
}

int cmd_membar(const std::vector<std::string>& /*args*/, const std::string& /*input*/, std::ostream& out, std::ostream& /*err*/, SysApi& sys) {
    auto info = sys.get_sysinfo();
    size_t total = info.total_memory;
    size_t used = info.used_memory;
    int bar_width = 40;
    int used_blocks = total > 0 ? static_cast<int>((double)used / total * bar_width) : 0;

    std::ostringstream oss;
    oss << "[Memory] [";
    for (int i = 0; i < bar_width; ++i) {
        oss << (i < used_blocks ? '#' : '-');
    }
    oss << "] " << (total > 0 ? (used * 100 / total) : 0) << "% used\n";

    out << oss.str();
    return 0;
}

} // namespace shell
