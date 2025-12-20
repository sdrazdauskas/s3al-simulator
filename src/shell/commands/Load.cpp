#include "shell/CommandAPI.h"
#include <memory>
#include <fstream>
#include <sstream>

namespace shell {

class LoadCommand : public ICommand {
public:
    int execute(const std::vector<std::string>& args,
                const std::string&,
                std::ostream& out,
                std::ostream& err,
                SysApi& sys) override
    {
        if (!requireArgs(args, 1, err)) return 1;
        
        auto fileName = args[0];
        
        // Read the file from the data folder on the host filesystem
        std::string dataPath = "/app/data/" + fileName;
        std::ifstream file(dataPath);
        
        if (!file.is_open()) {
            // Try alternative path for local development
            dataPath = "data/" + fileName;
            file.open(dataPath);
            
            if (!file.is_open()) {
                err << "Error: Cannot open file '" << fileName << "' from data folder\n";
                return 1;
            }
        }
        
        // Read the entire file content
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string fileContent = buffer.str();
        file.close();
        
        if (fileContent.empty()) {
            err << "Error: File '" << fileName << "' is empty\n";
            return 1;
        }
        
        // Create the file in the simulator's virtual filesystem
        auto createResult = sys.createFile(fileName);
        if (createResult != SysResult::OK && createResult != SysResult::AlreadyExists) {
            err << "Error: Cannot create file '" << fileName << "' in virtual filesystem: " 
                << toString(createResult) << "\n";
            return 1;
        }
        
        // Write the content to the virtual file
        auto writeResult = sys.writeFile(fileName, fileContent);
        if (writeResult != SysResult::OK) {
            err << "Error: Cannot write to file '" << fileName << "': " 
                << toString(writeResult) << "\n";
            return 1;
        }
        
        out << "Successfully loaded '" << fileName << "' from data folder into virtual filesystem\n";
        out << "File size: " << fileContent.size() << " bytes\n";
        
        return 0;
    }
    
    const char* getName() const override { return "load"; }
    const char* getDescription() const override { return "Load a file from data folder into virtual filesystem"; }
    const char* getUsage() const override { return "load <filename>"; }
};

std::unique_ptr<ICommand> createLoadCommand() {
    return std::make_unique<LoadCommand>();
}

} // namespace shell
