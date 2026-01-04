#pragma once

#include <string>
#include <functional>

namespace common {

/**
 * ANSI color codes and utilities for terminal output
 */
class ColorUtils {
public:
    // Control codes
    static constexpr const char* RESET = "\033[0m";
    static constexpr const char* BOLD = "\033[1m";
    static constexpr const char* DIM = "\033[2m";
    static constexpr const char* UNDERLINE = "\033[4m";
    
    // Basic colors
    static constexpr const char* BLACK = "\033[30m";
    static constexpr const char* RED = "\033[31m";
    static constexpr const char* GREEN = "\033[32m";
    static constexpr const char* YELLOW = "\033[33m";
    static constexpr const char* BLUE = "\033[34m";
    static constexpr const char* MAGENTA = "\033[35m";
    static constexpr const char* CYAN = "\033[36m";
    static constexpr const char* WHITE = "\033[37m";
    
    // Bright colors
    static constexpr const char* BRIGHT_BLACK = "\033[90m";
    static constexpr const char* BRIGHT_RED = "\033[91m";
    static constexpr const char* BRIGHT_GREEN = "\033[92m";
    static constexpr const char* BRIGHT_YELLOW = "\033[93m";
    static constexpr const char* BRIGHT_BLUE = "\033[94m";
    static constexpr const char* BRIGHT_MAGENTA = "\033[95m";
    static constexpr const char* BRIGHT_CYAN = "\033[96m";
    static constexpr const char* BRIGHT_WHITE = "\033[97m";
    
    // Underlined colors
    static constexpr const char* UNDERLINE_BLACK = "\033[4;30m";
    static constexpr const char* UNDERLINE_RED = "\033[4;31m";
    static constexpr const char* UNDERLINE_GREEN = "\033[4;32m";
    static constexpr const char* UNDERLINE_YELLOW = "\033[4;33m";
    static constexpr const char* UNDERLINE_BLUE = "\033[4;34m";
    static constexpr const char* UNDERLINE_MAGENTA = "\033[4;35m";
    static constexpr const char* UNDERLINE_CYAN = "\033[4;36m";
    static constexpr const char* UNDERLINE_WHITE = "\033[4;37m";
    
    /**
     * Wrap text with color codes
     * @param text The text to colorize
     * @param color The ANSI color code
     * @param bold Whether to make the text bold
     * @return Colorized text with reset code at the end
     */
    static std::string colorize(const std::string& text, const char* color, bool bold = false) {
        std::string result;
        if (bold) {
            result = std::string(BOLD) + color + text + RESET;
        } else {
            result = std::string(color) + text + RESET;
        }
        return result;
    }
    
    /**
     * Remove all ANSI color codes from a string
     * @param text The text to strip
     * @return Text without color codes
     */
    static std::string stripColors(const std::string& text) {
        std::string result;
        bool inEscape = false;
        
        for (char c : text) {
            if (c == '\033') {
                inEscape = true;
            } else if (inEscape && c == 'm') {
                inEscape = false;
            } else if (!inEscape) {
                result += c;
            }
        }
        
        return result;
    }
};

} // namespace common
