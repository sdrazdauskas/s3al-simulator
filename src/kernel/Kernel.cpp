#include "Kernel.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "Terminal.h"
#include "Shell.h"
#include "Logger.h"

using namespace std;

Kernel::Kernel()
    : m_is_running(true),
      m_scheduler(),
      m_mem_mgr(1024 * 1024),
      m_proc_manager(m_mem_mgr, m_scheduler) {
    auto logger_callback = [](const std::string& level, const std::string& module, const std::string& message){
        using namespace logging;
        if (level == "DEBUG") Logger::getInstance().log(LogLevel::DEBUG, module, message);
        else if (level == "INFO") Logger::getInstance().log(LogLevel::INFO, module, message);
        else if (level == "WARNING") Logger::getInstance().log(LogLevel::WARNING, module, message);
        else if (level == "ERROR") Logger::getInstance().log(LogLevel::ERROR, module, message);
    };

    m_proc_manager.setLogCallback(logger_callback);
    m_scheduler.setLogCallback(logger_callback);
    m_storage.setLogCallback(logger_callback);
    m_mem_mgr.setLogCallback(logger_callback);

    register_commands();
    std::cout << "Kernel initialized." << std::endl;
}


std::string Kernel::execute_command(const std::string& line) {
    if (!line.empty() && line.back() == '\n') {
        return process_line(line.substr(0, line.size() - 1));
    }
    return process_line(line);
}

std::string Kernel::execute_command(const std::string& cmd, const std::vector<std::string>& args) {
    std::string line = cmd;
    for (const auto& arg : args) line += " " + arg;
    return execute_command(line);
}

bool Kernel::is_running() const { return m_is_running; }

void Kernel::register_commands() {
    m_commands["help"] = [this](const auto& args){ return handle_help(args); };
    m_commands["echo"] = [this](const auto& args){ return handle_echo(args); };
    m_commands["add"]  = [this](const auto& args){ return handle_add(args); };
    m_commands["quit"] = [this](const auto& args){ return handle_quit(args); };
    m_commands["exit"] = [this](const auto& args){ return handle_quit(args); };

    m_commands["touch"] = [this](const auto& args){ return handle_touch(args); };
    m_commands["rm"]    = [this](const auto& args){ return handle_rm(args); };
    m_commands["write"] = [this](const auto& args){ return handle_write(args); };
    m_commands["cat"]   = [this](const auto& args){ return handle_cat(args); };
    m_commands["edit"]  = [this](const auto& args){ return handle_edit(args); };
    m_commands["mkdir"] = [this](const auto& args){ return handle_mkdir(args); };
    m_commands["rmdir"]= [this](const auto& args){ return handle_rmdir(args); };
    m_commands["cd"]    = [this](const auto& args){ return handle_cd(args); };
    m_commands["ls"]    = [this](const auto& args){ return handle_ls(args); };
    m_commands["pwd"]   = [this](const auto& args){ return handle_pwd(args); };

    m_commands["meminfo"] = [this](const auto& args){ return this->handle_meminfo(args); };
    m_commands["membar"]  = [this](const auto& args){ return this->handle_membar(args); };
}

std::string Kernel::process_line(const std::string& line) {
    if(line.empty()) return "";

    istringstream iss(line);
    string command_name;
    iss >> command_name;

    vector<string> args;
    string token;
    while(iss >> token) {
        if(!token.empty() && token.front()=='"') {
            string quoted = token.substr(1);
            while(iss && (quoted.empty() || quoted.back()!='"')) {
                if(!(iss>>token)) break;
                quoted+=" "+token;
            }
            if(!quoted.empty() && quoted.back()=='"') quoted.pop_back();
            args.push_back(quoted);
        } else {
            args.push_back(token);
        }
    }

    auto it = m_commands.find(command_name);
    if(it != m_commands.end()) {
        string result = it->second(args);
        m_proc_manager.execute_process(command_name, 1, 1, 0);
        return result;
    }

    return "Unknown command: '" + command_name + "'.";
}

