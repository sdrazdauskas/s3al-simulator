#pragma once
#include "scheduler/algorithms/SchedulingAlgorithm.h"
#include "common/LoggingMixin.h"
#include <deque>

namespace scheduler {


class RoundRobinAlgorithm : public SchedulingAlgorithm, protected common::LoggingMixin {
    std::string getModuleName() const override { return "ROUND-ROBIN"; }

public:
    explicit RoundRobinAlgorithm(int quantum) : quantum(quantum), sliceCounter(0), lastPid(-1) {}

    ScheduledTask* getNextTask(ScheduledTask* currentTask, const std::deque<ScheduledTask*>& readyQueue) override;

    std::string getName() const override { return "Round Robin"; }

private:

    void resetSliceIfProcessChanged(int currentPid);
    void incrementSlice();
    bool quantumExpired() const;
    ScheduledTask* selectNextProcess(ScheduledTask* currentTask, const std::deque<ScheduledTask*>& readyQueue);
    void resetSlice();

private:
    int quantum;
    int sliceCounter;
    int lastPid;
};

} // namespace scheduler
