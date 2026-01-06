#include "shell/CommandAPI.h"
#include <memory>
#include <filesystem>

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
        
        // Read the file content from host filesystem
        std::string fileContent;
        auto readResult = sys.readFileFromHost(fileName, fileContent);
        if (readResult != SysResult::OK) {
            err << "Error: Cannot load '" << fileName << "' from data folder: "
                << toString(readResult) << "\n";
            return 1;
        }
        
        // Extract just the filename (without path) for the virtual filesystem
        std::filesystem::path filePath(fileName);
        std::string virtualFileName = filePath.filename().string();
        
        // Create the file in the virtual filesystem if it doesn't exist
        auto createResult = sys.createFile(virtualFileName);
        if (createResult != SysResult::OK && createResult != SysResult::AlreadyExists) {
            err << "Error: Cannot create file '" << virtualFileName << "' in virtual filesystem: "
                << toString(createResult) << "\n";
            return 1;
        }
        
        // Write the content to the virtual filesystem
        auto writeResult = sys.writeFile(virtualFileName, fileContent);
        if (writeResult != SysResult::OK) {
            err << "Error: Cannot write to virtual file '" << virtualFileName << "': "
                << toString(writeResult) << "\n";
            return 1;
        }
        
        out << "Successfully loaded '" << fileName << "' into virtual filesystem as '" << virtualFileName << "'\n";
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
