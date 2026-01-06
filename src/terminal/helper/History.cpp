#include "terminal/helper/History.h"
#include <unistd.h>

namespace terminal {

    History::History() {
        // Save original terminal settings and configure raw mode
        tcgetattr(STDIN_FILENO, &originalMode);
        rawMode = originalMode;
        rawMode.c_lflag &= ~(ECHO | ICANON);
        rawMode.c_cc[VMIN] = 1;
        rawMode.c_cc[VTIME] = 0;
    }

    History::~History() {
        // Ensure terminal is restored to original mode
        if (rawModeEnabled) {
            disableRawMode();
        }
    }

    void History::enableRawMode() {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &rawMode);
        rawModeEnabled = true;
    }

    void History::disableRawMode() {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalMode);
        rawModeEnabled = false;
    }

    void History::temporarilyRestoreMode() {
        // Restore cooked mode temporarily (e.g., for command execution)
        if (rawModeEnabled) {
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalMode);
        }
    }

    void History::temporarilyEnableRawMode() {
        // Return to raw mode after temporary restore
        if (rawModeEnabled) {
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &rawMode);
        }
    }

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

    bool History::navigate(char key, std::string& buffer, size_t& cursor) {
        std::string hist;
        
        switch (key) {
            case 'A': // UP
                if (!prev(hist)) return false;
                buffer = hist;
                cursor = buffer.size();
                break;
            case 'B': // DOWN
                if (next(hist)) {
                    buffer = hist;
                    cursor = buffer.size();
                } else {
                    buffer.clear();
                    cursor = 0;
                }
                break;
            default:
                return false;
        }
        
        return true;
    }

} // namespace terminal
