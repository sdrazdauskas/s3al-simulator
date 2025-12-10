#include "shell/CommandAPI.h"
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
        auto info = sys.getSysInfo();
        double totalKb = static_cast<double>(info.totalMemory) / 1024.0;
        double usedKb  = static_cast<double>(info.usedMemory) / 1024.0;
        double freeKb  = totalKb > usedKb ? (totalKb - usedKb) : 0.0;

        std::ostringstream oss;
        oss << "=== Memory Info ===\n"
            << std::fixed << std::setprecision(2)
            << "Total: " << totalKb << " KB\n"
            << "Used : " << usedKb  << " KB\n"
            << "Free : " << freeKb  << " KB\n";

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
        auto info = sys.getSysInfo();
        double total = static_cast<double>(info.totalMemory);
        double used  = static_cast<double>(info.usedMemory);
        int barWidth = 40;

        double ratio = (total > 0.0) ? (used / total) : 0.0;
        int usedBlocks = static_cast<int>(ratio * barWidth);

        std::ostringstream oss;
        oss << "[Memory] [";
        for (int i = 0; i < barWidth; ++i)
            oss << (i < usedBlocks ? '#' : '-');
            oss << "] " << std::fixed << std::setprecision(2)
            << (ratio * 100.0) << "% used\n";

        out << oss.str();
        return 0;
    }
    
    const char* getName() const override { return "membar"; }
    const char* getDescription() const override { return "Display memory usage bar"; }
    const char* getUsage() const override { return "membar"; }
};

std::unique_ptr<ICommand> createMeminfoCommand() {
    return std::make_unique<MeminfoCommand>();
}

std::unique_ptr<ICommand> createMembarCommand() {
    return std::make_unique<MembarCommand>();
}

} // namespace shell
