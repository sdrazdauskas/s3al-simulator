#include "Terminal.h"
#include "Kernel.h"
#include <iostream>

int main() {
    // Create the kernel (provides command handlers)
    Kernel kernel;

    // Create the terminal (handles I/O)
    terminal::Terminal term;

    // Wire terminal input to kernel
    term.setSendCallback([&kernel, &term](const std::string& line) {
        // Execute the command via kernel
        std::string result = kernel.execute_command(line);
        
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