std::string Kernel::handle_help(const vector<string>& args) {
    (void)args;
    struct CmdInfo { string name, params, desc; };
    vector<CmdInfo> commands = {
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

    auto draw_line = [&](){ return "+"+string(max_name_len+2,'-')+"+"+
                                   string(max_param_len+2,'-')+"+"+string(50,'-')+"+\n"; };

    ostringstream oss;
    oss << draw_line();
    oss << "| "<<left<<setw(max_name_len)<<"Name"<<" | "
        <<left<<setw(max_param_len)<<"Parameters"<<" | Description |\n";
    oss << draw_line();

    for(auto& cmd: commands){
        oss << "| "<<left<<setw(max_name_len)<<cmd.name<<" | "
            <<left<<setw(max_param_len)<<cmd.params<<" | "
            <<cmd.desc<<" |\n";
    }
    oss << draw_line();
    return oss.str();
}

std::string Kernel::handle_echo(const vector<string>& args){
    if(args.empty()) return "Usage: echo [text]";
    string result;
    for(size_t i=0;i<args.size();++i) result+=args[i]+(i+1<args.size()?" ":"");
    return result;
}

std::string Kernel::handle_add(const vector<string>& args){
    if(args.empty()) return "Usage: add [number1] [number2] ...";
    double sum=0.0;
    for(auto& a: args){
        try{ sum+=stod(a); } catch(...){ return "Error: '"+a+"' is not a number"; }
    }
    return "Sum: "+to_string(sum);
}

std::string Kernel::handle_quit(const vector<string>& args){
    m_is_running=false;
    return "Shutting down kernel.";
}

std::string Kernel::handle_touch(const vector<string>& args){
    if(args.empty()) return "Usage: touch <filename>";
    return "[Kernel] "+storage::StorageManager::toString(m_storage.createFile(args[0]));
}

std::string Kernel::handle_rm(const vector<string>& args){
    if(args.empty()) return "Usage: rm <filename>";
    return "[Kernel] "+storage::StorageManager::toString(m_storage.deleteFile(args[0]));
}

std::string Kernel::handle_write(const vector<string>& args){
    if(args.size()<2) return "Usage: write <filename> <content>";
    string content;
    for(size_t i=1;i<args.size();++i) content+=args[i]+(i+1<args.size()?" ":"");
    auto exists=m_storage.fileExists(args[0]);
    if(exists!=storage::StorageManager::StorageResponse::OK) return "[Kernel] "+storage::StorageManager::toString(exists);
    return "[Kernel] "+storage::StorageManager::toString(m_storage.writeFile(args[0],content));
}

std::string Kernel::handle_cat(const vector<string>& args){
    if(args.empty()) return "Usage: cat <filename>";
    string out;
    auto res=m_storage.readFile(args[0],out);
    if(res!=storage::StorageManager::StorageResponse::OK) return "[Kernel] "+storage::StorageManager::toString(res);
    return out;
}

std::string Kernel::handle_edit(const vector<string>& args){
    if(args.size()<2) return "Usage: edit <filename> <content>";
    string content;
    for(size_t i=1;i<args.size();++i) content+=args[i]+(i+1<args.size()?" ":"");
    auto exists=m_storage.fileExists(args[0]);
    if(exists!=storage::StorageManager::StorageResponse::OK) return "[Kernel] "+storage::StorageManager::toString(exists);
    return "[Kernel] "+storage::StorageManager::toString(m_storage.appendToFile(args[0],content));
}

std::string Kernel::handle_mkdir(const vector<string>& args){
    if(args.empty()) return "Usage: mkdir <foldername>";
    return "[Kernel] "+storage::StorageManager::toString(m_storage.makeDir(args[0]));
}

std::string Kernel::handle_rmdir(const vector<string>& args){
    if(args.empty()) return "Usage: rmdir <foldername>";
    return "[Kernel] "+storage::StorageManager::toString(m_storage.removeDir(args[0]));
}

std::string Kernel::handle_cd(const vector<string>& args){
    if(args.empty()) return "Usage: cd <foldername|..>";
    return "[Kernel] "+storage::StorageManager::toString(m_storage.changeDir(args[0]));
}

std::string Kernel::handle_ls(const vector<string>& args){
    (void)args;
    auto entries=m_storage.listDir();
    ostringstream out;
    out<<"=== Contents of "<<m_storage.getWorkingDir()<<" ===\n";
    if(entries.empty()) out<<"(empty)\n";
    for(auto& e: entries) out<<e<<"\n";
    return out.str();
}

std::string Kernel::handle_pwd(const vector<string>& args){
    (void)args;
    return m_storage.getWorkingDir();
}

void Kernel::boot(){
    LOG_INFO("KERNEL", "Booting s3al OS...");
    shell::Shell sh([this](const string& cmd,const vector<string>& args){ return execute_command(cmd,args); });
    terminal::Terminal term;
    term.setSendCallback([&](const string& line){
        string result = sh.processCommandLine(line);
        if(!result.empty()){ term.print(result); if(result.back()!='\n') term.print("\n"); }
    });
    term.setSignalCallback([&](int){ term.print("^C\n"); });

    LOG_INFO("KERNEL", "Init process started");
    while(m_is_running){
        cout<<m_storage.getWorkingDir()<<"$ ";
        string line;
        getline(cin,line);
        if(line.empty()) continue;
        string output = execute_command(line);
        if(!output.empty()) cout<<output<<"\n";
    }
    LOG_INFO("KERNEL", "Shutdown complete");
}

std::string Kernel::handle_meminfo(const std::vector<std::string>& args) {
    (void)args;
    size_t total = m_mem_mgr.get_total_memory();
    size_t used  = m_mem_mgr.get_used_memory();
    size_t free  = total - used;

    std::ostringstream oss;
    oss << "=== Memory Info ===\n";
    oss << "Total: " << total / 1024 << " KB\n";
    oss << "Used : " << used / 1024 << " KB\n";
    oss << "Free : " << free / 1024 << " KB\n";
    return oss.str();
}

std::string Kernel::handle_membar(const std::vector<std::string>& args) {
    (void)args;
    size_t total = m_mem_mgr.get_total_memory();
    size_t used  = m_mem_mgr.get_used_memory();
    int bar_width = 40;
    int used_blocks = static_cast<int>((double)used / total * bar_width);

    std::ostringstream oss;
    oss << "[Memory] [";
    for (int i = 0; i < bar_width; ++i) {
        oss << (i < used_blocks ? '#' : '-');
    }
    oss << "] " << (used * 100 / total) << "% used\n";
    return oss.str();
}
