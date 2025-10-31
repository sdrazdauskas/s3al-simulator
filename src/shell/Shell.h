#pragma once

#include <string>
#include <vector>
#include <functional>
#include <sstream>

namespace shell {

    using KernelCallback = std::function<std::string(const std::string&, const std::vector<std::string>&)>;
    using OutputCallback = std::function<void(const std::string&)>;
    using LogCallback = std::function<void(const std::string& level, 
                                           const std::string& module, 
                                           const std::string& message)>;

    class Shell {
    private:
        KernelCallback kernelCallback;
        OutputCallback outputCallback;
        LogCallback log_callback;

        void log(const std::string& level, const std::string& message);
        std::string parseQuotedToken(std::istringstream& iss, std::string token);
        std::vector<std::string> splitByAndOperator(const std::string& commandLine);
        std::vector<std::string> splitByPipeOperator(const std::string& commandLine);

    public:
        explicit Shell(KernelCallback cb);
        
        void setLogCallback(LogCallback callback);
        void setOutputCallback(OutputCallback callback);
        
        void processCommandLine(const std::string& commandLine);
        std::string executeCommand(const std::string& command, const std::vector<std::string>& args, const std::string& input = "");
        void parseCommand(const std::string& commandLine, std::string& command, std::vector<std::string>& args);
        bool isConnectedToKernel() const;
    };

} // namespace shell
