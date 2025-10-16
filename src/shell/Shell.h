#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <vector>

namespace shell {
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
}
#endif
