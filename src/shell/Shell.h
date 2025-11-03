#pragma once

#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <ostream>
#include "CommandAPI.h"

namespace shell {

    using OutputCallback = std::function<void(const std::string&)>;
    using LogCallback = std::function<void(const std::string& level, 
                                           const std::string& module, 
                                           const std::string& message)>;
    using KernelCallback = std::function<void(const std::string& command, const std::vector<std::string>& args)>;

    class Shell {
    private:
        SysApi& sys;
        const CommandRegistry& registry;
        OutputCallback outputCallback;
        LogCallback log_callback;
        KernelCallback kernelCallback;

        void log(const std::string& level, const std::string& message);
        std::string parseQuotedToken(std::istringstream& iss, std::string token);
        std::vector<std::string> splitByAndOperator(const std::string& commandLine);
        std::vector<std::string> splitByPipeOperator(const std::string& commandLine);

    public:
    explicit Shell(SysApi& sys_, const CommandRegistry& reg, KernelCallback kernelCb = KernelCallback());
        
        void setLogCallback(LogCallback callback);
        void setOutputCallback(OutputCallback callback);
        void setKernelCallback(KernelCallback callback) { kernelCallback = std::move(callback); }
        
        void processCommandLine(const std::string& commandLine);
        std::string executeCommand(const std::string& command, const std::vector<std::string>& args, const std::string& input = "");
        void parseCommand(const std::string& commandLine, std::string& command, std::vector<std::string>& args);
        bool isConnectedToKernel() const;

        bool isCommandAvailable(const std::string& name) const {
            return registry.find(name) != nullptr;
        }
    };

} // namespace shell
