#pragma once
#include "CommandAPI.h"
#include <memory>

namespace shell {

// Forward declarations of factory functions
std::unique_ptr<ICommand> create_cat_command();
std::unique_ptr<ICommand> create_touch_command();
std::unique_ptr<ICommand> create_echo_command();
std::unique_ptr<ICommand> create_add_command();
std::unique_ptr<ICommand> create_rm_command();
std::unique_ptr<ICommand> create_write_command();
std::unique_ptr<ICommand> create_edit_command();
std::unique_ptr<ICommand> create_mkdir_command();
std::unique_ptr<ICommand> create_rmdir_command();
std::unique_ptr<ICommand> create_cd_command();
std::unique_ptr<ICommand> create_ls_command();
std::unique_ptr<ICommand> create_pwd_command();
std::unique_ptr<ICommand> create_help_command();
std::unique_ptr<ICommand> create_quit_command();
std::unique_ptr<ICommand> create_meminfo_command();
std::unique_ptr<ICommand> create_membar_command();
std::unique_ptr<ICommand> create_save_command();
std::unique_ptr<ICommand> create_load_command();
std::unique_ptr<ICommand> create_reset_command();

// Special function to create help command with registry
std::unique_ptr<ICommand> create_help_command_with_registry(CommandRegistry* reg);

inline void init_commands(CommandRegistry& reg) {
    reg.add(create_cat_command());
    reg.add(create_touch_command());
    reg.add(create_echo_command());
    reg.add(create_add_command());
    reg.add(create_rm_command());
    reg.add(create_write_command());
    reg.add(create_edit_command());
    reg.add(create_mkdir_command());
    reg.add(create_rmdir_command());
    reg.add(create_cd_command());
    reg.add(create_ls_command());
    reg.add(create_pwd_command());
    
    // Help needs registry access
    reg.add(create_help_command_with_registry(&reg));
    
    reg.add(create_quit_command());
    reg.add(create_meminfo_command());
    reg.add(create_membar_command());
    reg.add(create_save_command());
    reg.add(create_load_command());
    reg.add(create_reset_command());
    
    // Add alias for exit -> quit
    auto exit_cmd = create_quit_command();
    reg.map["exit"] = std::move(exit_cmd);
}

} // namespace shell
