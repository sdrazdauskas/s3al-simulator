#include "shell/Shell.h"
#include "shell/StringUtils.h"
#include "shell/CommandParser.h"
#include <sstream>
#include <iostream>
#include <atomic>
#include <vector>
#include <string>
#include <lua.hpp>
#include <unordered_set>

namespace shell {

// Global interrupt flag for Ctrl+C handling
std::atomic<bool> interruptRequested{false};

Shell::Shell(SysApi& sys, const CommandRegistry& reg)
    : sys(sys), registry(reg), luaState(nullptr) {}

void Shell::initLuaOnce() {
    if (luaState) return;
    luaState = luaL_newstate();
    if (!luaState) {
        logError("Failed to create Lua state");
        return;
    }
    luaL_openlibs(luaState);
    lua_pushlightuserdata(luaState, this);
    lua_setfield(luaState, LUA_REGISTRYINDEX, "__shell_ptr");

    // Set up interrupt hook - check every 1000 instructions
    lua_sethook(luaState, [](lua_State* L, lua_Debug* ar) {
        if (interruptRequested.load()) {
            luaL_error(L, "Script interrupted by user (Ctrl+C)");
        }
    }, LUA_MASKCOUNT, 1000);

    // Register function to run shell commands from Lua
    lua_pushcfunction(luaState, [](lua_State* L) -> int {
        const char* cmdLine = lua_tostring(L, 1);
        if (!cmdLine) {
            lua_pushstring(L, "Error: No command provided");
            return 1;
        }

        // Get Shell instance from registry
        lua_getfield(L, LUA_REGISTRYINDEX, "__shell_ptr");
        Shell* shell = (Shell*)lua_touserdata(L, -1);
        lua_pop(L, 1);

        if (!shell) {
            lua_pushstring(L, "Error: Shell instance not found");
            return 1;
        }

        auto originalCallback = shell->getOutputCallback();

        std::string capturedOutput;
        // Set a temp cb that captures output
        shell->setOutputCallback([&capturedOutput, originalCallback](const std::string& output) {

            if (originalCallback) {
                originalCallback(output);
            }

            if (!capturedOutput.empty()) capturedOutput += "\n";
            capturedOutput += output;
        });

        shell->processCommandLine(cmdLine);

        // Restore the original callback
        shell->setOutputCallback(originalCallback);

        // Return the captured output to Lua
        lua_pushstring(L, capturedOutput.c_str());
        return 1;
    });

    lua_setglobal(luaState, "sh");

    logInfo("Lua engine initialized");
}

std::string Shell::runLuaScript(const std::string &luaCode) {

    initLuaOnce();

    if (!luaState) {
        return "Error: Lua not initialized";
    }

    interruptRequested.store(false);

    // Execute Lua code
    int result = luaL_dostring(luaState, luaCode.c_str());

    if (result != LUA_OK) {
        const char *error = lua_tostring(luaState, -1);
        lua_pop(luaState, 1);
        return std::string("\nLua Error: ") + (error ? error : "unknown");
    }

    // Get any return value from Lua
    if (lua_gettop(luaState) > 0) {
        const char *ret = lua_tostring(luaState, -1);
        lua_pop(luaState, 1);
        if (ret) return ret;
    }

    return "OK";
}

void Shell::setOutputCallback(OutputCallback callback) {
    outputCallback = callback;
}

std::string Shell::handleInputRedirection(const std::string &segment) {
    std::string fileName = StringUtils::extractAfter(segment, "<");
    if (fileName.empty()) {
        logError("Input redirection missing fileName");
        return "";
    }

    ICommand* catCmd = registry.find("cat");
    if (!catCmd) {
        logError("No 'cat' command found for input redirection");
        return "";
    }

    std::ostringstream out, err;
    std::vector<std::string> readArgs = { fileName };
    int rc = catCmd->execute(readArgs, "", out, err, sys);
    if (!err.str().empty()) {
        logError(err.str());
        return "";
    }

    return out.str();
}

std::string Shell::handleOutputRedirection(std::string segment, const std::string &output) {
    std::string fileName = StringUtils::extractAfter(segment, ">");
    segment = StringUtils::extractBefore(segment, ">");
    if (fileName.empty()) {
        logError("Output redirection missing fileName");
        return "";
    }

    ICommand* writeCmd = registry.find("write");
    if (!writeCmd) {
        logError("No 'write' command found for output redirection");
        return "";
    }

    std::string cleaned = output;
    if (!cleaned.empty() && cleaned.back() == '\n')
        cleaned.pop_back();

    if (sys.fileExists(fileName) != shell::SysResult::OK) {
        sys.createFile(fileName);
    }

    std::ostringstream out, err;
    std::vector<std::string> writeArgs = { fileName, cleaned };
    int rc = writeCmd->execute(writeArgs, "", out, err, sys);
    if (!err.str().empty()) {
        logError(err.str());
        return "";
    }

    return out.str();
}

std::string Shell::handleAppendRedirection(std::string segment, const std::string &output) {
    std::string fileName = StringUtils::extractAfter(segment, ">>");
    segment = StringUtils::extractBefore(segment, ">>");

    if (fileName.empty()) {
        logError("Append redirection missing fileName");
        return "";
    }

    std::string cleaned = output;
    if (!cleaned.empty() && cleaned.back() == '\n')
        cleaned.pop_back();

    if (sys.fileExists(fileName) != shell::SysResult::OK) {
        sys.createFile(fileName);
    }

    auto result = sys.appendFile(fileName, cleaned);
    if (result != shell::SysResult::OK) {
        logError("appendFile failed: " + shell::toString(result));
        return "";
    }

    return "append: " + fileName + ": OK";
}

void Shell::processCommandLine(const std::string& commandLine) {
    if (commandLine.empty()) {
        logDebug("Empty command line received");
        if (outputCallback) outputCallback("Error: No command entered");
        return;
    }

    interruptRequested.store(false);

    logDebug("Processing command: " + commandLine);

    std::istringstream checkStream(commandLine);
    std::string firstWord;
    checkStream >> firstWord;

    if (firstWord == "write" || firstWord == "edit") {
        std::string command;
        std::vector<std::string> args;
        parseCommand(commandLine, command, args);

        ICommand* cmd = registry.find(command);
        if (cmd) {
            // write/edit commands should go directly to std::cout / std::cerr
            int rc = cmd->execute(args, "", std::cout, std::cerr, sys);

            std::cout.flush();
            std::cerr.flush();

            // handle interruption
            if (interruptRequested.load()) {
                logInfo("Command interrupted by user");
                interruptRequested.store(false);
                if (outputCallback) {
                    outputCallback("^C\nCommand interrupted\n");
                }
            }
        } else {
            logError("Unknown command: " + command);
            if (outputCallback) {
                outputCallback("Error: Unknown command: " + command);
            }
        }

        return;
    }

    // Parse the command line into structured chains
    auto chains = CommandParser::parse(commandLine);
    std::string combinedOutput;

    for (const auto& chain : chains) {
        std::string pipeInput;

        for (const auto& segment : chain.segments) {
            if (segment.command.empty())
                continue;

            // Handle input redirection
            std::string inputData;
            if (segment.inputRedirect) {
                inputData = handleInputRedirection("<" + segment.inputRedirect->fileName);
            }

            // Execute the command
            bool inPipeChain = segment.isPipedToNext || segment.outputRedirect.has_value();
            std::string result = executeCommand(
                segment.command, 
                segment.args, 
                inputData.empty() ? pipeInput : inputData, 
                inPipeChain
            );

            // Handle output redirection
            if (segment.outputRedirect) {
                if (segment.outputRedirect->type == Redirection::Type::APPEND) {
                    handleAppendRedirection(">>" + segment.outputRedirect->fileName, result);
                } else {
                    handleOutputRedirection(">" + segment.outputRedirect->fileName, result);
                }
                pipeInput.clear();
            } else {
                pipeInput = result;
            }
        }

        if (!combinedOutput.empty())
            combinedOutput += "\n";

        combinedOutput += pipeInput;

        // Stop chain on error or interruption
        if (pipeInput.rfind("Error", 0) == 0 || pipeInput == "Interrupted")
            break;
    }

    if (outputCallback && !combinedOutput.empty()) {
        outputCallback(combinedOutput);
    }
}

std::string Shell::executeCommand(const std::string& command,
                                  const std::vector<std::string>& args,
                                  const std::string& input,
                                  bool inPipeChain) {
    if (command.empty()) {
        logError("No command specified");
        return "Error: No command specified";
    }

    logInfo("Executing command: " + command);

    if (command.rfind("./", 0) == 0) {
        std::string fileName = command.substr(2);
        return executeScriptFile(fileName);
    }

    std::vector<std::string> argsWithInput = args;
    if (!input.empty())
        argsWithInput.push_back(input);

    if (isBuiltinCommand(command)) {
        ICommand* cmd = registry.find(command);
        if (!cmd) return "Error: Builtin missing: " + command;

        if (shellPid > 0) {
            sys.addCPUWork(shellPid, BUILTIN_CPU_WORK);
            if (!sys.waitForProcess(shellPid)) {
                // Interrupted - return error to stop the chain
                return "Interrupted";
            }
        }

        std::ostringstream out, err;
        
        if (inPipeChain) {
            std::cerr.flush();
            cmd->execute(argsWithInput, input, out, err, sys);
            if (!err.str().empty()) return out.str() + err.str();
            return out.str();
        } else {
            std::cerr.flush();
            CallbackStreamBuf out_buf(outputCallback);
            CallbackStreamBuf err_buf(outputCallback);
            std::ostream os(&out_buf);
            std::ostream es(&err_buf);
            cmd->execute(argsWithInput, input, os, es, sys);
            os.flush(); es.flush();
            return "";
        }
    }

    // External Commands
    ICommand* cmd = registry.find(command);
    if (!cmd) return "Error: Command '" + command + "' not found.";

    // Make it a dynamic, not real value
    int memNeeded = std::max(64, static_cast<int>(args.size()) * 1024);
    int cpuNeeded = std::max(2, static_cast<int>(args.size()) * 2);
    
    int pid = sys.fork(command, cpuNeeded, memNeeded);
    if (pid <= 0) return "Error: Fork failed.";

    logInfo("Process started: " + command + " (PID=" + std::to_string(pid) + ")");

    // Wait for scheduler to complete the process
    std::cerr.flush();
    if (!sys.waitForProcess(pid)) {
        logInfo("Process interrupted: " + command + " (PID=" + std::to_string(pid) + ")");
        sys.reapProcess(pid);
        return "Interrupted";
    }

    // Now execute the actual command after scheduler completes
    if (inPipeChain) {
        // Pipe output mode
        std::ostringstream out, err;
        std::cerr.flush();
        int returnCode = cmd->execute(argsWithInput, input, out, err, sys);
        // Command finished - call exit() then wait()/reap
        sys.exit(pid, returnCode);
        sys.reapProcess(pid);
        return out.str() + err.str();
    }

    // Real time output
    std::cerr.flush();
    CallbackStreamBuf out_buf(outputCallback);
    CallbackStreamBuf err_buf(outputCallback);
    std::ostream os(&out_buf);
    std::ostream es(&err_buf);
    int returnCode = cmd->execute(argsWithInput, input, os, es, sys);
    os.flush(); es.flush();

    // Command finished
    sys.exit(pid, returnCode);
    sys.reapProcess(pid);
    logInfo("Process finished: " + command + " (PID=" + std::to_string(pid) + ")");
    return "";
}

std::string Shell::executeScriptFile(const std::string &fileName) {
    logInfo("Executing script file: " + fileName);


    // Read file directly from memory filesystem
    std::string fileContent;
    auto readResult = sys.readFile(fileName, fileContent);

    if (readResult != SysResult::OK) {
        std::string error = "Error: Cannot read Lua file '" + fileName + "': " + shell::toString(readResult);
        logError(error);
        return error;
    }

    if (fileContent.empty()) {
        return "Error: Lua file is empty";
    }

    logDebug("Executing Lua content: " + fileContent.substr(0, 50) + "...");
    return runLuaScript(fileContent);
}

void Shell::parseCommand(const std::string& commandLine, std::string& command, std::vector<std::string>& args) {
    StringUtils::parseCommand(commandLine, command, args);
}

bool Shell::isBuiltinCommand(const std::string& cmd) const {
    static const std::unordered_set<std::string> builtins = {
        "cd", "pwd", "help", "quit", "exit", "kill", "meminfo", "membar", "reset", "save", "load", "listdata"
    };
    return builtins.count(cmd) > 0;
}

} // namespace shell
