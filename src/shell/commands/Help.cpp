#include "CommandAPI.h"
#include <sstream>
#include <iomanip>

namespace shell {

int cmd_help(const std::vector<std::string>& /*args*/,
             const std::string& /*input*/,
             std::ostream& out,
             std::ostream& /*err*/,
             SysApi& /*sys*/)
{
    struct CmdInfo { std::string name, params, desc; };
    std::vector<CmdInfo> commands = {
        {"help","", "Display this help message"},
        {"echo","[text]","Repeat the text back"},
        {"add","[num1] [num2] ...","Sum the numbers"},
        {"quit","","Exit the kernel"},
        {"exit","","Alias for quit"},
        {"touch","[filename]","Create a new empty file"},
        {"rm","[filename]","Delete a file"},
        {"write","[filename] [text]","Write text to a file (overwrite)"},
        {"cat","[filename]","Display file contents"},
        {"edit","[filename] [text]","Append text to a file"},
        {"mkdir","[foldername]","Create a new folder"},
        {"rmdir","[foldername]","Remove an empty folder"},
        {"cd","[foldername|..]","Change current directory"},
        {"ls","","List contents of current directory"},
        {"pwd","","Show current directory path"},
        {"membar", "", "Display memory usage bar"},
        {"meminfo", "", "Display memory info summary"}
    };

    size_t max_name_len=4, max_param_len=9;
    for(auto& cmd: commands){
        if(cmd.name.size()>max_name_len) max_name_len=cmd.name.size();
        if(cmd.params.size()>max_param_len) max_param_len=cmd.params.size();
    }

    auto draw_line = [&](){ return "+"+std::string(max_name_len+2,'-')+"+"+
                                   std::string(max_param_len+2,'-')+"+"+std::string(50,'-')+"\n"; };

    std::ostringstream oss;
    oss << draw_line();
    oss << "| " << std::left << std::setw(max_name_len) << "Name" << " | "
        << std::left << std::setw(max_param_len) << "Parameters" << " | Description |\n";
    oss << draw_line();

    for(auto& cmd: commands){
        oss << "| " << std::left << std::setw(max_name_len) << cmd.name << " | "
            << std::left << std::setw(max_param_len) << cmd.params << " | "
            << cmd.desc << " |\n";
    }
    oss << draw_line();

    out << oss.str();
    return 0;
}

} // namespace shell
