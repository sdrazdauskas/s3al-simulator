#include "../CommandAPI.h"
#include <sstream>
#include <memory>
#include <iomanip>

namespace shell {

class MeminfoCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& /*args*/,
                const std::string& /*input*/,
                std::ostream& out,
                std::ostream& /*err*/,
                SysApi& sys) override
    {
        auto info = sys.get_sysinfo();
        double total_kb = static_cast<double>(info.total_memory) / 1024.0;
        double used_kb  = static_cast<double>(info.used_memory) / 1024.0;
        double free_kb  = total_kb > used_kb ? (total_kb - used_kb) : 0.0;

        std::ostringstream oss;
        oss << "=== Memory Info ===\n"
            << std::fixed << std::setprecision(2)
            << "Total: " << total_kb << " KB\n"
            << "Used : " << used_kb  << " KB\n"
            << "Free : " << free_kb  << " KB\n";

        out << oss.str();
        return 0;
    }
    
    const char* getName() const override { return "meminfo"; }
    const char* getDescription() const override { return "Display memory info summary"; }
    const char* getUsage() const override { return "meminfo"; }
};

class MembarCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& /*args*/,
                const std::string& /*input*/,
                std::ostream& out,
                std::ostream& /*err*/,
                SysApi& sys) override
    {
        auto info = sys.get_sysinfo();
        double total = static_cast<double>(info.total_memory);
        double used  = static_cast<double>(info.used_memory);
        int bar_width = 40;

        double ratio = (total > 0.0) ? (used / total) : 0.0;
        int used_blocks = static_cast<int>(ratio * bar_width);

        std::ostringstream oss;
        oss << "[Memory] [";
        for (int i = 0; i < bar_width; ++i)
            oss << (i < used_blocks ? '#' : '-');
            oss << "] " << std::fixed << std::setprecision(2)
            << (ratio * 100.0) << "% used\n";

        out << oss.str();
        return 0;
    }
    
    const char* getName() const override { return "membar"; }
    const char* getDescription() const override { return "Display memory usage bar"; }
    const char* getUsage() const override { return "membar"; }
};

std::unique_ptr<ICommand> create_meminfo_command() {
    return std::make_unique<MeminfoCommand>();
}

std::unique_ptr<ICommand> create_membar_command() {
    return std::make_unique<MembarCommand>();
}

} // namespace shell
