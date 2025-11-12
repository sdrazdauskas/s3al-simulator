#include "../CommandAPI.h"
#include <sstream>
#include <memory>

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
