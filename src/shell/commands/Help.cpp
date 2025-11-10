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
        {"echo","<text>","Repeat the text back"},
        {"add","<num1> [num2] [num...]","Sum the numbers"},
        {"quit","","Exit the kernel"},
        {"exit","","Alias for quit"},
        {"touch","<filename> [filename...]","Create a new empty file or update the modification timestamp"},
        {"rm","<filename> [filename...]","Delete a file"},
        {"write","<filename> <content>","Write content to a file (overwrite)"},
        {"cat","<filename> [filename...]","Display file contents"},
        {"edit","<filename>","Open editor to append text to a file"},
        {"mkdir","<foldername>","Create a new folder"},
        {"rmdir","<foldername>","Delete a folder"},
        {"cd","<foldername> | ..","Change current directory"},
        {"ls","","List contents of current directory"},
        {"pwd","","Show current directory path"},
        {"save", "<name>", "Save current storage state to 'data/<name>.json'"},
        {"load", "<name>", "Load storage state from 'data/<name>.json'"},
        {"reset", "", "Clear current storage and start fresh"},
        {"membar", "", "Display memory usage bar"},
        {"meminfo", "", "Display memory info summary"},
        {"cp", "<source> <destination>", "Copy a file from source to destination"},
        {"cpdir", "<srcDir> <destDir>", "Recursively copy a directory"},
        {"mv", "<oldName> <newName>", "Move or rename a file"},
        {"mvdir", "<oldDir> <newDir>", "Move or rename a directory"}
    };

    size_t max_name_len=4, max_param_len=9, max_desc_len = 11;
    for(auto& cmd: commands){
        if(cmd.name.size()>max_name_len) max_name_len=cmd.name.size();
        if(cmd.params.size()>max_param_len) max_param_len=cmd.params.size();
        if (cmd.desc.size()>max_desc_len) max_desc_len = cmd.desc.size();
    }

    auto draw_line = [&](){ return "+"+std::string(max_name_len+2,'-')+"+"+
                                    std::string(max_param_len+2,'-')+"+"+
                                    std::string(max_desc_len+2,'-') + "+\n";
    };

    std::ostringstream oss;
    oss << draw_line();
    oss << "| " << std::left << std::setw(max_name_len) << "Name" << " | "
        << std::left << std::setw(max_param_len) << "Parameters" << " | "
        << std::left << std::setw(max_desc_len) << "Description" << " |\n";
    oss << draw_line();

    for(auto& cmd: commands){
        oss << "| " << std::left << std::setw(max_name_len) << cmd.name << " | "
            << std::left << std::setw(max_param_len) << cmd.params << " | "
            << std::left << std::setw(max_desc_len) << cmd.desc << " |\n";
    }
    oss << draw_line();

    out << oss.str();
    return 0;
}

} // namespace shell
