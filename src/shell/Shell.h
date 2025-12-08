#pragma once

#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <ostream>
#include <streambuf>
#include "CommandAPI.h"
#include <atomic>
#include <lua.hpp>

extern std::atomic<bool> g_interrupt_requested;

namespace shell {

    using OutputCallback = std::function<void(const std::string&)>;
    using LogCallback = std::function<void(const std::string& level,
                                           const std::string& module,
                                           const std::string& message)>;

    using KernelCallback = std::function<void(const std::string& command,
                                              const std::vector<std::string>& args)>;

    // Custom stream buffer that writes directly to output callback
    class CallbackStreamBuf : public std::streambuf {
    private:
        OutputCallback callback;
        std::string buffer;
        
    protected:
        int overflow(int c) override {
            if (c != EOF) {
                buffer += static_cast<char>(c);
                if (c == '\n' || buffer.size() >= 1024) {
                    flush_buffer();
                }
            }
            return c;
        }
        
        int sync() override {
            flush_buffer();
            return 0;
        }
        
        void flush_buffer() {
            if (!buffer.empty() && callback) {
                callback(buffer);
                buffer.clear();
            }
        }
        
    public:
        CallbackStreamBuf(OutputCallback cb) : callback(std::move(cb)) {}
        
        ~CallbackStreamBuf() {
            flush_buffer();
        }
    };

    class Shell {
    private:
        lua_State* luaState;
        SysApi& sys;
        const CommandRegistry& registry;
        OutputCallback outputCallback;
        LogCallback log_callback;
        KernelCallback kernelCallback;

        void initLuaOnce();
        std::string runLuaScript(const std::string& luaCode);
        void log(const std::string& level, const std::string& message);
        std::string parseQuotedToken(std::istringstream& iss, std::string token);
        std::vector<std::string> splitByAndOperator(const std::string& commandLine);
        std::vector<std::string> splitByPipeOperator(const std::string& commandLine);
        std::string executeScriptFile(const std::string& filename);
        std::string trim(const std::string &s);
        std::string extractAfterSymbol(const std::string &s, const std::string &symbol);
        std::string extractBeforeSymbol(const std::string &s, const std::string &symbol);

        std::string handleInputRedirection(const std::string &segment);
        std::string handleOutputRedirection(std::string segment, const std::string &output);
        std::string handleAppendRedirection(std::string segment, const std::string &output);

    public:
        explicit Shell(SysApi& sys_, const CommandRegistry& reg, KernelCallback kernelCb = KernelCallback());

        OutputCallback getOutputCallback() const { return outputCallback; }
        void setLogCallback(LogCallback callback);
        void setOutputCallback(OutputCallback callback);
        void setKernelCallback(KernelCallback callback) { kernelCallback = std::move(callback); }

        void processCommandLine(const std::string& commandLine);
        std::string executeCommand(const std::string& command,
                                   const std::vector<std::string>& args,
                                   const std::string& input = "",
                                   bool inPipeChain = false);
        void parseCommand(const std::string& commandLine, std::string& command, std::vector<std::string>& args);

        bool isConnectedToKernel() const;

        bool isCommandAvailable(const std::string& name) const {
            return registry.find(name) != nullptr;
        }

        bool isBuiltinCommand(const std::string& cmd) const;
    };

} // namespace shell
