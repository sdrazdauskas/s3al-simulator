#pragma once

#include "shell/CommandAPI.h"
#include <memory>

namespace shell {

// Forward declarations of factory functions
std::unique_ptr<ICommand> createAddCommand();
std::unique_ptr<ICommand> createCatCommand();
std::unique_ptr<ICommand> createCdCommand();
std::unique_ptr<ICommand> createCpCommand();
std::unique_ptr<ICommand> createCpdirCommand();
std::unique_ptr<ICommand> createCurlCommand();
std::unique_ptr<ICommand> createEchoCommand();
std::unique_ptr<ICommand> createEditCommand();
std::unique_ptr<ICommand> createHelpCommand(CommandRegistry* reg);
std::unique_ptr<ICommand> createKillCommand();
std::unique_ptr<ICommand> createListdataCommand();
std::unique_ptr<ICommand> createLoadCommand();
std::unique_ptr<ICommand> createLoadStateCommand();
std::unique_ptr<ICommand> createLsCommand();
std::unique_ptr<ICommand> createMembarCommand();
std::unique_ptr<ICommand> createMeminfoCommand();
std::unique_ptr<ICommand> createMkdirCommand();
std::unique_ptr<ICommand> createMvCommand();
std::unique_ptr<ICommand> createMvdirCommand();
std::unique_ptr<ICommand> createOsLogCommand();
std::unique_ptr<ICommand> createPsCommand();
std::unique_ptr<ICommand> createPwdCommand();
std::unique_ptr<ICommand> createQuitCommand();
std::unique_ptr<ICommand> createResetCommand();
std::unique_ptr<ICommand> createRmCommand();
std::unique_ptr<ICommand> createRmdirCommand();
std::unique_ptr<ICommand> createSaveStateCommand();
std::unique_ptr<ICommand> createSchedulerCommand();
std::unique_ptr<ICommand> createSleepCommand();
std::unique_ptr<ICommand> createTexedCommand();
std::unique_ptr<ICommand> createTouchCommand();
std::unique_ptr<ICommand> createWriteCommand();

inline void initCommands(CommandRegistry& reg) {
    reg.add(createAddCommand());
    reg.add(createCatCommand());
    reg.add(createCdCommand());
    reg.add(createCpCommand());
    reg.add(createCpdirCommand());
    reg.add(createCurlCommand());
    reg.add(createEchoCommand());
    reg.add(createEditCommand());
    reg.add(createKillCommand());
    reg.add(createListdataCommand());
    reg.add(createLoadCommand());
    reg.add(createLoadStateCommand());
    reg.add(createLsCommand());
    reg.add(createMeminfoCommand());
    reg.add(createMembarCommand());
    reg.add(createMkdirCommand());
    reg.add(createMvCommand());
    reg.add(createMvdirCommand());
    reg.add(createOsLogCommand());
    reg.add(createPsCommand());
    reg.add(createPwdCommand());
    reg.add(createResetCommand());
    reg.add(createRmCommand());
    reg.add(createRmdirCommand());
    reg.add(createSaveStateCommand());
    reg.add(createSchedulerCommand());
    reg.add(createSleepCommand());
    reg.add(createTexedCommand());
    reg.add(createTouchCommand());
    reg.add(createWriteCommand());
    
    reg.add(createHelpCommand(&reg));
    reg.add(createQuitCommand());
    // Add alias for exit -> quit
    auto exitCmd = createQuitCommand();
    reg.map["exit"] = std::move(exitCmd);
}

} // namespace shell
