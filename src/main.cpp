#include "Kernel.h"

int main() {
    // Kernel entry point - initializes subsystems and starts init process
    Kernel kernel;
    kernel.boot();
    
    return 0;
}
