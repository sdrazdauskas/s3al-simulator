#include "shell/Shell.h"
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

Shell::Shell(SysApi& sys_, const CommandRegistry& reg, KernelCallback kernelCb)
    : sys(sys_), registry(reg), kernelCallback(std::move(kernelCb)), luaState(nullptr) {}

void Shell::initLuaOnce() {
    if (luaState) return;
    luaState = luaL_newstate();
    if (!luaState) {
        log("ERROR", "Failed to create Lua state");
        return;
    }
    luaL_openlibs(luaState);
    lua_pushlightuserdata(luaState, this);
    lua_setfield(luaState, LUA_REGISTRYINDEX, "__shell_ptr");

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

        // Store the original output cb
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

    log("INFO", "Lua engine initialized");
}

std::string Shell::runLuaScript(const std::string &luaCode) {

    initLuaOnce();

    if (!luaState) {
        return "Error: Lua not initialized";
    }

    // Execute Lua code
    int result = luaL_dostring(luaState, luaCode.c_str());

    if (result != LUA_OK) {
        const char *error = lua_tostring(luaState, -1);
        lua_pop(luaState, 1);
        return std::string("Lua Error: ") + (error ? error : "unknown");
    }

    // Get any return value from Lua
    if (lua_gettop(luaState) > 0) {
        const char *ret = lua_tostring(luaState, -1);
        lua_pop(luaState, 1);
        if (ret) return ret;
    }

    return "OK";
}

void Shell::setLogCallback(LogCallback callback) {
    logCallback = callback;
}

void Shell::setOutputCallback(OutputCallback callback) {
    outputCallback = callback;
}

void Shell::log(const std::string& level, const std::string& message) {
    if (logCallback) {
        logCallback(level, "SHELL", message);
    }
}

std::string Shell::parseQuotedToken(std::istringstream& iss, std::string token) {
    std::string quoted = token.substr(1);
    std::string next;
    while (!quoted.empty() && quoted.back() != '"' && iss >> next) {
        quoted += " " + next;
    }
    if (!quoted.empty() && quoted.back() == '"')
        quoted.pop_back();
    return quoted;
}

std::vector<std::string> Shell::splitByAndOperator(const std::string& commandLine) {
    std::vector<std::string> commands;
    std::string temp = commandLine;
    size_t pos = 0;

    while ((pos = temp.find("&&")) != std::string::npos) {
        std::string part = temp.substr(0, pos);
        size_t start = part.find_first_not_of(" \t");
        size_t end = part.find_last_not_of(" \t");
        if (start != std::string::npos)
            commands.push_back(part.substr(start, end - start + 1));
        temp.erase(0, pos + 2);
    }

    size_t start = temp.find_first_not_of(" \t");
    size_t end = temp.find_last_not_of(" \t");
    if (start != std::string::npos)
        commands.push_back(temp.substr(start, end - start + 1));

    return commands;
}

std::vector<std::string> Shell::splitByPipeOperator(const std::string& commandLine) {
    std::vector<std::string> parts;
    std::string temp = commandLine;
    size_t pos = 0;

    while ((pos = temp.find("|")) != std::string::npos) {
        std::string part = temp.substr(0, pos);
        size_t start = part.find_first_not_of(" \t");
        size_t end = part.find_last_not_of(" \t");
        if (start != std::string::npos)
            parts.push_back(part.substr(start, end - start + 1));
        temp.erase(0, pos + 1);
    }

    size_t start = temp.find_first_not_of(" \t");
    size_t end = temp.find_last_not_of(" \t");
    if (start != std::string::npos)
        parts.push_back(temp.substr(start, end - start + 1));

    return parts;
}

