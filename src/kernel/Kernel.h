#ifndef KERNEL_H
#define KERNEL_H

#include <string>
#include <vector>
#include <map>
#include <functional>

class Kernel {
public:
    Kernel();

    void run();

private:
    std::string _process_line(const std::string& line);

    void _register_commands();

    std::string _handle_help(const std::vector<std::string>& args);
    std::string _handle_echo(const std::vector<std::string>& args);
    std::string _handle_add(const std::vector<std::string>& args);
    std::string _handle_quit(const std::vector<std::string>& args);

    using CommandHandler = std::function<std::string(const std::vector<std::string>&)>;

    std::map<std::string, CommandHandler> _commands;

    bool _is_running;
};

#endif