#pragma once
#include "scheduler/algorithms/SchedulingAlgorithm.h"

namespace scheduler {

class RoundRobinAlgorithm : public SchedulingAlgorithm {
public:
    explicit RoundRobinAlgorithm(int quantum) : quantum(quantum) {}
    
    int selectNext(const std::vector<ScheduledTask*>& readyQueue) override;
    
    bool shouldPreempt(const ScheduledTask* current, const std::vector<ScheduledTask*>& readyQueue, int currentSlice) override;
    
    void onCycleExecuted(int pid, int& currentSlice) override;
    
    void onContextSwitch(int& currentSlice) override;
    
    std::string getDebugInfo(int currentSlice, int quantum) const override;
    
    std::string getName() const override { return "Round Robin"; }
    
    void setQuantum(int q) { quantum = q; }
    int getQuantum() const { return quantum; }
    
private:
    int quantum;
};

} // namespace scheduler
