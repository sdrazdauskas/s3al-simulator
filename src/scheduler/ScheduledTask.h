#pragma once

namespace scheduler {
struct ScheduledTask {
    int id; //PiD
    int arrivalTime;  //When did process arrive to queue? 
                      // for now unit of "time" is one cycle of scheduler simulating CPU execution
    int burstTime;  //how much CPU time does a process need to "complete execution"?
                    //also using it as "remaining time" for now
    int priority;   //simple priority, for prioQ algorith
    int completionTime = 0; //when did process finish execuiting? Useful just for turnaround time calculation
    int turnaroundTime = 0; //metric
    //int remainingTime = 0; // for Round Robin. Just use the burstTime?

    ScheduledTask(int pid, int arrival, int burst, int prio = 0)
        : id(pid), arrivalTime(arrival), burstTime(burst),
          priority(prio) {}
};

} //namespace scheduler
