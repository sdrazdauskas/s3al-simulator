#include "terminal/helper/History.h"

namespace terminal {

    void History::add(const std::string& cmd) {
        if (cmd.empty()) return;
        entries.push_back(cmd);
        index = entries.size(); // Reset index to the "end" for new input
    }

    bool History::prev(std::string& out) {
        if (entries.empty() || index == 0) return false;
        --index;
        out = entries[index];
        return true;
    }

    bool History::next(std::string& out) {
        if (entries.empty() || index >= entries.size() - 1) {
            index = entries.size();
            out.clear();
            return false;
        }
        ++index;
        out = entries[index];
        return true;
    }

} // namespace terminal
