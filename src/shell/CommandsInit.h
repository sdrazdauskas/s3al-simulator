#pragma once
#include "shell/CommandAPI.h"
#include <memory>

namespace shell {

// Forward declarations of factory functions (camelCase)
std::unique_ptr<ICommand> createCatCommand();
std::unique_ptr<ICommand> createTouchCommand();
std::unique_ptr<ICommand> createEchoCommand();
std::unique_ptr<ICommand> createAddCommand();
std::unique_ptr<ICommand> createRmCommand();
std::unique_ptr<ICommand> createWriteCommand();
std::unique_ptr<ICommand> createEditCommand();
std::unique_ptr<ICommand> createTexedCommand();
std::unique_ptr<ICommand> createMkdirCommand();
std::unique_ptr<ICommand> createRmdirCommand();
std::unique_ptr<ICommand> createCdCommand();
std::unique_ptr<ICommand> createLsCommand();
std::unique_ptr<ICommand> createPwdCommand();
std::unique_ptr<ICommand> createQuitCommand();
std::unique_ptr<ICommand> createMeminfoCommand();
std::unique_ptr<ICommand> createMembarCommand();
std::unique_ptr<ICommand> createSaveStateCommand();
std::unique_ptr<ICommand> createSetSchedulerCommand();
std::unique_ptr<ICommand> createLoadStateCommand();
std::unique_ptr<ICommand> createLoadCommand();
std::unique_ptr<ICommand> createResetCommand();
std::unique_ptr<ICommand> createSleepCommand();
std::unique_ptr<ICommand> createHelpCommand(CommandRegistry* reg);
std::unique_ptr<ICommand> createListdataCommand();
std::unique_ptr<ICommand> createMvCommand();
std::unique_ptr<ICommand> createCpCommand();
std::unique_ptr<ICommand> createMvdirCommand();
std::unique_ptr<ICommand> createCpdirCommand();
std::unique_ptr<ICommand> createKillCommand();
std::unique_ptr<ICommand> createPsCommand();

inline void initCommands(CommandRegistry& reg) {
    reg.add(createCatCommand());
    reg.add(createTouchCommand());
    reg.add(createEchoCommand());
    reg.add(createAddCommand());
    reg.add(createRmCommand());
    reg.add(createWriteCommand());
    reg.add(createEditCommand());
    reg.add(createTexedCommand());
    reg.add(createMkdirCommand());
    reg.add(createRmdirCommand());
    reg.add(createCdCommand());
    reg.add(createLsCommand());
    reg.add(createPwdCommand());
    reg.add(createSaveStateCommand());
    reg.add(createSetSchedulerCommand());
    reg.add(createSleepCommand());
    reg.add(createLoadStateCommand());
    reg.add(createLoadCommand());
    reg.add(createResetCommand());
    reg.add(createListdataCommand());
    reg.add(createMvCommand());
    reg.add(createCpCommand());
    reg.add(createMvdirCommand());
    reg.add(createCpdirCommand());
    reg.add(createKillCommand());
    reg.add(createPsCommand());
    
    reg.add(createHelpCommand(&reg));
    
    reg.add(createQuitCommand());
    reg.add(createMeminfoCommand());
    reg.add(createMembarCommand());
    
    // Add alias for exit -> quit
    auto exitCmd = createQuitCommand();
    reg.map["exit"] = std::move(exitCmd);
}

} // namespace shell