std::string Shell::trim(const std::string &s) {
    auto start = s.find_first_not_of(" \t");
    auto end = s.find_last_not_of(" \t");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

std::string Shell::extractAfterSymbol(const std::string &s, const std::string &symbol) {
    size_t pos = s.find(symbol);
    if (pos == std::string::npos)
        return "";
    return trim(s.substr(pos + symbol.size()));
}

std::string Shell::extractBeforeSymbol(const std::string &s, const std::string &symbol) {
    size_t pos = s.find(symbol);
    if (pos == std::string::npos)
        return trim(s);
    return trim(s.substr(0, pos));
}

std::string Shell::handleInputRedirection(const std::string &segment) {
    std::string fileName = extractAfterSymbol(segment, "<");
    if (fileName.empty()) {
        log("ERROR", "Input redirection missing fileName");
        return "";
    }

    ICommand* catCmd = registry.find("cat");
    if (!catCmd) {
        log("ERROR", "No 'cat' command found for input redirection");
        return "";
    }

    std::ostringstream out, err;
    std::vector<std::string> readArgs = { fileName };
    int rc = catCmd->execute(readArgs, "", out, err, sys);
    if (!err.str().empty()) {
        log("ERROR", err.str());
        return "";
    }

    return out.str();
}

std::string Shell::handleOutputRedirection(std::string segment, const std::string &output) {
    std::string fileName = extractAfterSymbol(segment, ">");
    segment = extractBeforeSymbol(segment, ">");
    if (fileName.empty()) {
        log("ERROR", "Output redirection missing fileName");
        return "";
    }

    ICommand* writeCmd = registry.find("write");
    if (!writeCmd) {
        log("ERROR", "No 'write' command found for output redirection");
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
        log("ERROR", err.str());
        return "";
    }

    return out.str();
}

std::string Shell::handleAppendRedirection(std::string segment, const std::string &output) {
    std::string fileName = extractAfterSymbol(segment, ">>");
    segment = extractBeforeSymbol(segment, ">>");

    if (fileName.empty()) {
        log("ERROR", "Append redirection missing fileName");
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
        log("ERROR", "appendFile failed: " + shell::toString(result));
        return "";
    }

    return "append: " + fileName + ": OK";
}

void Shell::processCommandLine(const std::string& commandLine) {
    if (commandLine.empty()) {
        log("DEBUG", "Empty command line received");
        if (outputCallback) outputCallback("Error: No command entered");
        return;
    }

    log("DEBUG", "Processing command: " + commandLine);

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
            interruptRequested.store(false);

            int rc = cmd->execute(args, "", std::cout, std::cerr, sys);

            std::cout.flush();
            std::cerr.flush();

            // handle interruption
            if (interruptRequested.load()) {
                log("INFO", "Command interrupted by user");
                interruptRequested.store(false);
                if (outputCallback) {
                    outputCallback("^C\nCommand interrupted\n");
                }
            }
        } else {
            log("ERROR", "Unknown command: " + command);
            if (outputCallback) {
                outputCallback("Error: Unknown command: " + command);
            }
        }

        return;
    }

    std::vector<std::string> andCommands = splitByAndOperator(commandLine);
    std::string combinedOutput;

    for (const auto& andCmd : andCommands) {
        std::vector<std::string> pipeCommands = splitByPipeOperator(andCmd);
        std::string pipeInput;

        for (size_t i = 0; i < pipeCommands.size(); ++i) {
            std::string segment = pipeCommands[i];
            std::string segmentCopy = segment;
            std::string inputData;

            if (segmentCopy.find('<') != std::string::npos) {
                inputData = handleInputRedirection(segmentCopy);
                segmentCopy = extractBeforeSymbol(segmentCopy, "<");
            }

            if (segmentCopy.find(">>") != std::string::npos) {
                std::string cleanCommand = extractBeforeSymbol(segmentCopy, ">>");
                std::string fileName = extractAfterSymbol(segmentCopy, ">>");

                std::string command;
                std::vector<std::string> args;
                parseCommand(cleanCommand, command, args);

                if (command.empty())
                    continue;

                bool inPipeChain = true;
                std::string result = executeCommand(command, args, inputData.empty() ? pipeInput : inputData, inPipeChain);

                handleAppendRedirection(segmentCopy, result);
                pipeInput.clear();
                continue;
            }

            bool hasOutputRedirect = (segmentCopy.find('>') != std::string::npos && segmentCopy.find(">>") == std::string::npos);

            std::string fileName;
            if (hasOutputRedirect) {
                fileName = extractAfterSymbol(segmentCopy, ">");
                segmentCopy = extractBeforeSymbol(segmentCopy, ">");
            }

            std::string command;
            std::vector<std::string> args;
            parseCommand(segmentCopy, command, args);

            if (command.empty())
                continue;

            bool inPipeChain = hasOutputRedirect ? true : (i < pipeCommands.size() - 1);
            std::string result = executeCommand(command, args, inputData.empty() ? pipeInput : inputData, inPipeChain);

            if (hasOutputRedirect && !fileName.empty()) {
                handleOutputRedirection(">" + fileName, result);
                pipeInput.clear();
            } else {
                pipeInput = result;
            }

        }

        if (!combinedOutput.empty())
            combinedOutput += "\n";

        combinedOutput += pipeInput;

        if (pipeInput.rfind("Error", 0) == 0)
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
        log("ERROR", "No command specified");
        return "Error: No command specified";
    }

    log("INFO", "Executing command: " + command);

    if (command.rfind("./", 0) == 0) {
        std::string fileName = command.substr(2);
        return executeScriptFile(fileName);
    }

    std::vector<std::string> argsWithInput = args;
    if (!input.empty())
        argsWithInput.push_back(input);

    // Built in commands
    if (isBuiltinCommand(command)) {
        ICommand* cmd = registry.find(command);
        if (!cmd) return "Error: Builtin missing: " + command;

        // Add minimal CPU work to shell's own process and wait for scheduler
        if (shellPid > 0 && isConnectedToKernel()) {
            sys.addCPUWork(shellPid, 1);  // Built-ins use 1 cycle
            // Wait for scheduler to consume the cycles
            sys.waitForProcess(shellPid);
        }

        interruptRequested.store(false);
        std::ostringstream out, err;
        
        if (inPipeChain) {
            std::cerr.flush();  // Flush logs before command output
            cmd->execute(argsWithInput, input, out, err, sys);
            if (!err.str().empty()) return out.str() + err.str();
            return out.str();
        } else {
            std::cerr.flush();  // Flush logs before command output
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

    if (!isConnectedToKernel()) {
        std::ostringstream out, err;
        cmd->execute(argsWithInput, input, out, err, sys);
        std::string output = out.str();
        if (!err.str().empty()) {
            if (!output.empty()) output += "\n";
            output += "Error: " + err.str();
        }
        return output;
    }

    int memNeeded = std::max(64, static_cast<int>(args.size()) * 1024);
    int cpuNeeded = std::max(2, static_cast<int>(args.size()) * 2);
    
    // Clear any previous interrupt before starting new process
    interruptRequested.store(false);
    
    int pid = sys.fork(command, cpuNeeded, memNeeded);
    if (pid <= 0) return "Error: Fork failed.";

    log("INFO", "Process started: " + command + " (PID=" + std::to_string(pid) + ")");

    // Wait for scheduler to complete the process (simulate CPU time)
    std::cerr.flush();  // Flush logs before waiting
    if (!sys.waitForProcess(pid)) {
        log("INFO", "Process interrupted: " + command + " (PID=" + std::to_string(pid) + ")");
        return "";
    }

    // Now execute the actual command after scheduler completes
    if (inPipeChain) {
        // Pipe output mode
        std::ostringstream out, err;
        std::cerr.flush();  // Flush logs before command output
        cmd->execute(argsWithInput, input, out, err, sys);
        // Command finished - call exit() then wait()/reap
        sys.exit(pid);
        sys.reapProcess(pid);
        return out.str() + err.str();
    }

    // Real time output
    std::cerr.flush();  // Flush logs before command output
    CallbackStreamBuf out_buf(outputCallback);
    CallbackStreamBuf err_buf(outputCallback);
    std::ostream os(&out_buf);
    std::ostream es(&err_buf);
    cmd->execute(argsWithInput, input, os, es, sys);
    os.flush(); es.flush();

    // Command finished - call exit() then wait()/reap
    sys.exit(pid);
    sys.reapProcess(pid);
    log("INFO", "Process finished: " + command + " (PID=" + std::to_string(pid) + ")");
    return "";
}

std::string Shell::executeScriptFile(const std::string &fileName) {
    log("INFO", "Executing script file: " + fileName);


    // Read file directly from memory filesystem
    std::string fileContent;
    auto readResult = sys.readFile(fileName, fileContent);

    if (readResult != SysResult::OK) {
        std::string error = "Error: Cannot read Lua file '" + fileName + "': " + shell::toString(readResult);
        log("ERROR", error);
        return error;
    }

    if (fileContent.empty()) {
        return "Error: Lua file is empty";
    }

    log("DEBUG", "Executing Lua content: " + fileContent.substr(0, 50) + "...");
    return runLuaScript(fileContent);
}

void Shell::parseCommand(const std::string& commandLine, std::string& command, std::vector<std::string>& args) {
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

bool Shell::isConnectedToKernel() const { return kernelCallback != nullptr; }

bool Shell::isBuiltinCommand(const std::string& cmd) const {
    static const std::unordered_set<std::string> builtins = {
        "cd", "pwd", "help", "quit", "exit", "kill", "meminfo", "membar", "reset", "save", "load", "listdata"
    };
    return builtins.count(cmd) > 0;
}

} // namespace shell
