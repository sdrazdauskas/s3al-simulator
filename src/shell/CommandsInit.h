#pragma once
#include "CommandAPI.h"

namespace shell {

int cmd_cat(const std::vector<std::string>&,
            const std::string&, std::ostream&, std::ostream&, SysApi&);
int cmd_touch(const std::vector<std::string>&,
              const std::string&, std::ostream&, std::ostream&, SysApi&);
int cmd_echo(const std::vector<std::string>&,
             const std::string&, std::ostream&, std::ostream&, SysApi&);
int cmd_add(const std::vector<std::string>&,
            const std::string&, std::ostream&, std::ostream&, SysApi&);
int cmd_rm(const std::vector<std::string>&,
           const std::string&, std::ostream&, std::ostream&, SysApi&);
int cmd_write(const std::vector<std::string>&,
              const std::string&, std::ostream&, std::ostream&, SysApi&);
int cmd_edit(const std::vector<std::string>&,
             const std::string&, std::ostream&, std::ostream&, SysApi&);
int cmd_mkdir(const std::vector<std::string>&,
              const std::string&, std::ostream&, std::ostream&, SysApi&);
int cmd_rmdir(const std::vector<std::string>&,
              const std::string&, std::ostream&, std::ostream&, SysApi&);
int cmd_cd(const std::vector<std::string>&,
           const std::string&, std::ostream&, std::ostream&, SysApi&);
int cmd_ls(const std::vector<std::string>&,
           const std::string&, std::ostream&, std::ostream&, SysApi&);
int cmd_pwd(const std::vector<std::string>&,
            const std::string&, std::ostream&, std::ostream&, SysApi&);
int cmd_help(const std::vector<std::string>&,
             const std::string&, std::ostream&, std::ostream&, SysApi&);
int cmd_quit(const std::vector<std::string>&,
             const std::string&, std::ostream&, std::ostream&, SysApi&);
int cmd_meminfo(const std::vector<std::string>&,
                const std::string&, std::ostream&, std::ostream&, SysApi&);
int cmd_membar(const std::vector<std::string>&,
               const std::string&, std::ostream&, std::ostream&, SysApi&);

inline void init_commands(CommandRegistry& reg) {
    reg.add("cat", &cmd_cat);
    reg.add("touch", &cmd_touch);
    reg.add("echo", &cmd_echo);
    reg.add("add", &cmd_add);
    reg.add("rm", &cmd_rm);
    reg.add("write", &cmd_write);
    reg.add("edit", &cmd_edit);
    reg.add("mkdir", &cmd_mkdir);
    reg.add("rmdir", &cmd_rmdir);
    reg.add("cd", &cmd_cd);
    reg.add("ls", &cmd_ls);
    reg.add("pwd", &cmd_pwd);
    reg.add("help", &cmd_help);
    reg.add("quit", &cmd_quit);
    reg.add("exit", &cmd_quit);
    reg.add("meminfo", &cmd_meminfo);
    reg.add("membar", &cmd_membar);
}

} // namespace shell
