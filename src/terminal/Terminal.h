#pragma once

#include <memory>
#include <string>
#include <utility>

#include <functional>
#include <string>

namespace terminal {

class Terminal {
public:
    using sendCallback = std::function<void(const std::string&)>;
    using signalCallback = std::function<void(int)>;

    Terminal() = default;
    ~Terminal() = default;

    // Set the send callback which will receive lines read from stdin.
    void setSendCallback(sendCallback cb);

    // Set the signal callback which will be called when an interrupt
    void setSignalCallback(signalCallback cb);

    void runBlockingStdioLoop();

    // Called by shell to display output (writes directly to stdout)
    void print(const std::string& output);

private:
    sendCallback sendCb;
    signalCallback sigCb;
};

} // namespace terminal
