#ifndef SHELL_HPP
#define SHELL_HPP

#include <string>
#include <vector>

class Shell {
private:
    std::string (*kernelHandler)(const std::string&, const std::vector<std::string>&);
    std::string parseQuotedToken(std::istringstream& iss, std::string token);

public:
    Shell(std::string (*kernelFunc)(const std::string&, const std::vector<std::string>&));
    std::string processCommandLine(const std::string& commandLine);
    std::string executeCommand(const std::string& command, const std::vector<std::string>& args);
    void parseCommand(const std::string& commandLine, std::string& command, std::vector<std::string>& args);
    bool isConnectedToKernel() const;
};

#endif
