#include "Terminal.h"
#include "Shell.h"
#include "Kernel.h"
#include <iostream>

int main() {
    // Create the kernel (provides command handlers)
    Kernel kernel;

    // Create the shell with a callback to the kernel
    shell::Shell sh([&kernel](const std::string& cmd, const std::vector<std::string>& args) {
        return kernel.execute_command(cmd, args);
    });

    // Create the terminal (handles I/O)
    terminal::Terminal term;

    // Wire terminal input to shell, and shell output back to terminal
    term.setSendCallback([&sh, &term](const std::string& line) {
        // Process the command line through the shell
        std::string result = sh.processCommandLine(line);
        
        // Display the result
        if (!result.empty()) {
            term.print(result);
            if (result.back() != '\n') {
                term.print("\n");
            }
        }
    });

    // Handle Ctrl+C gracefully
    term.setSignalCallback([&term](int sig) {
        term.print("^C\n");
    });

    // Print welcome message
    std::cout << "s3al OS Simulator\n";
    std::cout << "Type 'help' for commands, 'quit' to exit\n\n";

    // Start the terminal I/O loop
    term.runBlockingStdioLoop();

    std::cout << "Goodbye!\n";
    return 0;
}
