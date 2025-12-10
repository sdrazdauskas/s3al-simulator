#pragma once
#include <vector>
#include <string>

namespace terminal {

    class History {
    public:
        void add(const std::string& cmd);
        bool prev(std::string& out);
        bool next(std::string& out);

    private:
        std::vector<std::string> entries;
        size_t index = 0;
    };

} // namespace terminal
