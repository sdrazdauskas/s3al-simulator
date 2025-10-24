#include "cpu_scheduler.h"

#include <stdexcept>
#include "sched_algos/fcfs.h"
//#include "sched_algos/roundrobin.h"
#include "sched_algos/priorityqueue.h"

CPUScheduler::CPUScheduler() 
    : sysTime(0), algo_(Algorithm::PriorityQueue), quantum_(1){}

void CPUScheduler::setAlgorithm(Algorithm a) { algo_ = a; }

void CPUScheduler::setQuantum(int q) { quantum_ = (q > 0 ? q : 1); }

void CPUScheduler::execute_process(int pid, int burstTime, int priorty) {
    cout << "[CPU scheduler] request to execute process PiD " <<pid<<" for "<<burstTime<<" ticks received."<< endl;
    processes_.emplace_back(pid, burstTime, priorty);
    run();
}

void CPUScheduler::setProcesses(const std::vector<Process>& plist) {
    processes_ = plist;
}

void CPUScheduler::clear() {
    processes_.clear();
}

void CPUScheduler::run() {
    if (processes_.empty()) return;

    switch (algo_) {
        
        //rrobin implementation will change vastly with implementation of multiprocessing, not available for usage for now.
        // case Algorithm::RoundRobin: {
        //     RoundRobin rr(processes_, quantum_);
        //     rr.run();
        //     break;
        // }

        case Algorithm::FCFS: {
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
            
            break;
        }

        case Algorithm::PriorityQueue: {
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
            break;
        }
        default:
            throw std::runtime_error("Unknown algorithm");
    }
}
