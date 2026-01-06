#pragma once
#include <vector>
#include <string>
#include <termios.h>

namespace terminal {

    class History {
    public:
        History();
        ~History();
        
        void add(const std::string& cmd);
        bool prev(std::string& out);
        bool next(std::string& out);
        
        // Navigate history with arrow keys (UP='A', DOWN='B')
        // Returns true if buffer was modified, updates buffer and cursor
        bool navigate(char key, std::string& buffer, size_t& cursor);
        
        // Terminal mode management
        void enableRawMode();
        void disableRawMode();
        void temporarilyRestoreMode();
        void temporarilyEnableRawMode();

    private:
        std::vector<std::string> entries;
        size_t index = 0;
        
        struct termios originalMode;
        struct termios rawMode;
        bool rawModeEnabled = false;
    };

} // namespace terminal
