#include "CommandAPI.h"
#include <iostream>

namespace shell {

int cmd_add(const std::vector<std::string>& args,
            const std::string& /*input*/,
            std::ostream& out,
            std::ostream& err,
            SysApi& /*sys*/)
{
    if (args.empty()) {
        err << "Usage: add [number1] [number2] ...\n";
        return 1;
    }
    double sum = 0.0;
    for (const auto& a : args) {
        try { sum += std::stod(a); } catch(...) { err << "Error: '" << a << "' is not a number\n"; return 1; }
    }
    out << sum << "\n";
    return 0;
}

} // namespace shell
