#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <ostream>
#include "SysCallsAPI.h"

namespace shell {

using CommandFn = int(*)(const std::vector<std::string>& args,
                         const std::string& input,
                         std::ostream& out,
                         std::ostream& err,
                         SysApi& sys);

struct CommandRegistry {
    std::unordered_map<std::string, CommandFn> map;
    void add(const std::string& name, CommandFn fn) { map[name] = fn; }
    CommandFn find(const std::string& name) const {
        auto it = map.find(name);
        return (it == map.end()) ? nullptr : it->second;
    }
};

} // namespace shell
