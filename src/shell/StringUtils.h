#pragma once

#include <string>
#include <vector>
#include <sstream>

namespace shell {

class StringUtils {
public:
    // Trim whitespace from both ends
    static std::string trim(const std::string& s) {
        auto start = s.find_first_not_of(" \t");
        auto end = s.find_last_not_of(" \t");
        return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    }

    // Extract substring after a symbol/delimiter
    static std::string extractAfter(const std::string& s, const std::string& symbol) {
        size_t pos = s.find(symbol);
        if (pos == std::string::npos)
            return "";
        return trim(s.substr(pos + symbol.size()));
    }

    // Extract substring before a symbol/delimiter
    static std::string extractBefore(const std::string& s, const std::string& symbol) {
        size_t pos = s.find(symbol);
        if (pos == std::string::npos)
            return trim(s);
        return trim(s.substr(0, pos));
    }

    // Split by delimiter, trimming each part
    static std::vector<std::string> splitBy(const std::string& str, const std::string& delimiter) {
        std::vector<std::string> parts;
        std::string temp = str;
        size_t pos = 0;

        while ((pos = temp.find(delimiter)) != std::string::npos) {
            std::string part = trim(temp.substr(0, pos));
            if (!part.empty()) {
                parts.push_back(part);
            }
            temp.erase(0, pos + delimiter.size());
        }

        std::string lastPart = trim(temp);
        if (!lastPart.empty()) {
            parts.push_back(lastPart);
        }

        return parts;
    }

    // Parse quoted token from stream (handles "quoted strings")
    static std::string parseQuotedToken(std::istringstream& iss, std::string token) {
        std::string quoted = token.substr(1);
        std::string next;
        while (!quoted.empty() && quoted.back() != '"' && iss >> next) {
            quoted += " " + next;
        }
        if (!quoted.empty() && quoted.back() == '"')
            quoted.pop_back();
        return quoted;
    }

    // Parse command line into command and arguments
    static void parseCommand(const std::string& commandLine, std::string& command, std::vector<std::string>& args) {
        std::istringstream iss(commandLine);
        iss >> command;

        args.clear();
        std::string token;
        while (iss >> token) {
            if (!token.empty() && token.front() == '"')
                args.push_back(parseQuotedToken(iss, token));
            else
                args.push_back(token);
        }
    }
};

} // namespace shell
