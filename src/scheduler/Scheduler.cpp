#include "Scheduler.h"
#include <stdexcept>
#include "sched_algos/Fcfs.h"
#include "sched_algos/PriorityQueue.h"

namespace scheduler {

CPUScheduler::CPUScheduler() 
    : sysTime(0), algo(Algorithm::PriorityQueue), quantum(1) {}

void CPUScheduler::setLogCallback(LogCallback callback) {
    log_callback = callback;
}

void CPUScheduler::log(const std::string& level, const std::string& message) {
    if (log_callback) {
        log_callback(level, "SCHEDULER", message);
    }
}

void CPUScheduler::setAlgorithm(Algorithm a) { 
    algo = a;
    std::string algoName = (a == Algorithm::FCFS) ? "FCFS" : "PriorityQueue";
    log("INFO", "Algorithm set to: " + algoName);
}

void CPUScheduler::setQuantum(int q) { 
    quantum = (q > 0 ? q : 1);
    log("INFO", "Quantum set to: " + std::to_string(quantum));
}

void CPUScheduler::execute_process(int pid, int burstTime, int priorty) {
    processes_.emplace_back(pid, burstTime, priorty);
    log("INFO", "Process " + std::to_string(pid) + " added to scheduler (burst=" + std::to_string(burstTime) + ", priority=" + std::to_string(priorty) + ")");
    run();
}

void CPUScheduler::setProcesses(const std::vector<Process>& plist) {
    processes_ = plist;
    log("INFO", "Loaded " + std::to_string(plist.size()) + " processes into scheduler");
}

void CPUScheduler::clear() {
    int count = processes_.size();
    processes_.clear();
    log("DEBUG", "Cleared " + std::to_string(count) + " processes from scheduler");
}

void CPUScheduler::run() {
    if (processes_.empty()) return;

    log("INFO", "Starting scheduler with " + std::to_string(processes_.size()) + " processes");

    switch (algo) {

        case Algorithm::FCFS: {
            log("DEBUG", "Running FCFS algorithm");
            FCFS fcfs(processes_);
            while (!processes_.empty())
            {
                fcfs.run();
                sysTime++;
                processes_.erase(
                    remove_if(processes_.begin(), processes_.end(),
                              [](const Process& p){ return p.burstTime <= 0; }),
                    processes_.end());
            }
            log("INFO", "FCFS scheduling completed");
            break;
        }

        case Algorithm::PriorityQueue: {
            log("DEBUG", "Running PriorityQueue algorithm");
            PriorityQueueSched pq(processes_);
            while (!processes_.empty())
            {
                pq.run();
                sysTime++;
                processes_.erase(
                    remove_if(processes_.begin(), processes_.end(),
                              [](const Process& p){ return p.burstTime <= 0; }),
                    processes_.end());
                
            }
            log("INFO", "PriorityQueue scheduling completed");
            break;
        }
        default:
            log("ERROR", "Unknown scheduling algorithm");
            throw std::runtime_error("Unknown algorithm");
    }
}
} //namespace scheduler