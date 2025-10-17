#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <vector>
#include <functional>

namespace shell {

using KernelCallback = std::function<std::string(const std::string&, const std::vector<std::string>&)>;

class Shell {
private:
    KernelCallback kernelCallback;
    std::string parseQuotedToken(std::istringstream& iss, std::string token);

public:
    Shell(KernelCallback cb);
    std::string processCommandLine(const std::string& commandLine);
    std::string executeCommand(const std::string& command, const std::vector<std::string>& args);
    void parseCommand(const std::string& commandLine, std::string& command, std::vector<std::string>& args);
    bool isConnectedToKernel() const;
};

}
#endif
