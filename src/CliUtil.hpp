#pragma once

#include <string>
#include <vector>
#include <map>

namespace util {
    inline std::map<std::string, std::vector<std::string>> parseCli(int argc, const char *argv[], const std::vector<std::string> &flags) {
        std::map<std::string, std::vector<std::string>> args;
        for (size_t idx = 1; idx < argc; idx++) {
            std::string arg = argv[idx];
            bool found = false;
            for (const auto &flag : flags) {
                if (arg.find(flag, 0) == 0) {
                    args[flag].push_back(arg.substr(flag.size()));
                    found = true;
                    break;
                }
            }
            if (!found) args[""].push_back(arg);
        }

        return args;
    }
}
