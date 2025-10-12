#include <string>

struct Process {
    std::string name;
    int pid;
    int cpuTimeNeeded; //TODO: decide on unit of measurement. Tick? Second? Unit of cpu time?
    int memoryNeeded; // TODO: decide on unit of measurement. Byte? Unit of memory?
    int priority; // TODO: decide on data type. Avoid magic number? TODO: do we need priority?
    int state; //TODO: data type: most likely enum class processState? 
    //For now- new=0, ready=1, running=2, sleeping/waiting/zombie/blocked...=3, terminated/finished=4

    Process(
        const std::string& n = "",
        int p = 0,
        int cpu = 0,
        int mem = 0,
        int prio = 0,
        int st = 0
    )
        : name(n),
          pid(p),
          cpuTimeNeeded(cpu),
          memoryNeeded(mem),
          priority(prio),
          state(st)
    {}
};
