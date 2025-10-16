#include "CommandParser.h"
#include <sstream>
#include <cctype>

namespace shell {

namespace {

// Helper class to track quote and escape state during parsing
class QuoteTracker {
public:
    bool inQuotes = false;
    bool inSingleQuotes = false;
    bool escape = false;

    // Process a character and return true if it's a special character (quote/escape)
    // that was handled and should not be added to output
    bool processChar(char c, bool& skipChar) {
        skipChar = false;

        if (escape) {
            escape = false;
            return false; // Character after escape should be added
        }

        if (c == '\\') {
            escape = true;
            return true; // Don't add backslash itself (unless caller wants to)
        }

        if (c == '"' && !inSingleQuotes) {
            inQuotes = !inQuotes;
            skipChar = true;
            return true;
        }

        if (c == '\'' && !inQuotes) {
            inSingleQuotes = !inSingleQuotes;
            skipChar = true;
            return true;
        }

        return false;
    }

    bool isInQuotes() const {
        return inQuotes || inSingleQuotes;
    }
};

// Tokenize a single command line into words, respecting quotes
std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::string current;
    QuoteTracker tracker;

    for (char c : line) {
        bool skipChar;
        tracker.processChar(c, skipChar);

        if (skipChar) {
            continue;
        }

        if (!tracker.isInQuotes() && std::isspace(static_cast<unsigned char>(c))) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            continue;
        }

        current += c;
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}

// Split input on && and newlines, returning individual command strings
std::vector<std::string> splitCommands(const std::string& input) {
    std::vector<std::string> commands;
    std::string current;
    QuoteTracker tracker;

    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];

        bool skipChar;
        bool isSpecial = tracker.processChar(c, skipChar);

        // For splitCommands, we want to preserve quotes in the output
        if (isSpecial && (c == '"' || c == '\'')) {
            current += c;
            continue;
        }

        if (tracker.escape) {
            current += c;
            continue;
        }

        // Check for && outside quotes
        if (!tracker.isInQuotes() && c == '&' && i + 1 < input.size() && input[i + 1] == '&') {
            if (!current.empty()) {
                commands.push_back(current);
                current.clear();
            }
            ++i; // Skip second &
            continue;
        }

        // Check for newline outside quotes
        if (!tracker.isInQuotes() && c == '\n') {
            if (!current.empty()) {
                commands.push_back(current);
                current.clear();
            }
            continue;
        }

        current += c;
    }

    if (!current.empty()) {
        commands.push_back(current);
    }

    return commands;
}

// Trim leading and trailing whitespace
std::string trim(const std::string& str) {
    size_t start = 0;
    size_t end = str.size();

    while (start < end && std::isspace(static_cast<unsigned char>(str[start]))) {
        ++start;
    }

    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        --end;
    }

    return str.substr(start, end - start);
}

} // anonymous namespace

std::vector<Command> parseCommands(const std::string& input) {
    std::vector<Command> result;

    // Split input into individual command strings
    std::vector<std::string> commandStrings = splitCommands(input);

    for (const auto& cmdStr : commandStrings) {
        std::string trimmed = trim(cmdStr);
        if (trimmed.empty()) {
            continue;
        }

        // Tokenize the command string
        std::vector<std::string> tokens = tokenize(trimmed);
        if (tokens.empty()) {
            continue;
        }

        Command cmd;
        cmd.command = tokens[0];
        if (tokens.size() > 1) {
            cmd.args.assign(tokens.begin() + 1, tokens.end());
        }

        result.push_back(cmd);
    }

    return result;
}

} // namespace shell
