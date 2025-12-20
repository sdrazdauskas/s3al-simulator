#pragma once
#include "scheduler/algorithms/SchedulingAlgorithm.h"

namespace scheduler {

class RoundRobinAlgorithm : public SchedulingAlgorithm {
public:
    std::string getDebugInfo(int pid) const override;
    explicit RoundRobinAlgorithm(int quantum) : quantum(quantum), sliceCounter(0), lastPid(-1) {}

    int selectNext(const std::vector<ScheduledTask*>& readyQueue) override;

    // Now manages its own slice counter
    bool shouldPreempt(const ScheduledTask* current, const std::vector<ScheduledTask*>& readyQueue) override;

    std::string getName() const override { return "Round Robin"; }

    void setQuantum(int q) { quantum = q; }
    int getQuantum() const { return quantum; }

    // Called by scheduler when a process is scheduled or context switch occurs
    void onSchedule(int pid) override;
    void onTick(int pid) override;

private:
    int quantum;
    int sliceCounter;
    int lastPid;
};

} // namespace scheduler
