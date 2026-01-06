#pragma once

#include "shell/StringUtils.h"
#include <string>
#include <vector>
#include <optional>

namespace shell {

// Represents redirection for a command
struct Redirection {
    enum class Type { INPUT, OUTPUT, APPEND };
    Type type;
    std::string fileName;
};

// Represents a single parsed command segment
struct CommandSegment {
    std::string command;
    std::vector<std::string> args;
    std::optional<Redirection> inputRedirect;
    std::optional<Redirection> outputRedirect;
    bool isPipedToNext = false;  // True if this command pipes to the next one
};

// Represents a chain of commands connected by AND (&&)
struct CommandChain {
    std::vector<CommandSegment> segments;
};

// Main command parser
class CommandParser {
public:
    // Parse a full command line into chains (separated by &&)
    static std::vector<CommandChain> parse(const std::string& commandLine) {
        std::vector<CommandChain> chains;
        
        // Split by AND operator
        auto andCommands = StringUtils::splitBy(commandLine, "&&");
        
        for (const auto& andCmd : andCommands) {
            CommandChain chain;
            chain.segments = parseChain(andCmd);
            chains.push_back(chain);
        }
        
        return chains;
    }

private:
    // Parse a single chain (commands separated by pipes)
    static std::vector<CommandSegment> parseChain(const std::string& chainText) {
        std::vector<CommandSegment> segments;
        
        // Split by pipe operator
        auto pipeCommands = StringUtils::splitBy(chainText, "|");
        
        for (size_t i = 0; i < pipeCommands.size(); ++i) {
            CommandSegment segment = parseSegment(pipeCommands[i]);
            segment.isPipedToNext = (i < pipeCommands.size() - 1);
            segments.push_back(segment);
        }
        
        return segments;
    }
    
    // Parse a single command segment with redirections
    static CommandSegment parseSegment(const std::string& segmentText) {
        CommandSegment segment;
        std::string cleanText = segmentText;
        
        // Check for input redirection (<)
        if (cleanText.find('<') != std::string::npos) {
            std::string fileName = StringUtils::extractAfter(cleanText, "<");
            if (!fileName.empty()) {
                segment.inputRedirect = Redirection{Redirection::Type::INPUT, fileName};
            }
            cleanText = StringUtils::extractBefore(cleanText, "<");
        }
        
        // Check for append redirection (>>)
        if (cleanText.find(">>") != std::string::npos) {
            std::string fileName = StringUtils::extractAfter(cleanText, ">>");
            if (!fileName.empty()) {
                segment.outputRedirect = Redirection{Redirection::Type::APPEND, fileName};
            }
            cleanText = StringUtils::extractBefore(cleanText, ">>");
        }
        // Check for output redirection (>) - only if >> not found
        else if (cleanText.find('>') != std::string::npos) {
            std::string fileName = StringUtils::extractAfter(cleanText, ">");
            if (!fileName.empty()) {
                segment.outputRedirect = Redirection{Redirection::Type::OUTPUT, fileName};
            }
            cleanText = StringUtils::extractBefore(cleanText, ">");
        }
        
        // Parse the clean command and arguments
        StringUtils::parseCommand(cleanText, segment.command, segment.args);
        
        return segment;
    }
};

} // namespace shell
