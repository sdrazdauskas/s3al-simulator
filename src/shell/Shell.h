#pragma once

#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <ostream>
#include <streambuf>
#include "shell/CommandAPI.h"
#include "common/LoggingMixin.h"
#include <atomic>
#include <lua.hpp>

extern std::atomic<bool> interruptRequested;

namespace shell {

    using OutputCallback = std::function<void(const std::string&)>;

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

    class Shell : public common::LoggingMixin {
    private:
        static constexpr int BUILTIN_CPU_WORK = 1;
        
        lua_State* luaState;
        SysApi& sys;
        const CommandRegistry& registry;
        OutputCallback outputCallback;
        int shellPid = -1;
        
        void initLuaOnce();
        std::string runLuaScript(const std::string& luaCode);
        std::string executeScriptFile(const std::string& fileName);

        std::string handleInputRedirection(const std::string &segment);
        std::string handleOutputRedirection(std::string segment, const std::string &output);
        std::string handleAppendRedirection(std::string segment, const std::string &output);

    public:
        explicit Shell(SysApi& sys, const CommandRegistry& reg);

        OutputCallback getOutputCallback() const { return outputCallback; }
        void setOutputCallback(OutputCallback callback);
        void setShellPid(int pid) { shellPid = pid; }

        void processCommandLine(const std::string& commandLine);
        std::string executeCommand(const std::string& command,
                                const std::vector<std::string>& args,
                                const std::string& input = "",
                                bool inPipeChain = false);
        void parseCommand(const std::string& commandLine, std::string& command, std::vector<std::string>& args);

        bool isCommandAvailable(const std::string& name) const {
            return registry.find(name) != nullptr;
        }

        bool isBuiltinCommand(const std::string& cmd) const;

    protected:
        std::string getModuleName() const override { return "SHELL"; }
    };
}  // namespace shell
