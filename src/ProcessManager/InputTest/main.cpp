#include <iostream>
#include <iomanip>
#include "../ProcessManager.h"
#include "../OutputTest/MemoryManager.h"
#include "../OutputTest/CPUScheduler.h"

static void print_header() {
    std::cout << std::left
              << std::setw(6)  << "PID"
              << std::setw(18) << "NAME"
              << std::setw(12) << "CPU_NEEDED"
              << std::setw(12) << "MEM_NEEDED"
              << std::setw(8)  << "PRIO"
              << std::setw(8)  << "STATE"
              << "\n";
}
static void print_row(const Process& p) {
    std::cout << std::left
              << std::setw(6)  << p.pid
              << std::setw(18) << p.name
              << std::setw(12) << p.cpuTimeNeeded
              << std::setw(12) << p.memoryNeeded
              << std::setw(8)  << p.priority
              << std::setw(8)  << p.state
              << "\n";
}
static void print_table(const std::vector<Process>& rows) {
    if (rows.empty()) { std::cout << "[Kernel] Empty array received.\n"; return; }
    print_header();
    for (const auto& p : rows) print_row(p);
}

int main() {
    MemoryManager memory;
    CPUScheduler scheduler;
    ProcessManager pm(memory, scheduler);

    std::cout << "[Kernel] user gives ps command:\n";
    print_table(pm.snapshot());

    std::cout << "\n[Kernel] User gives command to execute process called calculate:\n";
    pm.execute_process("calculation", 120, 256, 1);
    //std::cout << "[Kernel] execute_process returned pid=" << pid1 << "\n";

    std::cout << "\n[Kernel] ps:\n";
    print_table(pm.snapshot());

    return 0;
}
